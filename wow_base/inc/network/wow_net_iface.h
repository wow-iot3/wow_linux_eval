#ifndef __WOW_BASE_MONITOR_WOW_ETH_IFACE_H_
#define __WOW_BASE_MONITOR_WOW_ETH_IFACE_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#include "list/wow_string_list.h"

typedef struct	{
	uint16_t tcp[8192];
	uint16_t tcp_cnt;
	uint16_t udp[8192];
	uint16_t udp_cnt;
}NetworkPorts_T;

/*brief    获取所有iface接口名称
 *return： 成功返回iface接口名称链表 失败返回NULL
 *注： 返回值StringList_T链表需要调用wow_stringlist_free手动释放
 */
StringList_T* wow_eth_getIfaceName(void);

/*brief    检测iface接口是否存在
 *param ： pcIface: 网卡接口名称
 *return： 存在此接口返回0  执行错误<0
 */
int wow_eth_checkIfaceStatus(const char *pcIface);


/*brief    检测iface接口是否网线拔出
 *param ： pcIface: 网卡接口名称
 *return： 可使用未拔出返回0 不可使用已拔出<0
 */
int wow_eth_checkIfaceRunning(const char *pcIface);

/*brief    设置iface接口Ip地址
 *param ： pcIface: 网卡接口名称
 *param ： pcIp: 存储iface的ip地址--最少16个字节
 *return： 成功返回0 失败返回<0
 */
int wow_eth_setIfaceIp(char *pcIface, char *pcIp);

/*brief    获取iface接口Ip地址
 *param ： pcIface: 网卡接口名称
 *param ： pcIp: 存储iface的ip地址--最少16个字节
 *return： 成功返回0 失败返回<0
 */
int wow_eth_getIfaceIp(char *pcIface, char *pcIp);

/*brief    获取iface接口mask地址
 *param ： pcIface: 网卡接口名称
 *param ： pcMask : 存储iface的mask地址--最少16个字节
 *return： 成功返回0 失败返回<0
 */
int wow_eth_getIfaceMask(char *pcIface, char* pcMask);

/*brief    获取iface接口mac地址
 *param ： pcIface: 网卡接口名称
 *param ： pcMac  : 存储iface的mac地址--最少32个字节
 *return： 成功返回0 失败返回<0
 */
int wow_eth_getIfaceMac(char *pcIface, char *pcMac);

/*brief    设置iface接口mac地址
 *param ： pcIface: 网卡接口名称
 *param ： pcMac: 要设置的mac地址
 *return： 成功返回0 失败返回<0
 */
int wow_eth_setIfaceMac(char *pcIface, const char *pcMac);

/*brief    获取iface接口broard广播地址
 *param ： pcIface: 网卡接口名称
 *param ： pcBoard: 存储iface的broard广播地址--最少16个字节
 *return： 成功返回0 失败返回<0
 */
int wow_eth_getIfacBroardcast(char *pcIface, char* pcBoard);

/*brief    获取iface接口所用流量
 *param ： pcIface: 网卡接口名称
 *param ： snTx: 存储iface的发送字节数
 *param ： snRx: 存储iface的接收字节数
 *return： 成功返回0 失败返回<0
 */
int wow_eth_getIfaceRtx(const char *pcIface, size_t *snTx,size_t* snRx);

/*brief    获取所有iface接口速度
 *param ： pcIface: 网卡接口名称
 *param ： pdUpload: 上行速率
 *param ： pdDnload: 下行速率
 *return： 成功返回0 失败返回<0
 */
int wow_eth_getIfacSpeed(const char *pcIface,double *pdUpload, double *pdDnload);

/*brief    根据以太网TCP\UDP已占用端口号
 *param ： ptNPinfo: 存放获取内容缓存地址
 *return： 获取成功返回0 失败返回<0
 */
int wow_eth_getOccupiedPort(NetworkPorts_T *ptNPinfo);


/*brief    获取一本机IP地址
 *param ： pcIp: IP地址存储
 *return： 获取成功返回0 失败返回<0
 */
int wow_eth_getLocalIp(char *pcIp);

#ifdef __cplusplus
}
#endif

#endif
