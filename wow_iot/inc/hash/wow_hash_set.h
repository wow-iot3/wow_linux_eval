#ifndef __WOW_IOT_HASH_WOW_HASH_SET_H_
#define __WOW_IOT_HASH_WOW_HASH_SET_H_


#include "element/wow_element.h"
#include "hash/wow_hash_map.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef HashMap_T HashSet_T;

/*brief	:  hash set初始化
 *param ： eType:存储数据类型
 *return： 成功返回hash_set操作辨识，失败返回NULL
 */
HashSet_T* wow_hash_set_create(ElementType_E eType);

/*brief	:  退出hash set
 *param ： hash_set:hash set操作符
 *return： 无
 */
void wow_hash_set_destroy(HashSet_T** ppHashSet);

/*brief	:  hash set清空操作
 *param ： ptHashSet:hash set操作符
 *return： 无
 */
void wow_hash_set_clear(HashSet_T* ptHashSet);

/*brief	:  hash get获取操作
 *param ： ptHashSet:hash set操作符
 *param ： pData:待插入数据内容
 *return： 成功获取成功返回t0获取失败返回<0
 */
int wow_hash_set_get(HashSet_T* ptHashSet, const void* pData);


/*brief	:  hash set插入操作
 *param ： ptHashSet:hash set操作符
 *param ： pData:待插入数据内容
 *return： 成功返回0 失败返回<0
 */
int wow_hash_set_insert(HashSet_T* ptHashSet, const void* pData);

/*brief	:  hash set移除操作
 *param ： ptHashSet:hash set操作符
 *param ： pData:待移除数据内容
 *return： 无
 */
void wow_hash_set_remove(HashSet_T* ptHashSet, const void* pData);

/*brief	:  获取hash set元素个数
 *param ： ptHashSet:hash set操作符
 *param ： pData:待插入数据内容
 *return： 成功返回元素个数 失败返回<0
 */
size_t wow_hash_set_size(HashSet_T* ptHashSet);


#ifdef __cplusplus
}
#endif

#endif

