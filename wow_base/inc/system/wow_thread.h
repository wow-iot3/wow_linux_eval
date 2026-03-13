#ifndef __WOW_BASE_SYSTEM_WOW_THREAD_H_
#define __WOW_BASE_SYSTEM_WOW_THREAD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

typedef pthread_t pthread_idx_t;
typedef void* (*thread_callback_func_t)(void* priv);

/* brief :  创建普通线程
 * param :  pcName: 线程名称
 * param :  fCallback: 线程回调函数
 * param :  pArg: 线程回调函数私有变量
 * return:  成功返回线程操作符 失败返回NULL
 */
pthread_idx_t wow_thread_create(const char* pcName,thread_callback_func_t fCallback,void* pArg);

/* brief :  创建实时线程---线程调度使用（root用户）
 * param :  pcName: 线程名称
 * param :  fCallback: 线程回调函数
 * param :  pArg: 线程回调函数私有变量
 * param :  priority: 线程优先级 （1(低)~99(高)）
 * return:  成功返回线程操作符 失败返回NULL
 */
pthread_idx_t wow_thread_create_priority(const char* pcName,thread_callback_func_t fCallback,void* pArg,int nPriority);

/* brief :  阻塞的方式等待thread指定的线程结束
 * param :  nThread: 线程操作符
 * return:  成功返回0 失败返回<0
 */
int wow_thread_join(pthread_idx_t Thread);

/* brief :  非阻塞的方式等待thread指定的线程结束
 * param :  nThread: 线程操作符
 * return:  成功返回0 失败返回<0
 */
int wow_thread_detach(pthread_idx_t thread);

/* brief :  被动退出
 * param :  nThread: 线程操作符
 * return:  成功返回0 失败返回<0
 */
int wow_thread_cancel(pthread_idx_t nThread);

/* brief :  绑定线程cpu
 * param :  nThread: 线程操作符
 * param :  nCpuId: CPU标识
 * return:  成功返回0 失败返回<0
 */
int wow_thread_set_affinity(pthread_idx_t nThread,int nCpuId);

/* brief :  获取线程绑定cpu
 * param :  nThread: 线程操作符
 * return:  成功返回绑定标识 失败返回<0
 * 注：bit0标识为1 代表绑定CPU1 bit7标识为1 代表绑定cpu8
 */
int wow_thread_get_affinity(pthread_idx_t nThread);

#ifdef __cplusplus
}
#endif

#endif
