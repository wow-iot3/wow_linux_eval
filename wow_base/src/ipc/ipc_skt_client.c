#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

#include "ev.h"
#include "system/wow_lock.h"
#include "system/wow_cond.h"
#include "system/wow_thread.h"

#include "ipc_internal.h"


typedef struct{
	IpcCtx_T base;
	int sktfd;
	IpcPacket_t *s_packet;
	IpcPacket_t *r_packet;
	ipc_inner_recv_fun_t  skt_recv_cb;
	void* priv;
	
	int inited;
	sem_cond_t sem;	
	pthread_idx_t  pid;
	struct ev_io ior;			//读事件
	struct ev_loop *loop;
	mutex_lock_t send_lock;
	mutex_lock_t recv_lock;
	mutex_lock_t loop_lock;
	mutex_cond_t loop_cond;
}CliSktCtx_T;


static void client_recv_callback(struct ev_loop *loop, struct ev_io *w, int revents)
{
	int rlen    = -1;

    CliSktCtx_T *ctx = container_of(w,CliSktCtx_T,ior);
	CHECK_RET_VOID(ctx);

	//此处不处理断包 ----应用端使用不规范或网络断包则处理失败
	wow_mutex_lock(&ctx->recv_lock);
	rlen = recv(w->fd,ctx->r_packet,sizeof(IpcPacket_t) + MAX_IPC_MESSAGE_SIZE, 0);
	wow_mutex_unlock(&ctx->recv_lock);
	CHECK_RET_VOID(rlen >= sizeof(IpcPacket_t));
	
    switch (ctx->r_packet->header.flag) {
    case IPC_CMD_BIND: 
 		wow_sem_cond_signal(&(ctx->sem));
		memset((void*)ctx->r_packet,0,sizeof(IpcPacket_t) + MAX_IPC_MESSAGE_SIZE);
        break;
    case IPC_CMD_UNBIND:
		wow_sem_cond_signal(&(ctx->sem));
		memset((void*)ctx->r_packet,0,sizeof(IpcPacket_t) + MAX_IPC_MESSAGE_SIZE);
        break;
    case IPC_CMD_COMM:
    default:
		CHECK_RET_VOID(ctx->skt_recv_cb);
		wow_mutex_lock(&ctx->recv_lock);
		if(0 == ctx->skt_recv_cb((IpcCtx_T*)ctx,ctx->r_packet->header.func_id, 
							ctx->r_packet->payload, rlen-sizeof(IpcPacket_t),ctx->priv)){
			memset(ctx->r_packet,0,sizeof(IpcPacket_t) + MAX_IPC_MESSAGE_SIZE);
		}
		wow_mutex_unlock(&ctx->recv_lock);					
        break;
		
    }
}


static void* ev_loop_entry(void *arg)
{
	CliSktCtx_T *ctx = (CliSktCtx_T *)arg;

	ctx->loop = ev_loop_new(EVFLAG_AUTO);;
	ev_io_init(&ctx->ior, client_recv_callback, ctx->sktfd, EV_READ);
	ev_io_start(ctx->loop, &ctx->ior);
	
	ctx->inited = 1;
	while(ctx->inited){
		ev_run(ctx->loop, EVRUN_NOWAIT);//EVBREAK_ONE EVBREAK_ALL
		usleep(5*1000);
	}

	wow_mutex_lock(&ctx->loop_lock);
	wow_mutex_cond_signal(&ctx->loop_cond);
	wow_mutex_unlock(&ctx->loop_lock);

	return NULL;
}

static int ipc_skt_open(CliSktCtx_T* ctx)
{
	int ret = -1;
	char stub_name[256];
	struct sockaddr_un    sockaddr;
	
	ctx->sktfd = socket(PF_UNIX, SOCK_SEQPACKET, 0);
	CHECK_RET_VAL_P(ctx->sktfd >= 0, -1, "socket failed: %s\n", strerror(errno));

	snprintf(stub_name, sizeof(stub_name), "%s.%d", IPC_SERVER_NAME, ctx->base.info.serId);
	snprintf(sockaddr.sun_path, sizeof(sockaddr.sun_path), "/tmp/%s", stub_name);
	sockaddr.sun_family = PF_UNIX;
	
	ret = connect(ctx->sktfd, (struct sockaddr*)&sockaddr, sizeof(struct sockaddr_un));
	CHECK_RET_GOTO_P(ret == 0, out, "connect %s failed: %s\n", stub_name, strerror(errno));

	return 0;
out:
	close(ctx->sktfd);
	return -1;

}

