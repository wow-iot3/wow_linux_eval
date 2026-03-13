#ifndef __WOW_IOT_PROTOCOL_WOW_IEC104M_H_
#define __WOW_IOT_PROTOCOL_WOW_IEC104M_H_

#include "protocol/wow_proto_port.h"
#include "protocol/iec10x/wow_iec10x.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct iec104m_t Iec104m_T;

/*brief    Iec104主设备初始化
 *param ： u16Addr: 公共地址 
 *param ： ptEthPar:  硬件参数
 *return： 成功返回iec04m操作标识 失败返回NULL
 */
Iec104m_T* wow_iec104_m_init(uint16_t u16Addr,ProtoEthParam_T* ptEthPar);

/*brief    退出Iec104主设备
 *param ： ptIec104m:iec04m操作标识
 *return： 无
 */
void  wow_iec104_m_exit(Iec104m_T** ptIec104m);

/*brief    启动Iec104主设备
 *param ： ptIec104m:iec04m操作标识
 *return： 成功返回0 失败返回<0
 */
int  wow_iec104_m_start(Iec104m_T* ptIec104m);

/*brief    Iec104设置更新数据节点参数回调函数
 *param ： ptIec104m:iec04m操作标识
 *param ： fCallBack:   回调函数 
 *param ： pArg:  回调函数私有变量
 *return： 成功返回0 失败返回<0
 */
int wow_iec104_m_set_value_handle(Iec104m_T* ptIec104m,iec10x_set_value_func_t fCallBack,void* pArg);


/*brief    Iec104设置更新服务状态回调函数
 *param ： ptIec104m:iec04m操作标识
 *param ： fCallBack:   回调函数 
 *param ： pArg:  回调函数私有变量
 *return： 成功返回0 失败返回<0
 */
int wow_iec104_m_set_service_handle(Iec104m_T* ptIec104m,iec10x_set_service_func_t fCallBack,void* pArg);


/*brief    iec04总召
 *param ： ptIec104m:iec04m操作标识
 *return： 成功返回0 失败返回<0
 */
int wow_iec104_m_build_call_all(Iec104m_T* ptIec104m);

/*brief    iec04电度量总召
 *param ： ptIec104m:iec04m操作标识
 *return： 成功返回0 失败返回<0
 */
int wow_iec104_m_build_call_pulse(Iec104m_T* ptIec104m);

/*brief    iec04遥控
 *param ： ptIec104m:iec04m操作标识
 *param ： node:遥控节点信息
 *param ： count:遥控节点个数
 *return： 成功返回0 失败返回<0
 */
int wow_iec104_m_build_yk(Iec104m_T* ptIec104m,DevNodeCtrl_T* node,uint8_t count);

/*brief    iec04遥调
 *param ： ptIec104m:iec04m操作标识
 *param ： node:遥调节点信息
 *param ： count:遥调节点个数
 *return： 成功返回0 失败返回<0
 */
int wow_iec104_m_build_yt(Iec104m_T* ptIec104m,DevNodeCtrl_T* node, uint8_t count);

#ifdef __cplusplus
}
#endif

#endif
