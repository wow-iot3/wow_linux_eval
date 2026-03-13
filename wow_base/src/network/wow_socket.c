#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <sys/uio.h>
#include <sys/sendfile.h>
#include <sys/poll.h>
#include <sys/un.h>
#include <resolv.h>  

#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"
#include "utils/wow_type.h"

#include "network/wow_socket.h"


/* brief    创建socket（默认IPV4）
 * param ： eType  :socket类型
 * return： 成功返回操作标识 失败返回<0
 */
__EX_API__ int wow_socket_open(SocketType_E eType)
{
	switch(eType){
		case SOCKET_TYPE_TCP:
			return socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		case SOCKET_TYPE_UDP:
			return socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
		default:
			break;
	}
	
	return -PARAM_INPUT_ARG_INVALID;
}

/* brief    关闭socket
 * param ： nSktfd  :操作标识
 * return： 无
 */
__EX_API__ void wow_socket_close(int nSktfd)
{
    CHECK_RET_VOID(nSktfd >= 0);

	shutdown(nSktfd,SHUT_RDWR);
	close(nSktfd);
}

/* brief   创建socket对
* param ： eType  :socket类型
* param ： pnPair	:存储返回操作标识
* return： 成功返回0 失败返回<0
*/
__EX_API__ int wow_socket_pair(SocketType_E eType, int* pnPair)
{
	int fd[2] = {0};
	
	CHECK_RET_VAL_P(eType < SOCKET_TYPE_MAX,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");
	
	switch(eType){
		case SOCKET_TYPE_TCP:
			socketpair(AF_LOCAL,SOCK_STREAM,0, fd); //<unix域协议
			break;
		case SOCKET_TYPE_UDP:
			socketpair(AF_LOCAL,SOCK_DGRAM,0, fd); //<unix域协议
			break;
		case SOCKET_TYPE_MAX:
			return -PARAM_INPUT_ARG_INVALID;
	}
    *pnPair     = fd[0];
    *(pnPair+1) = fd[1];

    return 0;
}

static uint8_t _host_is_ip(const char *host)
{
	uint32_t idx = 0;

	if (strlen(host) >= 16) {
		return 0;
	}

	for (idx = 0; idx < strlen(host); idx++) {
		if ((host[idx] != '.') && (host[idx] < '0' || host[idx] > '9')) {
			return 0;
		}
	}

	return 1;
}

/* brief    socket连接
 * param ： nSktfd  :操作标识
 * param ： pcAddr  :连接目标地址
 * param ： u16Port :连接目标端口号
 * param ： u32TimeoutMs :连接超时时间(ms)
 * return： 成功返回0 失败返回< 0
 */
__EX_API__ int wow_socket_connect(int nSktfd,const char *pcAddr, uint16_t u16Port,uint32_t u32TimeoutMs)
{
	CHECK_RET_VAL_P(nSktfd >= 0 ,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");
	
	int ret = -1;
	
	if(pcAddr && _host_is_ip(pcAddr) == 0){
		int socktype;
		char port[6] = {0};
		struct addrinfo hints, *res, *p;
		socklen_t optlen = sizeof(socktype);

		ret = getsockopt(nSktfd, SOL_SOCKET, SO_TYPE, &socktype, &optlen);
		CHECK_RET_VAL_P(ret == 0,-SYSTEM_GETSOCKOPT_FAILED,"getsockopt failed!\n");	
	
		
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC;		// Allow IPv4 or IPv6
		hints.ai_socktype = socktype;		// Use the determined socket type

		U16toStr(u16Port,port,sizeof(port));

		ret = getaddrinfo(pcAddr, port, &hints, &res);
		CHECK_RET_VAL_P(ret == 0,-SYSTEM_GETSOCKOPT_FAILED,"getsockopt failed!\n");	

		for (p = res; p != NULL; p = p->ai_next) {
			ret = connect(nSktfd, p->ai_addr, p->ai_addrlen);
			if (ret == 0) break;
		}
	}else{
	   struct sockaddr_in saddr;
		bzero(&saddr, sizeof(saddr));
		saddr.sin_family = AF_INET;
		saddr.sin_port = htons(u16Port);
		saddr.sin_addr.s_addr = pcAddr? inet_addr(pcAddr):INADDR_ANY;
		ret = connect(nSktfd, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in));
	}

	if((ret == 0) ||  (ret < 0 && errno == EISCONN)){//EISCONN 已建立连接
		return 0;
	} else if (ret < 0 && errno == EINPROGRESS){//非阻塞模式下
		//判断链接是否成功
		fd_set wset;
    	struct timeval tv;
		tv.tv_sec  = u32TimeoutMs *1000;
		tv.tv_usec = u32TimeoutMs%1000 *1000;
		FD_ZERO(&wset);
		FD_SET(nSktfd, &wset);

		ret = select(nSktfd+1, NULL, &wset, NULL,&tv);
		CHECK_RET_VAL_P_A(ret > 0 ,-SYSTEM_SELECT_FAILED, "select failed!\n");
		if(FD_ISSET(nSktfd, &wset)){
			int err = -1;
			socklen_t len = sizeof(int);
			ret = getsockopt(nSktfd,  SOL_SOCKET, SO_ERROR ,&err, &len);
			CHECK_RET_VAL_P_A(ret == 0 ,-SOCKET_GET_OPT_FAILED, "select sockfd not set");
		}
		return 0;
    }
	
	return -WOW_EXE_FAILED;
}


/* brief    socket绑定
 * param ： nSktfd  :操作标识
 * param ： pcAddr  :绑定的IP地址,不指定则为默认本机任意地址
 * param ： u16Port :绑定端口信息
 * return： 成功返回0 失败返回< 0
 */
__EX_API__ int wow_socket_bind(int nSktfd,const char *pcAddr, uint16_t u16Port)
{
	CHECK_RET_VAL_P(nSktfd >= 0 ,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    int  ret = 0;
	
///<端口地址释放后立即生效 支持端口重载
///<端口复用最常用的用途应该是防止服务器重启时之前绑定的端口还未释放或者程序突然退出而系统没有释放端口
#ifdef SO_REUSEADDR
    int reuseaddr = 1;
	ret = setsockopt(nSktfd, SOL_SOCKET, SO_REUSEADDR, (int *)&reuseaddr, sizeof(reuseaddr));
	CHECK_RET_VAL_P_A(ret == 0,-SOCKET_SET_OPT_FAILED,"reuse addr failed!\n");
#endif

#ifdef SO_REUSEPORT
    int reuseport = 1;
    ret = setsockopt(nSktfd, SOL_SOCKET, SO_REUSEPORT, (int *)&reuseport, sizeof(reuseport));
	CHECK_RET_VAL_P_A(ret == 0,-SOCKET_SET_OPT_FAILED,"reuse port failed!\n");
#endif

    struct sockaddr_in saddr;
    /*为TCP链接设置IP和端口等信息*/
	bzero(&saddr, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = pcAddr? inet_addr(pcAddr):INADDR_ANY;
    saddr.sin_port = htons(u16Port);

	/*地址与socket绑定bind*/
    return bind(nSktfd, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in));
	
}

/* brief    socket监听
 * param ： nSktfd :操作标识
 * param ： snMaxc :可排队的最大连接个数
 * return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_socket_listen(int nSktfd, size_t snMaxc)
{
	CHECK_RET_VAL_P(nSktfd >= 0 ,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

	if(snMaxc == 0){
		char *ptr;
		if((ptr = getenv("LISTENQ")) != NULL){
			snMaxc = atoi(ptr);
		}else{
			snMaxc = 1024;
		}

	}
    return listen(nSktfd, snMaxc);
	
}

/* brief    socket监听
 * param ： nSktfd :操作标识
 * param ： pcAddr :已连接目标url --可为空
 * param ： u16Port :已连接目标端口号--可为空
 * return： 成功返回客户端操作标识 失败返回<0
 */
__EX_API__ int wow_socket_accept(int nSktfd,char *pcAddr, uint16_t* u16Port)
{
	CHECK_RET_VAL_P(nSktfd >= 0 ,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    struct sockaddr_in saddr = {0};
    socklen_t len = sizeof(struct sockaddr_in);

    int fd = accept(nSktfd, (struct sockaddr *)&saddr, &len);
    CHECK_RET_VAL(fd > 0,-SOCKET_ACCEPT_FAILED);
	
///<减少反应延迟
#ifdef TCP_NODELAY	
    int enable = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&enable, sizeof(enable));
#endif

	if(pcAddr){
		strcpy(pcAddr,inet_ntoa(saddr.sin_addr)); 
	}
	if(u16Port){
		*u16Port = ntohs(saddr.sin_port);
	} 

    return fd;
}



/* brief    接收tcp信息
 * param ： nSktfd :操作标识
 * param ： pu8Data :数据缓存
 * param ： snSize :数据缓存长度
 * return： 成功返回接收长度 失败返回<0
 */
__EX_API__ int wow_socket_trecv(int nSktfd, uint8_t* pu8Data, size_t snSize)
{
	CHECK_RET_VAL_P(nSktfd >= 0 && snSize > 0,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");
    CHECK_RET_VAL_P(pu8Data , -PARAM_INPUT_DATA_IS_NULL ,"param input data invalid!\n");

	//read(nSktfd,pu8Data,snSize);
    int real = recv(nSktfd, pu8Data, snSize, MSG_DONTWAIT);

    if (real >= 0) return real;
    if (errno == EINTR || errno == EAGAIN) return real;
	
    return -WOW_EXE_FAILED;
}

/* brief    发送tcp信息
 * param ： nSktfd :操作标识
 * param ： pu8Data :数据缓存
 * param ： snSize :数据缓存长度
 * return： 成功返回发送长度 失败返回<0
 */
__EX_API__ int wow_socket_tsend(int nSktfd,const uint8_t* pu8Data, size_t snSize)
{
	CHECK_RET_VAL_P(nSktfd >= 0 && snSize > 0,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");
    CHECK_RET_VAL_P(pu8Data , -PARAM_INPUT_DATA_IS_NULL ,"param input data invalid!\n");

	//write(nSktfd,pu8Data,snSize);
    int real = send(nSktfd, pu8Data,snSize, MSG_NOSIGNAL);

    if (real >= 0) return real;
    if (errno == EINTR || errno == EAGAIN) return real;
	
    return -WOW_EXE_FAILED;;
}

/* brief    接收udp信息
 * param ： nSktfd :操作标识
 * param ： pcAddr :接收目标地址
 * param ： u16Port :接收目标端口号
 * param ： pu8Data :数据缓存
 * param ： snSize :数据缓存长度
 * return： 成功返回接收长度 失败返回<0
 */
__EX_API__ int wow_socket_urecv(int nSktfd,char *pcAddr, uint16_t* u16Port,uint8_t* pu8Data, size_t snSize)
{
	CHECK_RET_VAL_P(nSktfd >= 0 && snSize > 0,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");
    CHECK_RET_VAL_P(pu8Data && pcAddr && u16Port, -PARAM_INPUT_DATA_IS_NULL ,"param input data invalid!\n");

    struct sockaddr_in saddr = {0};
    socklen_t  len = sizeof(struct sockaddr_in);
    int ret = recvfrom(nSktfd, pu8Data,snSize, 0, (struct sockaddr*)&saddr, &len);

	if (ret >= 0){
        if(pcAddr){
			strcpy(pcAddr,inet_ntoa(saddr.sin_addr)); 
		}
		if(u16Port){
			*u16Port = ntohs(saddr.sin_port);
		} 
        return ret;
    }
    if (errno == EINTR || errno == EAGAIN) return 0;
	
    return -WOW_EXE_FAILED;
}

/* brief    发送udp信息
 * param ： nSktfd :操作标识
 * param ： pcAddr :发送目标地址
 * param ： u16Port :发送目标端口号
 * param ： pu8Data :数据缓存
 * param ： snSize :数据缓存长度
 * return： 成功返回发送长度 失败返回<0
 */
__EX_API__ int wow_socket_usend(int nSktfd,const char *pcAddr, uint16_t u16Port,const uint8_t* pu8Data, size_t snSize)
{
	CHECK_RET_VAL_P(nSktfd >= 0 && snSize > 0,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");
    CHECK_RET_VAL_P(pu8Data && pcAddr && u16Port, -PARAM_INPUT_DATA_IS_NULL ,"param input data invalid!\n");;

	struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr(pcAddr);
    saddr.sin_port = htons(u16Port);

    int ret  = sendto(nSktfd, pu8Data, snSize, 0, (struct sockaddr*)&saddr, sizeof(struct sockaddr_in));
    if (ret  >= 0) return ret;
    if (errno == EINTR || errno == EAGAIN) return ret;
	
    return -WOW_EXE_FAILED;
}

/* brief    接收tcp信息
 * param ： nSktfd :操作标识
 * param ： ptList :数据缓存list
 * param ： snSize :数据缓存大小
 * return： 成功返回接收长度 失败返回<0
 */
__EX_API__ int wow_socket_trecvv(int nSktfd, struct iovec const* ptList, size_t snSize)
{
	CHECK_RET_VAL_P(nSktfd >= 0 && snSize > 0,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");
    CHECK_RET_VAL_P(ptList, -PARAM_INPUT_DATA_IS_NULL ,"param input data invalid!\n");;

    int real = readv(nSktfd, (struct iovec const*)ptList, snSize);

    if (real >= 0) return real;
    if (errno == EINTR || errno == EAGAIN) return real;
	
    return -WOW_EXE_FAILED;
}

/* brief	发送tcp信息
 * param ： nSktfd :操作标识
 * param ： ptList :数据缓存list
 * param ： snSize :数据缓存大小
 * return： 成功返回发送长度 失败返回<0
 */
__EX_API__ int wow_socket_tsendv(int nSktfd, struct iovec const* ptList, size_t snSize)
{
	CHECK_RET_VAL_P(nSktfd >= 0 && snSize > 0,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");
    CHECK_RET_VAL_P(ptList, -PARAM_INPUT_DATA_IS_NULL ,"param input data invalid!\n");;


    int real = writev(nSktfd, (struct iovec const*)ptList, snSize);

    if (real >= 0) return real;
    if (errno == EINTR || errno == EAGAIN) return real;
	
    return -WOW_EXE_FAILED;
}

/* brief    接收udp信息
 * param ： nSktfd :操作标识
 * param ： pcAddr :接收目标地址
 * param ： u16Port :接收目标端口号
 * param ： pList  :数据缓存list
 * param ： snSize  :数据缓存大小
 * return： 成功返回接收长度 失败返回<0
 */
__EX_API__ int wow_socket_urecvv(int nSktfd, char *pcAddr, uint16_t* u16Port, struct iovec const* ptList, size_t snSize)
{
	CHECK_RET_VAL_P(nSktfd >= 0 && snSize > 0,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");
    CHECK_RET_VAL_P(ptList && pcAddr && u16Port, -PARAM_INPUT_DATA_IS_NULL ,"param input data invalid!\n");;


    struct msghdr      msg   = {0};
    struct sockaddr_in saddr = {0};
    msg.msg_name            = (void*)&saddr;
    msg.msg_namelen         = sizeof( struct sockaddr_in);
    msg.msg_iov             = (struct iovec*)ptList;
    msg.msg_iovlen          = snSize;
    msg.msg_control         = NULL;
    msg.msg_controllen      = 0;
    msg.msg_flags           = 0;

    int ret = recvmsg(nSktfd, &msg, 0);
    if (ret >= 0){
        if(pcAddr){
			strcpy(pcAddr,inet_ntoa(saddr.sin_addr)); 
		}
		if(u16Port){
			*u16Port = ntohs(saddr.sin_port);
		} 
        return ret;
    }
    if (errno == EINTR || errno == EAGAIN) return ret;
	
    return -WOW_EXE_FAILED;
}

/* brief    发送udp信息
 * param ： nSktfd :操作标识
 * param ： pcAddr :发送目标地址
 * param ： u16Port :发送目标端口号
 * param ： ptList :数据缓存list
 * param ： snSize :数据缓存大小
 * return： 成功返回发送长度 失败返回<0
 */
__EX_API__ int wow_socket_usendv(int nSktfd,const char *pcAddr, uint16_t u16Port, struct iovec const* ptList, size_t snSize)
{
	CHECK_RET_VAL_P(nSktfd >= 0 && snSize > 0,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");
    CHECK_RET_VAL_P(ptList && pcAddr && u16Port, -PARAM_INPUT_DATA_IS_NULL ,"param input data invalid!\n");;

	struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr(pcAddr);
    saddr.sin_port = htons(u16Port);

    // init msg
    struct msghdr msg = {0};
    msg.msg_name        = (void*)&saddr;
    msg.msg_namelen     = sizeof(struct sockaddr_in);
    msg.msg_iov         = (struct iovec*)ptList;
    msg.msg_iovlen      = snSize;
    msg.msg_control     = NULL;
    msg.msg_controllen  = 0;
    msg.msg_flags       = 0;

    int ret = sendmsg(nSktfd, &msg, 0);
    if (ret >= 0) return ret;
    if (errno == EINTR || errno == EAGAIN) return ret;

    return -WOW_EXE_FAILED;
}

/* brief    设置/获取socket参数
 * param ： nSktfd  :操作标识
 * param ： eCtrl  :控制socket类型
 * param ： ...    :设置为int类型 获取为int*类型
 * return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_socket_setopt(int nSktfd, SocketCtrl_E eCtrl, ...)
{
	CHECK_RET_VAL_P(nSktfd >= 0 && eCtrl < SOCKET_CTRL_MAX,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    va_list args;
    va_start(args, eCtrl);

    switch (eCtrl)
    {
    case SOCKET_CTRL_SET_BLOCK://设置阻塞模式
        {
            int block = (int)va_arg(args, int);
			va_end(args);
			
            if (block){
				fcntl(nSktfd, F_SETFL, fcntl(nSktfd, F_GETFL) & ~O_NONBLOCK);
			} else{
				fcntl(nSktfd, F_SETFL, fcntl(nSktfd, F_GETFL) | O_NONBLOCK);
			} 
        }
        break;
    case SOCKET_CTRL_GET_BLOCK://获取阻塞模式
        {
            int* block = (int*)va_arg(args, int*);
			va_end(args);
			
            CHECK_RET_VAL_P(block, -PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
            *block = (fcntl(nSktfd, F_GETFL) & O_NONBLOCK)? 0 : 1;
        }
        break;
    case SOCKET_CTRL_SET_TCP_NODELAY://设置 nodelay 功能 1 表示打开，0 表示关闭
        {
            int nagle = (int)va_arg(args, int);
			va_end(args);
			int ret = setsockopt(nSktfd, IPPROTO_TCP, TCP_NODELAY, (char*)&nagle, sizeof(int));
			CHECK_RET_VAL_P_A(ret == 0, -SOCKET_SET_OPT_FAILED,"setsockopt false!\n");
        }
        break;
    case SOCKET_CTRL_GET_TCP_NODELAY://获取 nodelay 功能
        {
            // the penable
            int* nagle = (int*)va_arg(args, int*);
			va_end(args);
            CHECK_RET_VAL_P(nagle, -PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

			
            int enable = 0;
            socklen_t len = sizeof(enable);
			int ret = getsockopt(nSktfd, IPPROTO_TCP, TCP_NODELAY, (char*)&enable, &len);
			CHECK_RET_VAL_P_A(ret == 0, -SOCKET_GET_OPT_FAILED,"getsockopt false!\n");
			
			*nagle = enable;
        }
        break;
    case SOCKET_CTRL_SET_RECV_BUFF_SIZE://设置读缓冲区大小
        {
            int size = (int)va_arg(args, int);
			va_end(args);
			
			int ret = setsockopt(nSktfd, SOL_SOCKET, SO_RCVBUF, (char*)&size, sizeof(int));
			CHECK_RET_VAL_P_A(ret == 0, -SOCKET_SET_OPT_FAILED,"setsockopt false!\n");
        }
        break;
    case SOCKET_CTRL_GET_RECV_BUFF_SIZE://获取读缓冲区大小
        {
            int* size = (int*)va_arg(args, int*);
		    va_end(args);
            CHECK_RET_VAL_P(size, -PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

            int  real = 0;
            socklen_t len = sizeof(real);
			int ret = getsockopt(nSktfd, SOL_SOCKET, SO_RCVBUF, (char*)&real, &len);
            CHECK_RET_VAL_P_A(ret == 0, -SOCKET_GET_OPT_FAILED,"getsockopt false!\n");

			*size = real;
        }
        break;
    case SOCKET_CTRL_SET_SEND_BUFF_SIZE://设置写缓冲区大小 
        {
        	int size = (int)va_arg(args, int);
			va_end(args);
			
			int ret = setsockopt(nSktfd, SOL_SOCKET, SO_SNDBUF, (char*)&size, sizeof(int));
			CHECK_RET_VAL_P_A(ret == 0, -SOCKET_SET_OPT_FAILED,"setsockopt false!\n");
        }
        break;
    case SOCKET_CTRL_GET_SEND_BUFF_SIZE://获取写缓冲区大小 
        {
			int* size = (int*)va_arg(args, int*);
			va_end(args);
			CHECK_RET_VAL_P(size, -PARAM_INPUT_DATA_IS_NULL,"param data input invalid!\n");

			int  real = 0;
			socklen_t len = sizeof(real);
			int ret = getsockopt(nSktfd, SOL_SOCKET, SO_SNDBUF, (char*)&real, &len);
			CHECK_RET_VAL_P_A(ret == 0, -SOCKET_GET_OPT_FAILED,"getsockopt false!\n");

			*size = real;
        }
        break;
    case SOCKET_CTRL_SET_KEEPALIVE:
        {
            int enable = (int)va_arg(args, int);
			va_end(args);
			
            int ret = setsockopt(nSktfd, SOL_SOCKET, SO_KEEPALIVE, (char*)&enable, sizeof(int));
            CHECK_RET_VAL_P_A(ret == 0, -SOCKET_SET_OPT_FAILED,"setsockopt false!\n");
        }
        break;
    case SOCKET_CTRL_SET_TCP_KEEPINTVL:
        {
            int intvl = (int)va_arg(args, size_t);
			va_end(args);
			
            int ret = setsockopt(nSktfd, IPPROTO_TCP, TCP_KEEPINTVL, (char*)&intvl, sizeof(int));
			CHECK_RET_VAL_P_A(ret == 0, -SOCKET_SET_OPT_FAILED,"setsockopt false!\n");
        }
        break;
    default:
        return -WOW_EXE_FAILED;
    }


    return 0;
}

/* brief    获取连接目标地址信息
 * param ： nSktfd  :操作标识
 * param ： pcAddr  :目标IP地址信息
 * param ： u16Port  :目标IP端口号信息
 * return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_socket_peerName(int nSktfd,char *pcAddr, uint16_t* u16Port)
{
	int ret = -1;
	
	CHECK_RET_VAL_P(nSktfd >= 0,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");
    CHECK_RET_VAL_P(pcAddr && u16Port, -PARAM_INPUT_DATA_IS_NULL ,"param input data invalid!\n");


	struct sockaddr_in saddr;
	socklen_t len = sizeof(struct sockaddr_in);
	memset(&saddr, 0, len);

	ret = getpeername(nSktfd , (struct sockaddr *)&saddr, &len);
	CHECK_RET_VAL_P_A(ret==0,-SYSTEM_GETPEERNAME_FAILED,"getpeername false!");

	strcpy(pcAddr,inet_ntoa(saddr.sin_addr)); 
	*u16Port = ntohs(saddr.sin_port);

	return 0;
}

/* brief    获取本地地址信息
 * param ： nSktfd  :操作标识
 * param ： pcAddr  :本地IP地址信息
 * param ： u16Port  :本地IP端口号信息
 * return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_socket_localName(int nSktfd,char *pcAddr, uint16_t* u16Port)
{
	CHECK_RET_VAL_P(nSktfd >= 0,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");
    CHECK_RET_VAL_P(pcAddr && u16Port, -PARAM_INPUT_DATA_IS_NULL ,"param input data invalid!\n");

    struct sockaddr_in saddr = {0};
    socklen_t len = sizeof(struct sockaddr_in);
   	int  ret = getsockname(nSktfd, (struct sockaddr *)&saddr,&len);
	CHECK_RET_VAL_P_A(ret == 0,-SYSTEM_GETPEERNAME_FAILED,"getsockname fasle!\n");
	
	strcpy(pcAddr,inet_ntoa(saddr.sin_addr)); 
	*u16Port = ntohs(saddr.sin_port);

    return 0;
}

