#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "system/wow_time.h"
#include "system/wow_timer.h"

#include "greatest/greatest.h"
#include "wow_test_config.h"

#define MOD_TAG "[timer]"

static void CallBack_1second(void* param)
{
	printf(MOD_TAG"CallBack_1second----(%s)\n",wow_get_time_stamp());
}

static void CallBack_2second(void* param)
{
	printf(MOD_TAG"CallBack_2second----(%s)\n",wow_get_time_stamp());
}
static void CallBack_3second(void* param)
{
	printf(MOD_TAG"CallBack_3second----(%s)\n",wow_get_time_stamp());
}

static void CallBack_3second_block(void* param)
{
	printf(MOD_TAG"CallBack_2second----(%s)\n",wow_get_time_stamp());
	sleep(2);
}
static void CallBack_4second(void* param)
{
	Timer_T* timer = (Timer_T*)param;

	printf(MOD_TAG"CallBack_4second----(%s)\n",wow_get_time_stamp());
	wow_timer_stop(timer,0);
}
#if SUIT_RUN_ERROR_TEST
TEST test_timer_error(void)
{
	int ret = 0;
	
	printf(MOD_TAG"suit_timer---test_timer_error\n");
	Timer_T* timer_01 = wow_timer_create();
	GREATEST_ASSERT(timer_01);
	Timer_T* timer_02 = wow_timer_create();
	GREATEST_ASSERT(timer_02);
	Timer_T* timer_03 = wow_timer_create();
	GREATEST_ASSERT(timer_03);

	wow_timer_destroy(&timer_01);
	wow_timer_destroy(&timer_02);
	wow_timer_destroy(&timer_03);
	
	timer_01 = wow_timer_create();
	
	ret = wow_timer_add(timer_01,0,NULL,NULL,0);
    GREATEST_ASSERT(ret < 0);
	
	wow_timer_destroy(&timer_01);
	timer_01 = NULL;
	
	ret = wow_timer_add(timer_01,0,NULL,NULL,0);
    GREATEST_ASSERT(ret < 0);

	PASS();
}
#endif

TEST test_timer_fun1(void)
{
	printf(MOD_TAG"suit_timer---test_timer_fun1\n");

	int ret = 0;

	Timer_T* timer_01 = wow_timer_create();
	GREATEST_ASSERT(timer_01);

	ret = wow_timer_add(timer_01,0,CallBack_1second,NULL,1);
	GREATEST_ASSERT(ret == 0);
	
	ret = wow_timer_add(timer_01,1,CallBack_2second,NULL,2);
	GREATEST_ASSERT(ret == 0);
	
	ret = wow_timer_add(timer_01,2,CallBack_3second,NULL,3);
	GREATEST_ASSERT(ret == 0);
	

	ret = wow_timer_start(timer_01,0);
	GREATEST_ASSERT(ret == 0);
	ret = wow_timer_start(timer_01,1);
	GREATEST_ASSERT(ret == 0);
	ret = wow_timer_start(timer_01,2);
	GREATEST_ASSERT(ret == 0);
	
	sleep(5);
	ret = wow_timer_stop(timer_01,1);
	GREATEST_ASSERT(ret == 0);

	sleep(5);
	ret = wow_timer_stop(timer_01,2);
	GREATEST_ASSERT(ret == 0);

	sleep(5);
	ret = wow_timer_stop(timer_01,0);
	GREATEST_ASSERT(ret == 0);

	sleep(5);
	ret = wow_timer_start(timer_01,2);
	GREATEST_ASSERT(ret == 0);
	sleep(5);
	
	wow_timer_destroy(&timer_01);
	
	PASS();
}

/*此测试单元为说明 遇到单定时器阻塞时会影响其他定时器*/
TEST test_timer_fun2(void)
{
	printf(MOD_TAG"suit_timer---test_timer_fun2\n");

	int ret = 0;

	Timer_T* timer_01 = wow_timer_create();
	GREATEST_ASSERT(timer_01);

	ret = wow_timer_add(timer_01,0,CallBack_1second,NULL,1);
	GREATEST_ASSERT(ret == 0);
	
	ret = wow_timer_add(timer_01,1,CallBack_2second,NULL,2);
	GREATEST_ASSERT(ret == 0);
	
	ret = wow_timer_add(timer_01,2,CallBack_3second_block,NULL,3);
	GREATEST_ASSERT(ret == 0);
	

	ret = wow_timer_start(timer_01,0);
	GREATEST_ASSERT(ret == 0);
	ret = wow_timer_start(timer_01,1);
	GREATEST_ASSERT(ret == 0);
	ret = wow_timer_start(timer_01,2);
	GREATEST_ASSERT(ret == 0);
	
	sleep(5);
	ret = wow_timer_stop(timer_01,1);
	GREATEST_ASSERT(ret == 0);

	sleep(5);
	ret = wow_timer_stop(timer_01,2);
	GREATEST_ASSERT(ret == 0);

	sleep(5);
	ret = wow_timer_stop(timer_01,0);
	GREATEST_ASSERT(ret == 0);

	sleep(5);
	ret = wow_timer_start(timer_01,2);
	GREATEST_ASSERT(ret == 0);
	
	wow_timer_destroy(&timer_01);
	
	PASS();
}

