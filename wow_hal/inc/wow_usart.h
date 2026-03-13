#ifndef __WOW_HAL_WOW_USART_H_
#define __WOW_HAL_WOW_USART_H_

#include <termios.h>
#include <stdint.h>

/* UART baud rate */
typedef enum{
	Baud2400 = B2400,    //0x0000000b
	Baud4800 = B4800,    //0x0000000c
	Baud9600 = B9600,	 //0x0000000d //default
	Baud19200 = B19200,  //0x0000000e
	Baud38400 = B38400,  //0x0000000f
	Baud57600 = B57600,  //0x00001001
	Baud115200 = B115200,//0x00001002
}HalUartBaudRate_E;

/* UART data width */
typedef enum{
	Data5 = 5,
	Data6 = 6,
	Data7 = 7,
	Data8 = 8,			//default
}HalUartDataBits_E;

/* UART parity */
typedef enum{
	NoParity = 0,		//default
	EvenParity = 2,
	OddParity = 3,
}HalUartParity_E;

/* UART stop bits */
typedef enum{	
	OneStop = 1,		//default
	OneAndHalfStop = 3,
	TwoStop = 2,
}HalUartStopBits_E;

/* UART flow control */
typedef enum{
	NoFlowControl,		//default
	HardwareControl,
	SoftwareControl,
}HalUartFlowControl_E;

/* UART configuration */	
typedef struct {
    HalUartBaudRate_E    baudRate;
    HalUartDataBits_E    dataBits;
    HalUartParity_E      parity;
    HalUartStopBits_E    stopBits;
    HalUartFlowControl_E flowControl;
}HalUartParam_T;


typedef struct{
	int		        port;
	HalUartParam_T  config;
	void* 		    priv;
}UartDev_T;

typedef void (*uart_recv_callback_func_t)(int index,const void* priv);

/*brief    打开串口UART (初始设置9600/8/N/1/N)
 *param ： nIdx: 串口标识参数 
 *return： 成功返回0 失败返回<0
 */
int wow_hal_uart_open(int nIdx);

/*brief    设置串口UART参数
 *param ： nIdx: 串口标识参数
 *param ： param: 串口参数配置
 *return： 成功返回0 失败返回<0
 */
int wow_hal_uart_set_param(int nIdx,HalUartParam_T *param);

/*brief    获取串口UART参数
 *param ： nIdx: 串口标识参数
 *param ： param: 串口参数配置 为空设置为默认配置
 *return： 成功返回0 失败返回<0
 */
int wow_hal_uart_get_param(int nIdx,HalUartParam_T *param);

/*brief   串口UART发送数据
 *param ： nIdx: 串口标识参数
 *param ： pu8Data: 待发送数据缓存
 *param ： snSize: 待发送数据大小
 *return： 成功返回发送数据长度 失败返回<0
 */
int wow_hal_uart_send(int nIdx, const uint8_t *pu8Data, size_t snSize);
int wow_hal_uart_send_timeout(int nIdx, const uint8_t *pu8Data, size_t snSize, int nTimerouMs);

/*brief    串口UART设置接收回调函数
 *param ： nIdx: 串口标识参数
 *param ： fCallback: 接收回调函数
 *param ： pArg: 接收回调函数私有变量
 *return： 成功返回0 失败返回<0
 */
int wow_hal_uart_set_recv_cb(int nIdx,uart_recv_callback_func_t fCallback,const void* pArg); 

/*brief   串口UART接收数据
 *param ： nIdx: 串口标识参数
 *param ： pu8Data: 待发送接收缓存
 *param ： snSize: 待发送接收大小
 *return： 成功返回接收数据长度 失败返回<0
 */
int wow_hal_uart_recv(int nIdx, uint8_t *pu8Data, size_t snSize);
int wow_hal_uart_recv_timeout(int nIdx, uint8_t *pu8Data, size_t snSize,int nTimerouMs);

 /*brief   关闭串口UART
 *param ： nIdx: 串口标识参数
 *return： 成功返回0 失败返回<0
 */
int wow_hal_uart_close(int nIdx);

#endif