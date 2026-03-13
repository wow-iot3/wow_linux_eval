#ifndef __WOW_IOT_NETWORK_WOW_NETWORK_H_
#define __WOW_IOT_NETWORK_WOW_NETWORK_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum{
	NETWORK_MODE_PHY = 0,
	NETWORK_MODE_GPRS,
	NETWORK_MODE_MAX
}NetworkMode_E;
	
typedef enum {
	CORE_SYSDEP_NETWORK_SOCKET_TYPE,			 /* 需要建立的socket类型  数据类型: (core_sysdep_socket_type_t *) */
	CORE_SYSDEP_NETWORK_HOST,					 /* 用于建立网络连接的域名地址或ip地址, 内存与上层模块共用  数据类型: (char *) */
	CORE_SYSDEP_NETWORK_BACKUP_IP,				 /* 当建联DNS解析失败时, 使用此备用ip重试 */
	CORE_SYSDEP_NETWORK_PORT,					 /* 用于建立网络连接的端口号  数据类型: (uint16_t *) */
	CORE_SYSDEP_NETWORK_CONNECT_TIMEOUT_MS, 	 /* 建立网络连接的超时时间  数据类型: (uint32_t *) */
	CORE_SYSDEP_NETWORK_CRED,					 /* 用于设置网络层安全参数  数据类型: (aiot_sysdep_network_cred_t *) */
	CORE_SYSDEP_NETWORK_PSK,					 /* 用于配合PSK模式下的psk-id和psk  数据类型: (core_sysdep_psk_t *) */
	CORE_SYSDEP_NETWORK_MAX
}CoreSysdepNetOpt_E;

typedef enum {
    CORE_SYSDEP_NETWORK_CRED_NONE,
    CORE_SYSDEP_NETWORK_CRED_SVRCERT_CA,
    CORE_SYSDEP_NETWORK_CRED_SVRCERT_PSK,
    CORE_SYSDEP_NETWORK_CRED_MAX
} CoreSysdepNetCredOption_E;

typedef struct {
    CoreSysdepNetCredOption_E option;  /* 安全策略 */
    uint32_t      max_tls_fragment;
    uint8_t       sni_enabled;
    const char   *x509_server_cert;     
    uint32_t      x509_server_cert_len;
    const char   *x509_client_cert;     
    uint32_t      x509_client_cert_len;
    const char   *x509_client_privkey; 
    uint32_t      x509_client_privkey_len;
    char         *tls_extend_info;
}CoreSysdepNetworkCred_T;	

#define NETWORK_DEFAULT_CONNECT_TIMEOUT (5000)
#define NETWORK_DEFAULT_SEND_TIMEOUT (3000)
#define NETWORK_DEFAULT_RECV_TIMEOUT (3000)

typedef struct network_adapter_t NetworkAdapter_T;

/*brief    配置网络模式
 *param ： eMode   :网络模式
 *return： 成功返回0 失败返回-1
 */
int wow_network_config(NetworkMode_E eMode);


/*brief    网络适配器初始化
 *param ： tls   :是否为tls模式
 *return： 成功返回操作描述符 失败返回-1
 */
NetworkAdapter_T* wow_network_adapter_init(bool tls);

/*brief    网络适配器参数设置
 *param ： ptNetAdp :操作描述符
 *param ： option :操作标识
 *param ： eOption   :标识数据
 *return： 成功返回0 失败返回-1
 */
int wow_network_adapter_setopt(NetworkAdapter_T* ptNetAdp, CoreSysdepNetOpt_E eOption, void *pData);

/*brief    网络适配器建立连接
 *param ： ptNetAdp :操作描述符
 *return： 成功返回0 失败返回-1
 */
int wow_network_adapter_connect(NetworkAdapter_T* ptNetAdp);

/*brief    网络适配器接收数据
 *param ： ptNetAdp :操作描述符
 *param ： pu8Buff  :数据内容
 *param ： snLen    :数据长度
 *param ： u32TimeoutMs:超时时间
 *return： 成功返回接收长度 失败返回-1
 */
int wow_network_adapter_recv(NetworkAdapter_T* ptNetAdp, uint8_t *pu8Buff, size_t snLen, uint32_t u32TimeoutMs);

/*brief    网络适配器发送数据
 *param ： ptNetAdp :操作描述符
 *param ： pu8Buff  :数据内容
 *param ： snLen    :数据长度
 *param ： u32TimeoutMs:超时时间
 *return： 成功返回发送长度 失败返回-1
 */
int wow_network_adapter_send(NetworkAdapter_T* ptNetAdp, uint8_t *pu8Buff, size_t snLen, uint32_t u32TimeoutMs);

/*brief    网络适配器断开连接
 *param ： ptNetAdp :操作描述符
 *return： 成功返0 失败返回-1
 */
int wow_network_adapter_disconnect(NetworkAdapter_T* ptNetAdp);

/*brief    网络适配器去初始化
 *param ： ptNetAdp :操作描述符
 *return： 成功返回0 失败返回-1
 */
int wow_network_adapter_deinit(NetworkAdapter_T* *ptNetAdp);

#ifdef __cplusplus
}
#endif

#endif