static int ipc_skt_send_bind(CliSktCtx_T* ctx)
{
	int ret = 0;
		
	//发送连接信息
	ctx->s_packet->header.flag		  = IPC_CMD_BIND;
	ctx->s_packet->header.func_id     = BUILD_IPC_MSG_ID(ctx->base.info.serId,
												ctx->base.info.cliId,0xFFFF);
	ctx->s_packet->header.payload_len = 0;

	ret = send(ctx->sktfd,ctx->s_packet, sizeof(IpcPacket_t), 0);
	CHECK_RET_VAL_P(ret == sizeof(IpcPacket_t),-1,"send bind info failed!\n");
	
	ret = wow_sem_cond_wait(&ctx->sem,IPC_COMM_DEF_TIMEOUT);
	CHECK_RET_VAL_P(ret == 0,-1,"wait bind ack failed!\n");

	return 0;
}

static int ipc_skt_send_unbind(CliSktCtx_T* ctx)
{
	int ret = 0;
		
	//发送连接信息
	ctx->s_packet->header.flag		  = IPC_CMD_UNBIND;
	ctx->s_packet->header.func_id     = BUILD_IPC_MSG_ID(ctx->base.info.serId,
												ctx->base.info.cliId,0xFFFF);
	ctx->s_packet->header.payload_len = 0;

	ret = send(ctx->sktfd,ctx->s_packet, sizeof(IpcPacket_t), 0);
	CHECK_RET_VAL_P(ret == sizeof(IpcPacket_t),-1,"send unbind info failed!\n");

	return 0;
}

static void ipc_skt_exit(IpcCtx_T *self)
{    
	CliSktCtx_T *ctx = (CliSktCtx_T *)self; 
	CHECK_RET_VOID(ctx);

	//发送解绑消息
	if(ctx->sktfd > 0){
		ipc_skt_send_unbind(ctx);
		close(ctx->sktfd);
	}

	ctx->inited = 0;
	if(ctx->loop){
		ev_break(ctx->loop,EVBREAK_ONE);
	
		wow_mutex_lock(&ctx->loop_lock);
		wow_mutex_cond_wait(&ctx->loop_lock,&ctx->loop_cond,-1);
		wow_mutex_unlock(&ctx->loop_lock);
		
		ev_loop_destroy(ctx->loop);
	}
	
	wow_sem_cond_exit(&ctx->sem);
	wow_mutex_lock_exit(&ctx->loop_lock);
	wow_mutex_cond_exit(&ctx->loop_cond);
	wow_mutex_lock_exit(&ctx->send_lock);
	wow_mutex_lock_exit(&ctx->recv_lock);

	if(ctx->s_packet){
		FREE(ctx->s_packet);
	}
	
	if(ctx->r_packet){
		FREE(ctx->r_packet);
	}

	FREE(ctx);	
}

static int ipc_skt_set_recv_cb(IpcCtx_T *self, ipc_inner_recv_fun_t cb,void* priv)
{
	CliSktCtx_T *ctx = (CliSktCtx_T *)self;
	CHECK_RET_VAL_P(ctx,-1,"param input invalid!\n");
	
    ctx->skt_recv_cb = cb;
	ctx->priv = priv;
	
    return 0;
}


static int ipc_skt_send(IpcCtx_T *self,uint32_t func_id, const void *buf, size_t len)
{
	int size = 0;

	CliSktCtx_T *ctx = (CliSktCtx_T *)self;	
	CHECK_RET_VAL_P(ctx,-1,"param input invalid!\n");

	wow_mutex_lock(&ctx->send_lock);
	ctx->s_packet->header.flag		   = IPC_CMD_COMM;
	ctx->s_packet->header.func_id 	   = func_id;
	ctx->s_packet->header.payload_len  = len;
	memcpy(ctx->s_packet->payload, buf, len);

    size = send(ctx->sktfd, (const void*)ctx->s_packet, len + sizeof(IpcHeader_T), 0);
    wow_mutex_unlock(&ctx->send_lock);

	return (size - sizeof(IpcHeader_T));
}

