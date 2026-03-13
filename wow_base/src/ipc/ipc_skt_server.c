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
#include "list/wow_slist.h"

#include "ipc_internal.h"


typedef struct{
    int bind;			 //绑定标识
	int sktfd;      	 //从端socket
	int cid;			 //从端标识
	struct ev_io ior;	 //读事件
	IpcPacket_t* s_packet; //发送缓存
	IpcPacket_t* r_packet; //接收缓存
	mutex_lock_t send_lock;
	mutex_lock_t recv_lock;
	void* priv;
}CliSktInfo_T;

typedef struct{
	IpcCtx_T    base;
	int 		sktfd;
	Slist_T*    clist;
	ipc_inner_recv_fun_t  skt_recv_cb;
	void*       priv;
	
	int          inited;
	mutex_lock_t loop_lock;
	mutex_cond_t loop_cond;
	struct ev_io ioc;			//连接事件
	struct ev_loop *loop;
	pthread_idx_t pid;
}SerSktCtx_T;

static void server_recv_callback(struct ev_loop *loop, struct ev_io *w, int revents);


static bool list_func_find_by_cid(const void *data, const void *match_data)
{
	CliSktInfo_T* client = (CliSktInfo_T*)data;
	CHECK_RET_VAL(client && match_data, false);

	return (client->cid == (*(int*)match_data))?true:false;
}

static void ipc_add_client(SerSktCtx_T *ctx,int cli_skt)
{
	CliSktInfo_T* client = CALLOC(1, CliSktInfo_T);
	CHECK_RET_VOID_P(client,"malloc CliSktInfo_T failed!\n");
	
	client->sktfd  = cli_skt;
	client->priv   = ctx;
	client->bind   = 0;
	client->s_packet = (IpcPacket_t*)MALLOC(sizeof(IpcPacket_t) + MAX_IPC_MESSAGE_SIZE);
	CHECK_RET_GOTO_P(client->s_packet,out1,"malloc IpcPacket_t failed!\n");
	memset(client->s_packet,0,sizeof(IpcPacket_t) + MAX_IPC_MESSAGE_SIZE);
	
	client->r_packet = (IpcPacket_t *)MALLOC(sizeof(IpcPacket_t) + MAX_IPC_MESSAGE_SIZE);
	CHECK_RET_GOTO_P(client->r_packet,out2,"malloc IpcPacket_t failed!\n");
	memset(client->r_packet,0,sizeof(IpcPacket_t) + MAX_IPC_MESSAGE_SIZE);

	int ret = wow_slist_insert_tail(ctx->clist,client);
	CHECK_RET_GOTO_P(ret == 0,out3,"wow_list_insert_tail client failed!\n");

	wow_mutex_lock_init(&client->send_lock);
	wow_mutex_lock_init(&client->recv_lock);
	
	ev_io_init(&client->ior, server_recv_callback, cli_skt, EV_READ);
	ev_io_start(ctx->loop, &client->ior);

	return;
out3:
	FREE(client->r_packet);
out2:
	FREE(client->s_packet);
out1:
	FREE(client);
	return;
}

static void ipc_del_client(void *data)
{
	CliSktInfo_T* client = (CliSktInfo_T*)data;
	CHECK_RET_VOID(client);

	close(client->sktfd);

	wow_mutex_lock_exit(&client->send_lock);
	wow_mutex_lock_exit(&client->recv_lock);

	FREE(client->s_packet);
	FREE(client->r_packet);
	FREE(client);
}

static void ipc_ack_client_bind(CliSktInfo_T *client)
{
	
	client->bind = 1;
	client->cid = (int)GET_IPC_MSG_SRC(client->r_packet->header.func_id);
	//响应
	send(client->sktfd,client->r_packet,sizeof(IpcHeader_T), 0);

}

