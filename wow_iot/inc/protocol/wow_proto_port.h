#ifndef __WOW_IOT_PROTOCOL_WOW_PROTO_PORT_H_
#define __WOW_IOT_PROTOCOL_WOW_PROTO_PORT_H_


#include "prefix/wow_keyword.h"

#include "wow_usart.h"

typedef struct{
	char host[MAX_URL_SIZE];      		///<网口链接地址
	uint16_t port;						///<网口链接端口
}__data_aligned__ ProtoEthParam_T;

typedef struct {
	uint8_t   			 port;			///<串口索引值
	HalUartParam_T       param;			///<串口参数配置
}__data_aligned__ ProtoUartParam_T;

typedef struct proto_port_ctx_t{
	int   (*link_open)(struct proto_port_ctx_t* ctx);
	int   (*link_send)(struct proto_port_ctx_t* ctx,uint8_t *data,int len, int timeout_ms);
	int   (*link_recv)(struct proto_port_ctx_t* ctx,uint8_t *data,int len, int timeout_ms);
	void  (*link_close)(struct proto_port_ctx_t* ctx);
}__data_aligned__ ProtoPortCtx_T;

typedef void (*proto_port_cb_func_t)(void* priv);

/*brief    串口操作初始化
 *param ： ptUartPara: 串口参数
 *return： 成功返回串口操作符 失败返回NULL
 */
ProtoPortCtx_T* proto_port_uart_init(ProtoUartParam_T* ptUartPara);

/*brief    退出串口操作
 *param ： pptProtoCtx: 串口操作符
 *return： 无
 */
void proto_port_uart_exit(ProtoPortCtx_T** pptProtoCtx);

/*brief    网口(TCP)主设备初始化
 *param ： ptEthPara: 网口参数
 *return： 成功返回网口操作符 失败返回NULL
 */
ProtoPortCtx_T* proto_port_eth_master_init(ProtoEthParam_T* ptEthPara);

/*brief    退出网口(TCP)操作
 *param ： pptProtoCtx: 串口操作符
 *return： 无
 */
void proto_port_eth_master_exit(ProtoPortCtx_T** pptProtoCtx);

/*brief    网口(TCP)从设备初始化
 *param ： ptEthPara: 网口参数
 *return： 成功返回网口操作符  失败返回NULL
 */
ProtoPortCtx_T* proto_port_eth_slave_init(ProtoEthParam_T* ptEthPara);

/*brief    退出网口操作接口
 *param ： pptProtoCtx: 串口操作符
 *return： 无
 */
void proto_port_eth_slave_exit(ProtoPortCtx_T**  pptProtoCtx);

/*brief    设置网口(TCP)连接回调函数
 *param ： ptProtoCtx: 串口操作符
 *param ： fCallback: 回调函数
 *param ： priv: 回调函数私有变量
 *return： 成功返回0 失败返回<0
 */
int proto_port_eth_set_connected_cb(ProtoPortCtx_T* ptProtoCtx,proto_port_cb_func_t fCallback,void* priv);


/*brief    设置网口(TCP)断开函数
 *param ： ptProtoCtx: 串口操作符
 *param ： fCallback: 回调函数
 *param ： priv: 回调函数私有变量
 *return： 成功返回0 失败返回<0
 */
int proto_port_eth_set_disconnected_cb(ProtoPortCtx_T* ptProtoCtx,proto_port_cb_func_t fCallback,void* priv);




#endif
