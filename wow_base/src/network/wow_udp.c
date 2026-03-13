#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"

#include "system/wow_time.h"
#include "network/wow_socket.h"
#include "network/wow_udp.h"

struct __data_aligned__ udp_t{
	int   sktfd;
	char  local_addr[MAX_SOCKET_IP_SIZE];
	char  peer_addr[MAX_SOCKET_IP_SIZE];
	uint16_t local_port;
	uint16_t peer_port;
};



/* 防止在socket关闭后,write会产生SIGPIPE信号导致进程退出 */
//signal(SIGPIPE, SIG_IGN);

/*brief    创建udp操作符	---客户端/服务端函数
 *param ： pcLoaclAddr:本地IP地址
 *param ： u16LocalPort:本地端口号
 *return： 成功返回udp操作符 失败返回NULL
 */
__EX_API__ Udp_T* wow_udp_open(const char *pLoaclAddr, uint16_t nLocalPort)
{
	Udp_T* udp = CALLOC(1,Udp_T);
	CHECK_RET_VAL_ERRNO_P(udp,-SYSTEM_MALLOC_FAILED,"malloc Udp_T failed!\n");	

	udp->sktfd = wow_socket_open(SOCKET_TYPE_UDP);
	CHECK_RET_GOTO_ERRNO_P(udp->sktfd > 0,out,-SOCKET_OPEN_FAILED,"udp socket create failed!\n");

	int ret = wow_socket_bind(udp->sktfd,pLoaclAddr,nLocalPort);
	CHECK_RET_GOTO_ERRNO_P(ret == 0,out,-SOCKET_BIND_FAILED,"udp socket bind failed!\n");	
	
	if(pLoaclAddr && nLocalPort){
		strcpy(udp->local_addr,pLoaclAddr); 
		udp->local_port = nLocalPort;
	}
	else{
		wow_socket_localName(udp->sktfd,udp->local_addr,&udp->local_port);
	}

	return (Udp_T*)udp;
out:
	wow_udp_close((Udp_T*)udp);
	return NULL;
}


/*brief    udp客户端连接服务端---客户端函数
 *param ： ptUdp:udp操作符
 *param ： pcPeerAddr:服务器IP地址
 *param ： u16PeerPort:服务器端口号
 *return： 成功返回0 失败返回< 0
 */