static int ipc_skt_recv(IpcCtx_T *self,uint32_t func_id, void *buf, size_t len)
{
	int size = 0;

	CliSktCtx_T *ctx = (CliSktCtx_T *)self;	
	CHECK_RET_VAL_P(ctx ,-1,"param input invalid!\n");
	
	CHECK_RET_VAL(ctx->r_packet->header.payload_len > 0, -1);
	
	wow_mutex_lock(&ctx->recv_lock);
	size = MIN2(len,ctx->r_packet->header.payload_len);
	memcpy(buf,ctx->r_packet->payload,size);
	memset((void*)ctx->r_packet,0,sizeof(IpcPacket_t) + MAX_IPC_MESSAGE_SIZE);
	wow_mutex_unlock(&ctx->recv_lock);

	return size;

}

static int ipc_skt_broadcast(IpcCtx_T *self,uint16_t mod_id, const void *buf, size_t len)
{
	CliSktCtx_T *ctx = (CliSktCtx_T *)self;	
	CHECK_RET_VAL_P(ctx,-1,"param input invalid!\n");

	uint32_t func_id = BUILD_IPC_MSG_ID(ctx->base.info.serId,ctx->base.info.cliId,mod_id);

	wow_mutex_lock(&ctx->send_lock);
	ctx->s_packet->header.flag		   = IPC_CMD_COMM;
	ctx->s_packet->header.func_id 	   = func_id;
	ctx->s_packet->header.payload_len  = len;
	memcpy(ctx->s_packet->payload, buf, len);

    send(ctx->sktfd, (const void*)ctx->s_packet, len + sizeof(IpcHeader_T), 0);
	wow_mutex_unlock(&ctx->send_lock);

	return 0;
}


IpcCtx_T* ipc_skt_client_init(IpcInfo_T  * info)
{
	int ret = 0;
	
	CliSktCtx_T* ctx = CALLOC(1, CliSktCtx_T);
	CHECK_RET_VAL_P(ctx,NULL,"malloc SktCtx_T failed!\n");
	memcpy(&ctx->base.info,info,sizeof(IpcInfo_T));

	ctx->base.exit        = ipc_skt_exit;
	ctx->base.set_recv_cb = ipc_skt_set_recv_cb;
	ctx->base.send        = ipc_skt_send;
    ctx->base.recv        = ipc_skt_recv;
    ctx->base.broadcast   = ipc_skt_broadcast;

	wow_sem_cond_init(&ctx->sem);
	wow_mutex_lock_init(&ctx->recv_lock);
	wow_mutex_lock_init(&ctx->send_lock);
	wow_mutex_lock_init(&ctx->loop_lock);
	wow_mutex_cond_init(&ctx->loop_cond);
	
	ctx->s_packet = (IpcPacket_t *)MALLOC(sizeof(IpcPacket_t) + MAX_IPC_MESSAGE_SIZE);
	CHECK_RET_GOTO_P(ctx->s_packet,out,"malloc IpcPacket_t failed!\n")
	memset(ctx->s_packet,0,sizeof(IpcPacket_t) + MAX_IPC_MESSAGE_SIZE);
	
	ctx->r_packet = (IpcPacket_t *)MALLOC(sizeof(IpcPacket_t) + MAX_IPC_MESSAGE_SIZE);
	CHECK_RET_GOTO_P(ctx->r_packet,out,"malloc IpcPacket_t failed!\n")	;
	memset(ctx->r_packet,0,sizeof(IpcPacket_t) + MAX_IPC_MESSAGE_SIZE);

	ret = ipc_skt_open(ctx);
	CHECK_RET_GOTO_P(ret == 0,out,"ipc_skt_client_open failed!\n");

	ctx->pid = wow_thread_create("ipc_socket_client",ev_loop_entry, (void*)ctx);
	CHECK_RET_GOTO_P_A(ctx->pid, out,"thread_create failed!\n");
	
	ret = ipc_skt_send_bind(ctx);
	CHECK_RET_GOTO_P(ret == 0,out,"ipc_skt_send_bind failed!\n");
	

	return (IpcCtx_T*)ctx;
out:
	ipc_skt_exit((IpcCtx_T*)ctx);
	return NULL;
}



