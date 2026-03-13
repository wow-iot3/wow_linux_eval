#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"


#include "utils/wow_byte_array.h"

struct  byte_array_t{
    size_t size;        
    size_t capicity;      
    uint8_t*  data;        
}__data_aligned__;

/*brief    创建动态数组
*param ：  nSize:数组字节数
 *return： 成功返回创建动态数组指针，失败返回NULL
 */
__EX_API__ ByteArray_T* wow_byte_array_new(int nSize) 
{
    ByteArray_T* array = CALLOC(1, ByteArray_T);
	CHECK_RET_VAL_ERRNO_P(array,SYSTEM_MALLOC_FAILED,"malloc ByteArray_T failed!\n");
	memset(array,0,sizeof(ByteArray_T));
	
	array->size     = 0;
    array->capicity = (nSize==0)?DEFALUT_BYTE_ARRAY_SZIE:nSize;
	array->data     = MALLOC(array->capicity);
	CHECK_RET_GOTO_ERRNO_P(array->data,out,SYSTEM_MALLOC_FAILED,"malloc size(%zu) failed!\n",array->capicity);
	memset(array->data,0,array->capicity);

    return array;
out:
	FREE(array);
	return NULL;
}



/*brief    释放动态数组
 *param ： pptArray:动态数组指针
 *return： 无
 */
__EX_API__ void wow_byte_array_free(ByteArray_T** pptArray)
{
	CHECK_RET_VOID(pptArray && *pptArray);
	ByteArray_T* array = (ByteArray_T*)*pptArray;

	if(array->data){
		FREE(array->data);
	}

    array->data = NULL;
	FREE(array);
	*pptArray = NULL;
}

/*brief    获取动态数组存储数据大小
 *param ： array:动态数组指针
 *return： 成功返回数组大小 失败返回<0
 */
