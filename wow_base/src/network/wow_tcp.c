#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>

#include "ev.h"

#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"

#include "system/wow_lock.h"
#include "system/wow_cond.h"
#include "system/wow_thread.h"
#include "system/wow_time.h"
#include "system/wow_countdown.h"
#include "network/wow_socket.h"
#include "network/wow_tcp.h"

#define TCP_CONNECT_CONNECT_TIMEOUT (5000)
typedef struct{
	char  	 addr[MAX_SOCKET_IP_SIZE];
	uint16_t port;
	uint16_t back;
}__data_aligned__ TcpIpPortInfo_T;

typedef struct{
	int   			  inited;
	tcp_ctx_callback_func_t  callback;	
	void*			  arg;
	pthread_idx_t  	  thread;
	mutex_lock_t	  lock;
	mutex_cond_t 	  cond;
}__data_aligned__ TcpCallbackInfo_T;

struct __data_aligned__ tcp_t{
	int   sktfd;
	CountDown_T*    cdown;
	TcpIpPortInfo_T local;
	TcpIpPortInfo_T peer;
	struct ev_io 	   ior;			//读事件
    struct ev_loop     *loop;
	TcpCallbackInfo_T* read;
	TcpCallbackInfo_T* accept;
};

/*brief    创建tcp操作符	---客户端/服务端函数
 *param ： pcLoaclAddr:本地IP地址
 *param ： u16LocalPort:本地端口号
 *return： 成功返回tcp操作符 失败返回NULL
 */
__EX_API__ Tcp_T* wow_tcp_open(const char *pcLoaclAddr, uint16_t u16LocalPort)
{
	Tcp_T* tcp = CALLOC(1,Tcp_T);
	CHECK_RET_VAL_ERRNO_P(tcp,-SYSTEM_MALLOC_FAILED,"malloc UdpSkt_T failed!\n");	

	tcp->cdown = wow_cdown_new();
	CHECK_RET_GOTO_ERRNO_P(tcp->cdown,out,-CDOWN_CREATE_FAILED,"wow_cdown_new failed!\n");
	
	tcp->sktfd = wow_socket_open(SOCKET_TYPE_TCP);
	CHECK_RET_GOTO_ERRNO_P(tcp->sktfd > 0,out,-SOCKET_OPEN_FAILED,"wow_socket_open failed!\n");

	int ret = wow_socket_bind(tcp->sktfd,pcLoaclAddr,u16LocalPort);
	CHECK_RET_GOTO_ERRNO_P(ret == 0,out,-SOCKET_BIND_FAILED,"wow_socket_bind failed!\n");	

	if(pcLoaclAddr && u16LocalPort){
		strcpy(tcp->local.addr,pcLoaclAddr); 
		tcp->local.port = u16LocalPort;
	}
	else{
		wow_socket_localName(tcp->sktfd,tcp->local.addr,&tcp->local.port);
	}

	return (Tcp_T*)tcp;
out:
	wow_tcp_close((Tcp_T*)tcp);
	return NULL;
}

/*brief    tcp客户端连接服务端---客户端函数
 *param ： ptTcp:tcp操作符
 *param ： pcPeerAddr:本地IP地址
 *param ： u16PeerPort:本地端口号
 *return： 成功返回0 失败返回-1
 */
