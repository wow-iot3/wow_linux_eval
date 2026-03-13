#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "ev.h"

#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"
#include "utils/wow_type.h"

#include "list/wow_slist.h"

#include "system/wow_lock.h"
#include "system/wow_cond.h"
#include "system/wow_timer.h"
#include "system/wow_thread.h"

typedef struct{
	uint32_t 	id; 
	timer_tineout_callback_t  timer_cb;
	void* 	 priv;
	ev_timer timer; 
}__data_aligned__ TimerWatcher_T;
 
struct timer_t{
	int 	        inited;
	ev_timer        timer;
	Slist_T*	    tlist;
	pthread_idx_t	thread;
	mutex_lock_t	time_lock;
	mutex_lock_t	loop_lock;
	mutex_cond_t 	loop_cond;
	struct ev_loop 	*loop;
}__data_aligned__;

static void timer_watcher_cb(EV_P_ ev_timer *w, int revents)
{
	TimerWatcher_T *watcher = (TimerWatcher_T*)w->data;
	if(!watcher) return;
 
	if(watcher->timer_cb){
		watcher->timer_cb(watcher->priv);
	}
		
}
 
static void* timer_loop_entry(void *arg)
{
	Timer_T *timer = (Timer_T*)arg;

	CHECK_RET_VAL(timer && timer->inited == 0,NULL);
	
	timer->loop = ev_loop_new(0);
	
	ev_timer_init(&timer->timer, timer_watcher_cb,1 , 0);
	ev_timer_start(timer->loop, &timer->timer);

	timer->inited = 1;
	
	while(timer->inited){
		ev_run(timer->loop, EVRUN_NOWAIT);
		usleep(5*1000);
	}
	
	wow_mutex_lock(&timer->loop_lock);
	wow_mutex_cond_signal(&timer->loop_cond);
	wow_mutex_unlock(&timer->loop_lock);

	return NULL;
}
 
/*brief    创建定时器管理器
 *return： 成功返回定时器操作符 失败返回NULL
 */
__EX_API__ Timer_T* wow_timer_create(void)
{
	Timer_T *timer = CALLOC(1,Timer_T);
	CHECK_RET_VAL_ERRNO_P(timer,-SYSTEM_MALLOC_FAILED,"malloc Timer_T failed!\n");
	
	timer->inited = 0;
	
 	wow_mutex_lock_init(&timer->time_lock);
	wow_mutex_lock_init(&timer->loop_lock);
	wow_mutex_cond_init(&timer->loop_cond);

	timer->tlist = wow_slist_create();
	CHECK_RET_GOTO_ERRNO_P(timer->tlist, out1,-SLIST_CREATE_FAILED,"wow_slist_create failed!\n");

	timer->thread = wow_thread_create("timer",timer_loop_entry, (void*)timer);
	CHECK_RET_GOTO_ERRNO_P(timer->thread, out2,-THREAD_CREATE_FAILED,"wow_thread_create failed!\n");

		
	while(!timer->inited) sleep(1);	

	return timer;
out2:
	wow_slist_destroy(&timer->tlist,NULL);
out1:
	FREE(timer);
	return NULL;
}
 

static bool match_timer_list(const void *data, const void *match_data)
{
	TimerWatcher_T* watcher = (TimerWatcher_T*)data;
	CHECK_RET_VAL(watcher, false);
	
	return (watcher->id == p2u32(match_data))?true:false;
}

static void free_timer_list(void *data)
{
	FREE(data);
}

