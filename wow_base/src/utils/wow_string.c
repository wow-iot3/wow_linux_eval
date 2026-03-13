#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <ctype.h>

#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_common.h"
#include "prefix/wow_keyword.h"

#include "utils/wow_string.h"


#define DEFALUT_STRING_SZIE 256

struct  string_t{
	size_t		size;
	size_t		capicity;
	char*		data;
}__data_aligned__;

static int string_resize(String_T* string,size_t cap) 
{
	if(string->data){
	    void* new_data = REALLOC(string->data, cap);
		CHECK_RET_VAL_P_A(new_data ,-1,"calloc string failed!\n");	
		memset(new_data +string->capicity,0,cap - string->capicity);
		string->data = new_data;
	}else{
		string->data = MALLOC(cap);
		CHECK_RET_VAL_P_A(string->data ,-1,"malooc size(%zu) failed!\n",cap);
		memset(string->data,0,cap);
	}
	
    string->capicity = cap;
	
	return WOW_SUCCESS;
}

/*brief    创建字符串
 *return： 成功返回字符串指针，失败返回NULL
 */
__EX_API__ String_T* wow_string_new(void)
{
	String_T* string = CALLOC(1, String_T);
	CHECK_RET_VAL_ERRNO_P(string,-SYSTEM_MALLOC_FAILED,"malloc String_T failed!\n");

	string->size     = 0;
	string->capicity = 0;
	string->data     = NULL;
	if(string_resize(string,DEFALUT_STRING_SZIE) < 0){
		FREE(string);
		return NULL;

	}
	return (String_T*)string;
}

/*brief    释放字符串
 *param ： pptString:字符串指针
 *return： 无
 */
__EX_API__ void wow_string_free(String_T** pptString)
{
	CHECK_RET_VOID(pptString && *pptString);

	String_T* string = (String_T*)*pptString;
    if (string->data){
		FREE(string->data);
		string->data = NULL;
	}
	
    FREE(string);
	*pptString = NULL;
}

/*brief    获取字符串大小
 *param ： ptString:动态数组指针
 *return： 成功返回字符串大小 失败返回<0
 */
