#include <stdlib.h> 
#include <stdio.h> 
#include <string.h> 

#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"

#include "system/wow_lock.h"
#include "utils/wow_mem_leak.h" 


#define MALLOC_GARBAGE  0xA3

typedef struct BlockInfo{
    
    void* ptr;                      /* 存储块的开始位置 */
    size_t size;                    /* 存储块的长度 */
    int  fReferenced;               /* 曾经引用过吗？ */
	
	const char* fileName;			/*引用文件名称*/
	const char* funcName;			/*引用函数名称*/
	int line;						/*引用行标识*/
	
	struct BlockInfo* next;
}__data_aligned__ BlockInfo_T;


static mutex_lock_t gt_mutexLock;
static BlockInfo_T* gpt_blockHeadInfo = NULL;


#define fPtrLess(pLeft,pRight)               ((pLeft) < (pRight))
#define fPtrGrtr(pLeft,pRight)               ((pLeft) < (pRight))
#define fPtrEqual(pLeft, pRight)             ((pLeft) == (pRight))
#define fPtrLessEq(pLeft, pRight)            ((pLeft) <= (pRight))
#define fPtrGrtrEq(pLeft, pRight)            ((pLeft) >= (pRight))

static int create_block_info(void* ptr, size_t size, int line, 
									const char* filename, const char* funcName)
{
	BlockInfo_T* block = (BlockInfo_T*)malloc(sizeof(BlockInfo_T));
	CHECK_RET_VAL(block,-1);

	block->ptr  = ptr;
	block->size = size;
	block->line = line;
	block->fileName = filename;
	block->funcName = funcName;

	wow_mutex_lock(&gt_mutexLock);	
	block->next = gpt_blockHeadInfo;		
	gpt_blockHeadInfo = block;
	wow_mutex_unlock(&gt_mutexLock);	

	return 0;
}

static BlockInfo_T* get_block_info(void* ptr)
{
    BlockInfo_T* block = NULL;

	wow_mutex_lock(&gt_mutexLock);
    for(block = gpt_blockHeadInfo; block != NULL; block = block->next )
    {
    	//printf("get_block_info block:%p ptr:%p ptr:%p\n",block,block->ptr,ptr);
        void* pbStart = block->ptr;             
        void* pbEnd   = block->ptr+ block->size - 1;
		
        if( fPtrGrtrEq(ptr, pbStart) && fPtrLessEq(ptr, pbEnd) )
            break;
    }
	wow_mutex_unlock(&gt_mutexLock);	
    /* 没能找到指针？它是（ a）垃圾？(b) 指向一个已经释放了的存储块？
     * 或（ c）指向一个在由resize重置大小时而移动了的存储块？
     */
    return block;
}


static ssize_t get_block_size(void* ptr)
{
	BlockInfo_T* block = get_block_info(ptr);
	
	/* 必须指向一个存储块的开始位置 */
	CHECK_RET_VAL(block && ptr == block->ptr,-1);

	return (block->size);
}
									 
static void free_block_info(void* ptr)
{
    BlockInfo_T *block = NULL;
	BlockInfo_T *prev  = NULL;
	wow_mutex_lock(&gt_mutexLock);	

    for( block = gpt_blockHeadInfo; block != NULL; block = block->next )
    {
        if(fPtrEqual(block->ptr, ptr))
        {
            if(prev == NULL){
				 gpt_blockHeadInfo = block->next;
			}else{
				prev->next = block->next;
			}
            break;
        }
        prev = block;
    }
	
 	wow_mutex_unlock(&gt_mutexLock);	
    CHECK_RET_VOID(block);

    memset(block, MALLOC_GARBAGE, sizeof(BlockInfo_T));
    free(block);
}
 

/*brief   malloc内部调试接口
 *param : snSize:分配缓存大小
 *param : snLine:行信息
 *param : pcFile:文件信息
 *param : pcFunc:文件信息
 *return: 成功返回申请缓存 失败返回NULL
 */
__EX_API__ void* wow_mleak_malloc(size_t snSize, size_t snLine, const char* pcFile, const char* pcFunc)
{
	void* ptr = malloc(snSize);
	CHECK_RET_VAL(ptr,NULL);
	memset(ptr,MALLOC_GARBAGE,snSize);
	
	/* 如果无法创建日志块信息，那么模拟一个总的内存分配错误*/
	if(create_block_info(ptr, snSize, snLine, pcFile, pcFunc) != 0){
		free(ptr);
		ptr =  NULL;
	}
	
	return ptr;
}


