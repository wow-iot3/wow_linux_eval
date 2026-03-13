#ifndef __WOW_BASE_SYSTEM_WOW_TIMER_H_
#define __WOW_BASE_SYSTEM_WOW_TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 *注：为更好的节省线程资源，此框架采用线程运行管理，通过ID来管理添加的各定时器；
 *    此框架架构创建一定时器即为创建一个线程来运作，建议只创建一个定时器间，然后通add来实现;
 *    如某定时器回调函数为阻塞函数或者大于最小定时器的间隔时间时，将此定时器单独创建；
 */
typedef struct timer_t Timer_T;
typedef void (*timer_tineout_callback_t)(void*);

/*brief    创建定时器管理器
 *return： 成功返回定时器操作符 失败返回NULL
 */
Timer_T* wow_timer_create(void);

/*brief    销毁定时器管理器
 *param ： pptTimer:定时器管理器操作指针
 *return： 无
 */
void wow_timer_destroy(Timer_T** pptTimer);

/*brief    向定时器管理器添加一定时器
 *param ： ptTimer:定时器管理器操作指针
 *param ： u32Tid:待添加定时器ID
 *param ： fCallbcack:待添加定时器回调函数
 *param ： pArg:待添加定时器私有变量
 *param ： dInterval:待添加定时器超时时间(秒)
 *return： 成功返回0 失败返回<0
 */
int wow_timer_add(Timer_T* ptTimer,uint32_t u32Tid,timer_tineout_callback_t fCallbcack, void* pArg,double dInterval);

/*brief    启动定时器

 *param ： ptTimer:定时器管理器操作指针
 *param ： u32Tid:待启动定时器ID
 *return： 成功返回0 失败返回<0
 */
int wow_timer_start(Timer_T* ptTimer,uint32_t u32Tid);

/*brief    关闭定时器
 *param ： ptTimer:定时器管理器操作指针
 *param ： u32Tid:待关闭定时器ID
 *return： 成功返回0 失败返回<0
 */
int wow_timer_stop(Timer_T* ptTimer,uint32_t u32Tid);

/*brief    移除定时器
 *param ： ptTimer:定时器管理器操作指针
 *param ： u32Tid:待移除定时器ID
 *return： 成功返回0 失败返回<0
 */
int wow_timer_remove(Timer_T* ptTimer,uint32_t u32Tid);


#ifdef __cplusplus
}
#endif

#endif

