#ifndef __IEC10X_BASE_H_
#define __IEC10X_BASE_H_

#include "system/wow_lock.h"
#include "system/wow_cond.h"
#include "system/wow_thread.h"

/*
 * Asdu addr
 * */
#define     IEC10X_INFO_ADDR_SIG_BASE           0X0001
#define     IEC10X_INFO_ADDR_SIG_TEMP_HX_OFF    0x1000

/*
 * Asdu type (TI)
 * */
#define     IEC10X_M_SP_NA_1      1 	//! 单点信息
#define     IEC10X_M_DP_NA_1      3 	//! 双点信息
#define     IEC10X_M_ME_NA_1      9		//! 归一化遥测值
#define     IEC10X_M_ME_NC_1      13	//! 短浮点遥测值
#define 	IEC10X_M_IT_NA_1      15    //! 累计量
#define 	IEC10X_M_IT_TA_1      16    //! 带时标的累计量
#define     IEC10X_M_SP_TB_1      30 	//! 带CP56Time2a时标的单点信息
#define     IEC10X_M_DP_TB_1      31 	//! 带CP56Time2a时标的双点信息
#define     IEC10X_M_ME_TD_1      34	//! 带CP56Time2a时标的归一化遥测值
#define     IEC10X_M_ME_TF_1      36    //! 带CP56Time2a时标的短浮点遥测值
#define 	IEC10X_C_SC_NA_1      45    //! 单点命令
#define 	IEC10X_C_DC_NA_1      46    //! 双点命令
#define     IEC10X_C_SE_NA_1      48	//! 设定值命令，归一化值
#define     IEC10X_C_SE_NC_1      50	//! 设定值命令，短浮点数
#define     IEC10x_M_EI_NA_1      70	//! 初始化结束
#define     IEC10X_C_IC_NA_1      100   //! 总召唤命令
#define     IEC10X_C_CI_NA_1      101   //! 计数量召唤命令
#define     IEC10X_C_CS_NA_1      103	//! 时钟同步命令
#define     IEC10X_C_CD_NA_1      106	//! 收集传输延时

/*
 * Asdu reason (COT)
 * */
#define     IEC10X_COT_PER_CYC        1		//! 周期、循环
#define     IEC10X_COT_BACK           2		//! 背景扫描
#define     IEC10X_COT_SPONT          3		//! 自发、突发
#define     IEC10X_COT_INIT           4		//! 初始化
#define     IEC10X_COT_REQ            5		//! 请求或者被请求
#define     IEC10X_COT_ACT            6		//! 激活
#define     IEC10X_COT_ACTCON         7		//! 激活确认
#define 	IEC10X_COT_DEACT          8 	//! 停止激活
#define 	IEC10X_COT_DEACTCON       9 	//! 停止激活确认
#define 	IEC10X_COT_ACTTERM        10	//! 激活终止
#define     IEC10X_COT_RETREM         11	//! 远方命令引起的反送信息
#define     IEC10X_COT_RETLOC         12	//! 当地命令引起的反送信息 
#define     IEC10X_COT_INTROGEN       20	//! 响应站召唤
#define     IEC10X_COT_INTRO1         21	//! 响应第1组召唤
#define     IEC10X_COT_INTRO2         22	//! 响应第2组召唤
#define 	IEC10X_COT_REQCOGEN       37	//! 响应计数量召唤
#define 	IEC10X_COT_REQCO1         38	//! 响应第1组计数量召唤
#define 	IEC10X_COT_REQCO2         39	//! 响应第2组计数量召唤
#define 	IEC10X_COT_UNKNOW_TYPE    44    //! 未知的类型标识
#define 	IEC10X_COT_UNKNOW_COT     45    //! 未知的传送原因
#define 	IEC10X_COT_UNKNOW_ADDR    46    //! 未知的应用服务数据单元公共地址
#define 	IEC10X_COT_UNKNOW_INF     47    //! 未知的信息体对象地址
#define 	IEC10X_COT_ACTCON_NO      0x47  //! 激活否定确认

typedef enum {
	IEC10X_STA_LINK_INIT = 0x00,
	IEC10X_STA_LINK_OPEN,	
	IEC10X_STA_LINK_CONNECT,
	IEC10X_STA_LINK_CONNECT_OK,
	IEC10X_STA_LINK_DISCONNECT,
	IEC10X_STA_LINK_IDLE,
	IEC10X_STA_LINK_CLOSE,

	IEC10X_STA_CALL_ALL_IDLE,
	IEC10X_STA_CALL_ALL_START,
	IEC10X_STA_CALL_ALL_CONFIRM,
	IEC10X_STA_CALL_ALL_YX,
	IEC10X_STA_CALL_ALL_YC,
	IEC10X_STA_CALL_ALL_SUCCESS,
	IEC10X_STA_CALL_ALL_FAILD,

	IEC10X_STA_CALL_PULSE_IDLE,
	IEC10X_STA_CALL_PULSE_START,
	IEC10X_STA_CALL_PULSE_CONFIRM,
	IEC10X_STA_CALL_PULSE_YM,
	IEC10X_STA_CALL_PULSE_SUCCESS,
	IEC10X_STA_CALL_PULSE_FAILD,

	IEC10X_STA_YK_CMD_IDLE,
	IEC10X_STA_YK_CMD_START,
	IEC10X_STA_YK_CMD_CONFIRM,
	IEC10X_STA_YK_CMD_EXEC,
	IEC10X_STA_YK_CMD_SUCCESS,
	IEC10X_STA_YK_CMD_FAILD,

	IEC10X_STA_YT_CMD_IDLE,
	IEC10X_STA_YT_CMD_START,
	IEC10X_STA_YT_CMD_CONFIRM,
	IEC10X_STA_YT_CMD_EXEC,
	IEC10X_STA_YT_CMD_SUCCESS,
	IEC10X_STA_YT_CMD_FAILD,
	
	IE10X_STA_TESTER_IDLE,
	IE10X_STA_TESTER_START,
	IE10X_STA_TESTER_STOP
}Iec10xStat_E;



typedef struct{
	uint8_t init;
	uint8_t callall;
	uint8_t callpulse;
	uint8_t yt;
	uint8_t yk;
	uint8_t test;
}__data_aligned__ Iec10xState_T;

typedef struct{
	int64_t init;
	int64_t callall;
	int64_t callpulse;
	int64_t yt;
	int64_t yk;
	int64_t test;
	int64_t timer;
}__data_aligned__ Iec10xTime_T;


typedef struct{
	pthread_idx_t	thread;
	int 			tflag;
	mutex_lock_t 	lock;
	mutex_cond_t 	cond;
}__data_aligned__ Iec10xThreadInfo_T;

#endif
