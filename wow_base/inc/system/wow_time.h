#ifndef __WOW_BASE_SYSTEM_WOW_TIME_H_
#define __WOW_BASE_SYSTEM_WOW_TIME_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint16_t year;
    uint8_t  mon;
    uint8_t  day;
    uint8_t  hour;
    uint8_t  min;
    uint8_t  sec;
    uint16_t msec;
}TimeInfo_T;

typedef struct __attribute__ ((packed))
{
    uint8_t   milli_sec_l;
    uint8_t   milli_sec_h; // 0~59999
  
    uint8_t   min            : 6; // 0~59
    uint8_t   resv_1         : 1;
    uint8_t   iv             : 1;
    
    uint8_t   hour           : 5; // 0~23
    uint8_t   resv_2         : 2;
    uint8_t   su             : 1;
    
    uint8_t   mon_day        : 5; // 1~31
    uint8_t   week_day       : 3; // 1~7
    
    uint8_t   mon            : 4; // 1~12
    uint8_t   resv_3         : 4;
    
    uint8_t   year           : 7; // 0~99
    uint8_t   resv_4         : 1;  
    
}CP56Time2a_T;

/*brief    延迟 nMsec
 *return： 成功返回0 失败返回<0
 */
int wow_time_sleep_msec(size_t nMsec);

/*brief    获取当前时间
 *return： 返回当前时间字符串
 */
char* wow_get_time_stamp(void);

/*brief    获取时间信息
 *param ： ptInfo:时间信息结构体
 *return： 成功返回0 失败返回-1
 */
int wow_time_get_info(TimeInfo_T* ptInfo);
int wow_time_get_cp56(CP56Time2a_T *ptInfo);

/*brief    设置时间
 *param ： ptInfo:时间信息结构体
 *return： 成功返回0 失败返回-1
 */
int wow_time_set_info(TimeInfo_T *ptInfo);
int wow_time_set_cp56(CP56Time2a_T *ptInfo);
int wow_time_set_usec(uint64_t nUsec);

/*brief    获取当前秒数/毫秒数/微秒数
 *return： 成功返回秒数 失败返回<0
 */
int64_t wow_time_get_sec();
int64_t wow_time_get_msec(void);
int64_t wow_time_get_usec(void);

/*brief    将毫秒转换为固定格式
 *param ： nMsec:毫秒数
 *param ： ptInfo:时间信息结构体
 *return： 成功返回0 失败返回<0
 */
int wow_time_msec_to_rtc(uint64_t nMsec,TimeInfo_T* ptInfo);
int wow_time_msec_to_cp56(uint64_t nMsec,CP56Time2a_T* ptInfo);
void wow_time_msec_to_stamp(uint64_t nMsec,char pcBuff[20]);
void wow_time_usec_to_stamp(uint64_t nUsec,char pcBuff[27]);
void wow_time_sec_to_stamp(int64_t nSec,char pcBuff[20]);
// 格式化为 COMTRADE 标准格式: dd,mm,yyyy,hh,mm,ss.ssssss
void wow_time_usec_to_stamp_comtrade(uint64_t nUsec, char pcBuff[27]);

/*brief    将毫秒转换为固定格式
 *param ： ptInfo:时间信息结构体
 *param ： nMsec:毫秒数
 *return： 成功返回0 失败返回<0
 */
int wow_time_rtc_to_msec(TimeInfo_T* ptInfo,uint64_t* pMsec);
int wow_time_cp56_to_msec(CP56Time2a_T* ptInfo,uint64_t* pMsec);




#ifdef __cplusplus
}
#endif


#endif
