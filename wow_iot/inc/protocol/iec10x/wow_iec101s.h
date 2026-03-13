#ifndef __WOW_IOT_PROTOCOL_WOW_IEC101S_H_
#define __WOW_IOT_PROTOCOL_WOW_IEC101S_H_

#include "protocol/wow_proto_port.h"
#include "protocol/iec10x/wow_iec10x.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct iec101s_t Iec101s_T;

/*brief    Iec101从设备初始化
 *param ： u16Addr: 公共地址 
 *param ： ptUartPar:  硬件参数
 *return： 成功返回iec101s操作标识 失败返回NULL
 */
Iec101s_T*  wow_iec101_s_init(uint16_t u16Addr,ProtoUartParam_T* ptUartPar);

/*brief    退出Iec101从设备
 *param ： pptIec101s:iec101s操作标识
 *return： 无
 */
void  wow_iec101_s_exit(Iec101s_T** pptIec101s);

/*brief    启动Iec101从设备
 *param ： ptIec101s:iec101s操作标识
 *return： 成功返回0 失败返回<0
 */
int  wow_iec101_s_start(Iec101s_T* ptIec101s);

/*brief    Iec101设置数据节点信息
 *param ： ptIec101s:iec101s操作标识
 *param ： ptIec101s:iec101s操作标识
 *param ： ptNode:  节点信息
 *param ： nCount: 节点个数
 *return： 成功返回0 失败返回<0
 */
int wow_iec101_s_set_node_info(Iec101s_T* ptIec101s,DevNodeInfo_T* ptNode,int nCount);

/*brief    Iec101设置更新数据节点参数回调函数
 *param ： ptIec101s:iec101s操作标识
 *param ： fCallBack:   回调函数 
 *param ： pArg:  回调函数私有变量
 *return： 成功返回0 失败返回<0
 */
int wow_iec101_s_set_value_handle(Iec101s_T* ptIec101s,iec10x_set_value_func_t fCallBack,void* pArg);
int wow_iec104_s_get_value_handle(Iec101s_T* ptIec101s,iec10x_set_value_func_t fCallBack,void* pArg);


/*brief    Iec101设置更新服务状态回调函数
 *param ： ptIec101s:iec101s操作标识
 *param ： fCallBack:   回调函数 
 *param ： pArg:  回调函数私有变量
 *return： 成功返回0 失败返回<0
 */
int wow_iec101_s_set_service_handle(Iec101s_T* ptIec101s,iec10x_set_service_func_t fCallBack,void* pArg);

/*brief    iec04遥信突变
 *param ： ptIec101s:iec101s操作标识
 *param ： ptNode:遥信节点信息
 *param ： u8Count:遥信节点个数
 *return： 成功返回0 失败返回<0
 */
int wow_iec101_s_build_yx_spon(Iec101s_T* ptIec101s,DevNodeCtrl_T* ptNode,uint8_t u8Count);

/*brief    iec04遥测突变
 *param ： ptIec101s:iec101s操作标识
 *param ： ptNode:遥测节点信息
 *param ： u8Count:遥测节点个数
 *return： 成功返回0 失败返回<0
 */
int wow_iec101_s_build_yc_spon(Iec101s_T* ptIec101s,DevNodeCtrl_T* ptNode, uint8_t u8Count);

#ifdef __cplusplus
}
#endif

#endif
