#ifndef __WOW_BASE_SYSTEM_WOW_COND_H_
#define __WOW_BASE_SYSTEM_WOW_COND_H_

#include <semaphore.h>
#include <pthread.h>

#include "system/wow_lock.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 线程条件操作接口 */
typedef pthread_cond_t mutex_cond_t;
int wow_mutex_cond_init(mutex_cond_t *cond);
//超时等运行错误返回-1 正常返回0 
int wow_mutex_cond_wait(mutex_lock_t *mutex, mutex_cond_t *cond, int ms);
void wow_mutex_cond_signal(mutex_cond_t *cond);
void wow_mutex_cond_signal_all(mutex_cond_t *cond);
int wow_mutex_cond_exit(mutex_cond_t *cond);

/* 信号量操作接口 */
typedef sem_t sem_cond_t;
int wow_sem_cond_init(sem_cond_t *sem);
//超时等运行错误返回-1 正常返回0 
int wow_sem_cond_wait(sem_cond_t *sem, int ms);
int wow_sem_cond_trywait(sem_cond_t *sem);
int wow_sem_cond_signal(sem_cond_t *sem);
int wow_sem_cond_value(sem_cond_t *sem);
int wow_sem_cond_exit(sem_cond_t *sem);


#ifdef __cplusplus
}
#endif

#endif