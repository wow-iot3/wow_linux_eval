#ifndef __IPC_INTERNAL_H_
#define __IPC_INTERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"

#include "ipc/wow_ipc.h"


#define MAX_IPC_NUMNER 				(256)	
#define MAX_IPC_RESP_BUF_LEN        (1024)
#define MAX_IPC_MESSAGE_SIZE        (1024)
#define IPC_COMM_DEF_TIMEOUT		(2000)

//绑定的内容为16字节 
typedef enum{
    IPC_CMD_BIND   = 0x26544357,
    IPC_CMD_UNBIND = 0x57544361,
    IPC_CMD_COMM   = 0x54435761
}IpcTypeCmd_E ;

//ipc信息头
typedef struct{
	IpcTypeCmd_E   type;
	IpcTypeCmd_E   flag;
    uint32_t func_id;
    size_t   payload_len;
}__data_aligned__ IpcHeader_T;

//ipc包
typedef struct {
    IpcHeader_T header;
    uint8_t payload[0];
}__data_aligned__ IpcPacket_t;

typedef struct{
	uint8_t  serId;
	uint8_t  cliId;
	uint16_t back;
	void*   priv;
}__data_aligned__ IpcInfo_T;

typedef struct ipc_ctx IpcCtx_T;
typedef int (*ipc_inner_recv_fun_t)(IpcCtx_T *ctx,uint32_t func_id,void *in_arg, size_t in_len,void* priv);
struct __data_aligned__ ipc_ctx{
	IpcInfo_T  info;
    void  (*exit)(IpcCtx_T *ctx);
    int (*set_recv_cb)(IpcCtx_T *ctx, ipc_inner_recv_fun_t cb,void* priv);
    int (*send)(IpcCtx_T *ctx, uint32_t func_id,const void *buf, size_t len);
    int (*recv)(IpcCtx_T *ctx, uint32_t func_id,void *buf, size_t len);
    int (*broadcast)(IpcCtx_T *ctx,uint16_t func_id,const void *buf, size_t len);
};


#define IPC_SERVER_NAME "/IPC_SERVER"
#define IPC_CLIENT_NAME "/IPC_CLIENT"
IpcCtx_T* ipc_skt_server_init(IpcInfo_T  * info);
IpcCtx_T* ipc_skt_client_init(IpcInfo_T  * info);


#ifdef __cplusplus
}
#endif

#endif
