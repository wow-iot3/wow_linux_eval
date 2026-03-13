#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "greatest/greatest.h"
#include "wow_test_config.h"

#include "prefix/wow_byte.h"
#include "prefix/wow_common.h"
#include "utils/wow_type.h"
#include "plugin/wow_debug.h"

#include "protocol/modbus/wow_modbus_m.h"

#define MOD_TAG "modbus_m"

#define MODBUS_DEV_ADDR  0x01

typedef struct{
	uint32_t  	afn;    //设备转发功能码
	uint16_t 	addr;   //设备转发地址
	uint8_t		bulk;   //采集点bulk
	void       *data;
}ModbusCaptureInfo_T;


static uint8_t  gt_modbus_capture_info_01[] ={0,0,0,0,0,0,0,0,0,0,0,0};	
static uint8_t  gt_modbus_capture_info_02[] ={0,0,0,0,0,0,0,0,0,0,0,0};
static uint16_t gt_modbus_capture_info_03[] ={0,0,0,0,0,0,0,0};
static float    gt_modbus_capture_info_04[] ={0.0,0.0,0.0,0.0};
static uint16_t gt_modbus_capture_info_05[] ={0};
static uint32_t gt_modbus_capture_info_06[] ={0,0,0,0};
		

static ModbusCaptureInfo_T gt_capture_info[]={
	{MODBUS_READ_COILS,			    0x0000,12,gt_modbus_capture_info_01},
	{MODBUS_READ_DISCRETE_INPUTS,   0x0000,12,gt_modbus_capture_info_02},
	{MODBUS_READ_HOLDING_REGISTERS, 0x0000,8, gt_modbus_capture_info_03},
	{MODBUS_READ_HOLDING_REGISTERS, 0x0010,8, gt_modbus_capture_info_04},
	{MODBUS_READ_HOLDING_REGISTERS, 0x0020,1, gt_modbus_capture_info_05},
	{MODBUS_READ_INPUT_REGISTERS,   0x0000,8, gt_modbus_capture_info_06},
};
	


