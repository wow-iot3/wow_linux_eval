#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "prefix/wow_errno.h"
#include "prefix/wow_check.h"
#include "prefix/wow_common.h"
#include "prefix/wow_keyword.h"

#include "network/wow_tcp.h"

#include "protocol/wow_proto_port.h"

typedef struct{
	ProtoEthParam_T   	 info;
	Tcp_T*		      	 server;
	Tcp_T*		      	 client;
	proto_port_cb_func_t conn;
	void* 			  	 conn_priv;
	proto_port_cb_func_t disconn;
	void* 			  	 disconn_priv;	
}__data_aligned__ ProtoPortEth_T;

typedef struct{
	ProtoUartParam_T info;
}__data_aligned__ ProtoPortUart_T;

typedef union{
	ProtoPortEth_T  eth;
	ProtoPortUart_T uart;
}ProtoPortParam_U;

typedef struct{
	ProtoPortCtx_T  	ctx;
	ProtoPortParam_U    param;   
}ProtoPort_T;

/**************************************网口操作函数*************************************************/
static int eth_link_aceept_cb(Tcp_T* tcp,void *arg)  
{  
	ProtoPortEth_T* port = (ProtoPortEth_T*)arg;

	if(port->client){
		wow_tcp_close(port->client);
		port->client = NULL;
		sleep(1);
	}	

	if(port->conn){
		port->conn(port->conn_priv);

	}
	port->client = tcp;

    return 0;  
} 

static int eth_link_server_open(struct proto_port_ctx_t* ctx)
{
	int ret = 0;
	
	ProtoPort_T* port = (ProtoPort_T*)ctx;
	CHECK_RET_VAL(port,-1);

	port->param.eth.server = wow_tcp_open(port->param.eth.info.host,port->param.eth.info.port);
	CHECK_RET_VAL_P(port->param.eth.server,-1,"tcp open failed!\n");

	ret = wow_tcp_listen(port->param.eth.server,16);
	CHECK_RET_GOTO_P(ret == 0,out,"tcp listen failed!\n");

	ret = wow_tcp_accpet_cb(port->param.eth.server, eth_link_aceept_cb,&port->param.eth);
	CHECK_RET_GOTO_P(ret == 0,out,"tcp set accept cb failed!\n");

	return 0;
out:
	wow_tcp_close(port->param.eth.server);
	return -1;

}

static int eth_link_client_open(struct proto_port_ctx_t* ctx)
{
	ProtoPort_T* port = (ProtoPort_T*)ctx;
	CHECK_RET_VAL(port,-1);
	
    port->param.eth.client = wow_tcp_open(NULL,0);
    CHECK_RET_VAL_P(port->param.eth.client,-1,"wow_tcp_open failed.");

	int ret = wow_tcp_connect(port->param.eth.client,port->param.eth.info.host,port->param.eth.info.port);
	CHECK_RET_GOTO_P(ret == 0,out,"wow_tcp_connect %s:%d failed.",port->param.eth.info.host,port->param.eth.info.port);

	return 0;
	
out:
	wow_tcp_close(port->param.eth.client);
	port->param.eth.client = NULL;
	return -1;
}

static int eth_link_send(struct proto_port_ctx_t* ctx,uint8_t* data,int len, int timeout_ms)
{
	ProtoPort_T* port = (ProtoPort_T*)ctx;
	CHECK_RET_VAL(port && port->param.eth.client,-1);

	return wow_tcp_write_timeout(port->param.eth.client,data,len,timeout_ms);
}

static int eth_link_recv(struct proto_port_ctx_t* ctx,uint8_t* data,int len, int timeout_ms)
{
	ProtoPort_T* port = (ProtoPort_T*)ctx;
	CHECK_RET_VAL(port && port->param.eth.client,-1);
	return wow_tcp_read_timeout(port->param.eth.client,data,len,timeout_ms);
}

