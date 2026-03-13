#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "greatest/greatest.h"
#include "wow_test_config.h"

#include "prefix/wow_check.h"
#include "prefix/wow_common.h"

#include "plugin/wow_debug.h"
#include "system/wow_time.h"
#include "system/wow_thread.h"

#include "protocol/iec10x/wow_iec101m.h"

#define MOD_TAG "iec101m"

static int g_connect_flag = 0;

static DevNodeInfo_T gt_test_call[17] ={
	{DEV_TYPE_YX,0x0001,DATA_SINT_8},	//index --0
	{DEV_TYPE_YX,0x0002,DATA_SINT_8},
	{DEV_TYPE_YX,0x0003,DATA_SINT_8},
	{DEV_TYPE_YC,0x4001,DATA_SINT_16},	//index --3
	{DEV_TYPE_YC,0x4002,DATA_SINT_16},
	{DEV_TYPE_YC,0x4301,DATA_FLOAT},
	{DEV_TYPE_YC,0x4302,DATA_FLOAT},
	{DEV_TYPE_YC,0x4303,DATA_FLOAT},
	{DEV_TYPE_YM,0x6401,DATA_SINT_32},	//index --8
	{DEV_TYPE_YM,0x6402,DATA_SINT_32},
	{DEV_TYPE_YM,0x6403,DATA_SINT_32},
	{DEV_TYPE_YK,0x6001,DATA_SINT_8},	//index --11
	{DEV_TYPE_YK,0x6002,DATA_SINT_8},
	{DEV_TYPE_YK,0x6003,DATA_SINT_8},
	{DEV_TYPE_YT,0x6201,DATA_SINT_16},	//index --14
	{DEV_TYPE_YT,0x6202,DATA_SINT_16},
	{DEV_TYPE_YT,0x6203,DATA_SINT_16},
};

static DevNodeVal_T gt_test_val[17] ={};


static int iec101m_set_value(void* priv,DevNodeInfo_T* node,DevNodeVal_T* val)
{
	int i = 0;

	CHECK_RET_VAL(node,-1);
	for(i = 0; i < ARRAY_SIZE(gt_test_call); i++){
		if(node->addr == gt_test_call[i].addr && node->mode == gt_test_call[i].mode)
		{
			memcpy(&gt_test_val[i],val,sizeof(DevNodeVal_T));
			break;
		}
	}

	return 0;
}

static int iec101m_set_service(void* priv,uint8_t ser_id,void* arg)
{
	ModDebug_D(MOD_TAG,"----service_exec_cb ser_id:%d \n",ser_id);

	int i = 0;
	switch(ser_id){
		case IEC10X_CLOCK_SYNC:
			{
				CP56Time2a_T* cp56time2a = (CP56Time2a_T*)arg;
				ModDebug_D(MOD_TAG,"Clock cmd (20%d-%d-%d %d:%d:xx) \n",cp56time2a->year,cp56time2a->mon,
					cp56time2a->mon_day,cp56time2a->hour,cp56time2a->min);
			}
			break;
		case IEC10X_CONNECTED:
			g_connect_flag = 1;
			break;
		case IEC10X_CALL_ALL_SUCCESS:
			for(i = 0; i < ARRAY_SIZE(gt_test_call); i++){
				CHECK_RET_CONTINUE(gt_test_call[i].mode == DEV_TYPE_YX || gt_test_call[i].mode == DEV_TYPE_YC);
				printf("TYPE:%d %04x-----type:%d val-u32:%d val-f:%f \n",gt_test_call[i].mode,gt_test_call[i].addr,
									gt_test_call[i].type,gt_test_val[i].val.u32,gt_test_val[i].val.f);	
			}
			break;
		case IEC10X_CALL_PULSE_SUCCESS:
			for(i = 0; i < ARRAY_SIZE(gt_test_call); i++){
				CHECK_RET_CONTINUE(gt_test_call[i].mode == DEV_TYPE_YM);
				printf("TYPE:%d %04x-----type:%d val-u32:%d val-f:%f \n",gt_test_call[i].mode,gt_test_call[i].addr,
									gt_test_call[i].type,gt_test_val[i].val.u32,gt_test_val[i].val.f);	
			}
			break;
		default:
			break;
			            
	}
	return 0;
}
  

TEST test_iec101_m_fun(void)
{
	ModDebug_I(MOD_TAG,"suit_iec101m----test_iec101_m_fun\n");

	int ret = 0;
	Iec101m_T* iec101m = NULL;
	ProtoUartParam_T link;
	link.port = 0;
	link.param.baudRate 	= Baud9600;
	link.param.dataBits 	= Data8;
	link.param.parity		= NoParity;
	link.param.stopBits 	= OneStop;
	link.param.flowControl	= NoFlowControl;


	iec101m = wow_iec101_m_init(0x0001,&link);
	GREATEST_ASSERT(iec101m);
	
	ret = wow_iec101_m_set_value_handle(iec101m,iec101m_set_value,NULL);
	GREATEST_ASSERT(ret == 0);
	ret = wow_iec101_m_set_service_handle(iec101m,iec101m_set_service,NULL);
	GREATEST_ASSERT(ret == 0);

	ret = wow_iec101_m_start(iec101m);
	GREATEST_ASSERT(ret == 0);
	
	while(g_connect_flag == 0) sleep(1);

	ModDebug_D(MOD_TAG,"----wow_iec101_m_build_call_all----- \n");
	ret = wow_iec101_m_build_call_all(iec101m);
	GREATEST_ASSERT(ret == 0);


	ModDebug_D(MOD_TAG,"----wow_iec101_m_build_call_pulse----- \n");
	ret = wow_iec101_m_build_call_pulse(iec101m);
	GREATEST_ASSERT(ret == 0);
	sleep(5);

	DevNodeCtrl_T ctrl;
	ctrl.info.mode = DEV_TYPE_YK;
	ctrl.info.addr = 0x6001;
	ctrl.info.type = DATA_SINT_8;
	ctrl.value.val.u8   = 1;
	ModDebug_D(MOD_TAG,"----wow_iec101_m_build_yk----- \n");
	ret = wow_iec101_m_build_yk(iec101m,&ctrl,1);
	GREATEST_ASSERT(ret == 0);
	sleep(3);

	ModDebug_D(MOD_TAG,"----wow_iec101_m_build_yt----- \n");

	ctrl.info.mode = DEV_TYPE_YT;
	ctrl.info.addr = 0x6201;
	ctrl.info.type = DATA_SINT_8;
	ctrl.value.val.u8   = 127;
	ret = wow_iec101_m_build_yt(iec101m,&ctrl,1);
	GREATEST_ASSERT(ret == 0);
	sleep(3);

	wow_iec101_m_exit(&iec101m);
	PASS();
}


SUITE(suit_iec101m)
{
	wow_debug_setStatus(MOD_TAG,SUIT_RUN_DEBUG_FLAG);

    RUN_TEST(test_iec101_m_fun);
}



