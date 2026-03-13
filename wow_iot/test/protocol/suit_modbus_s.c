#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "greatest/greatest.h"
#include "wow_test_config.h"

#include "prefix/wow_check.h"
#include "prefix/wow_byte.h"
#include "prefix/wow_common.h"
#include "utils/wow_type.h"
#include "plugin/wow_debug.h"
#include "system/wow_time.h"
#include "network/wow_net_iface.h"

#include "protocol/wow_proto_data.h"
#include "protocol/modbus/wow_modbus_s.h"


#define MOD_TAG "modbus_s"


#define MODBUS_DEV_ADDR  0x01

typedef struct{
	uint32_t  	afn;    //设备转发功能码
	uint8_t  	type;   //采集数据类型
	uint16_t 	addr;   //设备转发地址
	uint8_t		bulk;   //采集点bulk
	void       *data;
}ModbusTransferInfo_T;


static uint8_t gt_modbus_calculate_info_01[] ={0,1,0,0,0,0,0,0,1,1,1,1};	
static uint8_t gt_modbus_calculate_info_02[] ={0,1,0,0,0,1,0,1,0,1,1,0};
static uint16_t gt_modbus_calculate_info_03[] ={1001,2002,3003,4004,5005,6006,7007,8008};
static float gt_modbus_calculate_info_04[] ={1.25,10.25,100.52,1000.34};
static uint16_t gt_modbus_calculate_info_05[] ={0x1001};
static uint32_t gt_modbus_calculate_info_06[] ={0x10012002,0x30034004,0x50056006,0x70078008};
		

static ModbusTransferInfo_T gt_transfer_info[]={
	{MODBUS_READ_COILS,			   DATA_UINT_8, 0x0000,12,gt_modbus_calculate_info_01},
	{MODBUS_READ_DISCRETE_INPUTS,  DATA_UINT_8, 0x1000,12,gt_modbus_calculate_info_02},
	{MODBUS_READ_HOLDING_REGISTERS,DATA_UINT_16,0x2000,8, gt_modbus_calculate_info_03},
	{MODBUS_READ_HOLDING_REGISTERS,DATA_FLOAT,  0x2010,8, gt_modbus_calculate_info_04},
	{MODBUS_READ_HOLDING_REGISTERS,DATA_UINT_16,0x2020,1, gt_modbus_calculate_info_05},
	{MODBUS_READ_INPUT_REGISTERS,  DATA_UINT_32,0x3000,8, gt_modbus_calculate_info_06},
};



static void modbus_get_data_random(uint8_t type,void *data)
{
	switch(type){
      case DATA_UINT_8:
	  	*(uint8_t*)data = wow_time_get_msec()%256;
	  	break;  
	  case DATA_UINT_16:
	  	*(uint16_t*)data = wow_time_get_msec()%10000;
	  	break;  
	  case DATA_UINT_32:
		*(uint32_t*)data = wow_time_get_msec();
		break;
	  case DATA_FLOAT:
		*(float*)data = wow_time_get_msec()%10000*0.1;
		break;
	  default:
	  	break;
   }
}

int modbus_get_afn_addr_val(void* priv,int slave,uint8_t afn,uint16_t addr,uint8_t *data,uint16_t len)
{
	CHECK_RET_VAL(slave == MODBUS_DEV_ADDR,-1);
	//printf("modbus_get_afn_addr_val afn:%d aadr:%d len:%d\n",afn,addr,len);
	int i= 0;
	int idx = 0;
	for( i = 0; i < ARRAY_SIZE(gt_transfer_info);i++){
		CHECK_RET_CONTINUE(afn == gt_transfer_info[i].afn);
		if(addr >= gt_transfer_info[i].addr && addr+len <= gt_transfer_info[i].addr+gt_transfer_info[i].bulk){
			modbus_get_data_random(gt_transfer_info[i].type,gt_transfer_info[i].data);
			

			switch(afn){
				case MODBUS_READ_COILS:
				case MODBUS_READ_DISCRETE_INPUTS:
					idx = addr-gt_transfer_info[i].addr;
					bits_to_bytes((uint8_t*)gt_transfer_info[i].data + idx,len,data,Align8(len));
					return 0;
				case MODBUS_READ_HOLDING_REGISTERS:
				case MODBUS_READ_INPUT_REGISTERS:
					idx = (addr-gt_transfer_info[i].addr)*2/wow_data_get_len(gt_transfer_info[i].type);
					wow_data_other_to_byte_offset(gt_transfer_info[i].type,gt_transfer_info[i].data,idx,
														len*2/wow_data_get_len(gt_transfer_info[i].type),data);
					return 0;
				default:
					return -1;
			}
			return -1;
		}
	}

	return -1;
}