__EX_API__ int wow_udp_connect(Udp_T* ptUdp,const char *pPeerAddr,uint16_t nPeerPort)
{
	CHECK_RET_VAL_P(ptUdp,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");

	memset(ptUdp->peer_addr,0,MAX_SOCKET_IP_SIZE);
	if(pPeerAddr){
		struct sockaddr_in sa;
		bzero(&sa, sizeof(sa));
		sa.sin_family = AF_INET;
		if (inet_pton(AF_INET,pPeerAddr, &sa.sin_addr) == 0) 
		{
			struct hostent *he;
			he = gethostbyname(pPeerAddr);
			if (he == NULL) return -1;
			memcpy(&sa.sin_addr, he->h_addr, sizeof(struct in_addr));
		}
		strcpy(ptUdp->peer_addr,inet_ntoa(sa.sin_addr)); 
	}
	ptUdp->peer_port = nPeerPort;

	return 0;
}

/*brief    获取udp连接信息---客户端/服务端函数
 *param ： ptUdp:udp操作符
 *param ： pcPeerAddr:连接端IP地址
 *param ： pu16PeerPort:连接端端口号
 *return： 成功返回0 失败返回< 0
 */
__EX_API__ int wow_udp_peer(Udp_T* ptUdp,char *pPeerAddr,uint16_t* nPeerPort)
{
	CHECK_RET_VAL_P(ptUdp,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(pPeerAddr && nPeerPort,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

	strcpy(pPeerAddr,ptUdp->peer_addr); 
	*nPeerPort = ptUdp->peer_port;

	return 0;
}

/*brief    关闭udp操作符---客户端/服务端函数
 *param ： ptUdp:udp操作符
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_udp_close(Udp_T* ptUdp)
{
	CHECK_RET_VAL_P(ptUdp,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(ptUdp->sktfd > 0,-SOCKET_NOT_OPENED,"param input ctrl invalid!\n");

	wow_socket_close(ptUdp->sktfd);
	FREE((void*)ptUdp);

	return 0;
}


/*brief    udp读取数据---客户端/服务端函数
 *param ： ptUdp:udp操作符
 *param ： pu8Data:接收数据内容存储地址
 *param ： snSize:接收数据内容存储长度
 *return： 成功返回接收长度 失败返回 <0
 */
__EX_API__ int wow_udp_read(Udp_T* ptUdp,uint8_t* pu8Data, size_t snSize)
{
	CHECK_RET_VAL_P(ptUdp,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(pu8Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(snSize > 0,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");
	CHECK_RET_VAL_P(ptUdp->sktfd > 0,-SOCKET_NOT_OPENED,"param input ctrl invalid!\n");
		
	return wow_socket_urecv(ptUdp->sktfd,ptUdp->peer_addr,&ptUdp->peer_port,pu8Data,snSize);
}

/*brief    udp写入数据---客户端/服务端函数
 *param ： ptUdp:udp操作符
 *param ： pu8Data:写入数据内容存储地址
 *param ： snSize:写入数据内容存储长度
 *return： 成功返回写入长度 失败返回 <0
 */
__EX_API__ int wow_udp_write(Udp_T* ptUdp,const uint8_t* pu8Data ,size_t snSize)
{
	CHECK_RET_VAL_P(ptUdp,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(pu8Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(snSize > 0,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");
	CHECK_RET_VAL_P(ptUdp->sktfd  > 0,-SOCKET_NOT_OPENED,"param input ctrl invalid!\n");

	return wow_socket_usend(ptUdp->sktfd,ptUdp->peer_addr,ptUdp->peer_port,pu8Data,snSize);
}

/*brief    udp写入数据(超时时间)---客户端/服务端函数
 *param ： ptUdp:udp操作符
 *param ： pu8Data:写入数据内容存储地址
 *param ： snSize:写入数据内容存储长度
 *param ： nMs:超时时间(ms)
 *return： 成功返回写入长度 失败返回 <0
 */
__EX_API__ int wow_udp_write_timeout(Udp_T* ptUdp, const uint8_t *pu8Data, size_t snSize, int nMs)
{
	CHECK_RET_VAL_P(ptUdp,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(pu8Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(snSize > 0,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");
	CHECK_RET_VAL_P(ptUdp->sktfd  > 0,-SOCKET_NOT_OPENED,"param input ctrl invalid!\n");

    int  ret = -1;
    int  slen = 0;
	int64_t  t_left = 0;
	int64_t  t_start = wow_time_get_msec() ; 
    fd_set   sets;
	struct timeval tv,*ptv;
    do {
		if(nMs < 0){
			ptv = NULL;
		}
		else{
			t_left = nMs - (wow_time_get_msec() - t_start);
			CHECK_RET_BREAK(t_left > 0);
			
			tv.tv_sec  = t_left / 1000;
			tv.tv_usec = (t_left % 1000) * 1000;
			ptv = &tv;
		}

        FD_ZERO(&sets);
        FD_SET(ptUdp->sktfd, &sets);
        ret = select(ptUdp->sktfd + 1, NULL, &sets, NULL, ptv);
		CHECK_RET_BREAK_P_A(ret > 0, "select failed!\n");

		/* 检查fd是否可写 */
		CHECK_RET_BREAK_P_A(FD_ISSET(ptUdp->sktfd, &sets) ,"select sockfd not set");

        ret = wow_socket_usend(ptUdp->sktfd,ptUdp->peer_addr,ptUdp->peer_port,
									(pu8Data + slen), snSize - slen);
		CHECK_RET_BREAK_P_A(ret >= 0,"send failed!\n");
        slen += ret;
    } while (slen < snSize);

    return slen;
}


/*此实现方式为获取到数据后退出，由上层组织接受内容*/
/*brief    udp读取数据(超时时间)---客户端/服务端函数
 *param ： ptUdp:udp操作符
 *param ： pu8Data:接收数据内容存储地址
 *param ： snSize:接收数据内容存储长度
 *param ： nMs:超时时间(ms)
 *return： 成功返回接收长度 失败返回-1
 */
__EX_API__ int wow_udp_read_timeout(Udp_T* ptUdp,uint8_t *pu8Data, size_t snSize, int nMs)
{
	CHECK_RET_VAL_P(ptUdp,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(pu8Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(snSize > 0,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");
	CHECK_RET_VAL_P(ptUdp->sktfd  > 0,-SOCKET_NOT_OPENED,"param input ctrl invalid!\n");

    int   ret = -1;
    fd_set   sets;
	struct timeval tv;
			
	tv.tv_sec  = nMs / 1000;
	tv.tv_usec = (nMs % 1000) * 1000;

	FD_ZERO(&sets);
	FD_SET(ptUdp->sktfd, &sets);
	ret = select(ptUdp->sktfd + 1, &sets, NULL, NULL, &tv);
	//CHECK_RET_VAL_P(ret >= 0,-SYSTEM_SELECT_FAILED, "select failed!\n");
	CHECK_RET_VAL(ret > 0,0);

	/* 检查fd是否可读 */
	CHECK_RET_VAL_P_A(FD_ISSET(ptUdp->sktfd, &sets) ,-SOCKET_READ_NO_DATA, "select sockfd not set");

	return wow_socket_urecv(ptUdp->sktfd, ptUdp->peer_addr,&ptUdp->peer_port,pu8Data, snSize);
}

__EX_API__ int wow_udp_joinMulticast(Udp_T* ptUdp, char *group)
{
    int ret = -1;
    int loop  = 1;
	
	CHECK_RET_VAL_P(ptUdp && ptUdp->sktfd > 0 && group ,-1,"param input is NULL!\n");

    ret = setsockopt(ptUdp->sktfd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
	CHECK_RET_VAL_P_A(ret == 0, -SOCKET_SET_OPT_FAILED,  "setsockopt IP_MULTICAST_LOOP failed\r\n");


    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(group);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY); /*default networt interface*/

    /*join to the mutilcast group*/
    ret = setsockopt(ptUdp->sktfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
    CHECK_RET_VAL_P_A(ret == 0, -SOCKET_SET_OPT_FAILED,  "setsockopt IP_ADD_MEMBERSHIP failed\r\n");

    return 0;
}

