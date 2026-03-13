#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <stdint.h>
#include <pthread.h>


#include "system/wow_cond.h"
#include "list/wow_slist.h"
#include "list/wow_kv_list.h"

#include "ipc/wow_ipc.h"
#include "ipc_internal.h"


typedef struct{
	ipc_recv_fun_t	func;
	void*           priv;
	uint32_t        fid;
}__data_aligned__ IpcNodtify_T;


struct __data_aligned__ ipc_t {
    IpcCtx_T*   ctx;	
    sem_cond_t  sem;	
    void*       resp_buf;
    uint32_t    resp_len;
    KvList_T*   kvlist;
 	Slist_T*    slist;
	ipc_recv_all_fun_t func;
	void*       priv;
};



#define BUILD_ACK_IPC_MSG_ID(func_id) \
			BUILD_IPC_MSG_ID(GET_IPC_MSG_SRC(func_id),GET_IPC_MSG_DEST(func_id),GET_IPC_MSG_CMD(func_id))

#define MSG_BOARDCASD_ID (0xFFFFFFFF)


static int push_sync_cmd(Ipc_T *ipc, uint32_t func_id)
{
    char cmd_str[11] = {0};
    snprintf(cmd_str, sizeof(cmd_str), "0x%08X", func_id);

	wow_kvlist_set(ipc->kvlist,cmd_str, (char*)"1");
	
    return WOW_SUCCESS;
}

static int pop_sync_cmd(Ipc_T *ipc, uint32_t func_id)
{
	char cmd_str[11] = {0};
    snprintf(cmd_str, sizeof(cmd_str), "0x%08X", func_id);
	
	void* pair = wow_kvlist_get(ipc->kvlist, cmd_str);
	CHECK_RET_VAL_P(pair,-1,"wow_kvlist_get failed!\n");

	wow_kvlist_delete(ipc->kvlist, cmd_str);
	
    return WOW_SUCCESS;
}

static bool  list_func_match_func_id(const void *data, const void *match_data)
{
	IpcNodtify_T* notify = (IpcNodtify_T*)data;
	CHECK_RET_VAL(notify && match_data,false);
	
	uint32_t func_id = *(uint32_t*)match_data;

	return (notify->fid == func_id)?true:false;
}

static int ipc_process_msg(IpcCtx_T *ctx, uint32_t func_id, void *buf, size_t len,void* priv)
{
	Ipc_T *ipc = (Ipc_T*)ctx->info.priv;	
	CHECK_RET_VAL(ipc,-1);
	uint32_t ack_id = BUILD_ACK_IPC_MSG_ID(func_id);

	if(wow_kvlist_size(ipc->kvlist) > 0){//存在同步应答帧等待
		int ret = pop_sync_cmd(ipc, ack_id);
		if(ret == 0){//此功能ID正在等待
			memcpy(ipc->resp_buf,buf,len);
			ipc->resp_len = len;
			wow_sem_cond_signal(&ipc->sem);
			return 0;
		}
	}
	
	//统一回调函数接口
	if(ipc->func){
		ipc->func((Ipc_T*)ipc,ack_id,buf,len,ipc->priv);
		return 0;
	}
	
	//匹配回调函数
	IpcNodtify_T* notify = NULL;
	notify = wow_slist_peek_by_match(ipc->slist,list_func_match_func_id,&func_id);
	if(notify){
		notify->func((Ipc_T*)ipc, buf,len,notify->priv);
	}else{
		return -1;
	}

    return WOW_SUCCESS;
}

/*brief    创建ipc服务端接口
 *param ： u8SerId :主进程/线程标识号
 *return： 返回ipc结构体指针 失败返回NULL
 */
__EX_API__ Ipc_T* wow_ipc_create_server(uint8_t u8SerId)
{
	IpcInfo_T info;
	
	Ipc_T *ipc = CALLOC(1, Ipc_T);
	CHECK_RET_VAL_ERRNO_P(ipc,-SYSTEM_MALLOC_FAILED, "malloc Ipc_T failed!\n");
	
	info.serId	= u8SerId;
	info.cliId	= u8SerId;
	info.priv	= ipc;
	
	ipc->ctx = ipc_skt_server_init(&info);
	CHECK_RET_GOTO_ERRNO_P(ipc->ctx,out1,-IPC_CREATE_FAILED, "ipc init invalid!\n");

	ipc->resp_buf = MALLOC(MAX_IPC_RESP_BUF_LEN);
	CHECK_RET_GOTO_ERRNO_P(ipc->resp_buf,out2,-SYSTEM_MALLOC_FAILED, "malloc size:%d failed!\n",MAX_IPC_RESP_BUF_LEN);	
	memset(ipc->resp_buf,0,MAX_IPC_RESP_BUF_LEN);

	ipc->slist = wow_slist_create();
	CHECK_RET_GOTO_ERRNO_P(ipc->slist,out3,-SLIST_CREATE_FAILED, "wow_list_create failed!\n");

	ipc->kvlist	= wow_kvlist_create();
	CHECK_RET_GOTO_ERRNO_P(ipc->kvlist,out4,-KVLIST_CREATE_FAILED, "wow_kvlist_create failed!\n");
		
	//注册接收回调函数
	ipc->ctx->set_recv_cb(ipc->ctx, ipc_process_msg,ipc->priv);

	wow_sem_cond_init(&ipc->sem);
	
	return (Ipc_T*)ipc;
out4:
	wow_slist_destroy(&ipc->slist,NULL);
out3:
	FREE(ipc->resp_buf);
out2:
	ipc->ctx->exit(ipc->ctx);
out1:
	FREE(ipc);
	return NULL;

}