__EX_API__ int wow_byte_array_size(ByteArray_T* ptArray) 
{
	CHECK_RET_VAL_P(ptArray,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");

	return ptArray->size;
}


/*brief    清空动态数组内容
 *param ： ptArray:动态数组指针
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_byte_array_clear(ByteArray_T* ptArray)
{
	CHECK_RET_VAL_P(ptArray,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	
	memset(ptArray->data,0,ptArray->size);
	ptArray->size = 0;
	return  WOW_SUCCESS;
}

static int byte_array_resize(ByteArray_T* ptArray,size_t nSize) 
{
	uint8_t* new_data = (uint8_t*)REALLOC(ptArray->data, nSize);
	CHECK_RET_VAL_P_A(new_data ,-1,"ralloc size(%zu) failed!\n",nSize);

	memset(&new_data[ptArray->capicity],0,nSize- ptArray->capicity);
	ptArray->data = new_data;
	
    ptArray->capicity = nSize;
	return 0;
}

/*brief    在动态数组追加内容
 *param ： ptArray:动态数组指针
 *param ： u8Value:待添加内容
 *return： 成功返回0,失败返回-1
 */
__EX_API__ int wow_byte_array_append_data(ByteArray_T* ptArray, uint8_t u8Value) 
{
	CHECK_RET_VAL_P(ptArray,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	
    if(ptArray->size >= ptArray->capicity) {
		byte_array_resize(ptArray,ptArray->size*2);
	}
    ptArray->data[ptArray->size++] = u8Value;
	
	return WOW_SUCCESS;
}

__EX_API__ int wow_byte_array_append_ndata(ByteArray_T* ptArray, uint8_t* u8Value,uint8_t snLen) 
{
	CHECK_RET_VAL_P(ptArray,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(u8Value,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	
    if(ptArray->size + snLen >= ptArray->capicity) {
		size_t nsize = (ptArray->size + snLen > (ptArray->size)*2)?ptArray->size + snLen:(ptArray->size)*2;
		byte_array_resize(ptArray,nsize);
	}
	memcpy(&ptArray->data[ptArray->size],u8Value,snLen);
    ptArray->size += snLen;

	return WOW_SUCCESS;
}

/*brief    在动态数组追加内容
 *param ： ptArray:动态数组指针
 *param ： ptToadd:待添加内容
 *return： 成功返回0,失败返回-1
 */
__EX_API__ int wow_byte_array_append_array(ByteArray_T* ptArray, ByteArray_T* ptToadd) 
{
	CHECK_RET_VAL_P(ptArray && ptToadd ,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(ptToadd->size < ptArray->capicity - ptArray->size ,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

	memcpy(&ptArray->data[ptArray->size],ptToadd->data,ptToadd->size);
	ptArray->size += ptToadd->size;

	return WOW_SUCCESS;
}


/*brief    在动态数组插入内容
 *param ： ptArray:动态数组指针
 *param ： snIdx:插入坐标
 *param ： u8Value:待添加内容
 *return： 成功返回0,失败返回-1
 */
__EX_API__ int wow_byte_array_insert(ByteArray_T* ptArray,size_t snIdx,uint8_t u8Value) 
{
    int i = 0;
	CHECK_RET_VAL_P(ptArray,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(snIdx < ptArray->size,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

	if(ptArray->size >= ptArray->capicity) {
		byte_array_resize(ptArray,ptArray->size*2);
	}
	
    //copy the data
    for ( i = ptArray->size; i > snIdx; i-- ) {
        ptArray->data[i] = ptArray->data[i - 1];    
    }
	
    ptArray->data[snIdx] = u8Value;
    ptArray->size++;
	
	return WOW_SUCCESS;
}

/*brief    截取动态数组左边len个长度
 *param ： ptArray:动态数组指针
 *param ： snLen:截取内容长度
 *return： 成功返回0,失败返回-1
 */
__EX_API__ int wow_byte_array_left(ByteArray_T* ptArray,size_t snLen)
{
	CHECK_RET_VAL_P(ptArray,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(ptArray->size > snLen,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

	memset(&ptArray->data[snLen],0,ptArray->size - snLen);
	ptArray->size = snLen;
	
	return WOW_SUCCESS;
}

/*brief    截取动态数组右边len个长度
 *param ： ptArray:动态数组指针
 *param ： snLen:截取内容长度
 *return： 成功返回0,失败返回-1
 */
__EX_API__ int wow_byte_array_right(ByteArray_T* ptArray,size_t snLen)
{
	CHECK_RET_VAL_P(ptArray,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(ptArray->size > snLen,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

	memmove(ptArray->data,&ptArray->data[ptArray->size - snLen],snLen);
	memset(&ptArray->data[snLen],0,ptArray->size - snLen);
	ptArray->size = snLen;

	return WOW_SUCCESS;
}

/*brief    截取动态数组idx位置len个长度
 *param ： ptArray:动态数组指针
 *param ： snIdx:截取内容坐标
 *param ： snLen:截取内容长度
 *return： 成功返回0,失败返回-1
 */
__EX_API__ int wow_byte_array_mid(ByteArray_T* ptArray,size_t snIdx, size_t snLen)
{
	CHECK_RET_VAL_P(ptArray,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(snIdx < ptArray->size,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

	if(ptArray->size > (snLen +snIdx)){
		memmove(ptArray->data,&ptArray->data[snIdx],ptArray->size - snLen);
		memset(&ptArray->data[snLen],0,ptArray->size - snLen);
		ptArray->size = snLen;
	}else{
		memmove(ptArray->data,&ptArray->data[snIdx],ptArray->size-snIdx);
		memset(&ptArray->data[snLen],0,ptArray->size-snIdx);
		ptArray->size = ptArray->size-snIdx;
	}
	
	return WOW_SUCCESS;
}


/*brief    获取动态数组内容
 *param ： ptArray:动态数组指针
 *param ： snIdx:指定坐标
 *param ： pu8Value:内容存储指针
 *return： 成功返回0，失败返回<0
 */
__EX_API__ int wow_byte_array_get( ByteArray_T* ptArray, size_t snIdx,uint8_t* pu8Value) 
{
	CHECK_RET_VAL_P(ptArray,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(snIdx < ptArray->size,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");
	
    *pu8Value = ptArray->data[snIdx];
	return 0;
}

/*brief    设置动态数组内容
 *param ： ptArray:动态数组指针
 *param ： snIdx:指定坐标
 *param ： u8Value:待设置内容
 *return： 成功返回0,失败返回<0
 */
__EX_API__ int wow_byte_array_set(ByteArray_T* ptArray, size_t snIdx,uint8_t u8Value) 
{
	CHECK_RET_VAL_P(ptArray,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(snIdx < ptArray->size,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    ptArray->data[snIdx] = u8Value;

    return WOW_SUCCESS;
}

/*brief    查找动态数组内容
 *param ： ptArray:动态数组指针
 *param ： u8Value:新内容
 *return： 成功返回位置标识, 失败返回<0
 */
__EX_API__ int wow_byte_array_find(ByteArray_T* ptArray,uint8_t u8Value) 
{
	int i = 0;
	CHECK_RET_VAL_P(ptArray,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	 
	for ( i = 0; i < ptArray->size; i++ ) {
        if (u8Value == ptArray->data[i]) {
			return i;
        }
    }
	
	return -BYTE_ARRAY_NOT_FIND;
}

/*brief    替换动态数组内容
 *param ： ptArray:动态数组指针
 *param ： u8Obj:待替换内容
 *param ： u8Value:新内容
 *return： 成功返回0,失败返回<0
 */
__EX_API__ int wow_byte_array_replace(ByteArray_T* ptArray,uint8_t u8Obj,uint8_t u8Value) 
{
	int i = 0;
	CHECK_RET_VAL_P(ptArray,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	 
	for ( i = 0; i < ptArray->size; i++ ) {
        if (u8Obj == ptArray->data[i]) {
			ptArray->data[i] = u8Value;
        }
    }
	
	return WOW_SUCCESS;
}

/*brief    删除动态数组内容
 *param ： ptArray:动态数组指针
 *param ： snIdx:指定坐标
  param ： snLen:删除内容长度
 *return： 成功返回0,失败返回<0
 */
__EX_API__ int wow_byte_array_remove(ByteArray_T* ptArray, size_t snIdx ,size_t snLen) 
{
	CHECK_RET_VAL_P(ptArray,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(snIdx < ptArray->size,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

	if(ptArray->size > (snLen +snIdx)){
		memmove(ptArray->data,&ptArray->data[snIdx+snLen],ptArray->size - snLen);
		memset(&ptArray->data[ptArray->size - snLen],0,snLen);
		ptArray->size = ptArray->size - snLen;
	}else{
		memset(&ptArray->data[snIdx],0,ptArray->size-snIdx);
		ptArray->size = snIdx;
	}
	
	return WOW_SUCCESS;
}

/*brief    获取动态数组内容
 *param ： ptArray:动态数组指针
 *return： 成功返回内容信息,失败返回NULL
 */
__EX_API__ uint8_t* wow_byte_array_data(ByteArray_T* ptArray) 
{
	CHECK_RET_VAL_ERRNO_P(ptArray,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");

	return  ptArray->data;
}

