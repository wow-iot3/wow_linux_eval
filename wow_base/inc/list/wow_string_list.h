#ifndef __WOW_BASE_LIST_WOW_STRING_LIST_H_
#define __WOW_BASE_LIST_WOW_STRING_LIST_H_

#include "list/wow_slist.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef Slist_T StringList_T;

/*brief    创建字符串链表
 *return： 成功返回创建动字符串链表指针，失败返回NULL
 */
StringList_T* wow_stringlist_new(void);

/*brief    释放字符串链表
 *param ： pptStrList:待操作字符串指针
 *return： 无
 */
void wow_stringlist_free(StringList_T** pptStrList);


/*brief    字符串链表添加元素
 *param ： ptStrList:待操作字符串指针
 *param ： pcElem:添加元素内容
 *return： 成功返回0 失败返回<0
 */
int wow_stringlist_append(StringList_T* ptStrList, const char *pcElem);

/*brief    获取串链表元素个数
 *param ： ptStrList:待操作字符串指针
 *return： 成功返回个数 失败返回<0
 */
int wow_stringlist_size(StringList_T* ptStrList);

/*brief    以字符串方式获取串链表元素内容
 *param ： ptStrList:待操作字符串指针
 *param ： nIdx:待操作字符串标识
 *return： 成功返回获取内容 失败返回NULL
 */
char* wow_stringlist_data(StringList_T* ptStrList,int nIdx);


/*brief    以整形模式获取串链表元素内容
 *param ： ptStrList:待操作字符串指针
 *param ： nIdx:待操作字符串标识
 *return： 成功返回获取内容 失败返回NULL
 */
int   wow_stringlist_toInt(StringList_T* ptStrList,int nIdx);
float wow_stringlist_toFloat(StringList_T* ptStrList,int nIdx);

/*brief    分割字符串
 *param ： pcStr:待分割字符串指针
 *param ： pcDelim:分隔符
 *return： 成功返回字符串指针 失败返回NULL
 * 注：使用后需使用wow_stringlist_free释放
 */
StringList_T* wow_stringlist_split(char *pcStr, const char *pcDelim);

#ifdef __cplusplus
}
#endif

#endif
