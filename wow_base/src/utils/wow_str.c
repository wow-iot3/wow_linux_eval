#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <ctype.h>
#include <stdarg.h>


#include "prefix/wow_check.h"
#include "prefix/wow_common.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_errno.h"
#include "utils/wow_str.h"



/*brief    将str字符串按delim分离参数
 *param ： pcStr:待执行指针
 *param ： pcDelim:分离参数标识
 *param ： pnCount:分离后参数个数
 *return： 成功返回分离结果
 *注 返回结果需进行free释放操作
 */
__EX_API__ char **wow_str_split(const char *pcStr, const char *pcDelim, int *pnCount) 
{
	CHECK_RET_VAL_ERRNO_P(pcStr && pcDelim && pnCount,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	
	*pnCount = 0;

	int i = 0;
	int size = 0;
	char *copy  = NULL;
	char *token = NULL;
	char** str_array = NULL;

	copy = strdup(pcStr);
	CHECK_RET_VAL_ERRNO_P(copy,-SYSTEM_STRDUM_FAILED, "strdup failed!\n");

	token = strsep(&copy, pcDelim);
	while (token){//获取个数
		size++;
		token  = strsep(&copy, pcDelim);
	}
	free(copy);

	str_array = MALLOC((size+1) * sizeof(char*));
	CHECK_RET_VAL_ERRNO_P(str_array,-SYSTEM_MALLOC_FAILED,"malloc pcStr failed!\n");

	copy = strdup(pcStr);
	CHECK_RET_GOTO_ERRNO_P(copy,out,-SYSTEM_STRDUM_FAILED, "strdup failed!\n");
	token = strsep(&copy, pcDelim);
	for(i = 0; i < size; i++){	
		str_array[i] = strdup(token);
		token  = strsep(&copy, pcDelim);	
	}
	str_array[i] = NULL;
	free(copy);
	
	*pnCount = size;
	return str_array;
out:
	FREE(str_array);
	return NULL;
}


__EX_API__ void wow_str_split_free(char** ppcStr)
{
	CHECK_RET_VOID(ppcStr);
	
	int i;
	for (i = 0; ppcStr[i] != NULL; i++){
		free (ppcStr[i]);
	}
	FREE (ppcStr);

}


/*brief    将str字符串合并
 *param ： nAmount:合并参数个数
 *param ： str1:待执行指针 （第一个参数不可为空）
 *return： 成功返回分离结果
 *注 返回结果需进行free释放操作
 */
__EX_API__ char *wow_str_combine(int nAmount, const char *str1, ...) 
{
	CHECK_RET_VAL_ERRNO_P(str1 && strlen(str1) >= 1,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_ERRNO_P(nAmount > 0,-PARAM_INPUT_SIZE_IS_SMALL,"param input size invalid!\n");

	char* p = NULL;
	char *temStr = NULL;
	size_t length   = strlen(str1) + 1;
	char *result = MALLOC(length);
	CHECK_RET_VAL_ERRNO_P(result,-SYSTEM_MALLOC_FAILED,"malloc size:%zu failed!\n",length);

	strncpy(result, str1, length);
	
	va_list args;
	va_start(args, str1);

	while (--nAmount > 0) {
		temStr = va_arg(args, char*);
		CHECK_RET_CONTINUE(temStr);

		length = length + strlen(temStr);
		p = (char *)MALLOC(length);
		CHECK_RET_GOTO_ERRNO_P(p, out,-SYSTEM_MALLOC_FAILED,"malloc size:%zu failed!\n",length);

		strncpy(p, result, length);
		strncat(p, temStr , length);
		
		FREE(result);
		result = p;
	}
	va_end(args);

	return result;

out:
	FREE(result);
	return NULL;
}


__EX_API__ void wow_str_combine_free(char** ppStr)
{
	CHECK_RET_VOID(ppStr && *ppStr);

	FREE(*(ppStr));
	*(ppStr) = NULL;
}


/*brief    str字符串是否含有尾部字符串
 *param ： pcStr:待执行指针
 *param ： pcSuffix:尾部字符串
 *return： 存在返回1 不存在返回0 无效信息返回<0
 */
__EX_API__ int wow_str_has_suffix (const char *pcStr,const char *pcSuffix)
{
	int str_len;
	int suffix_len;

	CHECK_RET_VAL_P(pcStr && pcSuffix,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

	str_len = strlen (pcStr);
	suffix_len = strlen (pcSuffix);

	if (str_len < suffix_len) return 0;

	return strcmp (pcStr + str_len - suffix_len, pcSuffix) == 0;
}

/*brief    str字符串是否含有头部字符串
 *param ： pcStr:待执行指针
 *param ： prefix:头部字符串
 *return： 存在返回1 不存在返回0 无效信息返回<0
 */
__EX_API__ int wow_str_has_prefix (const char *pcStr,const char *pcSuffix)
{
	CHECK_RET_VAL_P(pcStr && pcSuffix,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

    return strncmp (pcStr, pcSuffix, strlen (pcSuffix)) == 0;
}

static size_t getLen(size_t old, size_t size) {
	if (old == 1) old = 2;

	while (old < size) 
	{
		old = (old * 1.5);
	}
	return old;
}

static char *ensureSpace(char *dest, int *curSize, int size) 
{
	if (*curSize < size) 
	{
		*curSize = getLen(*curSize, size);
		char *tmp = REALLOC(dest, *curSize);
		if (tmp == NULL)
		{
			FREE(dest);
			return NULL;
		}

		return tmp;
	}

	return dest;
}

/*brief    将str字符串中pattern的替换为rep
 *param ： pcStr:待执行指针
 *param ： pPattern:被替换字符
 *param ： pcRep:待替换指针
 *return： 成功返回替换结果
 *注 返回结果需进行free释放操作
 */
__EX_API__ char *wow_str_replace(const char *pcStr, const char *pPattern, const char *pcRep) 
{
	CHECK_RET_VAL_ERRNO_P(pcStr && pPattern && pcRep,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	
	const char *s = pcStr;
	
	int start = 0;
	int oldLen = strlen(pcStr);
	int newLen = oldLen;
	int repLen = strlen(pcRep);
	int patternLen = strlen(pPattern);

	char *dest = MALLOC(oldLen + 1);
	CHECK_RET_VAL_ERRNO_P(dest,-SYSTEM_MALLOC_FAILED,"malloc size(%d) failed!\n",oldLen);

	while (1) 
	{
		char *p = strstr(pcStr, pPattern);
		if (p == NULL) 
		{  // remain does not contain pPattern
			int remain = (oldLen - (pcStr - s));
			int size = remain + start + 1;

			dest = ensureSpace(dest, &newLen, size);
			if (dest == NULL) return NULL;


			strcpy(dest + start, pcStr);
			dest[start + remain] = 0;
			break;
		}

		int len = p - pcStr;
		int size = start + len + repLen + 1;
		dest = ensureSpace(dest, &newLen, size);
		CHECK_RET_VAL_ERRNO_P(dest,-SYSTEM_MALLOC_FAILED,"realloc  size(%d) failed!\n",size);

		memcpy(dest + start, pcStr, len);
		pcStr += (len + patternLen);
		start += len;

		memcpy(dest + start, pcRep, repLen);
		start += repLen;
	}

	return dest;
}

__EX_API__ void wow_str_replace_free(char** ppStr)
{
	CHECK_RET_VOID(ppStr && *ppStr);

	FREE(*(ppStr));
	*(ppStr) = NULL;
}