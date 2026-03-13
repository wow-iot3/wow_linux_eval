#ifndef __WOW_BASE_UTILS_WOW_STRING_H_
#define __WOW_BASE_UTILS_WOW_STRING_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct string_t String_T;

/*brief    创建字符串
 *return： 成功返回字符串指针，失败返回NULL
 */
String_T* wow_string_new(void);

/*brief    释放字符串
 *param ： pptString:字符串指针
 *return： 无
 */
void wow_string_free(String_T** pptString);

/*brief    获取字符串大小
 *param ： ptString:动态数组指针
 *return： 成功返回字符串大小 失败返回<0
 */
int wow_string_size(String_T* ptString);

/*brief    截取字符串左边len个长度
 *param ： ptString:字符串指针
 *param ： len:截取内容长度
 *return： 成功返回0,失败返回<0
 */
int wow_string_left(String_T* ptString,size_t len);

/*brief    截取字符串右边len个长度
 *param ： ptString:动态数组指针
 *param ： len:截取内容长度
 *return： 成功返回0,失败返回<0
 */
int wow_string_right(String_T* ptString,size_t len);

/*brief    截取字符串idx位置len个长度
 *param ： ptString:动态数组指针
 *param ： idx:截取内容坐标
 *param ： len:截取内容长度
 *return： 成功返回0,失败返回<0
 */
int wow_string_mid(String_T* ptString,size_t idx, size_t len);

/*brief    在字符串添加
 *param ： ptString:动态数组指针
 *param ： pcData:待添加内容
 *return： 成功返回0,失败返回<0
 */
int wow_string_append_data(String_T* ptString, const char* pcData);

/*brief    在字符串追加内容
 *param ： ptString:动态数组指针
 *param ： ptADd:待添加内容
 *return： 成功返回0,失败返回<0
 */
int wow_string_append_string(String_T* ptString, String_T* ptADd);

/*brief    获取字符串内容
 *param ： ptString:动态数组指针
 *return： 成功返回内容信息,失败返回NULL
 */
const char* wow_string_data(String_T* ptString);

/*格式转化*/
int  wow_string_toBool(String_T* ptString,int* pnVal);
int  wow_string_toInt(String_T* ptString,int* pnVal);
int  wow_string_toFloat(String_T* ptString,float* pfVal);

/*brief    判断str是否为string的子串
 *param ： ptString:动态数组指针
 *param ： pcStr:待查找字符串
 *return： 成功返回首次出现子串的地址,失败返回NULL
 */
char* wow_string_str(String_T* ptString,const char* pcStr);

/*brief    判断str是否为string相同
 *param ： ptString:动态数组指针
 *param ： pcStr:待比较字符串
 *return： 相等返回0，大于返回>0 小于失败返回<0
 */
int wow_string_cmp(String_T* ptString, char const* pcStr);

#ifdef __cplusplus
}
#endif

#endif