/*brief    创建ipc客户端接口
 *param ： u8SerId :主进程/线程标识号
 *param ： u8CliId :从进程/线程标识号
 *return： 返回ipc结构体指针 失败返回NULL
 */
__EX_API__ Ipc_T* wow_ipc_create_client(uint8_t u8SerId,uint8_t u8CliId)
{
	IpcInfo_T info;
	
	Ipc_T *ipc = CALLOC(1, Ipc_T);
	CHECK_RET_VAL_ERRNO_P(ipc,-SYSTEM_MALLOC_FAILED, "malloc ipc failed!\n");

	info.serId	= u8SerId;
	info.cliId	= u8CliId;
	info.priv	= ipc;

	ipc->ctx = ipc_skt_client_init(&info);
	CHECK_RET_GOTO_ERRNO_P(ipc->ctx,out1,-IPC_CREATE_FAILED,  "ipc init invalid!\n");

	ipc->resp_buf = MALLOC(MAX_IPC_RESP_BUF_LEN);
	CHECK_RET_GOTO_ERRNO_P(ipc->resp_buf,out2,-SYSTEM_MALLOC_FAILED,  "malloc resp_buf failed!\n");	
	memset(ipc->resp_buf,0,MAX_IPC_RESP_BUF_LEN);
	

	ipc->slist = wow_slist_create();
	CHECK_RET_GOTO_ERRNO_P(ipc->slist,out3,-SLIST_CREATE_FAILED, "wow_list_create failed!\n");
	
	//注册接收回调函数
	ipc->ctx->set_recv_cb(ipc->ctx, ipc_process_msg,ipc->priv);

	ipc->kvlist = wow_kvlist_create();
	CHECK_RET_GOTO_ERRNO_P(ipc->kvlist,out4,-KVLIST_CREATE_FAILED, "wow_kvlist_create failed!\n");

	wow_sem_cond_init(&ipc->sem);
	
	return (Ipc_T*)ipc;
out4:
	wow_slist_destroy(&ipc->slist,NULL);
out3:
	FREE(ipc->resp_buf);
out2:
	ipc->ctx->exit(ipc->ctx);
out1:
	FREE(ipc);
	return NULL;

}

static void ipc_free_notify(void *data)
{
	IpcNodtify_T* notify = (IpcNodtify_T*)data;
	CHECK_RET_VOID(notify);

	FREE(notify);
}

/*brief    销毁ipc接口
 *param ： pptIpc :ipc结构体指针
 *return： 无
 */
__EX_API__ void wow_ipc_destroy(Ipc_T** pptIpc)
{
	CHECK_RET_VOID(pptIpc && *pptIpc);
	
	Ipc_T* ipc = (Ipc_T*)*pptIpc;

	wow_sem_cond_exit(&ipc->sem);
	wow_kvlist_destroy(&ipc->kvlist);

    ipc->ctx->exit(ipc->ctx);

	wow_slist_destroy(&ipc->slist,ipc_free_notify);

    FREE(ipc->resp_buf);
	
	FREE(ipc);
	*pptIpc = NULL;
}



/*brief    设置接收回调函数
 *param ： ptIpc :ipc结构体指针
 *param ： u32Fid :事件标识
 *param ： fCallBack :接收回调函数
 *param ： pArg :回调函数参数
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int  wow_ipc_set_notify(Ipc_T* ptIpc,uint32_t u32Fid, ipc_recv_fun_t fCallBack, void* pArg)
{
	CHECK_RET_VAL_P(ptIpc,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(fCallBack,-PARAM_INPUT_FUNC_IS_NULL,"param input func invalid!\n");

	IpcNodtify_T* notify = NULL;
	notify = wow_slist_peek_by_match(ptIpc->slist,list_func_match_func_id,&u32Fid);
	if(notify){//已存在覆盖
		notify->func = fCallBack;
		notify->priv = pArg;
	}else{//不存在 插入
		notify = CALLOC(1, IpcNodtify_T);
		CHECK_RET_VAL_P(notify,-SYSTEM_MALLOC_FAILED,"malloc IpcNodtify_T failed!\n");
		
		notify->fid  = u32Fid;
		notify->func = fCallBack;
		notify->priv = pArg;
		wow_slist_insert_tail(ptIpc->slist,notify);
	}
	
    return 0;
}

/*brief    设置接收回调函数
 *param ： ptIpc :ipc结构体指针
 *param ： fCallBack :接收回调函数
 *param ： pArg :回调函数参数
 *return： 成功返回0失败返回<0
 */
