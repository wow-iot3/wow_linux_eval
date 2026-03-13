#ifndef __WOW_BASE_NETWORK_WOW_SOCKET_H_
#define __WOW_BASE_NETWORK_WOW_SOCKET_H_


#include <stdint.h>
#include <sys/uio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum{
	SOCKET_TYPE_TCP = 0x00,
	SOCKET_TYPE_UDP,
	SOCKET_TYPE_MAX,
}SocketType_E;

typedef enum{
	SOCKET_EVENT_NONE                = 0x0000,
	SOCKET_EVENT_RECV                = 0x0001,
	SOCKET_EVENT_SEND                = 0x0002,
	SOCKET_EVENT_CONN                = SOCKET_EVENT_SEND ,
	SOCKET_EVENT_ACPT                = SOCKET_EVENT_RECV ,
	SOCKET_EVENT_EALL                = SOCKET_EVENT_RECV | SOCKET_EVENT_SEND
}SocketEvent_E;


typedef enum{
	SOCKET_CTRL_SET_BLOCK			 = 0,	//设置阻塞模式	1-阻塞 0-非阻塞	
	SOCKET_CTRL_GET_BLOCK			 = 1,	//获取阻塞模式		---参数int*
	SOCKET_CTRL_SET_RECV_BUFF_SIZE	 = 2,	//设置Socket接收缓存大小
	SOCKET_CTRL_GET_RECV_BUFF_SIZE	 = 3,	//获取Socket接收缓存大小
	SOCKET_CTRL_SET_SEND_BUFF_SIZE	 = 4,	//设置Socket发送缓存大小 针对图像等大数据传输时，可设置此大小
	SOCKET_CTRL_GET_SEND_BUFF_SIZE	 = 5,	//获取Socket发送缓存大小 针对图像等大数据传输时，可设置此大小
	SOCKET_CTRL_SET_TCP_NODELAY 	 = 6,	//设置是否开启Nagle算法
	SOCKET_CTRL_GET_TCP_NODELAY 	 = 7,	//获取是否开启Nagle算法
	SOCKET_CTRL_SET_TCP_KEEPINTVL	 = 8,	//设置两次探测之间的时间间隔
	SOCKET_CTRL_SET_KEEPALIVE		 = 9,	//设置Socket保持连接
	SOCKET_CTRL_MAX
}SocketCtrl_E;

 #define MAX_SOCKET_IP_SIZE 128
 
/* brief    创建socket（默认IPV4）
 * param ： eType  :socket类型
 * return： 成功返回操作标识 失败返回<0
 */
int wow_socket_open(SocketType_E eType);

 /* brief   创建socket对
 * param ： eType  :socket类型
 * param ： pnPair  :存储返回操作标识
 * return： 成功返回0 失败返回<0
 */
int wow_socket_pair(SocketType_E eType, int* pnPair);

/* brief    关闭socket
 * param ： nSktfd  :操作标识
 * return： 无
 */
void wow_socket_close(int nSktfd);

/* brief    skcket连接
 * param ： nSktfd  :操作标识
 * param ： pcAddr  :连接目标地址
 * param ： u16Port :连接目标端口号
 * param ： u32TimeoutMs :连接超时时间(ms)
 * return： 成功返回0 失败返回<0
 */
int wow_socket_connect(int nSktfd,const char *pcAddr, uint16_t u16Port,uint32_t u32TimeoutMs);


/* brief    skcket绑定
 * param ： nSktfd  :操作标识
 * param ： pcAddr  :绑定的IP地址,不指定则为默认本机任意地址
 * param ： u16Port :绑定端口信息
 * return： 成功返回0 失败返回<0
 */
int wow_socket_bind(int nSktfd,const char *pcAddr, uint16_t u16Port);

/* brief    skcket监听
 * param ： nSktfd :操作标识
 * param ： snMaxc :可排队的最大连接个数
 * return： 成功返回0 失败返回<0
 */
int wow_socket_listen(int nSktfd, size_t snMaxc);


/* brief    skcket监听
 * param ： nSktfd :操作标识
 * param ： pcAddr :已连接目标url --可为空
 * param ： u16Port :已连接目标端口号--可为空
 * return： 成功返回客户端操作标识 失败返回<0
 */
