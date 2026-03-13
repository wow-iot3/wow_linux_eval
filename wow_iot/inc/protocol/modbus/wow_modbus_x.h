#ifndef __WOW_IOT_PROTOCOL_MODBUS_X_H_
#define __WOW_IOT_PROTOCOL_MODBUS_X_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum{
	MODBUS_READ_COILS               = 0x01,	//读取线圈DO状态(bit)
	MODBUS_READ_DISCRETE_INPUTS     = 0x02, //读取线圈DI状态(bit)
	MODBUS_READ_HOLDING_REGISTERS   = 0x03, //读保持寄存器    (2byte)
	MODBUS_READ_INPUT_REGISTERS     = 0x04, //读输入寄存器(2byte)
	MODBUS_WRITE_SINGLE_COIL        = 0x05,	//写单个线圈寄存器
	MODBUS_WRITE_SINGLE_REGISTER    = 0x06,	//写单个保持寄存器
	MODBUS_READ_EXCEPTION_STATUS    = 0x07,	//读取异常状态
	MODBUS_WRITE_MULTIPLE_COILS     = 0x0F,	//写多个线圈寄存器
	MODBUS_WRITE_MULTIPLE_REGISTERS = 0x10,	//写多个保持寄存器
	MODBUS_REPORT_SLAVE_ID          = 0x11   //报告从机标识
}ModbusFunctionMode_E;
		
/* Protocol exceptions */
typedef enum {
    MODBUS_ILLEGAL_FUNCTION = 0x01,	//不合法功能代码
    MODBUS_ILLEGAL_DATA_ADDRESS,	//不合法数据地址
    MODBUS_ILLEGAL_DATA_VALUE,		//不合法数据
    MODBUS_SLAVE_OR_SERVER_FAILURE,	//服务器故障
    MODBUS_ACKNOWLEDGE,				//确认
    MODBUS_SLAVE_OR_SERVER_BUSY,	//服务器忙碌
    MODBUS_ERROR_MAX
}ModbusErrorCode_E;

#ifdef __cplusplus
}
#endif

#endif