/*此测试单元为fun2的解决方案*/
TEST test_timer_fun3(void)
{
	printf(MOD_TAG"suit_timer---test_timer_fun3\n");

	int ret = 0;

	Timer_T* timer_01 = wow_timer_create();
	GREATEST_ASSERT(timer_01);
	
	Timer_T* timer_02 = wow_timer_create();
	GREATEST_ASSERT(timer_01);

	ret = wow_timer_add(timer_01,0,CallBack_1second,NULL,1);
	GREATEST_ASSERT(ret == 0);
	
	ret = wow_timer_add(timer_01,1,CallBack_2second,NULL,2);
	GREATEST_ASSERT(ret == 0);
	
	ret = wow_timer_add(timer_02,2,CallBack_3second_block,NULL,2);
	GREATEST_ASSERT(ret == 0);
	

	ret = wow_timer_start(timer_01,0);
	GREATEST_ASSERT(ret == 0);
	ret = wow_timer_start(timer_01,1);
	GREATEST_ASSERT(ret == 0);
	ret = wow_timer_start(timer_02,2);
	GREATEST_ASSERT(ret == 0);
	
	sleep(5);
	ret = wow_timer_stop(timer_01,1);
	GREATEST_ASSERT(ret == 0);

	sleep(5);
	ret = wow_timer_stop(timer_02,2);
	GREATEST_ASSERT(ret == 0);

	sleep(5);
	ret = wow_timer_stop(timer_01,0);
	GREATEST_ASSERT(ret == 0);

	sleep(5);
	ret = wow_timer_start(timer_02,2);
	GREATEST_ASSERT(ret == 0);

	
	wow_timer_destroy(&timer_01);
	wow_timer_destroy(&timer_02);
	
	PASS();
}

TEST test_timer_fun4(void)
{
	printf(MOD_TAG"suit_timer---test_timer_fun4\n");

	int ret = 0;

	Timer_T* timer_01 = wow_timer_create();
	GREATEST_ASSERT(timer_01);

	ret = wow_timer_add(timer_01,0,CallBack_1second,NULL,1);
	GREATEST_ASSERT(ret == 0);
	ret = wow_timer_start(timer_01,0);
	GREATEST_ASSERT(ret == 0);
	sleep(5);
	ret = wow_timer_stop(timer_01,0);
	GREATEST_ASSERT(ret == 0);
	ret = wow_timer_remove(timer_01,0);
	GREATEST_ASSERT(ret == 0);

	ret = wow_timer_add(timer_01,0,CallBack_2second,NULL,2);
	GREATEST_ASSERT(ret == 0);
	ret = wow_timer_start(timer_01,0);
	GREATEST_ASSERT(ret == 0);
	sleep(5);
	ret = wow_timer_stop(timer_01,0);
	GREATEST_ASSERT(ret == 0);
	ret = wow_timer_remove(timer_01,0);
	GREATEST_ASSERT(ret == 0);

	ret = wow_timer_add(timer_01,0,CallBack_3second,NULL,3);
	GREATEST_ASSERT(ret == 0);
	ret = wow_timer_start(timer_01,0);
	GREATEST_ASSERT(ret == 0);
	sleep(5);
	ret = wow_timer_stop(timer_01,0);
	GREATEST_ASSERT(ret == 0);
	ret = wow_timer_remove(timer_01,0);
	GREATEST_ASSERT(ret == 0);

	ret = wow_timer_add(timer_01,0,CallBack_4second,timer_01,4);
	GREATEST_ASSERT(ret == 0);
	ret = wow_timer_start(timer_01,0);
	GREATEST_ASSERT(ret == 0);
	sleep(10);

	wow_timer_destroy(&timer_01);
	
	PASS();
}

SUITE(suit_timer)
{
#if SUIT_RUN_ERROR_TEST
	RUN_TEST(test_timer_error);
#endif
    //RUN_TEST(test_timer_fun1);
	//RUN_TEST(test_timer_fun2);
	//RUN_TEST(test_timer_fun3);
	RUN_TEST(test_timer_fun4);
}