int modbus_set_afn_addr_val(void* priv,int slave,uint8_t afn,uint16_t addr,uint8_t *data,uint16_t len)
{
	CHECK_RET_VAL(slave == MODBUS_DEV_ADDR,-1);
	int i= 0;
	int idx = 0;
	//printf("modbus_set_afn_addr_val afn:%d aadr:%d len:%d\n",afn,addr,len);
	
	for( i = 0; i < ARRAY_SIZE(gt_transfer_info);i++){
		if(addr >= gt_transfer_info[i].addr && addr+len <= gt_transfer_info[i].addr+gt_transfer_info[i].bulk){
			
			if((gt_transfer_info[i].afn == MODBUS_READ_COILS || gt_transfer_info[i].afn == MODBUS_READ_DISCRETE_INPUTS)
				 					&& (afn == MODBUS_WRITE_SINGLE_COIL)){
				
				idx = addr-gt_transfer_info[i].addr;

				*((uint8_t*)gt_transfer_info[i].data+idx) = *data;
				return 0;
			}
			else if((gt_transfer_info[i].afn == MODBUS_READ_COILS || gt_transfer_info[i].afn == MODBUS_READ_DISCRETE_INPUTS)
				 				&& (afn == MODBUS_WRITE_MULTIPLE_COILS)){
					idx = addr-gt_transfer_info[i].addr;
					bytes_to_bits(data,len,(uint8_t*)gt_transfer_info[i].data+idx,Align8(len));
				return 0;
			}else if((gt_transfer_info[i].afn == MODBUS_READ_HOLDING_REGISTERS || gt_transfer_info[i].afn == MODBUS_READ_INPUT_REGISTERS)
				 							&& (afn == MODBUS_WRITE_SINGLE_REGISTER || afn == MODBUS_WRITE_MULTIPLE_REGISTERS)){
				idx = (addr-gt_transfer_info[i].addr)*2/wow_data_get_len(gt_transfer_info[i].type);
				wow_data_byte_to_other_offset(gt_transfer_info[i].type,data,len*2,gt_transfer_info[i].data,idx);
				return 0;
			}
		}
	}
	
	return -1;
}

static ModbusSlaverDevCtx_T gt_modbus_s_ctx= {
	modbus_get_afn_addr_val,
	modbus_set_afn_addr_val
};

TEST test_modbus_s_rtu_fun(void)
{	
	ModDebug_D(MOD_TAG,"suit_modbus_rtu_s----test_modbus_s_rtu_fun\n");	

	ProtoUartParam_T link;
	link.port = 0;
	link.param.baudRate 	= Baud9600;
	link.param.dataBits 	= Data8;
	link.param.parity		= NoParity;
	link.param.stopBits 	= OneStop;
	link.param.flowControl	= NoFlowControl;

	ModbusSlaver_T* test_modbus_00 = wow_modbus_s_rtu_open(&link,&gt_modbus_s_ctx,NULL);
	GREATEST_ASSERT(test_modbus_00);

	sleep(100);
	
	wow_modbus_s_tcp_close(&test_modbus_00);
	
    PASS();
}

TEST test_modbus_s_tcp_fun(void)
{
	ModDebug_D(MOD_TAG,"suit_modbus_rtu_s----test_modbus_s_tcp_fun\n");	

	ProtoEthParam_T link;
	link.port = SUIT_TEST_MODBUS_TCP_PORT;
	GREATEST_ASSERT(wow_eth_getLocalIp(link.host) == 0);

	ModbusSlaver_T* test_modbus_00 = wow_modbus_s_tcp_open(&link,&gt_modbus_s_ctx,NULL);
	GREATEST_ASSERT(test_modbus_00);

	sleep(60);
	
	wow_modbus_s_tcp_close(&test_modbus_00);

    PASS();
}

SUITE(suit_modbus_s)
{
	wow_debug_setStatus(MOD_TAG,SUIT_RUN_DEBUG_FLAG);
    //RUN_TEST(test_modbus_s_rtu_fun);
	RUN_TEST(test_modbus_s_tcp_fun);
	
}


