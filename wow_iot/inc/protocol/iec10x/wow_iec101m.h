#ifndef __WOW_IOT_PROTOCOL_WOW_IEC101M_H_
#define __WOW_IOT_PROTOCOL_WOW_IEC101M_H_


#include "protocol/wow_proto_port.h"
#include "protocol/iec10x/wow_iec10x.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct iec101m_t Iec101m_T;

/*brief    Iec101主设备初始化
 *param ： u16Addr  : 公共地址 
 *param ： ptUartPar: 硬件参数
 *return： 成功返回iec04m操作标识 失败返回NULL
 */
Iec101m_T* wow_iec101_m_init(uint16_t u16Addr,ProtoUartParam_T* ptUartPar);

/*brief    退出Iec101主设备
 *param ： pptIec101m:iec04m操作标识
 *return： 无
 */
void  wow_iec101_m_exit(Iec101m_T** pptIec101m);

/*brief    启动Iec101主设备
 *param ： ptIec101m:iec04m操作标识
 *return： 成功返回0 失败返回<0
 */
int  wow_iec101_m_start(Iec101m_T* ptIec101m);

/*brief    Iec101设置更新数据节点参数回调函数
 *param ： ptIec101m:iec04m操作标识
 *param ： fCallBack: 回调函数 
 *param ： pArg:  回调函数私有变量
 *return： 成功返回0 失败返回<0
 */
int wow_iec101_m_set_value_handle(Iec101m_T* ptIec101m,
					iec10x_set_value_func_t fCallBack,void* pArg);

/*brief    Iec101设置更新服务状态回调函数
 *param ： ptIec101m:iec04m操作标识
 *param ： ctxfCallBack   回调函数 
 *param ： pArg:  回调函数私有变量
 *return： 成功返回0 失败返回<0
 */
int wow_iec101_m_set_service_handle(Iec101m_T* ptIec101m,
					iec10x_set_service_func_t fCallBack,void* pArg);

/*brief    iec04总召
 *param ： ptIec101m:iec04m操作标识
 *return： 成功返回0 失败返回<0
 */
int wow_iec101_m_build_call_all(Iec101m_T* ptIec101m);

/*brief    iec04电度量总召
 *param ： ptIec101m:iec04m操作标识
 *return： 成功返回0 失败返回<0
 */
int wow_iec101_m_build_call_pulse(Iec101m_T* ptIec101m);

/*brief    iec04遥控
 *param ： ptIec101m:iec04m操作标识
 *param ： ptNode:遥控节点信息
 *param ： u8Count:遥控节点个数
 *return： 成功返回0 失败返回<0
 */
int wow_iec101_m_build_yk(Iec101m_T* ptIec101m,DevNodeCtrl_T* ptNode,uint8_t u8Count);

/*brief    iec04遥调
 *param ： ptIec101m:iec04m操作标识
 *param ： ptNode:遥调节点信息
 *param ： u8Count:遥调节点个数
 *return： 成功返回0 失败返回<0
 */
int wow_iec101_m_build_yt(Iec101m_T* ptIec101m,DevNodeCtrl_T* ptNode, uint8_t u8Count);


#ifdef __cplusplus
}
#endif

#endif
