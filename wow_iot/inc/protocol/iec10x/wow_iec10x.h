#ifndef __WOW_IOT_PROTOCOL_WOW_IEC10X_H_
#define __WOW_IOT_PROTOCOL_WOW_IEC10X_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "prefix/wow_keyword.h"
#include "system/wow_time.h"
#include "list/wow_slist.h"

#include "protocol/wow_proto_data.h"

#define IEC10X_CIRCLE_TESTER_U    (30*1000)
#define IEC10X_TESTER_U_TIMEOUT   (3*1000)	
#define IEC10X_CONNECT_TIMEOUT    (3*1000)	
#define IEC10X_CALL_ALL_TIMEOUT   (30*1000)
#define IEC10X_CALL_PULSE_TIMEOUT (30*1000)
#define IEC10X_YK_CMD_TIMEOUT     (10*1000)
#define IEC10X_YT_CMD_TIMEOUT     (10*1000)

#define IEC10X_SEND_TIMEOUT   (100)
#define IEC10X_RECV_TIMEOUT   (100)

typedef enum{
	DEV_TYPE_YX = 0,
	DEV_TYPE_YC,
	DEV_TYPE_YM,
	DEV_TYPE_YT,
	DEV_TYPE_YK,
}DevTypeMod_E;

typedef struct{
	uint8_t    		mode;
	uint16_t   		addr;
	uint8_t    		type;
}__data_aligned__ DevNodeInfo_T;

typedef struct{
	DevValue_U 		val;
	CP56Time2a_T	time;	//时标	
}__data_aligned__ DevNodeVal_T;

typedef struct{
	DevNodeInfo_T   info;	//节点信息
	DevNodeVal_T    value;
}__data_aligned__ DevNodeCtrl_T;


typedef struct{
	Slist_T* yx_list;
	Slist_T* yc_n_list;
	Slist_T* yc_f_list;
	Slist_T* ym_list;
}__data_aligned__ NodeListInfo_T;


typedef int (*iec10x_get_value_func_t)(void* priv,DevNodeInfo_T *node,DevNodeVal_T *val);
typedef int (*iec10x_set_value_func_t)(void* priv,DevNodeInfo_T *node,DevNodeVal_T *val);
typedef int (*iec10x_set_service_func_t)(void* priv,uint8_t idx,void* arg);


typedef enum
{
	IEC10X_CLOCK_SYNC = 0x0000,	//arg - CP56Time2a_T类型
	
	IEC10X_CONNECTED,
	IEC10X_DISCONNECTED,
	 			
	IEC10X_CALL_ALL_SUCCESS,
	IEC10X_CALL_ALL_FAILD,
	
	IEC10X_CALL_PULSE_SUCCESS,
	IEC10X_CALL_PULSE_FAILD,

	IEC10X_YT_CMD_SUCCESS,
	IEC10X_YT_CMD_FAILD,

	IEC10X_YK_CMD_SUCCESS,
	IEC10X_YK_CMD_FAILD,

	IEC10X_YX_SPON_SUCCESS,
	IEC10X_YX_SPON_FAILD,

	IEC10X_YC_SPON_SUCCESS,
	IEC10X_YC_SPON_FAILD,

}Ie10xServeice_E;


#define     IEC10X_INFO_ADDR_NONE           0X00

#ifdef __cplusplus
}
#endif

#endif