__EX_API__ int wow_string_size(String_T* ptString)
{
	CHECK_RET_VAL_P(ptString,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	
    return ptString->size;
}

/*brief    截取字符串左边len个长度
 *param ： ptString:字符串指针
 *param ： snLen   :截取内容长度
 *return： 成功返回0,失败返回<0
 */
__EX_API__ int wow_string_left(String_T* ptString,size_t snLen)
{
	CHECK_RET_VAL_P(ptString ,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(snLen > 0 ,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

	if(ptString->size > snLen){
		memset(&ptString->data[snLen],0,ptString->size - snLen);
		ptString->size = snLen;
	}
	
	return WOW_SUCCESS;
}

/*brief    截取字符串右边len个长度
 *param ： ptString:动态数组指针
 *param ： snLen:截取内容长度
 *return： 成功返回0,失败返回<0
 */
__EX_API__ int wow_string_right(String_T* ptString,size_t snLen)
{
	CHECK_RET_VAL_P(ptString ,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(snLen > 0 ,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

	if(ptString->size > snLen){
		memmove(ptString->data,&ptString->data[ptString->size - snLen],snLen);
		memset(&ptString->data[snLen],0,ptString->size - snLen);
		ptString->size = snLen;
	}
	
	return WOW_SUCCESS;
}

/*brief    截取字符串idx位置len个长度
 *param ： ptString:动态数组指针
 *param ： snIdx:截取内容坐标
 *param ： snLen:截取内容长度
 *return： 成功返回0,失败返回<0
 */
__EX_API__ int wow_string_mid(String_T* ptString,size_t snIdx, size_t snLen)
{
	CHECK_RET_VAL_P(ptString ,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(snLen > 0 && snIdx < ptString->size ,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

	if(ptString->size > (snLen +snIdx)){
		memmove(ptString->data,&ptString->data[snIdx],ptString->size - snLen);
		memset(&ptString->data[snLen],0,ptString->size - snLen);
		ptString->size = snLen;
	}else{
		memmove(ptString->data,&ptString->data[snIdx],ptString->size-snIdx);
		memset(&ptString->data[snLen],0,ptString->size-snIdx);
		ptString->size = ptString->size-snIdx;
	}
	
	return WOW_SUCCESS;
}

/*brief    在字符串添加
 *param ： ptString:动态数组指针
 *param ： pcData:待添加内容
 *return： 成功返回0,失败返回<0
 */
__EX_API__ int wow_string_append_data(String_T* ptString, const char* pcData) 
{
	CHECK_RET_VAL_P(ptString ,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(pcData,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

	int snLen = strlen(pcData);
    if(ptString->size + snLen >= ptString->capicity) {
		if(snLen > ptString->capicity){
			string_resize(ptString,ptString->capicity + snLen);
		}else{
			string_resize(ptString,ptString->capicity * 2);
		}
	}
	strncpy(&ptString->data[ptString->size],pcData,snLen);
	ptString->size += snLen;
	
	return WOW_SUCCESS;
}


/*brief    在字符串追加内容
 *param ： ptString:动态数组指针
 *param ： add:待添加内容
 *return： 成功返回0,失败返回<0
 */
__EX_API__ int wow_string_append_string(String_T* ptString, String_T* ptAadd) 
{
	CHECK_RET_VAL_P(ptString && ptAadd,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");;
	
    if(ptAadd->size >= ptString->capicity - ptString->size) {
		string_resize(ptString,ptAadd->capicity + ptString->capicity);
	}

	strncpy(&ptString->data[ptString->size],ptAadd->data,ptAadd->size);
	ptString->size += ptAadd->size;
	
	return WOW_SUCCESS;
}

/*brief    获取字符串内容
 *param ： ptString:动态数组指针
 *return： 成功返回内容信息,失败返回NULL
 */
__EX_API__ const char* wow_string_data(String_T* ptString)
{
	CHECK_RET_VAL_ERRNO_P(ptString ,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");

    return ptString->data;
}



__EX_API__ int wow_string_toBool(String_T* ptString,int* pnVal)
{
	CHECK_RET_VAL_P(ptString ,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");

	if (!strcmp(ptString->data, "1") || !strcasecmp(ptString->data, "y") || !strcasecmp(ptString->data, "yes") || !strcasecmp(ptString->data, "true")){
		*pnVal = 1;
		return WOW_SUCCESS;

	}else if(!strcmp(ptString->data, "0") || !strcasecmp(ptString->data, "n") || !strcasecmp(ptString->data, "no") || !strcasecmp(ptString->data, "false")){
		*pnVal = 0;
		return WOW_SUCCESS;
	}else{
		return PARAM_INPUT_ARG_INVALID;
	}
}


__EX_API__ int wow_string_toInt(String_T* ptString,int* pnVal)
{
	CHECK_RET_VAL_P(ptString ,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");

    *pnVal = strtol(ptString->data,NULL,10);
	return WOW_SUCCESS;
}


__EX_API__ int wow_string_toFloat(String_T* ptString,float* pfVal)
{
	CHECK_RET_VAL_P(ptString ,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");

    *pfVal = (float)strtod(ptString->data,NULL);
	return WOW_SUCCESS;
}


/*brief    判断str是否为string的子串
 *param ： ptString:动态数组指针
 *param ： ptStr:待查找字符串
 *return： 成功返回首次出现子串的地址,失败返回NULL
 */
__EX_API__ char* wow_string_str(String_T* ptString,const char* ptStr)
{

	CHECK_RET_VAL_ERRNO_P(ptString ,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_ERRNO_P(ptStr,-PARAM_INPUT_DATA_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_ERRNO_P(strlen(ptStr) < ptString->size,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");


    return strstr(ptString->data, ptStr);
}

/*brief    判断str是否为string相同
 *param ： ptString:动态数组指针
 *param ： ptStr:待比较字符串
 *return： 相等返回0，大于返回>0 小于失败返回<0
 */
__EX_API__ int wow_string_cmp(String_T* ptString, const char* ptStr)
{
	CHECK_RET_VAL_P(ptString ,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(ptStr,-PARAM_INPUT_DATA_IS_NULL,"param input struct invalid!\n");

	return strncmp(ptString->data,ptStr, strlen(ptStr));
}


