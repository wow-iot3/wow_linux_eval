#ifndef __WOW_BASE_NETWORK_WOW_UDP_H_
#define __WOW_BASE_NETWORK_WOW_UDP_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct udp_t Udp_T;


/*brief    创建udp操作符	---客户端/服务端函数
 *param ： pcLoaclAddr:本地IP地址
 *param ： u16LocalPort:本地端口号
 *return： 成功返回udp操作符 失败返回NULL
 */
Udp_T* wow_udp_open(const char *pLoaclAddr, uint16_t nLocalPort);

/*brief    udp客户端连接服务端---客户端函数
 *param ： ptUdp:udp操作符
 *param ： pcPeerAddr:连接端IP地址
 *param ： u16PeerPort:连接端端口号
 *return： 成功返回0 失败返回<0
 */
int wow_udp_connect(Udp_T* ptUdp,const char *pPeerAddr,uint16_t nPeerPort);

/*brief    获取udp连接信息---客户端/服务端函数
 *param ： ptUdp:udp操作符
 *param ： pcPeerAddr:连接端IP地址
 *param ： pu16PeerPort:连接端端口号
 *return： 成功返回0 失败返回<0
 */
int wow_udp_peer(Udp_T* ptUdp,char *pPeerAddr,uint16_t* nPeerPort);

/*brief    关闭udp操作符---客户端/服务端函数
 *param ： ptUdp:udp操作符
 *return： 成功返回0 失败返回<0
 */
int wow_udp_close(Udp_T* ptUdp);


/*brief    udp读取数据---客户端/服务端函数
 *param ： ptUdp:udp操作符
 *param ： pu8Data:接收数据内容存储地址
 *param ： snSize:接收数据内容存储长度
 *return： 成功返回接收长度 失败返回<0
 */
int wow_udp_read(Udp_T* ptUdp,uint8_t* pu8Data, size_t snSIze);

/*brief    udp读取数据(超时时间)---客户端/服务端函数
 *param ： ptUdp:udp操作符
 *param ： pu8Data:接收数据内容存储地址
 *param ： snSize:接收数据内容存储长度
 *param ： nMs:超时时间(ms)
 *return： 成功返回接收长度 失败返回<0
 */
int wow_udp_read_timeout(Udp_T* ptUdp,uint8_t *pu8Data, size_t snSIze, int nMs);

/*brief    udp写入数据---客户端/服务端函数
 *param ： ptUdp:udp操作符
 *param ： pu8Data:写入数据内容存储地址
 *param ： snSize:写入数据内容存储长度
 *return： 成功返回写入长度 失败返回<0
 */
int wow_udp_write(Udp_T* ptUdp,const uint8_t* pu8Data ,size_t snSIze);


/*brief    udp写入数据(超时时间)---客户端/服务端函数
 *param ： ptUdp:udp操作符
 *param ： pu8Data:写入数据内容存储地址
 *param ： snSize:写入数据内容存储长度
 *param ： nMs:超时时间(ms)
 *return： 成功返回写入长度 失败返回<0
 */
int wow_udp_write_timeout(Udp_T* ptUdp, const uint8_t *pu8Data, size_t snSIze, int nMs);



#ifdef __cplusplus
}
#endif


#endif

