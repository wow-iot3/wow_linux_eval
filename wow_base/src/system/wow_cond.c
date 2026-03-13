#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>    
#include <sys/time.h>   
#include <fcntl.h>

#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"

#include "system/wow_cond.h"
#include "system/wow_time.h"

int wow_mutex_cond_init(mutex_cond_t *cond)
{
    CHECK_RET_VAL_P(cond,-1,"param input invalid!\n");
   
	CHECK_RET_VAL_P_A(pthread_cond_init(cond, NULL) == 0, -1,"pthread_cond_init failed!\n");
	
    return 0;
}

int wow_mutex_cond_exit(mutex_cond_t *cond)
{
    CHECK_RET_VAL_P(cond,-1,"param input invalid!\n");
	
	CHECK_RET_VAL_P(pthread_cond_destroy(cond) == 0, -1,"pthread_cond_destroy failed!\n");

	return 0;
}

int wow_mutex_cond_wait(mutex_lock_t *mutex, mutex_cond_t *cond, int ms)
{
	CHECK_RET_VAL_P(mutex && cond,-1,"param input invalid!\n");

    int ret = 0;
	
    if (ms <= 0) {
        //never return an error code
        pthread_cond_wait(cond, mutex);
	    if (ret != 0) {
	        fprintf(stderr, "pthread_cond_wait failed: %s\n", strerror(ret));
	    }
    } else {
    	struct timespec ts;
		clock_gettime(CLOCK_REALTIME,&ts);
        ts.tv_sec  += ms/1000;
        ts.tv_nsec += (ms%1000)*1000000; 
		///< ts.tv_nsec 的值是有限制的，必须小于1s（也就是值不能大于1000000000）
		if(ts.tv_nsec > 1000000000){
			ts.tv_sec  += 1;
			ts.tv_nsec -= 1000000000; 
		}
        ret = pthread_cond_timedwait(cond, mutex, &ts);
		CHECK_RET_VAL_P_A(ret == 0,-1,"pthread_cond_timedwait failed!\n");
    }
    return ret;
}

void wow_mutex_cond_signal(mutex_cond_t *cond)
{
	CHECK_RET_VOID(cond);
	  
    pthread_cond_signal(cond);
}

void wow_mutex_cond_signal_all(mutex_cond_t *cond)
{
    CHECK_RET_VOID(cond);

    pthread_cond_broadcast(cond);
}



int wow_sem_cond_init(sem_cond_t *sem)
{
	CHECK_RET_VAL_P(sem,-1,"param input invalid!\n");
	
    int pshared = 0;//0: threads, 1: processes

	CHECK_RET_VAL_P_A(sem_init(sem, pshared, 0) == 0,-1,"sem_init failed!\n");

    return 0;
}

int wow_sem_cond_exit(sem_cond_t *sem)
{
	CHECK_RET_VAL_P(sem,-1,"param input invalid!\n");

	CHECK_RET_VAL_P_A(sem_destroy(sem) == 0,-1,"sem_init failed!\n");
	
	return 0;
}
int wow_sem_cond_wait(sem_cond_t *sem, int ms)
{
	CHECK_RET_VAL_P(sem,-1,"param input invalid!\n");

    int ret = 0;
    struct timespec ts;
    if (ms < 0) {
        ret = sem_wait(sem);
		CHECK_RET_VAL_P_A(ret == 0,-1,"sem_wait failed!\n");

    } else {
		clock_gettime(CLOCK_REALTIME,&ts);
        ts.tv_sec  += ms/1000;
        ts.tv_nsec += ms%1000*1000000; 
		///< ts.tv_nsec 的值是有限制的，必须小于1s（也就是值不能大于1000000000）
		if(ts.tv_nsec > 1000000000){
			ts.tv_sec  += 1;
			ts.tv_nsec -= 1000000000; 
		}
        ret = sem_timedwait(sem, &ts);
   		CHECK_RET_VAL_P_A(ret == 0,-1,"sem_timedwait failed!\n");
    }
    return ret;
}

int wow_sem_cond_trywait(sem_cond_t *sem)
{
	CHECK_RET_VAL_P(sem,-1,"param input invalid!\n");

	CHECK_RET_VAL_P_A(sem_trywait(sem) == 0,-1,"sem_timedwait failed!\n");

    return 0;
}

int wow_sem_cond_value(sem_cond_t *sem)
{
    CHECK_RET_VAL_P(sem,-1,"param input invalid!\n");

    int value = 0;
    return (!sem_getvalue(sem, &value))? value : -1;
}

int wow_sem_cond_signal(sem_cond_t *sem)
{
	CHECK_RET_VAL_P(sem,-1,"param input invalid!\n");

	CHECK_RET_VAL_P_A(sem_post(sem) == 0,-1,"sem_post failed!\n");

    return 0;
}

