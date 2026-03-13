#ifndef __WOW_BASE_IPC_WOW_IPC_H_
#define __WOW_BASE_IPC_WOW_IPC_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IPC_DEST_BIT                (24)
#define IPC_DEST_MASK               0xFF
#define IPC_SRC_BIT                 (16)
#define IPC_SRC_MASK                0xFF
#define IPC_CMD_BIT                 (0)
#define IPC_CMD_MASK                0xFFFF

//约束 目的ID\源ID\取值范围都为0~255 目的ID与源ID可重复
//     标识ID 取值范围0~65536
#define BUILD_IPC_MSG_ID(dest,src, cmd) \
    (((((uint16_t)dest) & IPC_DEST_MASK) << IPC_DEST_BIT) | \
     ((((uint16_t)src) & IPC_SRC_MASK) << IPC_SRC_BIT) | \
     ((((uint16_t)cmd) & IPC_CMD_MASK) << IPC_CMD_BIT))
	
#define GET_IPC_MSG_DEST(func_id) \
			((func_id >>IPC_DEST_BIT) & IPC_DEST_MASK)
#define GET_IPC_MSG_SRC(func_id) \
			((func_id >>IPC_SRC_BIT) & IPC_SRC_MASK)
#define GET_IPC_MSG_CMD(func_id) \
			((func_id >>IPC_CMD_BIT) & IPC_CMD_MASK)


typedef struct ipc_t Ipc_T;

typedef int (*ipc_recv_fun_t)(Ipc_T* ipc,void *in_arg, size_t in_len, void* priv);
typedef int (*ipc_recv_all_fun_t)(Ipc_T* ipc,uint32_t func_id,void *in_arg, size_t in_len, void* priv);


/*brief    创建ipc服务端接口
 *param ： u8SerId :主进程/线程标识号
 *return： 返回ipc结构体指针 失败返回NULL
 */
Ipc_T* wow_ipc_create_server(uint8_t u8SerId);

/*brief    创建ipc客户端接口
 *param ： u8SerId :主进程/线程标识号
 *param ： u8CliId :从进程/线程标识号
 *return： 返回ipc结构体指针 失败返回NULL
 */
Ipc_T* wow_ipc_create_client(uint8_t u8SerId,uint8_t u8CliId);


/*brief    销毁ipc接口
 *param ： pptIpc :ipc结构体指针
 *return： 无
 */
void wow_ipc_destroy(Ipc_T** pptIpc);

/*brief    设置接收回调函数
 *param ： ptIpc :ipc结构体指针
 *param ： u32Fid :事件标识
 *param ： fCallBack :接收回调函数
 *param ： pArg :回调函数参数
 *return： 成功返回0 失败返回<0
 */
int  wow_ipc_set_notify(Ipc_T* ptIpc,uint32_t u32Fid, ipc_recv_fun_t fCallBack, void* pArg);

/*brief    设置接收回调函数
 *param ： ptIpc :ipc结构体指针
 *param ： fCallBack :接收回调函数
 *param ： pArg :回调函数参数
 *return： 成功返回0 失败返回<0
 */
int  wow_ipc_set_notify_all(Ipc_T* ptIpc,ipc_recv_all_fun_t fCallBack, void* pArg);

/*brief    IPC发送数据
 *param ： ptIpc :ipc结构体指针
 *param ： u32Fid :事件标识 
 *param ： pData :发送缓存
 *param ： snLen :发送缓存长度
 *return： 成功返回实际发送长度 失败返回<0
 */
int wow_ipc_send(Ipc_T* ptIpc,uint32_t u32Fid,const void *pData, size_t snLen);

/*brief    IPC接收数据
 *param ： ptIpc :ipc结构体指针
 *param ： u32Fid :事件标识 
 *param ： pData :接收缓存
 *param ： snLen :接收缓存长度
 *return： 成功返回实际接收长度 失败返回<0
 */
int wow_ipc_recv(Ipc_T* ptIpc,uint32_t u32Fid,void *pData, size_t snLen);

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
int wow_ipc_send_wait(Ipc_T* self,uint32_t u32Fid,uint16_t u16TimeoutMs,
						 	const void *pIdata, size_t snIlen,void *pOdata, size_t snOlen);

/*brief    IPC广播数据
 *param ： ptIpc :ipc结构体指针
 *param ： u16Mod :事件标识 
 *param ： pData :发送缓存
 *param ： snLen :发送缓存长度
 *return： 成功返回0 失败返回<0
 */
int wow_ipc_broadcast(Ipc_T* ptIpc,uint16_t u16Mod,const void *pData, size_t snLen);


#ifdef __cplusplus
}
#endif

#endif