TEST test_modbus_m_fun(ModbusMaster_T* test_modbus_00)
{
	int ret = -1;
	int i = 0;

	uint8_t data01[64]={0};
	uint8_t data02[64]={0};

	/*---------------afn:01--------------------*/
	ret = wow_modbus_m_read_data(test_modbus_00,MODBUS_DEV_ADDR,gt_capture_info[0].afn,
					gt_capture_info[0].addr,gt_capture_info[0].bulk,data01);
	GREATEST_ASSERT(ret == 0);
	
	 bytes_to_bits(gt_modbus_capture_info_01,gt_capture_info[1].bulk,data01,Align8(gt_capture_info[1].bulk)/8);		

	printf("afn:01----data:");
	for( i = 0; i < gt_capture_info[0].bulk; i++){
		printf("%d ",gt_modbus_capture_info_01[i]);
	}
	printf("\n");


	/*---------------afn:02--------------------*/
	ret = wow_modbus_m_read_data(test_modbus_00,MODBUS_DEV_ADDR,gt_capture_info[1].afn,
					gt_capture_info[1].addr,gt_capture_info[1].bulk,data01);
	GREATEST_ASSERT(ret == 0);

	 bytes_to_bits(gt_modbus_capture_info_02,gt_capture_info[1].bulk,data01,Align8(gt_capture_info[1].bulk)/8);						

	printf("afn:02----data:");
	for( i = 0; i < gt_capture_info[1].bulk; i++){
		printf("%d ",gt_modbus_capture_info_02[i]);
	}
	printf("\n");
	

	
	/*---------------afn:05--------------------*/
	gt_modbus_capture_info_01[0] = 1;
	ret = wow_modbus_m_write_data(test_modbus_00,MODBUS_DEV_ADDR,MODBUS_WRITE_SINGLE_COIL,
							gt_capture_info[0].addr,1,&gt_modbus_capture_info_01[0]);
	GREATEST_ASSERT(ret == 0);

	ret = wow_modbus_m_read_data(test_modbus_00,MODBUS_DEV_ADDR,gt_capture_info[0].afn,
					gt_capture_info[0].addr,gt_capture_info[0].bulk,data01);
	GREATEST_ASSERT(ret == 0);
	
	 bytes_to_bits(gt_modbus_capture_info_01,gt_capture_info[1].bulk,data01,Align8(gt_capture_info[1].bulk)/8);		
	GREATEST_ASSERT(gt_modbus_capture_info_01[0] == 1);

	
	
	/*---------------afn:0F--------------------*/
	gt_modbus_capture_info_01[10] = 0;
	bits_to_bytes(data01,Align8(gt_capture_info[0].bulk)/8,gt_modbus_capture_info_01,gt_capture_info[1].bulk);	
	ret = wow_modbus_m_write_data(test_modbus_00,MODBUS_DEV_ADDR,MODBUS_WRITE_MULTIPLE_COILS,
							gt_capture_info[0].addr,gt_capture_info[1].bulk,data01);
	GREATEST_ASSERT(ret == 0);

	ret = wow_modbus_m_read_data(test_modbus_00,MODBUS_DEV_ADDR,gt_capture_info[0].afn,
					gt_capture_info[0].addr,gt_capture_info[0].bulk,data01);
	GREATEST_ASSERT(ret == 0);
	
	 bytes_to_bits(gt_modbus_capture_info_01,gt_capture_info[1].bulk,data01,Align8(gt_capture_info[1].bulk)/8);		
	GREATEST_ASSERT(gt_modbus_capture_info_01[10]== 0);


	

	/*---------------afn:03--------------------*/
	ret = wow_modbus_m_read_data(test_modbus_00,MODBUS_DEV_ADDR,gt_capture_info[2].afn,
					gt_capture_info[2].addr,gt_capture_info[2].bulk,data01);
	GREATEST_ASSERT(ret == 0);
	
	printf("afn:03----data:");
	for( i = 0; i < gt_capture_info[2].bulk; i++){
		gt_modbus_capture_info_03[i] = byte_get_32_be_impl(data01 + 2*i);
		printf("%d ",gt_modbus_capture_info_03[i]);
	}
	printf("\n");


	ret = wow_modbus_m_read_data(test_modbus_00,MODBUS_DEV_ADDR,gt_capture_info[3].afn,
					gt_capture_info[3].addr,gt_capture_info[3].bulk,data01);
	GREATEST_ASSERT(ret == 0);
	printf("afn:03----data:");
	for( i = 0; i < gt_capture_info[3].bulk/2; i++){
		gt_modbus_capture_info_04[i] = byte_get_float_abcd(data01 + 4*i);
		printf("%f ",gt_modbus_capture_info_04[i]);
	}
	printf("\n");

	/*---------------afn:04--------------------*/
	ret = wow_modbus_m_read_data(test_modbus_00,MODBUS_DEV_ADDR,gt_capture_info[5].afn,
					gt_capture_info[5].addr,gt_capture_info[5].bulk,data01);
	GREATEST_ASSERT(ret == 0);
	printf("afn:04----data:");
	for( i = 0; i < gt_capture_info[5].bulk/2; i++){
		gt_modbus_capture_info_06[i] = byte_get_32_be_impl(data01 + 4*i);
		printf("%08x ",gt_modbus_capture_info_06[i]);
	}
	printf("\n");



	/*---------------afn:06--------------------*/
	byte_set_16_be_impl(data02,101);
	ret = wow_modbus_m_write_data(test_modbus_00,MODBUS_DEV_ADDR,MODBUS_WRITE_SINGLE_REGISTER,
							gt_capture_info[2].addr,1,data02);
	GREATEST_ASSERT(ret == 0);
	ret = wow_modbus_m_read_data(test_modbus_00,MODBUS_DEV_ADDR,gt_capture_info[2].afn,
					gt_capture_info[2].addr,gt_capture_info[2].bulk,data01);
	GREATEST_ASSERT(ret == 0);
	
	char2short_be(data01,gt_capture_info[2].bulk*2,gt_modbus_capture_info_03,gt_capture_info[2].bulk);
	GREATEST_ASSERT(gt_modbus_capture_info_03[0] == 101);

	/*---------------afn:16--------------------*/
	byte_set_16_be_impl(data02+12,707);
	byte_set_16_be_impl(data02+14,808);

	ret = wow_modbus_m_write_data(test_modbus_00,MODBUS_DEV_ADDR,MODBUS_WRITE_MULTIPLE_REGISTERS,
							gt_capture_info[2].addr,gt_capture_info[2].bulk,data02);
	GREATEST_ASSERT(ret == 0);

	ret = wow_modbus_m_read_data(test_modbus_00,MODBUS_DEV_ADDR,gt_capture_info[2].afn,
					gt_capture_info[2].addr,gt_capture_info[2].bulk,data01);
	GREATEST_ASSERT(ret == 0);
	
	char2short_be(data01,gt_capture_info[2].bulk*2,gt_modbus_capture_info_03,gt_capture_info[2].bulk);
	GREATEST_ASSERT(gt_modbus_capture_info_03[6] == 707 && gt_modbus_capture_info_03[7] == 808);



	PASS();
}


TEST test_modbus_m_rtu_fun(void)
{

	ModDebug_D(MOD_TAG,"suit_modbus_rtu_m----test_modbus_m_rtu_fun\n");	
	
	ProtoUartParam_T link;
	link.port = 0;
	link.param.baudRate 	= Baud9600;
	link.param.dataBits 	= Data8;
	link.param.parity		= NoParity;
	link.param.stopBits 	= OneStop;
	link.param.flowControl	= NoFlowControl;
	
	ModbusMaster_T* test_modbus_00 = wow_modbus_m_rtu_open(&link);
	GREATEST_ASSERT(test_modbus_00);

	
	test_modbus_m_fun(test_modbus_00);

	wow_modbus_m_rtu_close(&test_modbus_00);
	
    PASS();
}


TEST test_modbus_m_tcp_fun(void)
{
		
	ModDebug_D(MOD_TAG,"suit_modbus_rtu_m----test_modbus_m_tcp_fun\n");	

	ProtoEthParam_T link = {0};
	link.port = SUIT_TEST_MODBUS_TCP_PORT;
	memcpy(link.host,SUIT_TEST_MODBUS_TCP_HOST,strlen(SUIT_TEST_MODBUS_TCP_HOST));

	ModbusMaster_T* test_modbus_00 = wow_modbus_m_tcp_open(&link);
	GREATEST_ASSERT(test_modbus_00);

	test_modbus_m_fun(test_modbus_00);

	wow_modbus_m_tcp_close(&test_modbus_00);
	
    PASS();
}

SUITE(suit_modbus_m)
{
	wow_debug_setStatus(MOD_TAG,SUIT_RUN_DEBUG_FLAG);
   // RUN_TEST(test_modbus_m_rtu_fun);
	RUN_TEST(test_modbus_m_tcp_fun);
	
}

