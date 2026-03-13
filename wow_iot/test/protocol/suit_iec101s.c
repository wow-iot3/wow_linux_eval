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

#include "protocol/iec10x/wow_iec101s.h"

#define MOD_TAG "iec101s"

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

static void* test_iec101s_signal_spon(void* handle)
{
	ModDebug_D(MOD_TAG,"suit_iec101s----test_iec101s_signal_spon\n");

	 
	DevNodeCtrl_T node[2];
	while(1){
		sleep(1);
		if(g_connect_flag == 0) continue;
		if(g_connect_flag == 2) break;

		//不带时标测试
		memset(node,0,2*sizeof(DevNodeCtrl_T));
		memcpy(&node[0].info,&gt_test_call[0],sizeof(DevNodeInfo_T));
		memcpy(&node[1].info,&gt_test_call[1],sizeof(DevNodeInfo_T));
		node[0].value.val.s8 = 1;
		node[0].value.val.s8 = 0;

		
		wow_iec101_s_build_yx_spon(handle,node,2);
		sleep(5);
		
		//带时标测试
		memset(node,0,2*sizeof(DevNodeCtrl_T));
		memcpy(&node[0].info,&gt_test_call[0],sizeof(DevNodeInfo_T));
		node[0].value.val.s8 = 0;
		
		wow_time_get_cp56(&node[0].value.time);
		wow_iec101_s_build_yx_spon(handle,node,1);
		sleep(5);
		
		break;
	}
	
	return NULL;
}

static void* test_iec101s_detect_spon(void* handle)
{
	ModDebug_D(MOD_TAG,"suit_iec101s----test_iec101s_detect_spon\n");

	DevNodeCtrl_T node[2];
	while(1){
		sleep(1);
		if(g_connect_flag == 0) continue;
		if(g_connect_flag == 2) break;
		
		//不带时标整形测试
		memset(node,0,2*sizeof(DevNodeCtrl_T));
		memcpy(&node[0].info,&gt_test_call[3],sizeof(DevNodeVal_T));
		memcpy(&node[1].info,&gt_test_call[4],sizeof(DevNodeVal_T));
		node[0].value.val.s16 = 222;
		node[1].value.val.s16 = 211;
		wow_iec101_s_build_yc_spon(handle,node,2);
		sleep(5);
		
		//带时标整形测试
		memset(node,0,2*sizeof(DevNodeCtrl_T));
		memcpy(&node[0].info,&gt_test_call[3],sizeof(DevNodeVal_T));
		node[0].value.val.s16 = 122;
		wow_time_get_cp56(&node[0].value.time);
		wow_iec101_s_build_yc_spon(handle,node,1);
		sleep(5);
		
		//不带时标整形测试
		memset(node,0,2*sizeof(DevNodeCtrl_T));
		memcpy(&node[0].info,&gt_test_call[5],sizeof(DevNodeVal_T));
		memcpy(&node[1].info,&gt_test_call[6],sizeof(DevNodeVal_T));
		node[0].value.val.f = 22.2;
		node[1].value.val.f = 21.1;
		wow_iec101_s_build_yc_spon(handle,node,2);
		sleep(5);
		
		//带时标浮点型测试
		memset(node,0,2*sizeof(DevNodeCtrl_T));
		memcpy(&node[0].info,&gt_test_call[5],sizeof(DevNodeVal_T));
		node[0].value.val.f = 12.2;
		wow_time_get_cp56(&node[0].value.time);
		wow_iec101_s_build_yc_spon(handle,node,1);
		sleep(5);

		break;
	}

	return NULL;
}

static int iec101s_set_value(void* priv,DevNodeInfo_T* node,DevNodeVal_T* val)
{	
	int i = 0;

	CHECK_RET_VAL(node,-1);
	CHECK_RET_VAL(node->mode == DEV_TYPE_YK || node->mode == DEV_TYPE_YT,-1);

	for(i = 0; i < ARRAY_SIZE(gt_test_call); i++){
		CHECK_RET_CONTINUE(node->addr == gt_test_call[i].addr);
		printf("iec101s_set_value-----------addr:%d val.u32:%d val.f:%f\n",node->addr,val->val.u32,val->val.f);
		memcpy(&gt_test_val[i],val,sizeof(DevNodeVal_T));
		break;
	}

	return 0;
}

static int iec101s_get_value(void* priv,DevNodeInfo_T* node,DevNodeVal_T* val)
{	
	int i = 0;

	CHECK_RET_VAL(node,-1);
	CHECK_RET_VAL(node->mode == DEV_TYPE_YX || node->mode == DEV_TYPE_YC|| node->mode == DEV_TYPE_YM ,-1);

	for(i = 0; i < ARRAY_SIZE(gt_test_call); i++){
		CHECK_RET_CONTINUE(node->addr == gt_test_call[i].addr);

		printf("iec101s_get_value-----------addr:%d val.u32:%d val.f:%f\n",node->addr,gt_test_val[i].val.u32,gt_test_val[i].val.f);
		memcpy(val,&gt_test_val[i],sizeof(DevNodeVal_T));
		break;
	}

	return 0;
}


static int iec101s_set_service(void* priv,uint8_t ser_id,void* arg)
{
	ModDebug_D(MOD_TAG,"----service_exec_cb ser_id:%d \n",ser_id);

	switch(ser_id){
		case IEC10X_CLOCK_SYNC:
			break;
		case IEC10X_CONNECTED:
			g_connect_flag = 1;
			break;
		default:
			break;
			            

	}
	return 0;
}
    


TEST test_iec101s_fun(void)
{
	int ret = 0;
	
	ModDebug_I(MOD_TAG,"suit_iec101s----test_iec101s_fun\n");
	ProtoUartParam_T link;
	link.port = 0;
	link.param.baudRate 	= Baud9600;
	link.param.dataBits 	= Data8;
	link.param.parity		= NoParity;
	link.param.stopBits 	= OneStop;
	link.param.flowControl	= NoFlowControl;


	Iec101s_T* iec101s = wow_iec101_s_init(0x0001,&link);
	GREATEST_ASSERT(iec101s);

	ret = wow_iec101_s_set_node_info(iec101s,gt_test_call,ARRAY_SIZE(gt_test_call));
	GREATEST_ASSERT(ret == 0);
	ret = wow_iec101_s_set_value_handle(iec101s,iec101s_set_value,NULL);
	GREATEST_ASSERT(ret == 0);
	ret = wow_iec101_s_set_value_handle(iec101s,iec101s_get_value,NULL);
	GREATEST_ASSERT(ret == 0);
	ret = wow_iec101_s_set_service_handle(iec101s,iec101s_set_service,NULL);
	GREATEST_ASSERT(ret == 0);

	ret = wow_iec101_s_start(iec101s);
	GREATEST_ASSERT(ret == 0);
	

	wow_thread_create(MOD_TAG,test_iec101s_signal_spon, iec101s);
	wow_thread_create(MOD_TAG,test_iec101s_detect_spon, iec101s);

	sleep(100);
	
	g_connect_flag = 2;
	sleep(2);
	
	wow_iec101_s_exit(&iec101s);

	PASS();
}

SUITE(suit_iec101s)
{
	wow_debug_setStatus(MOD_TAG,SUIT_RUN_DEBUG_FLAG);

    RUN_TEST(test_iec101s_fun);
}


