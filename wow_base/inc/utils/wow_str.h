#ifndef __WOW_BASE_UTILS_WOW_STR_H_ 
#define __WOW_BASE_UTILS_WOW_STR_H_ 

#ifdef __cplusplus
extern "C" {
#endif

/*brief    str字符串是否含有头部字符串
 *param ： pcStr:待执行指针
 *param ： prefix:头部字符串
 *return： 存在返回1 不存在返回0 无效信息返回<0
 */
int wow_str_has_prefix (const char *pcStr,const char *pcPrefix);

/*brief    str字符串是否含有尾部字符串
 *param ： pcStr:待执行指针
 *param ： pcSuffix:尾部字符串
 *return： 存在返回1 不存在返回0 无效信息返回<0
 */
int wow_str_has_suffix (const char *pcStr,const char *pcSuffix);


/*brief    将str字符串按delim分离参数
 *param ： pcStr:待执行指针
 *param ： pcDelim:分离参数标识
 *param ： pcCount:分离后参数个数
 *return： 成功返回分离结果
 *注 返回结果需进行free释放操作
 */
char **wow_str_split(const char *pcStr, const char *pcDelim, int *pnCount);
void wow_str_split_free(char** ppcStr);


/*brief    将str字符串合并
 *param ： nAmount:合并参数个数
 *param ： str1:待执行指针 （第一个参数不可为空）
 *return： 成功返回分离结果
 *注 返回结果需进行free释放操作
 */
char *wow_str_combine(int nAmount, const char *str1, ...);
void wow_str_combine_free(char** ppStr);


/*brief    将str字符串中pattern的替换为rep
 *param ： pcStr:待执行指针
 *param ： pPattern:被替换字符
 *param ： pcRep:待替换指针
 *return： 成功返回替换结果
 *注 返回结果需进行free释放操作
 */
char *wow_str_replace(const char *pcStr, const char *pPattern, const char *pcRep);
void wow_str_replace_free(char** ppStr);

#ifdef __cplusplus
}
#endif

#endif
