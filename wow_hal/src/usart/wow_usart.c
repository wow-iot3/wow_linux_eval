#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>    
#include <sys/stat.h>   
#include <fcntl.h>
#include <termios.h>
#include <linux/serial.h>
#include <linux/fs.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <linux/types.h>

#include "ev.h"

#include "prefix/wow_check.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"
#include "system/wow_thread.h"

#include "hal_dev_conf.h"

#include "wow_usart.h"
#include "wow_hal_internal.h"
#include "wow_hal_errno.h"

typedef struct {
	bool 			   opened;
	int 			   chan;
    int                fd;
    HalUartParam_T     param;
    struct ev_io 	   ior;			//读事件
    struct ev_loop     *loop;	
	uart_recv_callback_func_t func;
	pthread_idx_t	   pid;
	const void* 	   priv;
}HalUartDev_T;

static HalUartParam_T gt_defParam ={
	Baud9600,
	Data8,
	NoParity,
	OneStop,
	NoFlowControl
};


static HalUsartConfig_T* gpt_uartConfig = NULL;
static HalUartDev_T* gpt_uartDev = NULL;

static int uart_set_config(int fd,HalUartParam_T *param)
{
	int ret = -1;
	struct termios options; 

	ret = tcgetattr(fd, &options);
	CHECK_RET_VAL_P(ret == 0,-1, "uart tcgetattr failed!\n");

	bzero( &options, sizeof(options) );
	options.c_cflag  |=  CLOCAL | CREAD; 
	options.c_cflag &= ~CSIZE;
	   
	//tcflush(gi_curFd[port], TCIOFLUSH);
	cfsetispeed(&options, param->baudRate);  
	cfsetospeed(&options, param->baudRate);  
	 
	switch (param->dataBits){
		case 5:	
			options.c_cflag |= CS5; 
			break;
		case 6: 
			options.c_cflag |= CS6; 
		case 7:	
			options.c_cflag |= CS7; 
			break;
		case 8: 
			options.c_cflag |= CS8; 
			break;   
		default: 
			return -1;   
	}
	switch (param->parity) { 
	case OddParity:     
		options.c_cflag |= (PARODD | PARENB);  
		options.c_iflag |= INPCK;            
		break;   
	case EvenParity:   
		options.c_cflag |= PARENB;     
		options.c_cflag &= ~PARODD;      
		options.c_iflag |= INPCK;      
		break;
	case NoParity:  /*as no parity*/   
	    options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		break;  
	default: 
		return -1;
	}
	
	switch (param->stopBits) {
	case OneStop: 
		options.c_cflag &= ~CSTOPB; 
		break;  
	case TwoStop: 
		options.c_cflag |= CSTOPB; 
		break;
	default:
		return -1;
	}
	
	//tcflush(gi_curFd[port], TCIFLUSH);
	options.c_cc[VTIME] = 0;
	options.c_cc[VMIN] = 0; 		/* Update the options and do it NOW */
	ret = tcsetattr(fd, TCSANOW, &options); 
	CHECK_RET_VAL_P(ret == 0,-1, "uart tcsetattr failed!\n");

	tcflush(fd,TCIOFLUSH);  
	return 0;	
}