static void eth_link_close(struct proto_port_ctx_t* ctx)
{	
	ProtoPort_T* port = (ProtoPort_T*)ctx;
	CHECK_RET_VOID(port && port->param.eth.client);
	
	wow_tcp_close(port->param.eth.client);
	port->param.eth.client = NULL;
}

/*brief    网口(TCP)从设备初始化
 *param ： ptEthPara: 网口参数
 *return： 成功返回网口操作符  失败返回NULL
 */
__IN_API__ ProtoPortCtx_T* proto_port_eth_slave_init(ProtoEthParam_T* ptEthPara)
{
	ProtoPort_T* port = CALLOC(1,ProtoPort_T);
	CHECK_RET_VAL_P(port, NULL, "malloc ProtoPort_T failed!\n");

	memcpy(&port->param.eth.info,ptEthPara,sizeof(ProtoEthParam_T));
	port->param.eth.server = NULL;
	port->param.eth.client = NULL;
	port->ctx.link_open  = eth_link_server_open;
	port->ctx.link_send  = eth_link_send;
	port->ctx.link_recv  = eth_link_recv;
	port->ctx.link_close = eth_link_close;
	
	return (ProtoPortCtx_T*)port;
}

/*brief    设置网口(TCP)连接回调函数
 *param ： ptProtoCtx: 串口操作符
 *param ： fCallback: 回调函数
 *param ： priv: 回调函数私有变量
 *return： 成功返回0 失败返回<0
 */
