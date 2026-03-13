#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "greatest/greatest.h"

#include "system/wow_lock.h"
#include "system/wow_thread.h"
#include "system/wow_time.h"
#include "file/wow_hfile.h"

#define MOD_TAG "[lock]"

static int gi_threadFlag = 0;

/**************************************atomic_lock 测试****************************************************************/
static void* atomic_lock_thread(void *arg)
{
	atomic_lock_t* lock = (atomic_lock_t*)arg;
	int ret = 0;
	
	while(gi_threadFlag)
	{
		//等待解锁后执行
		wow_atomic_lock(lock);
		wow_atomic_unlock(lock);
		
		ret = wow_atomic_trylock(lock);
		if(ret != 0) printf(MOD_TAG"wow_atomic_trylock failed!\n");
		//等待解锁后执行
		wow_atomic_lock(lock);
		printf(MOD_TAG"mutex_111------%s!\n",wow_get_time_stamp());
		sleep(1);
	}

	return NULL;
}

static void* atmoic_unlock_thread(void *arg)
{
	atomic_lock_t* lock = (atomic_lock_t*)arg;
	while(gi_threadFlag)
	{
		wow_atomic_unlock(lock);
		printf(MOD_TAG"atomic_222------%s!\n",wow_get_time_stamp());
		sleep(3);
	}

	return NULL;
}

TEST test_lock_atomic(void)
{
	printf(MOD_TAG"suit_lock----test_lock_atomic\n");

	atomic_lock_t lock;
	
	gi_threadFlag = 1;
	
	wow_thread_create(MOD_TAG,atomic_lock_thread,&lock);
	wow_thread_create(MOD_TAG,atmoic_unlock_thread,&lock);
	
	sleep(10);
	
	gi_threadFlag = 0;


	sleep(3);
	PASS();
}

/**************************************mutex_trylock 测试****************************************************************/
mutex_lock_t gt_mutex_lock;
void* mutex_trylock_thread(void* arg) {
    int ret = wow_mutex_trylock(&gt_mutex_lock);
    if (ret == 0) {
        printf("Thread %ld acquired the lock.\n", (long)arg);
        // Do some work while holding the lock

        wow_mutex_unlock(&gt_mutex_lock);
    } else {
        printf("Thread %ld failed to acquire the lock.\n", (long)arg);
    }
    return NULL;
}

TEST test_trylock_mutex(void)
{
	printf(MOD_TAG"suit_lock----test_trylock_mutex\n");

	int i = 0;
	char name[10] = {0};
	pthread_idx_t threads[5] = {0};

 	for (i = 0; i < 5; i++) {
	   snprintf(name,9,"thread-%d",i);
       threads[i] = wow_thread_create(name,mutex_trylock_thread,(void*)(long)i);
    }

   for (i = 0; i < 5; i++) {
        wow_thread_join(threads[i]);
    }

	PASS();
}

/**************************************mutex_lock 测试****************************************************************/
static void* mutex_lock_thread(void *arg)
{
	mutex_lock_t* lock = (mutex_lock_t*)arg;
	
	while(gi_threadFlag)
	{
		wow_mutex_lock(lock);
		wow_mutex_unlock(lock);
		
		wow_mutex_lock(lock);
		printf(MOD_TAG"mutex_111------%s!\n",wow_get_time_stamp());
		sleep(1);
	}

	return NULL;
}

static void* mutex_unlock_thread(void *arg)
{
	mutex_lock_t* lock = (mutex_lock_t*)arg;
	while(gi_threadFlag)
	{
		wow_mutex_unlock(lock);
		printf(MOD_TAG"mutex_222------%s!\n",wow_get_time_stamp());
		sleep(3);
	}

	return NULL;
}

TEST test_lock_mutex(void)
{
	printf(MOD_TAG"suit_lock----test_lock_mutex\n");

	mutex_lock_t lock;	

	gi_threadFlag = 1;
	
	wow_thread_create(MOD_TAG,mutex_lock_thread,&lock);
	wow_thread_create(MOD_TAG,mutex_unlock_thread,&lock);

	sleep(10);
	gi_threadFlag = 0;
	wow_mutex_unlock(&lock);	
	sleep(3);
	
	wow_mutex_lock(&lock);
	wow_mutex_unlock(&lock);
	
	PASS();
}
/**************************************sem_lock 测试****************************************************************/
static void* sem_lock_thread(void *arg)
{
	sem_lock_t* lock = (sem_lock_t*)arg;
	
	while(gi_threadFlag)
	{
		wow_sem_lock(lock);
		wow_sem_unlock(lock);
		
		wow_sem_lock(lock);
		printf(MOD_TAG"mutex_111------%s!\n",wow_get_time_stamp());
		sleep(1);
	}
	return NULL;
}

static void* sem_unlock_thread(void *arg)
{
	sem_lock_t* lock = (sem_lock_t*)arg;
	while(gi_threadFlag)
	{
		wow_sem_unlock(lock);
		printf(MOD_TAG"mutex_222------%s!\n",wow_get_time_stamp());
		sleep(3);
	}

	return NULL;
}

TEST test_lock_sem(void)
{
	printf(MOD_TAG"suit_lock----test_lock_sem\n");

	sem_lock_t* lock = wow_sem_new(11);	

	gi_threadFlag = 1;
	pthread_idx_t thread[2];
    thread[0] = wow_thread_create(MOD_TAG,sem_lock_thread,lock);
	thread[1] = wow_thread_create(MOD_TAG,sem_unlock_thread,lock);

	sleep(10);
	gi_threadFlag = 0;
	wow_sem_unlock(lock);	
	wow_thread_cancel(thread[0]);
	wow_thread_cancel(thread[1]);
	sleep(3);
	
	wow_sem_unlock(lock);
	wow_sem_free(lock);
	
	PASS();
}

SUITE(suit_lock)
{
    RUN_TEST(test_lock_atomic);
	RUN_TEST(test_trylock_mutex);
	RUN_TEST(test_lock_mutex);
	RUN_TEST(test_lock_sem);
}