/*brief    向定时器管理器添加一定时器
 *param ： ptTimer:定时器管理器操作指针
 *param ： u32Tid:待添加定时器ID
 *param ： fCallbcack:待添加定时器回调函数
 *param ： pArg:待添加定时器私有变量
 *param ： dInterval:待添加定时器超时时间(秒)
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_timer_add(Timer_T* ptTimer,uint32_t u32Tid,timer_tineout_callback_t fCallbcack, void* pArg,double dInterval)
{
	
	CHECK_RET_VAL_P(ptTimer,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(fCallbcack,-PARAM_INPUT_FUNC_IS_NULL,"param input func invalid!\n");
	
	wow_mutex_lock(&ptTimer->time_lock);
	
	TimerWatcher_T* watcher = wow_slist_peek_by_match(ptTimer->tlist,match_timer_list,(const void*)u2p(u32Tid));
	if(watcher){
		ev_timer_stop(ptTimer->loop, &watcher->timer);
	}else{
		watcher = CALLOC(1,TimerWatcher_T);
		CHECK_RET_VAL_P_A(watcher, -SYSTEM_MALLOC_FAILED,"malloc TimerWatcher_T failed!\n");
		wow_slist_insert_tail(ptTimer->tlist,watcher);
	}
	watcher->timer_cb   = fCallbcack;
	watcher->priv       = pArg;
	watcher->id         = u32Tid;
	watcher->timer.data = (void*)watcher;
	
	ev_timer_init(&watcher->timer, timer_watcher_cb,dInterval, dInterval);
	
	wow_mutex_unlock(&ptTimer->time_lock); 

	return WOW_SUCCESS;
}


/*brief    启动定时器
 *param ： ptTimer:定时器管理器操作指针
 *param ： tid:待启动定时器ID
 *return： 成功返回0 失败返回<0
 */ 
__EX_API__ int wow_timer_start(Timer_T* ptTimer,uint32_t u32Tid)
{
	CHECK_RET_VAL_P(ptTimer,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");

	wow_mutex_lock(&ptTimer->time_lock);

	TimerWatcher_T* watcher = wow_slist_peek_by_match(ptTimer->tlist,match_timer_list,(const void*)u2p(u32Tid));
	if(watcher){
		ev_timer_start(ptTimer->loop, &watcher->timer);
		wow_mutex_unlock(&ptTimer->time_lock);
		return WOW_SUCCESS;
	}else{
		wow_mutex_unlock(&ptTimer->time_lock);
		return -TIMER_FOUND_FAILED;
	}
}

/*brief    关闭定时器
 *param ： ptTimer:定时器管理器操作指针
 *param ： tid:待关闭定时器ID
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_timer_stop(Timer_T* ptTimer,uint32_t u32Tid)
{
	CHECK_RET_VAL_P(ptTimer,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");

	wow_mutex_lock(&ptTimer->time_lock);
	TimerWatcher_T* watcher = wow_slist_peek_by_match(ptTimer->tlist,match_timer_list,(const void*)u2p(u32Tid));
	if(watcher){
		ev_timer_stop(ptTimer->loop, &watcher->timer);
		wow_mutex_unlock(&ptTimer->time_lock);
		return WOW_SUCCESS;
	}else{
		wow_mutex_unlock(&ptTimer->time_lock);
		return -TIMER_FOUND_FAILED;
	}
}

/*brief    移除定时器
 *param ： ptTimer:定时器管理器操作指针
 *param ： tid:待移除定时器ID
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_timer_remove(Timer_T* ptTimer,uint32_t u32Tid)
{
	CHECK_RET_VAL_P(ptTimer,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");

	wow_mutex_lock(&ptTimer->time_lock);
	TimerWatcher_T* watcher = wow_slist_peek_by_match(ptTimer->tlist,match_timer_list,(const void*)u2p(u32Tid));
	if(watcher){
		ev_timer_stop(ptTimer->loop, &watcher->timer);
		wow_slist_remove(ptTimer->tlist,watcher);
		FREE(watcher);
	}
 
	wow_mutex_unlock(&ptTimer->time_lock);
	
	return WOW_SUCCESS;
}


/*brief    销毁定时器管理器
 *param ： pptTimer:定时器管理器操作指针
 *return： 无
 */
__EX_API__ void wow_timer_destroy(Timer_T** pptTimer)
{
	CHECK_RET_VOID(pptTimer && *pptTimer);
	Timer_T* timer = (Timer_T*)*pptTimer;

	wow_mutex_lock_exit(&timer->time_lock);
	
	timer->inited = 0;
	

	if(timer->loop){
		ev_break(timer->loop,EVBREAK_ALL);
		
		wow_mutex_lock(&timer->loop_lock);
		wow_mutex_cond_wait(&timer->loop_lock,&timer->loop_cond,-1);
		wow_mutex_unlock(&timer->loop_lock);
		
		ev_loop_destroy(timer->loop);
	}
	
	wow_mutex_lock_exit(&timer->loop_lock);
	wow_mutex_cond_exit(&timer->loop_cond);

	
	wow_slist_destroy(&timer->tlist,free_timer_list);

	FREE(timer);
	*pptTimer = NULL;
}