__IN_API__ int proto_port_eth_set_connected_cb(ProtoPortCtx_T* ptProtoCtx,proto_port_cb_func_t fCallback,void* priv)
{
	CHECK_RET_VAL_P(ptProtoCtx,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(fCallback,-PARAM_INPUT_FUNC_IS_NULL,"param input func invalid!\n");

	ProtoPort_T* port = (ProtoPort_T*)ptProtoCtx;
	port->param.eth.conn      = fCallback;
	port->param.eth.conn_priv = priv;

	return 0;
}


/*brief    设置网口(TCP)断开函数
 *param ： ptProtoCtx: 串口操作符
 *param ： fCallback: 回调函数
 *param ： priv: 回调函数私有变量
 *return： 成功返回0 失败返回<0
 */
__IN_API__ int proto_port_eth_set_disconnected_cb(ProtoPortCtx_T* ptProtoCtx,proto_port_cb_func_t fCallback,void* priv)
{
	CHECK_RET_VAL_P(ptProtoCtx,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(fCallback,-PARAM_INPUT_FUNC_IS_NULL,"param input func invalid!\n");

	ProtoPort_T* port = (ProtoPort_T*)ptProtoCtx;
	port->param.eth.disconn      = fCallback;
	port->param.eth.disconn_priv = priv;

	return WOW_SUCCESS;
}

/*brief    退出网口操作接口
 *param ： pptProtoCtx: 串口操作符
 *return： 无
 */
__IN_API__ void proto_port_eth_slave_exit(ProtoPortCtx_T** pptProtoCtx)
{
	CHECK_RET_VOID(pptProtoCtx && *pptProtoCtx);
	ProtoPort_T* port = (ProtoPort_T*)*pptProtoCtx;

	if(port->param.eth.client){
		wow_tcp_close(port->param.eth.client);
	}
	
	if(port->param.eth.server){
		wow_tcp_close(port->param.eth.server);
	}

	FREE(port);
	*pptProtoCtx = NULL;
}

/*brief    网口(TCP)主设备初始化
 *param ： ptEthPara: 网口参数
 *return： 成功返回网口操作符 失败返回NULL
 */
__IN_API__ ProtoPortCtx_T* proto_port_eth_master_init(ProtoEthParam_T* ptEthPara)
{
	ProtoPort_T* port = CALLOC(1,ProtoPort_T);
	CHECK_RET_VAL_P(port, NULL, "malloc ProtoPort_T failed!\n");

	memcpy(&port->param.eth.info,ptEthPara,sizeof(ProtoEthParam_T));
	port->param.eth.server = NULL;
	port->param.eth.client = NULL;
	port->ctx.link_open  = eth_link_client_open;
	port->ctx.link_send  = eth_link_send;
	port->ctx.link_recv  = eth_link_recv;
	port->ctx.link_close = eth_link_close;
	
	return (ProtoPortCtx_T*)port;	
}

/*brief    退出网口(TCP)操作
 *param ： pptProtoCtx: 串口操作符
 *return： 无
 */
__IN_API__ void proto_port_eth_master_exit(ProtoPortCtx_T** pptProtoCtx)
{
	CHECK_RET_VOID(pptProtoCtx && *pptProtoCtx);

	ProtoPort_T* port = (ProtoPort_T*)*pptProtoCtx;

	if(port->param.eth.client){
		wow_tcp_close(port->param.eth.client);
	}
	
	FREE(port);
	*pptProtoCtx = NULL;
}


/**************************************串口操作函数*************************************************/
static int uart_link_open(struct proto_port_ctx_t* ctx)
{
	int ret = 0;
	ProtoPort_T* port = (ProtoPort_T*)ctx;
	CHECK_RET_VAL_P(port,-1,"param input invalid!\n");

	ret = wow_hal_uart_open(port->param.uart.info.port);
	CHECK_RET_VAL_P(ret == 0,-1,"uart open uart%d failed\n",port->param.uart.info.port);
	
	wow_hal_uart_set_param(port->param.uart.info.port,&port->param.uart.info.param);

	return 0;
}

static int uart_link_send(struct proto_port_ctx_t* ctx,uint8_t* data,int len, int timeout_ms)
{
	ProtoPort_T* port = (ProtoPort_T*)ctx;
	CHECK_RET_VAL_P(port,-1,"param input invalid!\n");
	
	return wow_hal_uart_send_timeout(port->param.uart.info.port,data,len,timeout_ms);
}

static int uart_link_recv(struct proto_port_ctx_t* ctx,uint8_t* data,int len, int timeout_ms)
{
	ProtoPort_T* port = (ProtoPort_T*)ctx;
	CHECK_RET_VAL_P(port,-1,"param input invalid!\n");
	
	return wow_hal_uart_recv_timeout(port->param.uart.info.port,data,len,timeout_ms);
}

static void uart_link_close(struct proto_port_ctx_t* ctx)
{
	ProtoPort_T* port = (ProtoPort_T*)ctx;
	CHECK_RET_VOID(port);

	wow_hal_uart_close(port->param.uart.info.port);
}

/*brief    串口操作初始化
 *param ： ptUartPara: 串口参数
 *return： 成功返回串口操作符 失败返回NULL
 */
__IN_API__ ProtoPortCtx_T* proto_port_uart_init(ProtoUartParam_T* ptUartPara)
{
	ProtoPort_T* port = CALLOC(1,ProtoPort_T);
	CHECK_RET_VAL_P(port, NULL, "malloc ProtoPort_T failed!\n");

	memcpy(&port->param.uart.info,ptUartPara,sizeof(ProtoUartParam_T));
	port->ctx.link_open  = uart_link_open;
	port->ctx.link_send  = uart_link_send;
	port->ctx.link_recv  = uart_link_recv;
	port->ctx.link_close = uart_link_close;
	
	return (ProtoPortCtx_T*)port;	

}

/*brief    退出串口操作
 *param ： pptProtoCtx: 串口操作符
 *return： 无
 */
__IN_API__ void proto_port_uart_exit(ProtoPortCtx_T** pptProtoCtx)
{
	CHECK_RET_VOID(pptProtoCtx && *pptProtoCtx);

	ProtoPort_T* port = (ProtoPort_T*)*pptProtoCtx;

	wow_hal_uart_close(port->param.uart.info.port);
	
	FREE(port);
	*pptProtoCtx = NULL;
}





