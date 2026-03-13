#ifndef __WOW_BASE_LIST_WOW_SLIST_H_
#define __WOW_BASE_LIST_WOW_SLIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef struct slist_t Slist_T;

typedef void (*slist_destroy_func_t)(void *data);
typedef void (*slist_foreach_func_t)(const void *data, const void *user_data);
/* true-匹配成功 false-匹配失败 */
typedef bool  (*slist_match_func_t)(const void *data, const void *match_data);



/* brief    创建链表
 * return： 成功返回创建指针  失败返回NUUL
 */
Slist_T* wow_slist_create(void);

/* brief    销毁链表
 * param ： pptSlist ：待键入data的链表
 * parma ： fDestroy ：销毁回调函数 可为NULL
 * return： 成功返回创建指针  失败返回NUUL
 */
void wow_slist_destroy(Slist_T** pptSlist, slist_destroy_func_t fDestroy);


/* brief    将data的内容push到链表头部
 * param ： ptSlist ：待键入data的链表
 * parma ： pData   ：实例化数据
 * return： 成功返回0  失败返回<0
 */
int wow_slist_insert_head(Slist_T* ptSlist, void *pData);

/* brief   将data的内容push到链表底部
 * param ： ptSlist ：待键入data的链表
 * parma ： pData   ：实例化数据
 * return： 成功返回0  失败返回<0
 */
int wow_slist_insert_tail(Slist_T* ptSlist, void *pData);

/* brief   将data的内容push到index位置处
 * param ： ptSlist ：待键入data的队列
 * parma ： nIdx    ：index标识
 * parma ： pData   ：实例化数据
 * return： 成功返回0  失败返回<0
 */
int wow_slist_insert_by_index(Slist_T* ptSlist, int nIdx, void *pData);

/* brief   将data的内容push到某个实例的后面(可增加优先级机制)--未查找即追加
 * param ： ptSlist：待键入data的队列
 * parma ： fMatch ：匹配回调函数 不可为NULL  1-匹配成功 0-匹配失败
 * parma ： pEntry ：位置实例化数据
 * parma ： pData  ：实例化数据
 * return： 成功返回0  失败返回<0
 */
int wow_slist_insert_by_match(Slist_T* ptSlist, slist_match_func_t fMatch,void *pEntry, void *pData);


/* brief    获取链表中头部内容 
 * param ： ptSlist ：待操作的链表
 * return： 成功返回头部实例内容  失败返回NULL
 */
void* wow_slist_peek_head(Slist_T* ptSlist);

/* brief    获取链表中底部内容 
 * param ： ptSlist ：待操作的链表
 * return： 成功返回头部实例内容  失败返回NULL
 */
void* wow_slist_peek_tail(Slist_T* ptSlist);

/* brief    通过索引获取链表内容 
 * param ： ptSlist ：待操作的链表
 * parma ： nIdx    ：index标识
 * return： 成功返回头部实例内容  失败返回NULL
 */
void* wow_slist_peek_by_index(Slist_T* ptSlist,int nIdx);

/* brief    通过匹配获取链表内容 
 * param ： ptSlist ：待操作的链表
 * parma ： fMatch  ：匹配回调函数 匹配回调函数 不可为NULL  1-匹配成功 0-匹配失败
 * parma ： pArg    ：匹配内容  不可为NULL
 * return： 成功返回头部实例内容  失败返回NULL
 */
void* wow_slist_peek_by_match(Slist_T* ptSlist,slist_match_func_t fMatch,const void *pArg);


/* brief    移除链表中头部实例
 * param ： ptSlist    ：待操作链表
 * return： 成功返回0  失败返回<0
 */
int wow_slist_remove_head(Slist_T* ptSlist);

/* brief    移除链表中尾部实例
 * param ： ptSlist    ：待操作链表
 * return： 成功返回0  失败返回<0
 */
int wow_slist_remove_tail(Slist_T* ptSlist);

/* brief    匹配实例化内容进行删除
 * param ： ptSlist ：待操作链表
 * parma ： pData   ：实例化数据
 * return： 成功返回0  失败返回<0
 */
int wow_slist_remove(Slist_T* ptSlist,void* pData);

/* brief    移除链表中第index个实例
 * param ： ptSlist：待操作链表
 * parma ： nIdx   ：index标识
 * return： 成功返回0	失败返回<0
 */
int wow_slist_remove_by_index(Slist_T* ptSlist, int nIdx);

/* brief   当链表中存在data匹配内容时移除此实例内容
 * param ： ptSlist  ：待操作链表
 * parma ： fMatch   ：匹配回调函数 匹配回调函数 不可为NULL  1-匹配成功 0-匹配失败
 * parma ： pArg     ：需要移除的实例内容 不可为NULL
 * return： 成功返回0  失败返回<0
 */
int wow_slist_remove_by_match(Slist_T* ptSlist, slist_match_func_t fMatch,void *pArg);

/* brief    替换头部数据内容
 * param ： ptSlist   ：待操作单项链表
 * parma ： pNdata    ：新内容		
 * return： 成功返回0  失败返回<0
 */
int wow_slist_replace_head(Slist_T* ptSlist,void *pNdata);

/* brief    替换尾部部数据内容
 * param ： ptSlist  ：待操作单项链表
 * parma ： pNdata   ：新内容		
 * return： 成功返回0  失败返回<0
 */
int wow_slist_replace_tail(Slist_T* ptSlist,void *pNdata);


/* brief    通过索引替换链表数据内容
 * param ： ptSlist：待操作的单项链表
 * parma ： nIdx   ：index标识
 * parma ： pNdata ：新内容		
 * return： 成功返回0  失败返回<0
 */
int wow_slist_replace_by_index(Slist_T* ptSlist,int nIdx,void *pNdata);


/* brief    通过查找替换链表内容
 * param ： ptSlist    ：待操作单项链表
 * parma ： fMatch     ：匹配回调函数  1-匹配成功 0-匹配失败
 * parma ： pMatchData ：被替换内容  
 * parma ： pNewData   ：新内容				
 * return： 成功返回0  失败返回<0
 */
int wow_slist_replace_by_match(Slist_T* ptSlist, slist_match_func_t fMatch,
								void *pMatchData,void *pNewData);

/* brief   获取链表中的实例内容
 * param ： ptSlist	 ：待操作链表
 * parma ： fForeach ：匹配回调函数 不可为NULL 
 * parma ： pArg     ：需要操作内容 可为NULL
 * return： 成功返回0  失败返回<0
 */
int wow_slist_foreach(Slist_T* ptSlist, slist_foreach_func_t fForeach,void *pArg);


/* brief    移除链表内的全部实例
 * param ： ptSlist    ：待操作链表
 * parma ： fDestroy   ：销毁回调函数 可为NULL
 * return： 成功返回0  失败返回<0
 */
int wow_slist_clear(Slist_T* ptSlist,  slist_destroy_func_t fDestroy);

/* brief    获取链表实例个数
 * param ： ptSlist    ：待操作链表
 * return： 成功返回链表实例个数 失败返回<0
 */
int wow_slist_size(Slist_T* ptSlist);


#ifdef __cplusplus
}
#endif

#endif