/*brief   calloc内部调试接口
 *param : snCount:分项个数
 *param : snSize:分项大小
 *param : snLine:行信息
 *param : pcFile:文件信息
 *param : pcFunc:文件信息
 *return: 成功返回申请缓存 失败返回NULL
 */
__EX_API__ void* wow_mleak_calloc(size_t snCount, size_t snSize, size_t snLine, const char* pcFile, const char* pcFunc)
{
	void* ptr = calloc(snCount, snSize);
	CHECK_RET_VAL(ptr,NULL);
	
	/* 如果无法创建日志块信息，那么模拟一个总的内存分配错误*/
	if(create_block_info(ptr, snCount * snSize, snLine, pcFile, pcFunc) != 0){
		free(ptr);
		ptr =  NULL;
	}

	return ptr;
}

/*brief   ralloc内部调试接口
 *param : pBuff:缓存
 *param : snSize:缓存大小
 *param : snLine:行信息
 *param : pcFile:文件信息
 *param : pcFunc:文件信息
 *return: 成功返回申请缓存 失败返回NULL
 */
__EX_API__ void* wow_mleak_realloc(void* pBuff, size_t snSize, size_t snLine, const char* pcFile, const char* pcFunc)
{
	CHECK_RET_VAL(pBuff && snSize > 0,NULL);

	void* ptrNew = NULL;
	ssize_t sizeOld = get_block_size(pBuff);
	CHECK_RET_VAL(sizeOld > 0,pBuff);
	
	/* 如果缩小，先把将被释放的内存空间填写上废料
	 * 如果扩大，通过模拟 realloc的操作来迫使新的内存块产生移动
	 *（不让它在原有的位置扩展）如果新块和老块的长度相同，不
	 * 做任何事情
	 */
	if(snSize < sizeOld){
		memset(pBuff+snSize, MALLOC_GARBAGE, sizeOld-snSize);
	}
	else if(snSize > sizeOld){
		ptrNew = wow_mleak_malloc(snSize,snLine,pcFile,pcFunc);
		if(ptrNew){
			memcpy(ptrNew, pBuff, sizeOld);
			wow_mleak_free(pBuff);
		}
	}

	return ptrNew;
}

/*brief   free内部调试接口
 *param : pBuff:缓存
 *return: 无
 */
__EX_API__ void wow_mleak_free(void* ptr)
{
	CHECK_RET_VOID(ptr);

	ssize_t size = get_block_size(ptr);
	CHECK_RET_VOID(size > 0);
	
	free_block_info(ptr);
	
	memset(ptr, MALLOC_GARBAGE,size );

    free(ptr);
}

/*brief   打印内存泄漏信息
 *return: 无
 */
__EX_API__ void wow_mleak_print(void)
{
	size_t sum = 0;
	int nbBlocks = 0;
	BlockInfo_T* tmp = NULL;

	wow_mutex_lock(&gt_mutexLock); 

	BlockInfo_T* block = gpt_blockHeadInfo ;
	if (block != NULL){
		printf("Memory Leaks detected.\n");
	}
	
	while(block != NULL){
		printf("wow_print_leaks block:%p\n",block);
		tmp = block->next;
		printf("\n%zu bytes lost\n", block->size);
		printf("address : %p in %s\t%s:%d\n", block->ptr, block->funcName, block->fileName, block->line);
		printf("\n====================================\n");
		sum += block->size;
		free(block);
		block = tmp;
		nbBlocks++;
	}

	printf("SUMMARY :\n%zu bytes lost in %d blocks\n", sum, nbBlocks);

	wow_mutex_unlock(&gt_mutexLock);
}


/*内存泄漏接口初始化*/
__IN_API__ int _wow_mleak_init(void)
{
	wow_mutex_lock_init(&gt_mutexLock);

	return WOW_SUCCESS;
}

/*内存泄漏接口退出*/
__IN_API__ int _wow_mleak_exit(void)
{
	wow_mleak_print();
	wow_mutex_lock_exit(&gt_mutexLock);

	return WOW_SUCCESS;
}
