#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"

#include "list/wow_slist.h"

typedef struct slist_entry{
    void *data;		  		//内容 可为数组、结构体任何形式
    struct slist_entry *next;//指向上一个节点地址
}__data_aligned__ SlistEntry_T;  
 
struct  slist_t{
	SlistEntry_T *head;
	SlistEntry_T *tail;
    uint32_t entries;
}__data_aligned__;

/* brief    创建单项链表
 * return： 成功返回创建指针  失败返回NUUL
 */
__EX_API__ Slist_T* wow_slist_create(void)
{
	Slist_T *slist = CALLOC(1,Slist_T);
	CHECK_RET_VAL_ERRNO_P(slist,-SYSTEM_MALLOC_FAILED,"malloc Slist_T failed!\n");
	
	slist->head = NULL;
	slist->tail = NULL;
	slist->entries = 0;

	return (Slist_T*)slist;
}

/* brief   销毁链表
 * param ： pptSlist ：待键入data的链表
 * parma ： fDestroy ：销毁回调函数 可为NULL
 * return： 成功返回创建指针  失败返回NUUL
 */
__EX_API__ void wow_slist_destroy(Slist_T** pptSlist, slist_destroy_func_t destroy)
{
	CHECK_RET_VOID(pptSlist && *pptSlist);

	Slist_T* slist = (Slist_T*)*pptSlist;
	wow_slist_clear(slist, destroy);

	FREE(slist);
	*pptSlist = NULL;
}

/* brief    将data的内容push到链表头部
 * param ： ptSlist ：待键入data的链表
 * parma ： pData   ：实例化数据
 * return： 成功返回0  失败返回<0
 */
__EX_API__ int wow_slist_insert_head(Slist_T* ptSlist, void *pData)
{
	return wow_slist_insert_by_index(ptSlist,0,pData);
}

/* brief    将data的内容push到单项链表底部
 * param ： ptSlist ：待键入data的单项链表
 * parma ： pData ：实例化数据
 * return： 成功返回0  失败返回<0
 */
__EX_API__ int wow_slist_insert_tail(Slist_T* ptSlist, void *pData)
{
	return wow_slist_insert_by_index(ptSlist,wow_slist_size(ptSlist),pData);
}

/* brief    将data的内容push到index位置处
 * param ： ptSlist ：待键入data的队列
 * parma ： nIdx   ：index标识
 * parma ： pData ：实例化数据
 * return： 成功返回0  失败返回<0
 */