static void ipc_ack_client_unbind(CliSktInfo_T *client)
{
	CHECK_RET_VOID(client->bind);
	
	//响应
	send(client->sktfd,client->r_packet,sizeof(IpcHeader_T), 0);

	SerSktCtx_T *ctx = (SerSktCtx_T *)client->priv;
	ev_io_stop(ctx->loop, &client->ior);
	wow_slist_remove(ctx->clist,client);
	
	ipc_del_client(client);
}

static void server_recv_callback(struct ev_loop *loop, struct ev_io *w, int revents)
{
	int rlen    = -1;
	
    CliSktInfo_T *client = container_of(w,CliSktInfo_T,ior);
	CHECK_RET_VOID(client);
	SerSktCtx_T *ctx = (SerSktCtx_T *)client->priv;
	CHECK_RET_VOID(ctx);
	
	//此处不处理断包 ----应用端使用不规范或网络断包则处理失败
	wow_mutex_lock(&client->recv_lock);
	rlen = recv(client->sktfd,client->r_packet,sizeof(IpcPacket_t) + MAX_IPC_MESSAGE_SIZE, 0);
	wow_mutex_unlock(&client->recv_lock);
	CHECK_RET_VOID(rlen >= sizeof(IpcPacket_t));
	
    switch (client->r_packet->header.flag) {
    case IPC_CMD_BIND:
		{
			wow_mutex_lock(&ctx->loop_lock);
			ipc_ack_client_bind(client);
			wow_mutex_unlock(&ctx->loop_lock);
		}
        break;
    case IPC_CMD_UNBIND:
		{
			wow_mutex_lock(&ctx->loop_lock);
			ipc_ack_client_unbind(client);
			wow_mutex_unlock(&ctx->loop_lock);
		}
        break;
    case IPC_CMD_COMM:
    default:
		CHECK_RET_VOID(client->bind);
		CHECK_RET_VOID(ctx->skt_recv_cb);
		wow_mutex_lock(&client->recv_lock);
		if(0 == ctx->skt_recv_cb((IpcCtx_T*)ctx,client->r_packet->header.func_id, 
						client->r_packet->payload, rlen-sizeof(IpcPacket_t),ctx->priv)){
			memset(client->r_packet,0,sizeof(IpcPacket_t) + MAX_IPC_MESSAGE_SIZE);
		}
		wow_mutex_unlock(&client->recv_lock);
        break;
    }

}


static void socket_connect_callback(struct ev_loop *loop, struct ev_io *w, int revents)
{
	char stub_name[256];
	int  cli_skt;
	struct sockaddr_un sockaddr;
	
	//printf("-------------connect_callback-------------\n");
	SerSktCtx_T *ctx = container_of(w,SerSktCtx_T,ioc);
	CHECK_RET_VOID(ctx && ctx->clist);
	CHECK_RET_VOID_P(wow_slist_size(ctx->clist) < MAX_IPC_NUMNER,"client is full!\n")

    socklen_t len = 0;//must not <0
    snprintf(stub_name, sizeof(stub_name), "%s.%d", IPC_SERVER_NAME, ctx->base.info.serId);
    snprintf(sockaddr.sun_path, sizeof(sockaddr.sun_path), "/tmp/%s", stub_name);
	sockaddr.sun_family = PF_UNIX;
	
    cli_skt = accept(ctx->sktfd, (struct sockaddr *)&sockaddr, &len);
	CHECK_RET_VOID_P(cli_skt >= 0,"accept failed: %s\n", strerror(errno));
	//printf("connect_callback fd:%d\n",ctx->skt_c[rindex]);

	wow_mutex_lock(&ctx->loop_lock);
	ipc_add_client(ctx,cli_skt);
	wow_mutex_unlock(&ctx->loop_lock);

}

