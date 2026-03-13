#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "greatest/greatest.h"


#include "system/wow_time.h"

#define MOD_TAG "[time]"

TEST test_time_fun1(void)
{
	printf(MOD_TAG"suit_time----test_time_fun1\n");

	
	int64_t msec1 = wow_time_get_msec();
	wow_time_sleep_msec(100);
	int64_t msec2 = wow_time_get_msec();
	printf(MOD_TAG"1-1-msec1:%ld msec2:%ld\n",msec1,msec2);
	
    PASS();
}


TEST test_time_fun2(void)
{
	printf(MOD_TAG"suit_time----test_time_fun1\n");
	
	char buff[20] = {0};
	int64_t msec1 = wow_time_get_msec();
	wow_time_msec_to_stamp(msec1,buff);
	printf(MOD_TAG"2-2-date1:%s date2:%s\n",buff,wow_get_time_stamp());

	
	PASS();
}

TEST test_time_fun3(void)
{
	printf(MOD_TAG"suit_time----test_time_fun3\n");

	TimeInfo_T info;

	int ret = wow_time_get_info(&info);
	GREATEST_ASSERT(ret == 0);
	printf(MOD_TAG"3-1-date:%04d-%02d-%02d %02d:%02d:%02d::%d\n",
				info.year,info.mon,info.day,info.hour,info.min,info.sec,info.msec);

	info.year--;
	ret = wow_time_set_info(&info);
	GREATEST_ASSERT(ret == 0);

	printf(MOD_TAG"3-2-date:%s\n",wow_get_time_stamp());

	info.year++;
	ret = wow_time_set_info(&info);
	GREATEST_ASSERT(ret == 0);

	printf(MOD_TAG"3-3-date:%s\n",wow_get_time_stamp());

	PASS();
}

TEST test_time_fun4(void)
{
	printf(MOD_TAG"suit_time----test_time_fun4\n");

	CP56Time2a_T info;

	int ret = wow_time_get_cp56(&info);
	GREATEST_ASSERT(ret == 0);
	printf(MOD_TAG"3-1-date:%04d-%02d-%02d %02d:%02d::%d\n",
				info.year,info.mon,info.mon_day,info.hour,info.min,info.milli_sec_h*256+info.milli_sec_l);

	info.year--;
	ret = wow_time_set_cp56(&info);
	GREATEST_ASSERT(ret == 0);

	printf(MOD_TAG"3-2-date:%s\n",wow_get_time_stamp());

	info.year++;
	ret = wow_time_set_cp56(&info);
	GREATEST_ASSERT(ret == 0);

	printf(MOD_TAG"3-3-date:%s\n",wow_get_time_stamp());

	PASS();
}

TEST test_time_fun5(void)
{
	printf(MOD_TAG"suit_time----test_time_fun5\n");

	int ret = 0;
	int64_t msec = 0;
	TimeInfo_T tinfo;
	CP56Time2a_T cinfo;
	uint64_t back;

	msec = wow_time_get_msec();
	
	ret =  wow_time_msec_to_rtc(msec,&tinfo);
	GREATEST_ASSERT(ret == 0);
	printf(MOD_TAG"4-1-date:%04d-%02d-%02d %02d:%02d:%02d::%d\n",
				tinfo.year,tinfo.mon,tinfo.day,tinfo.hour,tinfo.min,tinfo.sec,tinfo.msec);

	ret =  wow_time_msec_to_cp56(msec,&cinfo);
	GREATEST_ASSERT(ret == 0);
	printf(MOD_TAG"4-2-date:%04d-%02d-%02d %02d:%02d::%d\n",
				cinfo.year,cinfo.mon,cinfo.mon_day,cinfo.hour,cinfo.min,cinfo.milli_sec_h*256+cinfo.milli_sec_l);

	ret = wow_time_rtc_to_msec(&tinfo,&back);
	GREATEST_ASSERT(ret == 0);
	printf("---------------msec:%ld back:%ld\n",msec,back);
	GREATEST_ASSERT(back == msec);

	ret = wow_time_cp56_to_msec(&cinfo,&back);
	GREATEST_ASSERT(ret == 0);
	printf("---------------msec:%ld back:%ld\n",msec,back);
	GREATEST_ASSERT(back == msec);

	PASS();
}

SUITE(suit_time)
{
    RUN_TEST(test_time_fun1);
    RUN_TEST(test_time_fun2);
	RUN_TEST(test_time_fun3);
	RUN_TEST(test_time_fun4);
	RUN_TEST(test_time_fun5);
}


