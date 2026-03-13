#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#define __USE_GNU
#include <sched.h>
#define _GNU_SOURCE 
#include <pthread.h>

#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"
#include "prefix/wow_cpu.h"

#include "system/wow_thread.h"

/* brief :  创建普通线程
 * param :  pcName: 线程名称
 * param :  fCallback: 线程回调函数
 * param :  pArg: 线程回调函数私有变量
 * return:  成功返回线程操作符 失败返回NULL
 */
__EX_API__ pthread_idx_t wow_thread_create(const char* pcName,thread_callback_func_t fCallback,void* pArg)
{
	return wow_thread_create_priority(pcName,fCallback,pArg,0);
}

/* brief :  创建实时线程---线程调度使用
 * param :  pcName: 线程名称
 * param :  fCallback: 线程回调函数
 * param :  pArg: 线程回调函数私有变量
 * param :  priority: 线程优先级 （1(低)~99(高)）
 * return:  成功返回线程操作符 失败返回NULL
 */
__EX_API__ pthread_idx_t wow_thread_create_priority(const char* pcName,thread_callback_func_t fCallback,void* pArg,int nPriority)
{
	int ret = -1;

	CHECK_RET_VAL_P(fCallback, -PARAM_INPUT_FUNC_IS_NULL, "param input func invalid!\n");
	
	pthread_idx_t pid;
	if ( (nPriority >= 1) && (nPriority <= 100) ){
		CHECK_RET_VAL_P(getuid() == 0, -SYSTEM_GETUID_FAILED,"user is not root!\n");
		
		pthread_attr_t	attr;
		struct sched_param sp;
	
		pthread_attr_init(&attr);
		
		memset(&sp, 0, sizeof(struct sched_param));
		sp.sched_priority = nPriority;
		pthread_attr_setschedpolicy(&attr, SCHED_RR); //SCHED_FIFO
		pthread_attr_setschedparam(&attr, &sp);
		//pthread_attr_setstacksize(&attr , 0x10000 );
		// 设置线程属性：不要继承 main 线程的调度策略和优先级。
        pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

		ret = pthread_create(&pid, &attr, fCallback, pArg); 
		
		pthread_attr_destroy(&attr);
	}else{
		ret = pthread_create(&pid, NULL, fCallback, pArg);
	}
	CHECK_RET_VAL_P(ret == 0 && pid!= 0,-SYSTEM_THREAD_CREATE_FAILED,"pthread create failed!\n");

	if(pcName){
		pthread_setname_np(pid, pcName);
	}
	
	return pid;
}


/* brief :  阻塞的方式等待thread指定的线程结束
 * param :  nThread: 线程操作符
 * return:  成功返回0 失败返回-1
 */
__EX_API__ int wow_thread_join(pthread_idx_t nThread)
{
	pthread_join(nThread, NULL);

	return 0;
}


/* brief :  非阻塞的方式等待thread指定的线程结束
 * param :  nThread: 线程操作符
 * return:  成功返回0 失败返回<0
 */
__EX_API__ int wow_thread_detach(pthread_idx_t nThread)
{
	pthread_detach(nThread);
	return 0;
}

/* brief :  被动退出----需调用pthread_testcancel提供契机处理
 * param :  nThread: 线程操作符
 * return:  成功返回0 失败返回<0
 */
__EX_API__ int wow_thread_cancel(pthread_idx_t nThread)
{
	//判断线程是否存在
	int ret = pthread_tryjoin_np(nThread, NULL);
    if (ret != 0 && EBUSY == ret) {
        pthread_cancel(nThread);
    }


	return 0;
}



/* brief :  绑定线程cpu
 * param :  nThread: 线程操作符
 * param :  nCpuId: CPU标识
 * return:  成功返回0 失败返回<0
 */
__EX_API__ int wow_thread_set_affinity(pthread_idx_t nThread,int nCpuId)
{
	int ret = -1;

	CHECK_RET_VAL_P(nCpuId < wow_cpu_count(),-PARAM_INPUT_ARG_INVALID,"param input  invalid!\n");

	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(nCpuId, &mask);

	/* 设置cpu 亲和性(affinity)*/
	ret = pthread_setaffinity_np(nThread, sizeof(mask), &mask);
	CHECK_RET_VAL_P(ret >= 0,-SYSTEM_THREAD_SET_FAILED,"set thread affinity failed\n");
	
	return 0;
}



/* brief :  获取线程绑定cpu
 * param :  nThread: 线程操作符
 * return:  成功返回绑定标识 失败返回<0
 * 注：bit0标识为1 代表绑定CPU1 bit7标识为1 代表绑定cpu8
 */
__EX_API__ int wow_thread_get_affinity(pthread_idx_t nThread)
{
	int i  = 0;
	size_t cpuset = 0;

	/* 查看cpu 亲和性(affinity)*/
	cpu_set_t get;
	CPU_ZERO(&get);
	int ret = pthread_getaffinity_np(nThread, sizeof(get), &get);
	CHECK_RET_VAL_P(ret >= 0,-SYSTEM_THREAD_GET_FAILED,"get thread affinity failed\n");

	/* 查看当前线程所运行的所有cpu*/
	for (i = 0; i < wow_cpu_count(); i++) {
		if (CPU_ISSET(i, &get)){
			cpuset |= (1 << i);
		}
	}	
	
	return cpuset;
}