/*brief    串口UART初始化
 *param ： nIdx: 串口标识参数(从0开始)
 *param ： param: 串口参数配置
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_hal_uart_open(int nIdx)
{
	CHECK_RET_VAL_P(gpt_uartDev && gpt_uartConfig,-USART_NOT_INITED,"uart env not inited!\n");
	CHECK_RET_VAL_P(nIdx < gpt_uartConfig->num,-PARAM_INPUT_ARG_INVALID,"param input arg invlaid!\n");
	CHECK_RET_VAL(gpt_uartDev[nIdx].opened == false,0);

    int fd = open(gpt_uartConfig->name[nIdx], O_RDWR|O_NONBLOCK);
	CHECK_RET_VAL_P(fd > 0,-USART_OPEN_FAILED, "open uart %s failed!\n",gpt_uartConfig->name[nIdx]);

	int ret = uart_set_config(fd,&gt_defParam);
	CHECK_RET_GOTO_P(ret == 0,out, "set uart param failed!\n");

	gpt_uartDev[nIdx].opened = true;
	gpt_uartDev[nIdx].chan   = nIdx;
	gpt_uartDev[nIdx].fd     = fd;
	memcpy(&gpt_uartDev[nIdx].param,&gt_defParam,sizeof(HalUartParam_T));

	return 0;

out:
	close(fd);
	return -1;
}

/*brief    设置串口UART参数
 *param ： nIdx: 串口标识参数
 *param ： ptUartPara: 串口参数配置
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_hal_uart_set_param(int nIdx,HalUartParam_T *ptUartPara)
{
	CHECK_RET_VAL_P(gpt_uartDev && gpt_uartConfig,-USART_NOT_INITED,"uart env not inited!\n");
	CHECK_RET_VAL_P(nIdx < gpt_uartConfig->num,-PARAM_INPUT_ARG_INVALID,"param input arg invlaid!\n");
	CHECK_RET_VAL_P(ptUartPara,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");

	int ret = uart_set_config(gpt_uartDev[nIdx].fd,ptUartPara);
	CHECK_RET_VAL_P(ret == 0,-USART_SET_PARAM_FAILED, "set uart param failed!\n");

	memcpy(&gpt_uartDev[nIdx].param,ptUartPara,sizeof(HalUartParam_T));

	return 0;
}

/*brief    获取串口UART参数
 *param ： nIdx: 串口标识参数
 *param ： ptUartPara: 串口参数配置 为空设置为默认配置
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_hal_uart_get_param(int nIdx,HalUartParam_T *ptUartPara)
{
	CHECK_RET_VAL_P(gpt_uartDev && gpt_uartConfig,-USART_NOT_INITED,"uart env not inited!\n");
	CHECK_RET_VAL_P(nIdx < gpt_uartConfig->num,-PARAM_INPUT_ARG_INVALID,"param input arg invlaid!\n");
	CHECK_RET_VAL_P(ptUartPara,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");

	memcpy(ptUartPara,&gpt_uartDev[nIdx].param,sizeof(HalUartParam_T));

	return 0;
}


/*brief   串口UART发送数据
 *param ： nIdx: 串口标识参数(从0开始)
 *param ： pu8Data: 待发送数据缓存
 *param ： snSize: 待发送数据大小
 *return： 成功返回发送数据长度 失败返回<0
 */
__EX_API__ int wow_hal_uart_send(int nIdx, const uint8_t *pu8Data, size_t snSize)
{
	CHECK_RET_VAL_P(gpt_uartDev && gpt_uartConfig,-USART_NOT_INITED,"uart env not inited!\n");
	CHECK_RET_VAL_P(nIdx < gpt_uartConfig->num && pu8Data && snSize,-PARAM_INPUT_ARG_INVALID,"param input arg invlaid!\n");
	CHECK_RET_VAL_P(gpt_uartDev[nIdx].opened,-USART_NOT_OPENED,"uart not opened!\n");

	return write(gpt_uartDev[nIdx].fd,pu8Data, snSize);
}

__EX_API__ int wow_hal_uart_send_timeout(int nIdx, const uint8_t *pu8Data, size_t snSize, int timeout)
{
	CHECK_RET_VAL_P(gpt_uartDev && gpt_uartConfig,-USART_NOT_INITED,"uart env not inited!\n");
	CHECK_RET_VAL_P(nIdx < gpt_uartConfig->num && pu8Data && snSize,-PARAM_INPUT_ARG_INVALID,"param input arg invlaid!\n");
	CHECK_RET_VAL_P(gpt_uartDev[nIdx].opened,-USART_NOT_OPENED,"uart not opened!\n");

	struct timeval tv;
	tv.tv_sec  = timeout/1000;
	tv.tv_usec = timeout%1000*1000;

	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(gpt_uartDev[nIdx].fd, &rfds);
	select(gpt_uartDev[nIdx].fd + 1, &rfds, NULL, NULL, &tv);
	
	return write(gpt_uartDev[nIdx].fd,pu8Data, snSize);
}

static void uart_read_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
    HalUartDev_T *uart = container_of(w,HalUartDev_T,ior);
	if(uart->func){
		uart->func(uart->chan,uart->priv);
	}
}

static void* ev_loop_entry(void *arg)
{
	struct ev_loop *loop = (struct ev_loop*)arg;

	ev_run(loop, 0);

	return NULL;
}

/*brief    串口UART设置接收回调函数
 *param ： nIdx: 串口标识参数
 *param ： fCallback: 接收回调函数
 *param ： pArg: 接收回调函数私有变量
 *return： 成功返回0 失败返回-1
 */
