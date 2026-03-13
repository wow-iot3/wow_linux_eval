#ifndef __WOW_IOT_HASH_WOW_HASH_MAP_H_
#define __WOW_IOT_HASH_WOW_HASH_MAP_H_


#include "element/wow_element.h"

#ifdef __cplusplus
 extern "C" {
#endif


 
typedef struct hash_map_T HashMap_T;
typedef void (*hashmap_foreach_func_t)(const void *data, const void *priv);

/*brief	:  hash map初始化
 *param ： key_type:key数据类型
 *param ： val_type:val数据类型
 *return： 成功返回hash_map操作辨识，失败返回NULL
 */
HashMap_T* wow_hash_map_create(ElementType_E eKtype,ElementType_E eVtype);

/*brief	:  释放hash map
 *param ： ptHashMap:hash_map操作符
 *return： 无
 */
void  wow_hash_map_destroy(HashMap_T** ptHashMap);

/*brief	:  hash map清空操作
 *param ： ptHashMap:hash_map操作符
 *return： 无
 */
void  wow_hash_map_clear(HashMap_T* ptHashMap);

/*brief	:  hash map插入key映射信息
 *param ： ptHashMap:hash_map操作符
 *param ： pKey:key内容
 *param ： pVal:val内容
 *return： 成功返回0，失败返回-1
  注：已存在"key"关键信息 则进行替换
 */
int wow_hash_map_insert(HashMap_T* ptHashMap, const void* pKey, const void* pVal);

/*brief	:  hash map获取key映射信息
 *param ： ptHashMap:hash_map操作符
 *param ： pKey:key内容
 *return： 成功返回key映射内容，失败返回NULL
 */
void* wow_hash_map_get(HashMap_T* ptHashMap, const void* pKey);


/*brief	:  hash map移除key映射信息
 *param ： ptHashMap:hash_map操作符
 *param ： pKey:key内容
 *return： 无
 */
void   wow_hash_map_remove(HashMap_T* ptHashMap, const void* pKey);

/*brief	:  hash map获取key映射个数
 *param ： hash_map:hash_map操作符
 *return： 返回已存在映射个数
 */
size_t  wow_hash_map_size(HashMap_T* ptHashMap);

/* brief    获取hash map的实例内容
 * param ： hash_map : hash_map操作符
 * parma ： fCallBack：匹配回调函数 不可为NULL 
 * parma ： pArg ：需要操作内容 可为NULL
 * return： 无
 */
void wow_hash_map_foreach(HashMap_T* ptHashMap, hashmap_foreach_func_t fCallBack,void *pArg);


#ifdef __cplusplus
}
#endif

#endif

