#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "greatest/greatest.h"

#include "prefix/wow_cpu.h"
#include "system/wow_thread.h"

#define MOD_TAG "[thread]"


pthread_idx_t thread_01;
pthread_idx_t thread_02;
static void* thread_routine_fun1(void* priv)
{
	printf("------thread_routine_fun1_start----\n");
	while(1){
		printf("------thread_routine_fun1_fun1----\n");
		sleep(1);
	}
	printf("------thread_routine_fun1_stop----\n");
	return NULL;
}

static void* thread_routine_fun2(void* priv)
{
	printf("------thread_routine_fun2_start----\n");
	sleep(10);
	wow_thread_cancel(thread_01);
	wow_thread_cancel(thread_02);
	printf("------thread_routine_fun2_stop----\n");
	return NULL;
}



// 线程函数，
static void* thread_routine_fun3(void *args)
{
    int i = 0;
    int j = 0;
  
    int thread_index = *(int *)args;
    
    // 为了确保所有的线程都创建完毕，让线程睡眠1秒。
    sleep(1);
	
    printf("====>------thread_routine_fun3_start(%d)----\n", thread_index);

    long num = 0;
    for (i = 0; i < 10; i++)
    {
        for (j = 0; j < 5000000; j++)
        {
            // 没什么意义，纯粹是模拟 CPU 密集计算。
            float f1 = ((i+1) * 345.45) * 12.3 * 45.6 / 78.9 / ((j+1) * 4567.89);
            float f2 = (i+1) * 12.3 * 45.6 / 78.9 * (j+1);
            float f3 = f1 / f2;
        }
        
        // 打印计数信息，为了能看到某个线程正在执行
        printf("thread_index %d: num = %ld \n", thread_index, num++);
    }
    
    printf("====>------thread_routine_fun3_stop(%d)----\n", thread_index);
	return NULL;
}


TEST test_thread_common(void)
{
	printf(MOD_TAG"suit_thread----test_thread_common\n");

	thread_01 = wow_thread_create(MOD_TAG,thread_routine_fun1,NULL);
    GREATEST_ASSERT(thread_01);

	thread_02 = wow_thread_create(MOD_TAG,thread_routine_fun2,NULL);
    GREATEST_ASSERT(thread_02);

	sleep(12);
	
	PASS();
}


//本测试环境为1CPU4核测试
//虽然设计了线程优先级 CPU含4核被分配到各核执行 即设计无效
TEST test_thread_priority_01(void)
{
	int i= 0;
	int thread_num = wow_cpu_count();
	int index[4] = {1, 2, 3, 4};
	pthread_idx_t thread[4];
	
	printf(MOD_TAG"suit_thread----test_thread_priority_01\n");
	for (i = 0; i < thread_num; i++)
	{
		if (i <= 1)	  // 前2个创建实时线程
		{
			thread[i] = wow_thread_create_priority(MOD_TAG,thread_routine_fun3,(void *)&index[i],51 + i);
		}
		else 	   // 后2个创建普通线程
		{
			thread[i] = wow_thread_create(MOD_TAG,thread_routine_fun3,(void *)&index[i]);
		}
		GREATEST_ASSERT(thread[i]);
	}
	
	// 等待 4 个线程执行结束
	for (i = 0; i < thread_num; i++)
	{
		wow_thread_join(thread[i]);
	}
	PASS();
}


TEST test_thread_priority_02(void)
{
	int i= 0;
	int thread_num = wow_cpu_count();
	int index[4] = {1, 2, 3, 4};
	pthread_idx_t thread[4];
	
	printf(MOD_TAG"suit_thread----test_thread_priority_02\n");
	for (i = 0; i < thread_num; i++)
	{
		if (i <= 1)	  // 前2个创建实时线程
		{
			thread[i] = wow_thread_create_priority(MOD_TAG,thread_routine_fun3,(void *)&index[i],51 + i);
		}
		else 	   // 后2个创建普通线程
		{
			thread[i] = wow_thread_create(MOD_TAG,thread_routine_fun3,(void *)&index[i]);
		}
		GREATEST_ASSERT(thread[i]);
		//针对多核多CPU设置
		wow_thread_set_affinity(thread[i],i);
	}
	
	for (i = 0; i < thread_num; i++)
	{
		wow_thread_join(thread[i]);
	}
	PASS();
}

TEST test_thread_priority_03(void)
{
	int i= 0;
	int thread_num = wow_cpu_count();
	int index[4] = {1, 2, 3, 4};
	pthread_idx_t thread[4];
	
	printf(MOD_TAG"suit_thread----test_thread_priority_03\n");
	for (i = 0; i < thread_num; i++)
	{
		if (i <= 1)	  // 前2个创建实时线程
		{
			thread[i] = wow_thread_create_priority(MOD_TAG,thread_routine_fun3,(void *)&index[i],51 + i);
		}
		else 	   // 后2个创建普通线程
		{
			thread[i] = wow_thread_create(MOD_TAG,thread_routine_fun3,(void *)&index[i]);
		}
		GREATEST_ASSERT(thread[i]);
		//相当于单核测试
		wow_thread_set_affinity(thread[i],0);
		
	}
	
	// 等待 4 个线程执行结束
	for (i = 0; i < thread_num; i++)
	{
		wow_thread_join(thread[i]);
	}
	sleep(10);
	PASS();
}

//优先执行实时线程
TEST test_thread_priority_04(void)
{
	int i= 0;
	int thread_num = 10;
	int index[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	pthread_idx_t thread[10];
	
	printf(MOD_TAG"suit_thread----test_thread_priority_04\n");
	for (i = 0; i < thread_num; i++)
	{
		
		if (i <= 3)	  // 前4个创建实时线程
		{
			printf("AA----index[%d]:%d\n",i,index[i]);
			thread[i] = wow_thread_create_priority(MOD_TAG,thread_routine_fun3,(void *)&index[i],51 + i);
		}
		else 	   // 后6个创建普通线程
		{
			printf("BB----index[%d]:%d\n",i,index[i]);
			thread[i] = wow_thread_create(MOD_TAG,thread_routine_fun3,(void *)&index[i]);
		}
		usleep(10*1000);
		GREATEST_ASSERT(thread[i]);
		
	}

	// 等待 10 个线程执行结束
	for (i = 0; i < thread_num; i++)
	{
		wow_thread_detach(thread[i]);
	}
	
	PASS();
}


SUITE(suit_thread)
{
    RUN_TEST(test_thread_common);
	RUN_TEST(test_thread_priority_01);
	RUN_TEST(test_thread_priority_02);
	RUN_TEST(test_thread_priority_03);
	RUN_TEST(test_thread_priority_04);
}


