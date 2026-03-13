#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/times.h>

#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"

#include "system/wow_time.h"

#define SEC_TIME_ZONE       (8*60*60)  //Beijing,GMT+8， 时区差

static char gs_nowTimeStr[20] = {0};
static uint16_t gs_mon_days[2][12] = {
	{0,31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
	{0,31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335},
};

static int rtc_data_leap(uint16_t year)
{
	return (year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0);
}

/*brief    获取当前时间
 *return： 返回当前时间字符串
 */
__EX_API__ char* wow_get_time_stamp(void)
{
    struct timeval tv;
    struct tm now_tm;
	time_t now_sec;

    gettimeofday(&tv, NULL);
    now_sec = tv.tv_sec;
    localtime_r(&now_sec, &now_tm);

	strftime(gs_nowTimeStr, 20, "%Y-%m-%d %H:%M:%S", &now_tm);
	return gs_nowTimeStr;
}

/*brief    延时
 *param ： nMsec:延时时间 单位ms
 *return： 成功返回0 失败返回<0
 */
int wow_time_sleep_msec(size_t nMsec)
{
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = nMsec*1000;
    return select(0, NULL, NULL, NULL, &tv);
}


#define TIME_FORMAT "%Y%m%d%H%M%S"
/*brief    获取时间信息
 *param ： ptInfo:时间信息结构体
 *return： 成功返回0 失败返回-1
 */
int wow_time_get_info(TimeInfo_T *ptInfo)
{
	int ret = -1;
	time_t utc;
    struct timeval tv;
    struct timezone tz;
    struct tm *now;

	ret = time(&utc);
	CHECK_RET_VAL_P(ret != -1,-1,"time faild\n");
	
    now = localtime(&utc);
	CHECK_RET_VAL_P(now,-1,"localtime faild\n");

	//获取毫秒
    ret =  gettimeofday(&tv, &tz);
    CHECK_RET_VAL_P_A(ret == 0,-1,"gettimeofday faild\n");

    ptInfo->year = now->tm_year + 1900;
    ptInfo->mon = now->tm_mon + 1;
    ptInfo->day = now->tm_mday;
    ptInfo->hour = now->tm_hour;
    ptInfo->min = now->tm_min;
    ptInfo->sec = now->tm_sec;
    ptInfo->msec = tv.tv_usec/1000;

    return 0;
}

int wow_time_get_cp56(CP56Time2a_T *ptInfo)
{
	int ret = -1;
	time_t utc;
    struct timeval tv;
    struct timezone tz;
    struct tm *now;

	ret = time(&utc);
	CHECK_RET_VAL_P_A(ret != -1,-1,"time faild\n");
	
    now = localtime(&utc);
	CHECK_RET_VAL_P_A(now,-1,"localtime faild\n");

	//获取毫秒
    ret =  gettimeofday(&tv, &tz);
    CHECK_RET_VAL_P_A(ret == 0,-1,"gettimeofday faild\n");

    ptInfo->year    = now->tm_year + 1900-2000;
    ptInfo->mon     = now->tm_mon + 1;
    ptInfo->mon_day = now->tm_mday;
    ptInfo->hour    = now->tm_hour;
    ptInfo->min     = now->tm_min;
	time_t msec = now->tm_sec*1000 + tv.tv_usec/1000;
	ptInfo->milli_sec_h  = (msec >> 0x08) & 0x00FF;
	ptInfo->milli_sec_l  = msec & 0x00FF;

    return 0;
}

/*brief    设置时间
 *param ： ptInfo:时间信息结构体
 *return： 成功返回0 失败返回-1
 */
int wow_time_set_info(TimeInfo_T *ptInfo)
{
    time_t timep;
    struct tm tm;
    struct timeval tv;
    
    tm.tm_sec  = ptInfo->sec;
    tm.tm_min  = ptInfo->min;
    tm.tm_hour = ptInfo->hour;
    tm.tm_mday = ptInfo->day;
    tm.tm_mon  = ptInfo->mon - 1;     // 月份从0开始，所以需要减去1
    tm.tm_year = ptInfo->year - 1900;// 年份需要减去1900
    tm.tm_isdst = -1;  // 表示不考虑夏令时


    timep = mktime(&tm);
	CHECK_RET_VAL_P_A(timep >0,-1,"mktime faild\n");
    tv.tv_sec = timep;
    tv.tv_usec = 0;

	//settimeofday函数时加入对当前时间的比较，以确保设置的时间不早于当前时间
	//struct timeval tv1;
	//struct timezone tz;
    //gettimeofday(&tv1, &tz);
    //int ret = settimeofday(&tv, &tv1);
    int ret = settimeofday(&tv, NULL);
    CHECK_RET_VAL_P_A(ret == 0,-1,"settimeofday faild\n");
   
    return 0;
}
int wow_time_set_usec(uint64_t nUsec)
{
	CHECK_RET_VAL_P_A(nUsec > 0,-PARAM_INPUT_SIZE_IS_ZERO,"param input size invalid!\n");

    struct timeval tv;
    tv.tv_sec = nUsec/1000000;
    tv.tv_usec = nUsec%1000000;
    int ret = settimeofday(&tv, NULL);
    CHECK_RET_VAL_P_A(ret == 0,-1,"settimeofday faild\n");
   
    return 0;
}



int wow_time_set_cp56(CP56Time2a_T *ptInfo)
{
    time_t timep;
    struct tm tm;
    struct timeval tv;

	CHECK_RET_VAL_P(ptInfo,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");

	time_t msec = (ptInfo->milli_sec_h << 8) + ptInfo->milli_sec_l;
    tm.tm_sec  = msec/1000;
    tm.tm_min  = ptInfo->min;
    tm.tm_hour = ptInfo->hour;
    tm.tm_mday = ptInfo->mon_day;
    tm.tm_mon  = ptInfo->mon - 1;     // 月份从0开始，所以需要减去1
    tm.tm_year = ptInfo->year + 2000 - 1900;// 年份需要减去1900
    tm.tm_isdst = -1;  // 表示不考虑夏令时

    timep = mktime(&tm);
    tv.tv_sec = timep;
    tv.tv_usec = 0;
    int ret = settimeofday(&tv, NULL);
    CHECK_RET_VAL_P_A(ret == 0,-1,"settimeofday faild\n");

	return 0;
}

/*brief    将毫秒转换为固定格式
 *param ： nMsec:毫秒数
 *param ： ptInfo:时间信息结构体
 *return： 成功返回0 失败返回<0
 */
int wow_time_msec_to_rtc(uint64_t nMsec,TimeInfo_T* ptInfo)
{
	int  i = 0;
	CHECK_RET_VAL_P(ptInfo,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	
	memset(ptInfo,0,sizeof(TimeInfo_T));
	ptInfo->msec = nMsec%1000;
	uint64_t sec_time = nMsec/1000;
	
	ptInfo->sec=(uint8_t)(sec_time % 60);//计算当前秒数
	sec_time = sec_time/60;
	
	ptInfo->min=(uint8_t)(sec_time % 60);//计算当分前钟数
	sec_time = sec_time/60;
	
	ptInfo->hour=(uint8_t)(sec_time % 24);//计算当前小时数
	sec_time = sec_time/24;
	
	
	//以4年为基准计算
	ptInfo->year = (uint16_t)(sec_time/1461L)*4 + 1970; 
	sec_time      = sec_time%1461; 

	//校正闰年影响的年份
	while(1){
		int days = 365;
		if ((ptInfo->year & 3) == 0) days = 366;
		if (sec_time < days) break;
		ptInfo->year++;		//计算当前年份
		sec_time -= days;
	}

	sec_time = sec_time+1;
	//ptInfo->week = (uint8_t)((sec_time+4)%7);
	//校正闰年的月份
	if((ptInfo->year & 3) == 0) {
		for(i = 12;i > 0;i--){
			if(gs_mon_days[1][i-1] < sec_time){
				ptInfo->mon = (uint8_t)i;									//计算当前月份
				ptInfo->day = (uint8_t)(sec_time - gs_mon_days[1][i-1]);	//计算当前日份
				break;
			} 
		}
	}else{
		for(i = 12;i > 0;i--){
			if(gs_mon_days[0][i-1] < sec_time){
				ptInfo->mon = (uint8_t)i;									//计算当前月份
				ptInfo->day = (uint8_t)(sec_time - gs_mon_days[0][i-1]);	//计算当前日份
				break;
			} 
		}		
	}

	return 0;
}

/*brief    将毫秒转换为固定格式
 *param ： nMsec:毫秒数
 *param ： ptInfo:时间信息结构体
 *return： 成功返回0 失败返回<0
 */
int wow_time_msec_to_cp56(uint64_t nMsec,CP56Time2a_T* ptInfo)
{
	int  i = 0;
	CHECK_RET_VAL_P(ptInfo,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	
	memset(ptInfo,0,sizeof(CP56Time2a_T));
	ptInfo->milli_sec_l = (uint8_t)(nMsec%60000);
	ptInfo->milli_sec_h = (uint8_t)((nMsec%60000)>>8);
	
	uint64_t sec_time = nMsec/60000;
	
	ptInfo->min=(uint8_t)(sec_time % 60);//计算当分前钟数
	sec_time = sec_time/60;
	
	ptInfo->hour=(uint8_t)(sec_time % 24);//计算当前小时数
	sec_time = sec_time/24;
	
	
	//以4年为基准计算
	ptInfo->year = (uint16_t)(sec_time/1461L)*4 + 1970 - 2000; 
	sec_time   = sec_time%1461; 

	//校正闰年影响的年份
	while(1){
		int days = 365;
		if ((ptInfo->year & 3) == 0) days = 366;
		if (sec_time < days) break;
		ptInfo->year++;		//计算当前年份
		sec_time -= days;
	}

	sec_time = sec_time+1;
	//ptInfo->week = (uint8_t)((sec_time+4)%7);
	//校正闰年的月份
	if((ptInfo->year & 3) == 0) {
		for(i = 12;i > 0;i--){
			if(gs_mon_days[1][i-1] < sec_time){
				ptInfo->mon = (uint8_t)i;										//计算当前月份
				ptInfo->mon_day = (uint8_t)(sec_time - gs_mon_days[1][i-1]);	//计算当前日份
				break;
			} 
		}
	}else{
		for(i = 12;i > 0;i--){
			if(gs_mon_days[0][i-1] < sec_time){
				ptInfo->mon = (uint8_t)i;										//计算当前月份
				ptInfo->mon_day = (uint8_t)(sec_time - gs_mon_days[0][i-1]);	//计算当前日份
				break;
			} 
		}		
	}

	return 0;
}

void wow_time_msec_to_stamp(uint64_t nMsec,char pcBuff[20])
{
	struct tm now_tm;
	time_t now_sec = nMsec/1000;

	localtime_r((time_t*)&now_sec, &now_tm);
	strftime(pcBuff, 20, "%Y-%m-%d %H:%M:%S", &now_tm);
}

void wow_time_usec_to_stamp(uint64_t nUsec, char pcBuff[27])
{
    struct tm now_tm;
    time_t now_sec  = nUsec / 1000000;
    long now_usec = nUsec % 1000000;

    localtime_r(&now_sec, &now_tm);
    
    // 先格式化日期和时间部分
    strftime(pcBuff, 20, "%Y-%m-%d %H:%M:%S", &now_tm);
    
    // 然后手动添加微秒部分
    sprintf(pcBuff + 19, ".%06ld", now_usec);
}

// 格式化为 COMTRADE 标准格式: dd,mm,yyyy,hh,mm,ss.ssssss
void wow_time_usec_to_stamp_comtrade(uint64_t nUsec, char pcBuff[27])
{
    struct tm now_tm;
    time_t now_sec  = nUsec / 1000000;
    long now_usec = nUsec % 1000000;

    localtime_r(&now_sec, &now_tm);
    
    
    sprintf(pcBuff, "%02d,%02d,%04d,%02d,%02d,%02d.%06ld",
            now_tm.tm_mday,           // 日 (01-31)
            now_tm.tm_mon + 1,        // 月 (01-12, tm_mon 是 0-11)
            now_tm.tm_year + 1900,    // 年 (完整年份)
            now_tm.tm_hour,           // 时 (00-23)
            now_tm.tm_min,            // 分 (00-59)
            now_tm.tm_sec,            // 秒 (00-59)
            now_usec);                // 微秒 (000000-999999)
}


void wow_time_sec_to_stamp(int64_t nSec,char pcBuff[20])
{
	struct tm now_tm;
	localtime_r((time_t*)&nSec, &now_tm);
	strftime(pcBuff, 20, "%Y-%m-%d %H:%M:%S", &now_tm);
}

/*brief    将固定格式转换为毫秒
 *param ： ptInfo:时间信息结构体
 *param ： nMsec:毫秒数
 *return： 成功返回0 失败返回<0
 */
int wow_time_rtc_to_msec(TimeInfo_T* ptInfo,uint64_t* pMsec)
{
	int i = 0;
	CHECK_RET_VAL_P(ptInfo,-1,"param input invalid!\n");
	
	uint64_t  msec = 0;
	// 计算当前年秒数
	msec = (ptInfo->year - 1970) * 365 * 24 * 3600;
	for(i = 1970; i < ptInfo->year; i++)
	{
		if(rtc_data_leap(i)) {
			msec += 24 * 3600;
		}
	}
	//计算当前月秒数
	msec += gs_mon_days[rtc_data_leap(ptInfo->year)][ptInfo->mon-1]* 24 * 3600;
	//计算当前日秒数
	msec += (ptInfo->day - 1) * 24 * 3600;
	//计算当前时间段秒数
	msec += ptInfo->hour * 3600 + ptInfo->min * 60 + ptInfo->sec;
	
	///< !!!根据需求添加
	//msec -= SEC_TIME_ZONE;
	
	*pMsec= msec*1000 + ptInfo->msec;

	return  0;
}

int wow_time_cp56_to_msec(CP56Time2a_T* ptInfo,uint64_t* pMsec)
{
	int i = 0;
	CHECK_RET_VAL_P(ptInfo,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	
	uint64_t  msec = 0;
	// 计算当前年秒数
	msec = (ptInfo->year +2000 - 1970) * 365 * 24 * 3600;
	for(i = 1970; i < ptInfo->year +2000; i++)
	{
		if(rtc_data_leap(i)) {
			msec += 24 * 3600;
		}
	}

	//计算当前月秒数
	msec += gs_mon_days[rtc_data_leap(ptInfo->year)][ptInfo->mon-1]* 24 * 3600;
	//计算当前日秒数
	msec += (ptInfo->mon_day - 1) * 24 * 3600;
	//计算当前时间段秒数
	msec += ptInfo->hour * 3600 + ptInfo->min * 60;
	///< !!!根据需求添加
	//msec -= SEC_TIME_ZONE;
	
	*pMsec= msec*1000 + ptInfo->milli_sec_h*256 +ptInfo->milli_sec_l;

	return  0;
}

/*brief    获取当前秒数
 *return： 成功返回秒数 失败返回<0
 */
int64_t wow_time_get_sec()
{
	return time(NULL);
}

/*brief    获取当前毫秒数
 *return： 成功返回毫秒数 失败返回<0
 */
int64_t wow_time_get_msec()
{
    struct timeval tv = {0};
    gettimeofday(&tv, NULL);

	int64_t sec  = tv.tv_sec;
	int64_t msec = sec * 1000 + tv.tv_usec / 1000;
    return msec;
}

/*brief    获取当前微秒数
 *return： 成功返回微秒数 失败返回<0
 */
int64_t wow_time_get_usec()
{
    struct timeval tv = {0};
    gettimeofday(&tv, NULL);
	int64_t sec  = tv.tv_sec;
	int64_t usec = sec * 1000000 + tv.tv_usec;
    return usec;
}
 