__EX_API__ int wow_slist_insert_by_index(Slist_T* ptSlist, int nIdx, void *pData)
{
	SlistEntry_T *new_entry = NULL;
	CHECK_RET_VAL_P(ptSlist,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(pData,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

	new_entry = CALLOC(1, SlistEntry_T);
	CHECK_RET_VAL_P(new_entry,-SYSTEM_MALLOC_FAILED,"malloc SlistEntry_T faild!\n");

	new_entry->data = pData;
	
	if(nIdx <= 0){
		new_entry->next = ptSlist->head;
		ptSlist->head = new_entry;
		if (ptSlist->tail == NULL){
			ptSlist->tail = new_entry;
		}		
	}else if(nIdx > ptSlist->entries){	
		if (ptSlist->tail){
			ptSlist->tail->next = new_entry;
		}
		
		ptSlist->tail = new_entry;
		if (ptSlist->head == NULL){
			ptSlist->head = new_entry;
		}
	}else{
		//查找索引位置
		SlistEntry_T *entry = NULL;
		for (entry = ptSlist->head; entry && nIdx > 1; entry = entry->next,nIdx--);
		if(entry == NULL){
			FREE(new_entry);
			return -WOW_FAILED;
		}
		new_entry->next = entry->next;

		if (!entry->next)
			ptSlist->tail = new_entry;

		entry->next = new_entry;
	}

	ptSlist->entries++;
	return WOW_SUCCESS;
}

/* brief   将data的内容push到某个实例的后面(可增加优先级机制)--未查找即追加
 * param ： ptSlist：待键入data的队列
 * parma ： fMatch ：匹配回调函数 不可为NULL  1-匹配成功 0-匹配失败
 * parma ： pEntry ：位置实例化数据
 * parma ： pData  ：实例化数据
 * return： 成功返回0  失败返回<0
 */
__EX_API__ int wow_slist_insert_by_match(Slist_T* ptSlist, slist_match_func_t fMatch,void *pEntry, void *pData)
{
	SlistEntry_T *entry = NULL;
	SlistEntry_T *new_entry = NULL;
	
	CHECK_RET_VAL_P(ptSlist,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(fMatch,-PARAM_INPUT_FUNC_IS_NULL,"param input func invalid!\n");
	CHECK_RET_VAL(ptSlist->head,-SLIST_NO_ENTRY);

	for (entry = ptSlist->head; entry; entry = entry->next){
		if(fMatch(entry->data,pEntry)){
			new_entry = CALLOC(1, SlistEntry_T);
			CHECK_RET_VAL_P(new_entry,-1,"malloc list entry failed!\n");
			new_entry->data = pData;
			new_entry->next = entry->next;
		
			if (!entry->next)
				ptSlist->tail = new_entry;
		
			entry->next = new_entry;
			ptSlist->entries++;

			return WOW_SUCCESS;
		} 
	}

	return -SLIST_NOT_MATCH;
}


/* brief    获取链表中头部内容 非pop
 * param ： ptSlist ：待操作的单项链表
 * return： 成功返回头部实例内容  失败返回NULL
 */
__EX_API__ void *wow_slist_peek_head(Slist_T* ptSlist)
{
	CHECK_RET_VAL_ERRNO_P(ptSlist,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL(ptSlist->head,NULL);


	return ptSlist->head->data;
}

/* brief    获取链表中底部内容 非pop
 * param ： ptSlist ：待操作的单项链表
 * return： 成功返回头部实例内容  失败返回NULL
 */
__EX_API__ void *wow_slist_peek_tail(Slist_T* ptSlist)
{
	CHECK_RET_VAL_ERRNO_P(ptSlist,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL(ptSlist->head,NULL);
	
	return ptSlist->tail->data;
}


/* brief    获取链表中头部内容 非pop
 * param ： ptSlist ：待操作的单项链表
 * parma ： nIdx    ：index标识
 * return： 成功返回头部实例内容  失败返回NULL
 */
__EX_API__ void *wow_slist_peek_by_index(Slist_T* ptSlist,int nIdx)
{
	SlistEntry_T *entry = NULL;

	CHECK_RET_VAL_ERRNO_P(ptSlist,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_ERRNO_P(nIdx >= 0 && nIdx < ptSlist->entries,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");	

	for (entry = ptSlist->head; entry && nIdx--; entry = entry->next);
	CHECK_RET_VAL_ERRNO_P(entry,SLIST_NOT_FIND,"peek nIdx entry failed!\n");

	return entry->data;
}

/* brief    获取链表中头部内容 非pop
 * param ： ptSlis   ：待操作的链表
 * parma ： fMatch   ：匹配回调函数 匹配回调函数 不可为NULL  1-匹配成功 0-匹配失败
 * parma ： pArg     ：匹配内容  不可为NULL
 * return： 成功返回头部实例内容  失败返回NULL
 */
__EX_API__ void* wow_slist_peek_by_match(Slist_T* ptSlist,slist_match_func_t fMatch,const void *pArg)
{
	SlistEntry_T *entry;

	CHECK_RET_VAL_ERRNO_P(ptSlist,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_ERRNO_P(fMatch,-PARAM_INPUT_FUNC_IS_NULL,"param input func invalid!\n");	
	//CHECK_RET_VAL_ERRNO_P(pArg,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");	

	for (entry = ptSlist->head; entry; entry = entry->next){
		if (fMatch(entry->data, pArg)){
			return entry->data;	
		}
	}

	return NULL;
}


/* brief    移除链表中头部实例
 * param ： ptSlist    ：待操作链表
 * return： 成功返回0  失败返回-1
 */
__EX_API__ int wow_slist_remove_head(Slist_T* ptSlist)
{
	return wow_slist_remove_by_index(ptSlist,0);
}

/* brief    移除链表中尾部实例
 * param ： ptSlist    ：待操作链表
 * return： 成功返回0  失败返回-1
 */
__EX_API__ int wow_slist_remove_tail(Slist_T* ptSlist)
{
	return wow_slist_remove_by_index(ptSlist,wow_slist_size(ptSlist)-1);
}

/* brief    匹配实例化内容进行删除
 * param ： ptSlist ：待操作链表
 * parma ： pData   ：实例化数据
 * return： 成功返回0  失败返回<0
 */
__EX_API__ int wow_slist_remove(Slist_T* ptSlist,void* pData)
{
	SlistEntry_T *entry = NULL;
	SlistEntry_T *prev	= NULL;

	CHECK_RET_VAL_P(ptSlist,PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(pData,PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");	

	for (entry = ptSlist->head, prev = NULL; entry;prev = entry, entry = entry->next) 
	{
		if (entry->data != pData) continue;

		if (prev)
			prev->next = entry->next;
		else
			ptSlist->head = entry->next;

		if (!entry->next) ptSlist->tail = prev;

		FREE(entry);
		ptSlist->entries--;

		return WOW_SUCCESS;
	}

	return -WOW_FAILED;
}

/* brief   当链表中实例内容
 * param ： ptSlist	 ：待操作链表
 * parma ： nIdx     ：index标识
 * return： 成功返回0	失败返回<0
 */
__EX_API__ int wow_slist_remove_by_index(Slist_T* ptSlist, int nIdx)
{
	SlistEntry_T *entry = NULL;
	SlistEntry_T *prev	= NULL;

	CHECK_RET_VAL_P(ptSlist,PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(nIdx >= 0 && nIdx < ptSlist->entries,PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");	

	for (entry = ptSlist->head; entry && nIdx--;prev = entry, entry = entry->next);
	CHECK_RET_VAL_P(entry,-1,"remove nIdx entry failed!\n");
		
	if (prev)
		prev->next = entry->next;
	else
		ptSlist->head = entry->next;

	if (!entry->next) ptSlist->tail = prev;

	FREE(entry);
	ptSlist->entries--;

	return WOW_SUCCESS;
}

/* brief   当单项链表中存在data匹配内容时移除此实例内容
 * param ： ptSlist ：待操作单项链表
 * parma ： fMatch  ：匹配回调函数 不可为NULL 1-匹配成功 0-匹配失败
 * parma ： pArg    ：需要移除的实例内容
 * return： 成功返回0  失败返回<0
 */
__EX_API__ int wow_slist_remove_by_match(Slist_T* ptSlist, slist_match_func_t fMatch,void *pArg)
{
	SlistEntry_T *entry= NULL;
	SlistEntry_T *prev = NULL;

	CHECK_RET_VAL_P(ptSlist,PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(fMatch,PARAM_INPUT_DATA_IS_NULL,"param input func invalid!\n");	
	CHECK_RET_VAL_P(pArg,PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");	
	
	for (entry = ptSlist->head, prev = NULL; entry;prev = entry, entry = entry->next) 
	{
		if (fMatch(entry->data, pArg) == 0) continue;

		if (prev)
			prev->next = entry->next;
		else
			ptSlist->head = entry->next;

		if (!entry->next) ptSlist->tail = prev;

		FREE(entry);
		ptSlist->entries--;

		return WOW_SUCCESS;
	}

	return -WOW_FAILED;
}


/* brief    替换头部数据内容
 * param ： ptSlist ：待操作单项链表
 * parma ： pData   ：新内容		
 * return： 成功返回0  失败返回<0
 */
int wow_slist_replace_head(Slist_T* ptSlist,void *pData)
{
	CHECK_RET_VAL_P(ptSlist,PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL(ptSlist->head,-SLIST_NO_ENTRY);


	ptSlist->head->data = pData;
	return WOW_SUCCESS;
}

/* brief    替换尾部部数据内容
 * param ： ptSlist  ：待操作单项链表
 * parma ： pData    ：新内容		
 * return： 成功返回0  失败返回<0
 */
int wow_slist_replace_tail(Slist_T* ptSlist,void *pData)
{
	CHECK_RET_VAL_P(ptSlist,PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL(ptSlist->tail,-SLIST_NO_ENTRY);


	ptSlist->tail->data = pData;
	return WOW_SUCCESS;
}


/* brief    通过索引替换链表数据内容
 * param ： ptSlist：待操作的单项链表
 * parma ： nIdx   ：index标识
 * parma ： npData ：新内容		
 * return： 成功返回0  失败返回-1
 */
int wow_slist_replace_by_index(Slist_T* ptSlist,int nIdx,void *pData)
{
	SlistEntry_T *entry = NULL;
	
	CHECK_RET_VAL_P(ptSlist,PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(nIdx >= 0 && nIdx < ptSlist->entries,PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");	

	for (entry = ptSlist->head; entry && nIdx--; entry = entry->next);
	CHECK_RET_VAL_P(entry,-1,"peek nIdx entry failed!\n");

	entry->data = pData;
	return WOW_SUCCESS;
}

/* brief    通过查找替换链表内容
 * param ： ptSlist    ：待操作单项链表
 * parma ： fMatch     ：匹配回调函数  1-匹配成功 0-匹配失败
 * parma ： pMatchData ：被替换内容  
 * parma ： pNewData   ：新内容				
 * return： 成功返回0  失败返回<0
 */
int wow_slist_replace_by_match(Slist_T* ptSlist, slist_match_func_t fMatch,
								void *pMatchData,void *pNewData)
{
	SlistEntry_T *entry;
	
	CHECK_RET_VAL_P(ptSlist,PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(fMatch,PARAM_INPUT_DATA_IS_NULL,"param input func invalid!\n");	
	CHECK_RET_VAL_P(pMatchData ,PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");	

	
	for (entry = ptSlist->head; entry; entry = entry->next){
		if (fMatch(entry->data, pMatchData)){
			entry->data = pNewData;
		}
	}
		
	return WOW_SUCCESS;
}
								
/* brief   获取链表中的实例内容
 * param ： ptSlist ：待操作单项链表
 * parma ： fForeach：匹配回调函数 不可为NULL
 * parma ： pArg    ：需要操作内容 可为NULL
 * return： 成功返回0  失败返回<0
 */
__EX_API__ int wow_slist_foreach(Slist_T* ptSlist, slist_foreach_func_t fForeach,void *pArg)
{
	SlistEntry_T *entry = NULL;
	
	CHECK_RET_VAL_P(ptSlist,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(fForeach,-PARAM_INPUT_DATA_IS_NULL,"param input func invalid!\n");	


	for (entry = ptSlist->head; entry ; entry = entry->next) {
		fForeach(entry->data,pArg);
	}
#if 0
	if (flag) { ///<1-正序
		for (entry = ptSlist->head.next; entry != &ptSlist->head; entry = entry->next) {
			fForeach(entry->data,pArg);
		}
	}else{ ///<0-逆序
		for (entry = ptSlist->head.prev; entry != &ptSlist->head; entry = entry->prev){
			fForeach(entry->data,pArg);
		}
	}
#endif

	return WOW_SUCCESS;
}

/* brief    移除单项链表内的全部实例
 * param ： ptSlist    ：待操作单项链表
 * parma ： fDestroy ：销毁回调函数 可为NULL
 * return： 成功返回0  失败返回<0
 */
__EX_API__ int wow_slist_clear(Slist_T* ptSlist,  slist_destroy_func_t fDestroy)
{
	SlistEntry_T *entry = NULL;
	SlistEntry_T *tmp   = NULL;
	
	CHECK_RET_VAL_P(ptSlist,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL(ptSlist->head,0);


	for (entry = ptSlist->head; entry; entry = tmp){
		if(fDestroy) {
			fDestroy(entry->data);
		}
		tmp = entry->next;
		FREE(entry);
	}
	
	memset(ptSlist, 0, sizeof(Slist_T));
	
	ptSlist->head = NULL;
	ptSlist->tail = NULL;
	ptSlist->entries = 0; 
	return WOW_SUCCESS;
}


/* brief    获取链表实例个数
 * param ： ptSlist    ：待操作单项链表
 * return： 成功返回链表实例个数 失败返回<0
 */
__EX_API__ int wow_slist_size(Slist_T* ptSlist)
{
	CHECK_RET_VAL_P(ptSlist,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");

	return ptSlist->entries;
}


