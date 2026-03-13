#ifndef __WOW_BASE_NETWORK_WOW_TCP_H_
#define __WOW_BASE_NETWORK_WOW_TCP_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tcp_t Tcp_T;

typedef int (*tcp_ctx_callback_func_t)(Tcp_T* ptTcp,void* arg);

/*brief    创建tcp操作符	---客户端/服务端函数
 *param ： pcLoaclAddr:本地IP地址
 *param ： u16LocalPort:本地端口号
 *return： 成功返回tcp操作符 失败返回NULL
 */
Tcp_T* wow_tcp_open(const char *pcLoaclAddr, uint16_t u16LocalPort);

/*brief    tcp客户端连接服务端---客户端函数
 *param ： ptTcp:tcp操作符
 *param ： pcPeerAddr:本地IP地址
 *param ： u16PeerPort:本地端口号
 *return： 成功返回0 失败返回-1
 */
int wow_tcp_connect(Tcp_T* ptTcp,const char *pcPeerAddr,uint16_t u16PeerPort);

/*brief    tcp服务端设置监听个数	---服务端函数
 *param ： ptTcp:tcp操作符
 *param ： u16Max:监听个数
 *return： 成功返回0 失败返回-1
 */
int wow_tcp_listen(Tcp_T* ptTcp,uint16_t u16Max);

/*brief    tcp服务端接收客户端连接---服务端函数
 *param ： ptTcp:tcp操作符
 *return： 成功返回tcp操作符 失败返回NULL
 */
Tcp_T* wow_tcp_accpet(Tcp_T* ptTcp);

/*brief    设置tcp服务端设置接收回调	---服务端函数
 *param ： ptTcp:tcp操作符
 *param ： fCallback:接收客户端后服务端回调函数
 *param ： pArg:接收客户端后服务端回调函数私有变量
 *return： 成功返回0 失败返回-1
 */
int wow_tcp_accpet_cb(Tcp_T* ptTcp,tcp_ctx_callback_func_t fCallback,void* pArg);

/*brief    tcp读取数据---客户端/服务端函数
 *param ： ptTcp:tcp操作符
 *param ： pu8Data:接收数据内容存储地址
 *param ： snSize:接收数据内容存储长度
 *return： 成功返回接收长度 失败返回-1
 */
int wow_tcp_read(Tcp_T* ptTcp,uint8_t* pu8Data, size_t snSize);

/*brief    tcp读取数据(超时时间)---客户端/服务端函数
 *param ： ptTcp:tcp操作符
 *param ： pu8Data:接收数据内容存储地址
 *param ： snSize:接收数据内容存储长度
 *param ： nMs:超时时间(ms)
 *return： 成功返回接收长度 失败返回-1
 */
int wow_tcp_read_timeout(Tcp_T* ptTcp, uint8_t *pu8Data, size_t snSize, int nMs);

/*brief    设置tcp读取回调函数---客户端/服务端函数(优化)
 *param ： ptTcp:tcp操作符
 *param ： fCallback:接收数据回调函数
 *param ： pArg:接收数据回调函数私有变量
 *return： 成功返回0 失败返回-1
 */
int wow_tcp_read_cb(Tcp_T* ptTcp,tcp_ctx_callback_func_t fCallback,void* pArg);

/*brief    tcp写入数据---客户端/服务端函数
 *param ： ptTcp:tcp操作符
 *param ： pu8Data:写入数据内容存储地址
 *param ： snSize:写入数据内容存储长度
 *return： 成功返回写入长度 失败返回-1
 */
int wow_tcp_write(Tcp_T* ptTcp,const uint8_t* pu8Data ,size_t snSize);

/*brief    tcp写入数据(超时时间)---客户端/服务端函数
 *param ： ptTcp:tcp操作符
 *param ： pu8Data:写入数据内容存储地址
 *param ： snSize:写入数据内容存储长度
 *param ： nMs:超时时间(ms)
 *return： 成功返回写入长度 失败返回-1
 */
int wow_tcp_write_timeout(Tcp_T* ptTcp, const uint8_t *pu8Data, size_t snSize, int nMs);

/*brief    关闭tcp操作符---客户端/服务端函数
 *param ： ptTcp:tcp操作符
 *return： 无
 */
void wow_tcp_close(Tcp_T* ptTcp);


/*brief    获取tcp连接信息---客户端/服务端函数
 *param ： ptTcp:tcp操作符
 *param ： pcPeerAddr:连接端IP地址
 *param ： pu16PeerPort:连接端端口号
 *return： 成功返回0 失败返回-1
 */
 int wow_tcp_peer(Tcp_T* ptTcp,char *pcPeerAddr,uint16_t* pu16PeerPort);

#ifdef __cplusplus
 }
#endif

#endif