static void* ev_loop_entry(void *arg)
{
	SerSktCtx_T *ctx = (SerSktCtx_T *)arg;

	ctx->loop = ev_loop_new(EVFLAG_AUTO);;
	ev_io_init(&ctx->ioc, socket_connect_callback, ctx->sktfd, EV_READ);
	ev_io_start(ctx->loop, &ctx->ioc);
	
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


static int ipc_skt_open(SerSktCtx_T *ctx)
{
	int ret = -1;
    char stub_name[256];
	struct sockaddr_un   sockaddr;
	
	ctx->sktfd = socket(PF_UNIX, SOCK_SEQPACKET, 0);
	CHECK_RET_VAL_P(ctx->sktfd >= 0,-1 ,"socket failed: %s\n", strerror(errno));

    snprintf(stub_name, sizeof(stub_name), "%s.%d", IPC_SERVER_NAME, ctx->base.info.serId);
    snprintf(sockaddr.sun_path, sizeof(sockaddr.sun_path), "/tmp/%s", stub_name);
	sockaddr.sun_family = PF_UNIX;
	
	//已存在 删除
    char cmd[MAX_CMD_SIZE];
	snprintf(cmd, sizeof(cmd), "rm -rf %s", sockaddr.sun_path);
	system(cmd);

    ret = bind(ctx->sktfd, (struct sockaddr*)&sockaddr, sizeof(struct sockaddr_un));
	CHECK_RET_GOTO_P(ret != -1, out1, "bind %s failed: %s\n", stub_name, strerror(errno));

    ret = listen(ctx->sktfd, SOMAXCONN);
	CHECK_RET_GOTO_P(ret != -1, out1, "listen failed: %s\n", strerror(errno));

    return 0;

out1:
    close(ctx->sktfd);
    return -1;
}


static void ipc_skt_exit(IpcCtx_T *self)
{    
	SerSktCtx_T *ctx = (SerSktCtx_T *)self; 
	CHECK_RET_VOID(ctx);

	//发送解绑消息
	if(ctx->sktfd > 0){
		//ipc_skt_send_unbind(ctx);
		close(ctx->sktfd);
	}
	
	if(ctx->clist){
		wow_slist_destroy(&ctx->clist,ipc_del_client);
		FREE(ctx->clist);
	}
		
	ctx->inited = 0;
	if(ctx->loop){
		ev_break(ctx->loop,EVBREAK_ALL);
		wow_mutex_lock(&ctx->loop_lock);
		wow_mutex_cond_wait(&ctx->loop_lock,&ctx->loop_cond,-1);
		wow_mutex_unlock(&ctx->loop_lock);
		
		ev_loop_destroy(ctx->loop);
	}	

	wow_mutex_lock_exit(&ctx->loop_lock);
	wow_mutex_cond_exit(&ctx->loop_cond);
	
	FREE(ctx);
}

static int ipc_skt_set_recv_cb(IpcCtx_T *self, ipc_inner_recv_fun_t cb, void* priv)
{
	SerSktCtx_T *ctx = (SerSktCtx_T *)self;
	CHECK_RET_VAL_P(ctx,-1,"param input invalid!\n");
	
    ctx->skt_recv_cb = cb;
	ctx->priv = priv;
	
    return 0;
}

static int ipc_skt_send(IpcCtx_T *self,uint32_t func_id, const void *buf, size_t len)
{	
	int cid  = 0;
	int size = 0;
	SerSktCtx_T *ctx = (SerSktCtx_T *)self;	
	CHECK_RET_VAL_P(ctx && ctx->clist && len < MAX_IPC_MESSAGE_SIZE,-1,"param input invalid!\n");

	cid = GET_IPC_MSG_DEST(func_id);
	CliSktInfo_T* client = wow_slist_peek_by_match(ctx->clist,list_func_find_by_cid,&cid);
	CHECK_RET_VAL_P(client,-1,"client %d not bind!\n",cid);

	wow_mutex_lock(&client->send_lock);
	client->s_packet->header.flag		   = IPC_CMD_COMM;
	client->s_packet->header.func_id 	   = func_id;
	client->s_packet->header.payload_len = len;
	memcpy(client->s_packet->payload, buf, len);

    size = send(client->sktfd, (const void*)client->s_packet, len + sizeof(IpcHeader_T), 0);
	wow_mutex_unlock(&client->send_lock);

	return (size - sizeof(IpcHeader_T));
}

static int ipc_skt_recv(IpcCtx_T *self, uint32_t func_id, void *buf, size_t len)
{
	int cid  = 0;
	int size = 0;

	SerSktCtx_T *ctx = (SerSktCtx_T *)self;	
	CHECK_RET_VAL_P(ctx && ctx->clist ,-1,"param input invalid!\n");

	cid = GET_IPC_MSG_DEST(func_id);
	CliSktInfo_T* client = wow_slist_peek_by_match(ctx->clist,list_func_find_by_cid,&cid);
	CHECK_RET_VAL_P(client,-1,"client %d not bind!\n",cid);
	CHECK_RET_VAL(client->r_packet->header.payload_len > 0, -1);
	
	wow_mutex_lock(&client->recv_lock);
	size = MIN2(len,client->r_packet->header.payload_len);
	memcpy(buf,client->r_packet->payload,size);
	memset((void*)client->r_packet,0,sizeof(IpcPacket_t) + MAX_IPC_MESSAGE_SIZE);
	wow_mutex_unlock(&client->recv_lock);

	return size;
}

static int ipc_skt_broadcast(IpcCtx_T *self,uint16_t mod_id,const void *buf, size_t len)
{
    SerSktCtx_T *ctx = (SerSktCtx_T *)self;
	CHECK_RET_VAL_P(ctx && ctx->clist && len < MAX_IPC_MESSAGE_SIZE,-1,"param input invalid!\n");

	int index = 0;
	CliSktInfo_T* client = NULL;
	int nsize = wow_slist_size(ctx->clist);

	wow_mutex_lock(&ctx->loop_lock);
	for(index = 0; index < nsize; index++){
		client = wow_slist_peek_by_index(ctx->clist,index);
		CHECK_RET_CONTINUE(client);
		CHECK_RET_CONTINUE(client->bind);
		
		wow_mutex_lock(&client->send_lock);
		client->s_packet->header.flag		   = IPC_CMD_COMM;
		client->s_packet->header.func_id 	   = BUILD_IPC_MSG_ID(client->cid,ctx->base.info.serId,mod_id);
		client->s_packet->header.payload_len = len;
		memcpy(client->s_packet->payload, buf, len);
		send(client->sktfd, (const void*)client->s_packet, len + sizeof(IpcHeader_T), 0);

		wow_mutex_unlock(&client->send_lock);
	}
	wow_mutex_unlock(&ctx->loop_lock);
    return 0;
}


IpcCtx_T* ipc_skt_server_init(IpcInfo_T  * info)
{
	int ret = 0;
	
	SerSktCtx_T* ctx = CALLOC(1, SerSktCtx_T);
	CHECK_RET_VAL_P(ctx,NULL,"malloc SerSktCtx_T failed!\n");
	
	memcpy(&ctx->base.info,info,sizeof(IpcInfo_T));
	ctx->base.exit        = ipc_skt_exit;
	ctx->base.set_recv_cb = ipc_skt_set_recv_cb;
	ctx->base.send        = ipc_skt_send;
    ctx->base.recv        = ipc_skt_recv;
    ctx->base.broadcast   = ipc_skt_broadcast;

	wow_mutex_lock_init(&ctx->loop_lock);
	wow_mutex_cond_init(&ctx->loop_cond);
	
	ctx->clist = wow_slist_create();
	CHECK_RET_GOTO_P(ctx->clist,out,"wow_list_create failed!\n");
	
	ret = ipc_skt_open(ctx);
	CHECK_RET_GOTO_P(ret == 0, out, "ipc_skt_open failed!\n");

	ctx->pid = wow_thread_create("ipc_socket_server",ev_loop_entry, (void*)ctx);
	CHECK_RET_GOTO_P_A(ctx->pid, out,"thread_create failed!\n");
	

	return (IpcCtx_T*)ctx;
out:
	ipc_skt_exit((IpcCtx_T*)ctx);
	return NULL;	
}

