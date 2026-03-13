#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"

#include "list/wow_slist.h"
#include "list/wow_string_list.h"

typedef struct {
	char* data;
}__data_aligned__ StringListEntry_T;


/*brief    创建字符串链表
 *return： 成功返回创建动字符串链表指针，失败返回NULL
 */
StringList_T* wow_stringlist_new(void)
{
	return (StringList_T*)wow_slist_create();	
}

static void slist_free_func(void *data)
{
	CHECK_RET_VOID(data);
	FREE(data);
}

/*brief    释放字符串链表
 *param ： pptStrList:待操作字符串指针
 *return： 无
 */
void wow_stringlist_free(StringList_T** pptStrList)
{
	wow_slist_destroy(pptStrList,slist_free_func);
}


/*brief    字符串链表添加元素
 *param ： ptStrList:待操作字符串指针
 *param ： pcElem:添加元素内容
 *return：  成功返回0 失败返回<0
 */
int wow_stringlist_append(StringList_T* ptStrList, const char *pcElem)
{
	CHECK_RET_VAL_P(ptStrList,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(pcElem && strlen(pcElem) > 0,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

	int len = strlen(pcElem);
	
	char* data = MALLOC(len + 1);
	CHECK_RET_VAL_P(data,-SYSTEM_MALLOC_FAILED,"malloc data size!\n");
	memset(data,0,len + 1);
	memcpy(data,pcElem,len);
	
	return wow_slist_insert_tail(ptStrList,data);
}

/*brief    获取串链表元素个数
 *param ： ptStrList:待操作字符串指针
 *return： 成功返回个数 失败返回<0
 */
int wow_stringlist_size(StringList_T* ptStrList)
{
	return wow_slist_size(ptStrList);
}


/*brief    以字符串方式获取串链表元素内容
 *param ： ptStrList:待操作字符串指针
 *param ： nIdx:待操作字符串标识
 *return： 成功返回获取内容 失败返回NULL
 */
char* wow_stringlist_data(StringList_T* ptStrList,int nIdx)
{
	return (char*)wow_slist_peek_by_index(ptStrList,nIdx);
}

int wow_stringlist_toInt(StringList_T* ptStrList,int nIdx)
{
	char* data = (char*)wow_slist_peek_by_index(ptStrList,nIdx);
	CHECK_RET_ABORT(data);
	
	return strtol(data,NULL,10);
}

float wow_stringlist_toFloat(StringList_T* ptStrList,int nIdx)
{
	char* data = (char*)wow_slist_peek_by_index(ptStrList,nIdx);
	CHECK_RET_ABORT(data);

	return (float)strtod(data,NULL);
}

/*brief    分割字符串
 *param ： pcStr:待分割字符串指针
 *param ： pcDelim:分隔符
 *return： 成功返回字符串指针 失败返回NULL
 */
StringList_T* wow_stringlist_split(char *pcStr, const char *pcDelim)
{
	StringList_T* ptStrList = wow_stringlist_new();
	CHECK_RET_VAL_P(ptStrList,NULL, "string_list new failed!\n");

	char *copy = strdup(pcStr);
	CHECK_RET_GOTO_P_A(copy,out, "strdup failed!\n");

	int ret = -1;
	char *token = strtok(copy, pcDelim);
	while (token){
		ret = wow_stringlist_append(ptStrList, token);
		CHECK_RET_GOTO(ret == 0,out);
	
		token  = strtok(NULL, pcDelim);
	}

	free(copy);
	return ptStrList;

out:
	wow_stringlist_free(&ptStrList);
	free(copy);
	return NULL;
}



