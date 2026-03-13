#ifndef __WOW_IOT_PROTOCOL_WOW_MODBUS_S_H_
#define __WOW_IOT_PROTOCOL_WOW_MODBUS_S_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "wow_usart.h"
#include "protocol/wow_proto_port.h"
#include "protocol/modbus/wow_modbus_x.h"

typedef struct modbus_slaver_t ModbusSlaver_T;;

typedef struct{
	int (*get_afn_addr_val)(void* priv,int slave,uint8_t afn,uint16_t addr,uint8_t *data,uint16_t len);
	int (*set_afn_addr_val)(void* priv,int slave,uint8_t afn,uint16_t addr,uint8_t *data,uint16_t len);
}ModbusSlaverDevCtx_T;


/*brief    打开modbus-rtu从设备
 *param ： ptUartPara: 设备端硬件参数
 *param ： ptDevCtx  : 参数读取/设置回调函数 
 *param ： pArg:  回调函数私有变量 
 *return： 成功返回modbus操作符失败返回NULL
 */
ModbusSlaver_T* wow_modbus_s_rtu_open(ProtoUartParam_T* ptUartPara,ModbusSlaverDevCtx_T* ptDevCtx,void* pArg);

/*brief    关闭modbus主设备
 *param ： pptModbusCtx: 设备操作符 
 *return： 无
 */
void wow_modbus_s_rtu_close(ModbusSlaver_T** pptModbusCtx);

/*brief    打开modbus-tcp从设备
 *param ： ptEthPara: 设备端硬件参数
 *param ： ptDevCtx : 参数读取/设置回调函数 
 *param ： pArg:  回调函数私有变量 
 *return： 成功返回modbus操作符失败返回NULL
 */
ModbusSlaver_T* wow_modbus_s_tcp_open(ProtoEthParam_T* ptEthPara,ModbusSlaverDevCtx_T* ptDevCtx,void* pArg);

/*brief    关闭modbus从设备
 *param ： pptModbusCtx: 从设备操作符 
 *return： 无
 */
void wow_modbus_s_tcp_close(ModbusSlaver_T** pptModbusCtx);

#ifdef __cplusplus
}
#endif

#endif 
