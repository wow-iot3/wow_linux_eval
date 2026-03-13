#ifndef __WOW_IOT_PROTOCOL_WOW_MODBUS_M_H_
#define __WOW_IOT_PROTOCOL_WOW_MODBUS_M_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "wow_usart.h"
#include "protocol/wow_proto_port.h"
#include "protocol/modbus/wow_modbus_x.h"

typedef struct modbus_master_t ModbusMaster_T;

/*brief    打开modbus-rtu主设备
 *param ： ptUartPara: 设备端硬件参数
 *return： 成功返回modbus操作符 失败返回NULL
 */
ModbusMaster_T* wow_modbus_m_rtu_open(ProtoUartParam_T* ptUartPara);


/*brief    关闭modbus主设备
 *param ： pptModbusCtx: 设备操作符 
 *return： 无
 */
void wow_modbus_m_rtu_close(ModbusMaster_T** pptModbusCtx);

/*brief    打开modbus-tcp主设备
 *param ： ptEthPara: 设备端硬件参数
 *return： 成功返回modbus操作符 失败返回NULL
 */
ModbusMaster_T* wow_modbus_m_tcp_open(ProtoEthParam_T* ptEthPara);

/*brief    关闭modbus主设备
 *param ： pptModbusCtx: 设备操作符 
 *return： 无
 */
void wow_modbus_m_tcp_close(ModbusMaster_T** pptModbusCtx);


/*brief    设置modbus主采集超时时间
 *param ： ptModbusCtx:  设备操作符 
 *param ： u32Msec: 采集超时时间(ms)  默认500ms
 *return： 成功返回0 失败返回-1
 */
int wow_modbus_m_set_timeout(ModbusMaster_T* ptModbusCtx, uint32_t u32Msec);


/*brief    modbus读取操作
 *param ： ptModbusCtx:   设备操作符 
 *param ： nSlave:   从设备地址
 *param ： eAfn:     采集功能码
 *param ： u32Addr:  采集地址
 *param ： u32Size:  采集长度
 *param ： pu8Data:  采集内容存储
 *return： 成功返回0 接收失败返回-1 发送失败返回-2 
 */
 int  wow_modbus_m_read_data(ModbusMaster_T* ptModbusCtx,int nSlave,ModbusFunctionMode_E eAfn,
										uint32_t u32Addr, uint32_t u32Size, uint8_t* pu8Data);

/*brief    modbus写入操作
 *param ： ptModbusCtx:   设备操作符 
 *param ： nSlave:   从设备地址
 *param ： eAfn:     采集功能码
 *param ： u32Addr:  采集地址
 *param ： u32Size:  采集长度
 *param ： pu8Data:  采集内容存储
 *return： 成功返回0 失败返回-1
 */
int  wow_modbus_m_write_data(ModbusMaster_T* ptModbusCtx,int nSlave,ModbusFunctionMode_E eAfn,
										uint32_t u32Addr, uint32_t u32Size, uint8_t* pu8Data);

#ifdef __cplusplus
}
#endif

#endif 