__EX_API__ int wow_tcp_connect(Tcp_T* ptTcp,const char *pcPeerAddr,uint16_t u16PeerPort)
{
	CHECK_RET_VAL_P(ptTcp,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(pcPeerAddr,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(u16PeerPort > 0,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");
	CHECK_RET_VAL_P(ptTcp->sktfd  > 0,-SOCKET_NOT_OPENED,"param input ctrl invalid!\n");

	int ret = wow_socket_connect(ptTcp->sktfd,pcPeerAddr,u16PeerPort,TCP_CONNECT_CONNECT_TIMEOUT);
	CHECK_RET_VAL(ret == 0, -1);
	
	memset(ptTcp->peer.addr,0,MAX_SOCKET_IP_SIZE);
	if(pcPeerAddr){
		strcpy(ptTcp->peer.addr,pcPeerAddr); 
	}
	ptTcp->peer.port = u16PeerPort;

	return WOW_SUCCESS;
}

/*brief    获取tcp连接信息---客户端/服务端函数
 *param ： ptTcp:tcp操作符
 *param ： pcPeerAddr:连接端IP地址
 *param ： pu16PeerPort:连接端端口号
 *return： 成功返回0 失败返回-1
 */
__EX_API__ int wow_tcp_peer(Tcp_T* ptTcp,char *pcPeerAddr,uint16_t* pu16PeerPort)
{
	CHECK_RET_VAL_P(ptTcp,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(pcPeerAddr && pu16PeerPort,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

	strcpy(pcPeerAddr,ptTcp->peer.addr); 
	*pu16PeerPort = ptTcp->peer.port;

	return WOW_SUCCESS;
}

/*brief    tcp服务端设置监听个数	---服务端函数
 *param ： ptTcp:tcp操作符
 *param ： u16Max:监听个数
 *return： 成功返回0 失败返回-1
 */
__EX_API__ int wow_tcp_listen(Tcp_T* ptTcp,uint16_t u16Max)
{
	CHECK_RET_VAL_P(ptTcp,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(ptTcp->sktfd  > 0,-SOCKET_NOT_OPENED,"param input ctrl invalid!\n");


    return wow_socket_listen(ptTcp->sktfd,u16Max);
}

static void* tcp_accept_thread(void *arg)  
{   
	int 	 sktfd = 0;
	char     addr[MAX_SOCKET_IP_SIZE];
	uint16_t port;
	Tcp_T* ptTcp = (Tcp_T*)arg;
	CHECK_RET_VAL(ptTcp && ptTcp->accept && ptTcp->accept->callback,NULL);
	
	while(ptTcp->accept->inited)
	{
		//socket close 退出阻塞
		sktfd = wow_socket_accept(ptTcp->sktfd,addr,&port);
		CHECK_RET_BREAK(sktfd >= 0);
		
		Tcp_T* client = CALLOC(1,Tcp_T);
		CHECK_RET_BREAK_P(client,"malloc UdpSkt_T failed!\n"); 
		
		client->sktfd      = sktfd;
		client->peer.port  = port;
		memcpy(client->peer.addr,addr,MAX_SOCKET_IP_SIZE);

		ptTcp->accept->callback((Tcp_T*)client,ptTcp->accept->arg);
		memset(addr,0,MAX_SOCKET_IP_SIZE);
	} 

	wow_mutex_lock(&ptTcp->accept->lock);
	wow_mutex_cond_signal(&ptTcp->accept->cond);
	wow_mutex_unlock(&ptTcp->accept->lock);	

	return NULL;	
} 

/*brief    设置tcp服务端设置接收回调	---服务端函数
 *param ： ptTcp:tcp操作符
 *param ： fCallback:接收客户端后服务端回调函数
 *param ： pArg:接收客户端后服务端回调函数私有变量
 *return： 成功返回0 失败返回-1
 */
__EX_API__ int wow_tcp_accpet_cb(Tcp_T* ptTcp,tcp_ctx_callback_func_t fCallback,void* pArg)
{
	CHECK_RET_VAL_P(ptTcp,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(fCallback,-PARAM_INPUT_FUNC_IS_NULL,"param input func invalid!\n");

	ptTcp->accept = CALLOC(1, TcpCallbackInfo_T);
	CHECK_RET_VAL_P(ptTcp->accept,-SYSTEM_MALLOC_FAILED,"malloc TcpCallbackInfo_T failed!\n");
	
	ptTcp->accept->callback = fCallback;
	ptTcp->accept->arg      = pArg;

	ptTcp->accept->inited = 1;	
	ptTcp->accept->thread = wow_thread_create("tcp_accept",tcp_accept_thread,(void *)ptTcp);
	CHECK_RET_GOTO_P(ptTcp->accept->thread,out,"wow_thread_create is failed\n");
	
	wow_mutex_lock_init(&ptTcp->accept->lock);
	wow_mutex_cond_init(&ptTcp->accept->cond);

	return WOW_SUCCESS;
out:
	FREE(ptTcp->accept);
	return -THREAD_CREATE_FAILED;
}

/*brief    tcp服务端接收客户端连接---服务端函数
 *param ： ptTcp:tcp操作符
 *return： 成功返回tcp操作符 失败返回NULL
 */
__EX_API__ Tcp_T* wow_tcp_accpet(Tcp_T* ptTcp)
{
	CHECK_RET_VAL_P(ptTcp,NULL,"param input invalid!\n");

	char     addr[MAX_SOCKET_IP_SIZE];
	uint16_t port;

	int sktfd= wow_socket_accept(ptTcp->sktfd,addr,&port);
	CHECK_RET_VAL(sktfd > 0,NULL);
		
	Tcp_T* client = CALLOC(1,Tcp_T);
	CHECK_RET_VAL_P(client,NULL,"malloc UdpSkt_T failed!\n"); 
	
	client->sktfd      = sktfd;
	client->peer.port  = port;
	memcpy(client->peer.addr,addr,MAX_SOCKET_IP_SIZE);
		
	return (Tcp_T*)client;
}

static void tcp_read_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
    Tcp_T *ptTcp = container_of(w,Tcp_T,ior);
	CHECK_RET_VOID(ptTcp && ptTcp->read && ptTcp->read->callback);


	int ret = ptTcp->read->callback((Tcp_T*)ptTcp,ptTcp->read->arg);
	CHECK_RET_VOID(ret != 0);

	if(ptTcp->loop){
		ev_io_stop(ptTcp->loop, &ptTcp->ior);
		ev_break(ptTcp->loop,EVBREAK_ALL);
	}
	
}

static void* ev_read_loop_entry(void *arg)
{
	Tcp_T *ptTcp = (Tcp_T*)arg;
	CHECK_RET_VAL(ptTcp,NULL);
	
	ptTcp->loop = ev_loop_new(0);
	ev_io_init(&ptTcp->ior, tcp_read_cb, ptTcp->sktfd, EV_READ);
	ev_io_start(ptTcp->loop, &ptTcp->ior);

	ptTcp->read->inited = 1;
	while(ptTcp->read->inited){
		ev_run(ptTcp->loop, EVRUN_NOWAIT);
		usleep(5*1000);
	}

	wow_mutex_lock(&ptTcp->read->lock);
	wow_mutex_cond_signal(&ptTcp->read->cond);
	wow_mutex_unlock(&ptTcp->read->lock);

	return NULL;
}

/*brief    设置tcp读取回调函数---客户端/服务端函数(优化)
 *param ： ptTcp:tcp操作符
 *param ： fCallback:接收数据回调函数
 *param ： pArg:接收数据回调函数私有变量
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_tcp_read_cb(Tcp_T* ptTcp,tcp_ctx_callback_func_t fCallback,void* pArg)
{	
	CHECK_RET_VAL_P(ptTcp,-PARAM_INPUT_FUNC_IS_NULL,"param input func invalid!\n");

	ptTcp->read = CALLOC(1, TcpCallbackInfo_T);
	CHECK_RET_VAL_P(ptTcp->read,-SYSTEM_MALLOC_FAILED,"malloc TcpCallbackInfo_T failed!\n");
	
	ptTcp->read->callback = fCallback;
	ptTcp->read->arg      = pArg;

	ptTcp->read->inited = 0;	
	ptTcp->read->thread = wow_thread_create("tcp_read",ev_read_loop_entry,(void *)ptTcp);
	CHECK_RET_GOTO_P(ptTcp->read->thread,out,"wow_thread_create is failed\n");

	while(!ptTcp->read->inited) sleep(0); 
		
	wow_mutex_lock_init(&ptTcp->read->lock);
	wow_mutex_cond_init(&ptTcp->read->cond);

	return WOW_SUCCESS;
out:
	FREE(ptTcp->read);
	return -THREAD_CREATE_FAILED;
}

/*brief    tcp读取数据---客户端/服务端函数
 *param ： ptTcp:tcp操作符
 *param ： pu8Data:接收数据内容存储地址
 *param ： snSize:接收数据内容存储长度
 *return： 成功返回接收长度 失败返回<0
 */
__EX_API__ int wow_tcp_read(Tcp_T* ptTcp,uint8_t* pu8Data, size_t snSize)
{
	CHECK_RET_VAL_P(ptTcp,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(pu8Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(snSize > 0,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");
	CHECK_RET_VAL_P(ptTcp->sktfd  > 0,-SOCKET_NOT_OPENED,"param input ctrl invalid!\n");

		
	return wow_socket_trecv(ptTcp->sktfd,pu8Data,snSize);
}

/*brief    tcp写入数据---客户端/服务端函数
 *param ： ptTcp:tcp操作符
 *param ： pu8Data:写入数据内容存储地址
 *param ： snSize:写入数据内容存储长度
 *return： 成功返回写入长度 失败返回<0
 */
__EX_API__ int wow_tcp_write(Tcp_T* ptTcp,const uint8_t* pu8Data ,size_t snSize)
{
	CHECK_RET_VAL_P(ptTcp,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(pu8Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(snSize > 0,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");
	CHECK_RET_VAL_P(ptTcp->sktfd  > 0,-SOCKET_NOT_OPENED,"param input ctrl invalid!\n");


	return wow_socket_tsend(ptTcp->sktfd,pu8Data,snSize);
}

/*brief    tcp写入数据(超时时间)---客户端/服务端函数
 *param ： ptTcp:tcp操作符
 *param ： pu8Data:写入数据内容存储地址
 *param ： snSize:写入数据内容存储长度
 *param ： nMs:超时时间(ms)
 *return： 成功返回写入长度 失败返回<0
 */
__EX_API__ int wow_tcp_write_timeout(Tcp_T* ptTcp, const uint8_t *pu8Data, size_t snSize, int nMs)
{
	CHECK_RET_VAL_P(ptTcp,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(pu8Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(snSize > 0,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");
	CHECK_RET_VAL_P(ptTcp->sktfd  > 0,-SOCKET_NOT_OPENED,"param input ctrl invalid!\n");


    int   ret = -1;
    int   slen = 0;
	
	wow_cdown_reset(ptTcp->cdown);
	wow_cdown_set(ptTcp->cdown,nMs);

    while ((slen < snSize) && (wow_cdown_is_expired(ptTcp->cdown) == 0)){

        ret = wow_socket_tsend(ptTcp->sktfd,pu8Data + slen, snSize - slen);
		CHECK_RET_BREAK_P_A(ret >= 0,"wow_socket_tsend failed!\n");
        slen += ret;
    } 

    return slen;
}

/*brief    tcp读取数据(超时时间)---客户端/服务端函数
 *param ： ptTcp:tcp操作符
 *param ： pu8Data:接收数据内容存储地址
 *param ： snSize:接收数据内容存储长度
 *param ： nMs:超时时间(ms)
 *return： 成功返回接收长度 失败返回<0
 */
__EX_API__ int wow_tcp_read_timeout(Tcp_T* ptTcp, uint8_t *pu8Data, size_t snSize, int nMs)
{
	CHECK_RET_VAL_P(ptTcp,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(pu8Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(snSize > 0,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");
	CHECK_RET_VAL_P(ptTcp->sktfd  > 0,-SOCKET_NOT_OPENED,"param input ctrl invalid!\n");

	int rlen = 0;
	int ret  = 0;

	wow_cdown_reset(ptTcp->cdown);
	wow_cdown_set(ptTcp->cdown,nMs);
	
	do{
		ret = wow_socket_trecv(ptTcp->sktfd, pu8Data + rlen, snSize - rlen);
		if(ret > 0){
			rlen += ret;
		}else if(ret == -1){
			fd_set	 sets;
			struct timeval tv,*ptv;
			if(nMs < 0){
				ptv = NULL;
			}else{
			
				tv.tv_sec  = nMs / 1000;
				tv.tv_usec = (nMs % 1000) * 1000;
				ptv = &tv;
			}
			
			FD_ZERO(&sets);
			FD_SET(ptTcp->sktfd, &sets);
			ret = select(ptTcp->sktfd + 1, &sets, NULL, NULL, ptv);
			CHECK_RET_BREAK(ret > 0);
			
			 /* 检查fd是否可读 */
			CHECK_RET_BREAK_P_A(FD_ISSET(ptTcp->sktfd, &sets) ,"select sockfd not set");
		}else{
			break;
		}
	}while((rlen < snSize)&& (wow_cdown_is_expired(ptTcp->cdown) == 0));
		
    return rlen;
}


/*brief    关闭tcp操作符---客户端/服务端函数
 *param ： ptTcp:tcp操作符
 *return： 无
 */
__EX_API__ void wow_tcp_close(Tcp_T* ptTcp)
{
	CHECK_RET_VOID(ptTcp);

	wow_socket_close(ptTcp->sktfd);
	ptTcp->sktfd = -1;

	if(ptTcp->accept){
		wow_mutex_lock(&ptTcp->accept->lock);
		ptTcp->accept->inited = 0;
		//wow_socket_close已退出线程 不在等待信号
		wow_mutex_cond_wait(&ptTcp->accept->lock,&ptTcp->accept->cond,-1);
		wow_mutex_unlock(&ptTcp->accept->lock);
		
		wow_mutex_lock_exit(&ptTcp->accept->lock);
		wow_mutex_cond_exit(&ptTcp->accept->cond);

		FREE(ptTcp->accept);
	}

	if(ptTcp->read){
		ev_break(ptTcp->loop,EVBREAK_ALL);
		
		wow_mutex_lock(&ptTcp->read->lock);
		ptTcp->read->inited = 0;
		wow_mutex_cond_wait(&ptTcp->read->lock,&ptTcp->read->cond,-1);
		wow_mutex_unlock(&ptTcp->read->lock);
		
		ev_loop_destroy(ptTcp->loop);

		wow_mutex_lock_exit(&ptTcp->read->lock);
		wow_mutex_cond_exit(&ptTcp->read->cond);

		FREE(ptTcp->read);
	}

	wow_cdown_free(&ptTcp->cdown);
	
	FREE((void*)ptTcp);
}

