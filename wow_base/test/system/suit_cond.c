#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "greatest/greatest.h"


#include "prefix/wow_check.h"
#include "prefix/wow_common.h"

#include "system/wow_cond.h"
#include "system/wow_thread.h"
#include "system/wow_time.h"
#include "file/wow_hfile.h"

#define MOD_TAG "[cond]"


typedef struct{
	mutex_lock_t run_lock;
	mutex_cond_t run_cond;
	mutex_lock_t exit_lock;
	mutex_cond_t exit_cond;
	pthread_idx_t thread;
	int 	  	  flag;
}MutexTestInfo_T;

static void* mutex_cond_wait1_thread(void *arg)
{
	int ret = 0;
	MutexTestInfo_T* info = (MutexTestInfo_T*)arg;
	
	while(info->flag)
	{
		wow_mutex_lock(&info->run_lock);
		ret = wow_mutex_cond_wait(&info->run_lock,&info->run_cond,1500);
		wow_mutex_unlock(&info->run_lock);
		CHECK_RET_CONTINUE_P(ret == 0,"mutex_cond_111------timeout!\n");
		printf(MOD_TAG"mutex_cond_111---%s!\n",wow_get_time_stamp());
	}

	wow_mutex_lock(&info->exit_lock);
	wow_mutex_cond_signal(&info->exit_cond);
	wow_mutex_unlock(&info->exit_lock);

	return NULL;
}

static void* mutex_cond_wait2_thread(void *arg)
{
	int ret = 0;
	MutexTestInfo_T* info = (MutexTestInfo_T*)arg;
	
	while(info->flag)
	{
		wow_mutex_lock(&info->run_lock);
		ret = wow_mutex_cond_wait(&info->run_lock,&info->run_cond,-1);
		wow_mutex_unlock(&info->run_lock);
		CHECK_RET_CONTINUE_P(ret == 0,"mutex_cond_222------timeout!\n");
		printf(MOD_TAG"mutex_cond_222---%s!\n",wow_get_time_stamp());
	}

	wow_mutex_lock(&info->exit_lock);
	wow_mutex_cond_signal(&info->exit_cond);
	wow_mutex_unlock(&info->exit_lock);
	
	return NULL;
}

TEST test_mutex_cond_fun1(void)
{
	printf(MOD_TAG"suit_cond----test_mutex_cond_fun1\n");

	int i = 0;
	MutexTestInfo_T *info  = CALLOC(1, MutexTestInfo_T);
	GREATEST_ASSERT(info);

	wow_mutex_lock_init(&info->run_lock);
	wow_mutex_cond_init(&info->run_cond);
	info->flag = 1;
	info->thread = wow_thread_create(MOD_TAG,mutex_cond_wait1_thread,info);
	
	for(i = 0; i < 5; i++){
	   sleep(1);
	   wow_mutex_lock(&info->run_lock);
	   wow_mutex_cond_signal(&info->run_cond);
	   wow_mutex_unlock(&info->run_lock);
	}
	
	sleep(1);
	wow_mutex_lock(&info->run_lock);
	wow_mutex_cond_signal_all(&info->run_cond);
	wow_mutex_unlock(&info->run_lock);
	
	//安全退出线程
	wow_mutex_lock_init(&info->exit_lock);
	wow_mutex_cond_init(&info->exit_cond);
	wow_mutex_lock(&info->exit_lock);
	
	info->flag  = 0;
	///<优化退出 使线程中wait退出
	wow_mutex_cond_exit(&info->run_cond);
	
	wow_mutex_cond_wait(&info->exit_lock,&info->exit_cond,-1);
	wow_mutex_unlock(&info->exit_lock);

	wow_mutex_lock_exit(&info->run_lock);
	
	wow_mutex_lock_exit(&info->exit_lock);
	wow_mutex_cond_exit(&info->exit_cond);

	FREE(info);
	PASS();
}


TEST test_mutex_cond_fun2(void)
{
	printf(MOD_TAG"suit_cond----test_mutex_cond_fun2\n");

	int i = 0;
	MutexTestInfo_T *info  = CALLOC(1, MutexTestInfo_T);
	GREATEST_ASSERT(info);
	
	wow_mutex_lock_init(&info->run_lock);
	wow_mutex_cond_init(&info->run_cond);
	info->flag = 1;
	info->thread = wow_thread_create(MOD_TAG,mutex_cond_wait2_thread,info);
	
	for(i = 0; i < 5; i++){
	   sleep(1);
	   wow_mutex_lock(&info->run_lock);
	   wow_mutex_cond_signal(&info->run_cond);
	   wow_mutex_unlock(&info->run_lock);
	}

	sleep(1);
	wow_mutex_lock(&info->run_lock);
	wow_mutex_cond_signal_all(&info->run_cond);
	wow_mutex_unlock(&info->run_lock);
	
	//安全退出线程
	wow_mutex_lock_init(&info->exit_lock);
	wow_mutex_cond_init(&info->exit_cond);
	wow_mutex_lock(&info->exit_lock);
	
	info->flag  = 0;
	///<优化退出 使线程中wait退出
	wow_mutex_cond_exit(&info->run_cond);
	
	wow_mutex_cond_wait(&info->exit_lock,&info->exit_cond,-1);
	wow_mutex_unlock(&info->exit_lock);

	wow_mutex_lock_exit(&info->run_lock);
	
	wow_mutex_lock_exit(&info->exit_lock);
	wow_mutex_cond_exit(&info->exit_cond);

	FREE(info);
	PASS();
}