int wow_socket_accept(int nSktfd,char* pcAddr, uint16_t* u16Port);


/* brief    接收tcp信息
 * param ： nSktfd :操作标识
 * param ： pu8Data :数据缓存
 * param ： snSize :数据缓存长度
 * return： 成功返回接收长度 失败返回<0
 */
int wow_socket_trecv(int nSktfd, uint8_t* pu8Data, size_t snSize);

/* brief    发送tcp信息
 * param ： nSktfd :操作标识
 * param ： pu8Data :数据缓存
 * param ： snSize :数据缓存长度
 * return： 成功返回发送长度 失败返回<0
 */
int wow_socket_tsend(int nSktfd, const uint8_t* pu8Data, size_t snSize);


/* brief    接收tcp信息
 * param ： nSktfd :操作标识
 * param ： ptList :数据缓存list
 * param ： snSize :数据缓存大小
 * return： 成功返回接收长度 失败返回<0
 */
int wow_socket_trecvv(int nSktfd, struct iovec const* ptList, size_t snSize);

/* brief	发送tcp信息
 * param ： nSktfd :操作标识
 * param ： ptList :数据缓存list
 * param ： snSize :数据缓存大小
 * return： 成功返回发送长度 失败返回<0
 */
int wow_socket_tsendv(int nSktfd, struct iovec const* ptList, size_t snSize);

/* brief    接收udp信息
 * param ： nSktfd :操作标识
 * param ： pcAddr :接收目标地址
 * param ： u16Port :接收目标端口号
 * param ： pu8Data :数据缓存
 * param ： snSize :数据缓存长度
 * return： 成功返回接收长度 失败返回<0
 */
int wow_socket_urecv(int nSktfd, char* pcAddr, uint16_t* u16Port,
								uint8_t* pu8Data, size_t snSize);

/* brief    发送udp信息
 * param ： nSktfd :操作标识
 * param ： pcAddr :发送目标地址
 * param ： u16Port :发送目标端口号
 * param ： pu8Data :数据缓存
 * param ： snSize :数据缓存长度
 * return： 成功返回发送长度 失败返回<0
 */
int wow_socket_usend(int nSktfd, const char* pcAddr, uint16_t u16Port, 
								const uint8_t* pu8Data, size_t snSize);
    

/* brief    接收udp信息
 * param ： nSktfd :操作标识
 * param ： pcAddr :接收目标地址
 * param ： u16Port :接收目标端口号
 * param ： ptList  :数据缓存list
 * param ： snSize  :数据缓存大小
 * return： 成功返回接收长度 失败返回<0
 */
int wow_socket_urecvv(int nSktfd,char* pcAddr, uint16_t* u16Port, 
									struct iovec const* ptList, size_t snSize);

/* brief    发送udp信息
 * param ： nSktfd :操作标识
 * param ： pcAddr :发送目标地址
 * param ： u16Port :发送目标端口号
 * param ： ptList :数据缓存list
 * param ： snSize :数据缓存大小
 * return： 成功返回发送长度 失败返回<0
 */
int wow_socket_usendv(int nSktfd, const char* pcAddr, uint16_t u16Port, 
									struct iovec const* ptList, size_t snSize);


/* brief    设置/获取socket参数
 * param ： nSktfd  :操作标识
 * param ： eCtrl  :控制socket类型
 * param ： ...    :设置为int类型 获取为int*类型
 * return： 成功返回0 失败返回<0
 */
int wow_socket_setopt(int nSktfd, SocketCtrl_E eCtrl, ...);

/* brief    获取连接目标地址信息
 * param ： nSktfd  :操作标识
 * param ： pcAddr  :目标IP地址信息
 * param ： u16Port  :目标IP端口号信息
 * return： 成功返回0 失败返回<0
 */
int wow_socket_peerName(int nSktfd,char *pcAddr, uint16_t* u16Port);

/* brief    获取本地地址信息
 * param ： nSktfd  :操作标识
 * param ： pcAddr  :本地IP地址信息
 * param ： u16Port  :本地IP端口号信息
 * return： 成功返回0 失败返回<0
 */
int wow_socket_localName(int nSktfd,char *pcAddr, uint16_t* u16Port);


#ifdef __cplusplus
}
#endif

#endif