__EX_API__ int  wow_ipc_set_notify_all(Ipc_T* ptIpc,ipc_recv_all_fun_t fCallBack, void* pArg)
{
	CHECK_RET_VAL_P(ptIpc,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(fCallBack,-PARAM_INPUT_FUNC_IS_NULL,"param input func invalid!\n");

	ptIpc->func = fCallBack;	
	ptIpc->priv = pArg;
	
	return WOW_SUCCESS;
}

/*brief    IPC发送数据
 *param ： ptIpc :ipc结构体指针
 *param ： u32Fid :事件标识 
 *param ： pData :发送缓存
 *param ： snLen :发送缓存长度
 *return： 成功返回实际发送长度 失败返回<0
 */
__EX_API__ int wow_ipc_send(Ipc_T* ptIpc,uint32_t u32Fid,const void *pData, size_t snLen)
{
	CHECK_RET_VAL_P(ptIpc,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(pData,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(snLen > 0,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");


    return ptIpc->ctx->send(ptIpc->ctx,u32Fid,pData,snLen);
}

/*brief    IPC接收数据
 *param ： ipc :ipc结构体指针
 *param ： u32Fid :事件标识 
 *param ： pData :接收缓存
 *param ： snLen :接收缓存长度
 *return： 成功返回实际接收长度 失败返回<0
 */
__EX_API__ int wow_ipc_recv(Ipc_T* ptIpc,uint32_t u32Fid,void *pData, size_t snLen)
{
	CHECK_RET_VAL_P(ptIpc,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(pData,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(snLen > 0,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");


    return ptIpc->ctx->recv(ptIpc->ctx,u32Fid,pData,snLen);
}


/*brief    IPC发送数据 等待回复
 *param ： ipc :ipc结构体指针
 *param ： u32Fid :事件标识 
 *param ： u16TimeoutMs :超时时间 --ms
 *param ： pIdata :发送缓存
 *param ： snIlen :发送缓存长度
 *param ： pOdata :接收缓存
 *param ： snOlen :接收缓存长度
 *return： 成功返回实际接收长度 失败返回<0
 */
__EX_API__ int wow_ipc_send_wait(Ipc_T* ptIpc,uint32_t u32Fid,uint16_t u16TimeoutMs,
						 	const void *pIdata, size_t snIlen,void *pOdata, size_t snOlen)
{
	int ret = -1;
	int size = 0;
	
	CHECK_RET_VAL_P(ptIpc,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(pIdata && pOdata,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(snIlen > 0  && snOlen > 0,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");
	
	ret = push_sync_cmd(ptIpc, u32Fid);
	CHECK_RET_VAL_P(ret == 0,-IPC_SET_ID_FAILED,"push_async_cmd failed!\n");

    ret = ptIpc->ctx->send(ptIpc->ctx,u32Fid,pIdata,snIlen);
	CHECK_RET_GOTO_EXE_P(ret != -1,out,ret = -IPC_SEND_FAILED,"send msg  failed!\n");
	
	//信号量等待接收数据
	ret = wow_sem_cond_wait(&ptIpc->sem,u16TimeoutMs);
	CHECK_RET_GOTO_EXE_P(ret == 0,out,ret = -IPC_TIME_OUT,"sem wait timeout!\n");

	size = MIN2(ptIpc->resp_len,snOlen);
    memcpy(pOdata, ptIpc->resp_buf, size);
	
    return size;
out:
	pop_sync_cmd(ptIpc, u32Fid);
	return ret;
}

/*brief    IPC广播数据
 *param ： ptIpc :ipc结构体指针
 *param ： pData :发送缓存
 *param ： snLen :发送缓存长度
 *return： 成功返回0 失败返回<0
 */
 __EX_API__ int wow_ipc_broadcast(Ipc_T* ptIpc,uint16_t u16Mod,const void *pData, size_t snLen)
 {
	CHECK_RET_VAL_P(ptIpc,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(pData,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(snLen > 0,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

	 
	 return ptIpc->ctx->broadcast(ptIpc->ctx,u16Mod,pData,snLen);
 }

