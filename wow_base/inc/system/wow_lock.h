#ifndef __WOW_BASE_SYSTEM_WOW_LOCK_H_
#define __WOW_BASE_SYSTEM_WOW_LOCK_H_

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 注：
 * 1、不要去解锁一个未被加锁的mutex锁；
 * 2、不要一个线程中加锁而在另一个线程中解锁；
 * 3、使用mutex锁用于保护临界资源，严格按照“加锁-->写入/读取临界资源-->解锁”的流程执行，对于线程间同步的需求使用条件变量或信号量实现。
 */

/* 原子锁操作接口 */
typedef int atomic_lock_t;
int wow_atomic_trylock(atomic_lock_t *lock);
int wow_atomic_lock(atomic_lock_t *lock);
int wow_atomic_unlock(atomic_lock_t *lock);


/* 线程锁操作接口 */
typedef pthread_mutex_t mutex_lock_t;
int wow_mutex_lock_init(mutex_lock_t *lock);
int wow_mutex_lock_exit(mutex_lock_t *lock);
int wow_mutex_trylock(mutex_lock_t *lock);
int wow_mutex_lock(mutex_lock_t *lock);
int wow_mutex_unlock(mutex_lock_t *lock);


/* 信号量锁操作接口 */
typedef void* sem_lock_t;
sem_lock_t wow_sem_new(size_t key);
int wow_sem_lock(sem_lock_t sem);
int wow_sem_unlock(sem_lock_t sem);
int wow_sem_free(sem_lock_t sem);

/* 文件读写锁操作接口 */
int wow_file_read_lock(int fd);
int wow_file_write_lock(int fd);
int wow_file_unlock(int fd);

#ifdef __cplusplus
}
#endif


#endif
