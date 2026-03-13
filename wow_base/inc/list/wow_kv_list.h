#ifndef __WOW_BASE_LIST_WOW_KV_LIST_H_
#define __WOW_BASE_LIST_WOW_KV_LIST_H_

typedef struct kv_list_t KvList_T;

/*brief    初始化键值链表
 *return： 成功返回键值链表操作符
 */
KvList_T* wow_kvlist_create(void);

/*brief    退出键值链表
 *param ： ptKvlist:键值链表操作符
 *return： 无
 */
void wow_kvlist_destroy(KvList_T** pptKvlist);

/*brief    设置键值链表
 *param ： ptKvlist:键值链表操作符
 *param ： pcKey:名称
 *param ： pcVal:值
 *return： 成功返回0 失败返回<0
 */
int wow_kvlist_set(KvList_T *ptKvlist,char *pcKey,char *pcVal);

/*brief    获取键值链表
 *param ： ptKvlist:键值链表操作符
 *param ： pcKey:键值
 *return： 成功返回值 失败返回NULL
 */
char* wow_kvlist_get(KvList_T *ptKvlist, char *pcKey);

/*brief    更新键值链表
 *param ： ptKvlist:键值链表操作符
 *param ： pcKey:名称
 *param ： pcVal:值
 *return： 成功返回0 失败返回<0
 */
int wow_kvlist_update(KvList_T *ptKvlist,char *pcKey,char *pcVal);

/*brief    删除键值链表
 *param ： ptKvlist:键值链表操作符
 *return： 成功返回0 失败返回<0
 */
int wow_kvlist_delete(KvList_T *ptKvlist, char *pcKey);

/*brief    获取链表键值个数
 *param ： ptKvlist:键值链表操作符
 *return： 成功返回个数 失败返回<0
 */
int wow_kvlist_size(KvList_T *ptKvlist);

/*brief    遍历链表
 *param ： ptKvlist:键值链表操作符
 *param ： ppcKeys:名称存储地址
 *param ： ppcValues:值存储地址
 *param ： pnCount:键值对个数存储地址  
 *return： 成功返回0 失败返回<0
 */
int wow_kvlist_foreach(KvList_T *ptKvlist,char **ppcKeys, char **ppcValues, int *pnCount);


#endif