__EX_API__ int wow_hal_uart_set_recv_cb(int nIdx, uart_recv_callback_func_t fCallback,const void* pArg)
{
	CHECK_RET_VAL_P(gpt_uartDev && gpt_uartConfig,-USART_NOT_INITED,"uart env not inited!\n");
	CHECK_RET_VAL_P(nIdx < gpt_uartConfig->num ,-PARAM_INPUT_ARG_INVALID,"param input arg invlaid!\n");
	CHECK_RET_VAL_P(fCallback,-PARAM_INPUT_FUNC_IS_NULL,"param input func invlaid!\n");
	CHECK_RET_VAL_P(gpt_uartDev[nIdx].opened,-USART_NOT_OPENED,"uart not opened!\n");

	
	gpt_uartDev[nIdx].func =  fCallback;
	gpt_uartDev[nIdx].priv =  pArg;
	gpt_uartDev[nIdx].loop =  ev_loop_new(0);
	
	ev_io_init(&gpt_uartDev[nIdx].ior, uart_read_cb, gpt_uartDev[nIdx].fd, EV_READ);
	ev_io_start(gpt_uartDev[nIdx].loop, &gpt_uartDev[nIdx].ior);

	gpt_uartDev[nIdx].pid = wow_thread_create("hal_uart",ev_loop_entry, gpt_uartDev[nIdx].loop);
	CHECK_RET_VAL_P(gpt_uartDev[nIdx].pid > 0, -THREAD_CREATE_FAILED,"wow_thread_create failed!\n");

	return 0;
}




/*brief   串口UART接收数据
 *param ： nIdx: 串口标识参数(从0开始)
 *param ： pu8Data: 待发送接收缓存
 *param ： snSize: 待发送接收大小
 *return： 成功返回接收数据长度 失败返回<0
 */
__EX_API__ int wow_hal_uart_recv(int nIdx, uint8_t *pu8Data, size_t snSize) 
{
	CHECK_RET_VAL_P(gpt_uartDev && gpt_uartConfig,-USART_NOT_INITED,"uart env not inited!\n");
	CHECK_RET_VAL_P(nIdx < gpt_uartConfig->num && pu8Data && snSize,-PARAM_INPUT_ARG_INVALID,"param input arg invlaid!\n");
	CHECK_RET_VAL_P(gpt_uartDev[nIdx].opened,-USART_NOT_OPENED,"uart not opened!\n");

	return read(gpt_uartDev[nIdx].fd,pu8Data, snSize);

}

 __EX_API__ int wow_hal_uart_recv_timeout(int nIdx, uint8_t *pu8Data, size_t snSize,int timeout) 
 {
	CHECK_RET_VAL_P(gpt_uartDev && gpt_uartConfig,-USART_NOT_INITED,"uart env not inited!\n");
	CHECK_RET_VAL_P(nIdx < gpt_uartConfig->num && pu8Data && snSize,-PARAM_INPUT_ARG_INVALID,"param input arg invlaid!\n");
	CHECK_RET_VAL_P(gpt_uartDev[nIdx].opened,-USART_NOT_OPENED,"uart not opened!\n");

	 struct timeval tv;
	 tv.tv_sec	= timeout/1000;
	 tv.tv_usec = timeout%1000*1000;
 
	 fd_set rfds;
	 FD_ZERO(&rfds);
	 FD_SET(gpt_uartDev[nIdx].fd, &rfds);
	 select(gpt_uartDev[nIdx].fd + 1, &rfds, NULL, NULL, &tv);

	 return read(gpt_uartDev[nIdx].fd,pu8Data, snSize);
 
 }

 /*brief    串口UART退出
 *param ： nIdx: 串口标识参数(从0开始)
 *return： 成功返回0 失败返回-1
 */
__EX_API__ int wow_hal_uart_close(int nIdx)
{
	CHECK_RET_VAL_P(gpt_uartDev && gpt_uartConfig,-USART_NOT_INITED,"uart env not inited!\n");
	CHECK_RET_VAL_P(nIdx < gpt_uartConfig->num,-PARAM_INPUT_ARG_INVALID,"param input arg invlaid!\n");
	CHECK_RET_VAL_P(gpt_uartDev[nIdx].opened,-USART_NOT_OPENED,"uart not opened!\n");


	close(gpt_uartDev[nIdx].fd);
	gpt_uartDev[nIdx].fd = -1;
	gpt_uartDev[nIdx].opened = false;
	
	return 0;
}



__IN_API__ int hal_uart_env_init(HalUsartConfig_T* uart)
{
	CHECK_RET_VAL_P(uart,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(uart->num > 0,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

	gpt_uartConfig = uart;
	gpt_uartDev  = CALLOC(uart->num,HalUartDev_T);
	CHECK_RET_VAL_P(gpt_uartDev,-1,"malloc HalUartDev_T failed!\n");
	
	return 0;
}

__IN_API__ int hal_uart_env_exit(void)
{
	FREE(gpt_uartDev);
	gpt_uartDev    = NULL;
	gpt_uartConfig = NULL;
	
	return 0;
}