/****************************************sem 测试*****************************************/
typedef struct{
	sem_cond_t   run_cond;
	mutex_lock_t exit_lock;
	mutex_cond_t exit_cond;
	pthread_idx_t thread;
	int 	  	  flag;
}SemTestInfo_T;

static void* sem_cond_wait1_thread(void *arg)
{
	int ret = 0;
	
	SemTestInfo_T* info = (SemTestInfo_T*)arg;
	while(info->flag)
	{
		ret = wow_sem_cond_wait(&info->run_cond,1000);
		printf(MOD_TAG"sem_cond_111---(%d)---%s!\n",ret,wow_get_time_stamp());
		
		ret = wow_sem_cond_value(&info->run_cond);
		printf(MOD_TAG"sem_val_111---(%d)---%s!\n",ret,wow_get_time_stamp());
	}
	
	wow_mutex_lock(&info->exit_lock);
	wow_mutex_cond_signal(&info->exit_cond);
	wow_mutex_unlock(&info->exit_lock);

	return NULL;
}

static void* sem_cond_wait2_thread(void *arg)
{
	int ret = 0;
	
	SemTestInfo_T* info = (SemTestInfo_T*)arg;
	while(info->flag)
	{
		ret = wow_sem_cond_wait(&info->run_cond,1000);
		printf(MOD_TAG"sem_cond_222---(%d)---%s!\n",ret,wow_get_time_stamp());
		
		ret = wow_sem_cond_value(&info->run_cond);
		printf(MOD_TAG"sem_val_222---(%d)---%s!\n",ret,wow_get_time_stamp());
	}
	
	wow_mutex_lock(&info->exit_lock);
	wow_mutex_cond_signal(&info->exit_cond);
	wow_mutex_unlock(&info->exit_lock);
	
	return NULL;
}

TEST test_sem_cond_fun1(void)
{
	printf(MOD_TAG"suit_cond----test_sem_cond_fun1\n");

	int i = 0;
	SemTestInfo_T* info = CALLOC(1,SemTestInfo_T);
	GREATEST_ASSERT(info);
	wow_sem_cond_init(&info->run_cond);
	
	info->flag = 1;
	info->thread = wow_thread_create(MOD_TAG,sem_cond_wait1_thread,info);
	
	for(i = 0; i < 3; i++){
		usleep(800*1000);
		wow_sem_cond_signal(&info->run_cond);
	}
	
	//安全退出线程
	wow_mutex_lock_init(&info->exit_lock);
	wow_mutex_cond_init(&info->exit_cond);
	wow_mutex_lock(&info->exit_lock);
	
	info->flag  = 0;
	///<优化退出 使线程中wait退出
	wow_sem_cond_exit(&info->run_cond);

	wow_mutex_cond_wait(&info->exit_lock,&info->exit_cond,-1);
	wow_mutex_unlock(&info->exit_lock);
	
	wow_mutex_lock_exit(&info->exit_lock);
	wow_mutex_cond_exit(&info->exit_cond);

	FREE(info);
	
	PASS();
}


TEST test_sem_cond_fun2(void)
{
	printf(MOD_TAG"suit_cond----test_sem_cond_fun2\n");

	int i = 0;
	SemTestInfo_T* info = CALLOC(1,SemTestInfo_T);
	GREATEST_ASSERT(info);
	wow_sem_cond_init(&info->run_cond);
	
	info->flag = 1;
	info->thread = wow_thread_create(MOD_TAG,sem_cond_wait2_thread,info);
	
	for(i = 0; i < 3; i++){
		usleep(800*1000);
		wow_sem_cond_signal(&info->run_cond);
	}
	
	//安全退出线程
	wow_mutex_lock_init(&info->exit_lock);
	wow_mutex_cond_init(&info->exit_cond);
	wow_mutex_lock(&info->exit_lock);
	
	info->flag  = 0;
	///<优化退出 使线程中wait退出
	wow_sem_cond_exit(&info->run_cond);
	
	wow_mutex_cond_wait(&info->exit_lock,&info->exit_cond,-1);
	wow_mutex_unlock(&info->exit_lock);
	
	wow_mutex_lock_exit(&info->exit_lock);
	wow_mutex_cond_exit(&info->exit_cond);

	FREE(info);
	
	PASS();
}



SUITE(suit_cond)
{
	RUN_TEST(test_mutex_cond_fun1);
	RUN_TEST(test_mutex_cond_fun2);
	RUN_TEST(test_sem_cond_fun1);
	RUN_TEST(test_sem_cond_fun2);
}


