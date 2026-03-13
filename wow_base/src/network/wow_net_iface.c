#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"

#include "network/wow_net_iface.h"



/*brief    获取所有iface接口名称
 *return： 成功返回iface接口名称链表 失败返回NULL
 *注： 返回值StringList_T链表需要调用wow_stringlist_free手动释放
 */
__EX_API__ StringList_T* wow_eth_getIfaceName(void)
{
	int fd  = 0;
	int ret = 0;
	int if_num = 0;
    struct ifconf ifc;
    struct ifreq  buf[MAX_IP_SIZE];
    StringList_T*  list = NULL;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
	CHECK_RET_VAL_ERRNO_P(fd > 0,-SOCKET_OPEN_FAILED,"socket faild\n");
	
    memset(&ifc, 0, sizeof(struct ifconf));
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = (caddr_t)buf;
    ret = ioctl(fd, SIOCGIFCONF, &ifc);
  	CHECK_RET_GOTO_ERRNO_P(ret == 0, out,-SYSTEM_IOCTRL_FAILED,"ioctl SIOCGIWESSID faild\n");
	
    if_num = ifc.ifc_len / sizeof(struct ifreq);
    CHECK_RET_GOTO_ERRNO_P(if_num > 0, out,-SYSTEM_IOCTRL_FAILED,"ioctl SIOCGIWESSID faild\n");

    list = wow_stringlist_new();
    while (if_num-- > 0) {
        ret = wow_stringlist_append(list,buf[if_num].ifr_name);
        CHECK_RET_BREAK_P(ret == 0,"wow_stringlist_append failed.");
    }

	close(fd);
	return list;

out:
    close(fd);
    return NULL;    
}


/*brief    检测iface接口是否存在
 *param ： pcIface: 网卡接口名称
 *return： 存在此接口返回0  执行错误<0
 */
__EX_API__ int wow_eth_checkIfaceStatus(const char *pcIface)
{
	CHECK_RET_VAL_P(pcIface,-PARAM_INPUT_DATA_IS_NULL, "param input data invalid!\n");

	char net_dev[128] = {0};
    snprintf(net_dev, sizeof(net_dev), "%s/%s", "/sys/class/net", pcIface);///sys/class类

	int ret = access(net_dev, F_OK|W_OK|R_OK);
	CHECK_RET_VAL_P(ret == 0,-SYSTEM_ACCESS_FAILED, "access failed!\n");
		
	return WOW_SUCCESS;
}

/*brief    检测iface接口是否网线拔出
 *param ： pcIface: 网卡接口名称
 *return： 可使用未拔出返回0 不可使用已拔出<0
 */
__EX_API__ int wow_eth_checkIfaceRunning(const char *pcIface)
{
	int ret = -1;
	int sockfd = 0;
	
	CHECK_RET_VAL_P(pcIface,-PARAM_INPUT_DATA_IS_NULL, "param input data invalid!\n");

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	CHECK_RET_VAL_P(sockfd  > 0,-SOCKET_OPEN_FAILED,"socket faild\n");
	
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, pcIface,IFNAMSIZ);
	
    ret = ioctl(sockfd, SIOCGIFFLAGS, &ifr);
  	CHECK_RET_VAL_EXE_P(ret == 0,-SYSTEM_IOCTRL_FAILED,close(sockfd),"ioctl SIOCGIWESSID faild\n");

	
	close(sockfd);

	//拔除网络线时会造成 IFF_RUNNING 的改变，IFF_UP 不会因插拔网络线而有任何变化
    if (ifr.ifr_flags & IFF_RUNNING) {
        return 1;
    } 
	return  (ifr.ifr_flags & IFF_RUNNING)?0:-WOW_FAILED;

}

/*brief    设置iface接口Ip地址
 *param ： pcIface: 网卡接口名称
 *param ： pcIp: 存储iface的ip地址--最少16个字节
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_eth_setIfaceIp(char *pcIface, char *pcIp)
{
	int ret = -1;
    int sockfd = -1;
    struct ifreq ifr;
    struct sockaddr_in *addr = NULL;

	CHECK_RET_VAL_P(pcIface && pcIp,-PARAM_INPUT_DATA_IS_NULL, "param input data invalid!\n");
	
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, pcIface,IFNAMSIZ);
    addr = (struct sockaddr_in *)&ifr.ifr_addr;
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = inet_addr(pcIp);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	CHECK_RET_VAL_P(sockfd > 0,-SOCKET_OPEN_FAILED,"socket faild\n");
	
	//get the pcIp of this interface
    ret = ioctl(sockfd, SIOCSIFADDR, &ifr);
  	CHECK_RET_VAL_EXE_P(ret == 0,-SYSTEM_IOCTRL_FAILED,close(sockfd),"ioctl SIOCGIWESSID faild\n");

	close(sockfd);
    return WOW_SUCCESS;
}


/*brief    获取iface接口Ip地址
 *param ： pcIface: 网卡接口名称
 *param ： pcIp: 存储iface的ip地址--最少16个字节
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_eth_getIfaceIp(char *pcIface, char *pcIp)
{
	int ret = -1;
    int sockfd = -1;
    struct ifreq ifr;
    struct sockaddr_in *addr = NULL;

	CHECK_RET_VAL_P(pcIface && pcIp,-PARAM_INPUT_DATA_IS_NULL, "param input data invalid!\n");
	
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, pcIface,IFNAMSIZ);
    addr = (struct sockaddr_in *)&ifr.ifr_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	CHECK_RET_VAL_P(sockfd > 0,-SOCKET_OPEN_FAILED,"socket faild\n");
	
	 //get the pcIp of this interface
	ret = ioctl(sockfd, SIOCGIFADDR, &ifr);
  	CHECK_RET_VAL_EXE_P(ret == 0,-SYSTEM_IOCTRL_FAILED,close(sockfd),"ioctl SIOCGIFADDR faild\n");
	
    strncpy(pcIp, inet_ntoa(addr->sin_addr), MAX_IP_SIZE);
    close(sockfd);
    return WOW_SUCCESS;
}



/*brief    获取iface接口mac地址
 *param ： pcIface: 网卡接口名称
 *param ： pcMac: 存储iface的mac地址--最少32个字节
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_eth_getIfaceMac(char *pcIface, char *pcMac)
{
	int ret = -1;
    int sockfd = -1;
    struct ifreq ifr;

	CHECK_RET_VAL_P(pcIface && pcMac,-PARAM_INPUT_DATA_IS_NULL, "param input data invalid!\n");
	
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, pcIface,IFNAMSIZ);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	CHECK_RET_VAL_P(sockfd > 0,-SOCKET_OPEN_FAILED,"socket faild\n");

	ret = ioctl(sockfd, SIOCGIFHWADDR, &ifr);
  	CHECK_RET_VAL_EXE_P(ret == 0,-SYSTEM_IOCTRL_FAILED,close(sockfd),"ioctl SIOCGIFHWADDR faild\n");
	
	//get the mac of this interface
    snprintf(pcMac, 32,
        "%02X:%02X:%02X:%02X:%02X:%02X",
        (unsigned char)ifr.ifr_hwaddr.sa_data[0],
        (unsigned char)ifr.ifr_hwaddr.sa_data[1],
        (unsigned char)ifr.ifr_hwaddr.sa_data[2],
        (unsigned char)ifr.ifr_hwaddr.sa_data[3],
        (unsigned char)ifr.ifr_hwaddr.sa_data[4],
        (unsigned char)ifr.ifr_hwaddr.sa_data[5]);
	
    close(sockfd);
    return WOW_SUCCESS;
}

/*brief    设置iface接口mac地址
 *param ： pcIface: 网卡接口名称
 *param ： pcMac: 要设置的mac地址
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_eth_setIfaceMac(char *pcIface, const char *pcMac)
{
    int ret = -1;
    int res = -1;
    int sockfd = -1;
    struct ifreq ifr;
    unsigned int mac[6];

    CHECK_RET_VAL_P(pcIface && pcMac, -PARAM_INPUT_DATA_IS_NULL, "param input data invalid!\n");
    
    // 解析MAC地址字符串
    ret = sscanf(pcMac, "%02X:%02X:%02X:%02X:%02X:%02X", 
               &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
    CHECK_RET_VAL_P(ret == 6, -PARAM_INPUT_DATA_ERROR, "param input data invalid!\n");

    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, pcIface, IFNAMSIZ);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    CHECK_RET_VAL_P(sockfd > 0, -SOCKET_OPEN_FAILED, "socket failed\n");

    // 先获取当前的MAC地址信息
    ret = ioctl(sockfd, SIOCGIFHWADDR, &ifr);
    CHECK_RET_VAL_EXE_P(ret == 0, -SYSTEM_IOCTRL_FAILED, close(sockfd), "ioctl SIOCGIFHWADDR failed\n");

    // 设置新的MAC地址
    ifr.ifr_hwaddr.sa_data[0] = (unsigned char)mac[0];
    ifr.ifr_hwaddr.sa_data[1] = (unsigned char)mac[1];
    ifr.ifr_hwaddr.sa_data[2] = (unsigned char)mac[2];
    ifr.ifr_hwaddr.sa_data[3] = (unsigned char)mac[3];
    ifr.ifr_hwaddr.sa_data[4] = (unsigned char)mac[4];
    ifr.ifr_hwaddr.sa_data[5] = (unsigned char)mac[5];

    // 需要先关闭网卡
    ret = ioctl(sockfd, SIOCGIFFLAGS, &ifr);
    CHECK_RET_VAL_EXE_P(ret == 0, -SYSTEM_IOCTRL_FAILED, close(sockfd), "ioctl SIOCGIFFLAGS failed\n");
    
    ifr.ifr_flags &= ~IFF_UP; // 关闭接口
    ret = ioctl(sockfd, SIOCSIFFLAGS, &ifr);
    CHECK_RET_VAL_EXE_P(ret == 0, -SYSTEM_IOCTRL_FAILED, close(sockfd), "ioctl SIOCSIFFLAGS (down) failed\n");

    // 设置MAC地址
    ret = ioctl(sockfd, SIOCSIFHWADDR, &ifr);
    // 重新启用接口
    ifr.ifr_flags |= IFF_UP;
    res = ioctl(sockfd, SIOCSIFFLAGS, &ifr);
    close(sockfd);

    CHECK_RET_VAL_P(ret == 0, -SYSTEM_IOCTRL_FAILED, "ioctl SIOCSIFHWADDR  failed\n");
    CHECK_RET_VAL_P(res == 0, -SYSTEM_IOCTRL_FAILED, "ioctl SIOCSIFFLAGS (up) failed\n");

    
    return WOW_SUCCESS;
}

/*brief    获取iface接口broard广播地址
 *param ： pcIface: 网卡接口名称
 *param ： pcBoard: 存储iface的broard广播--最少16个字节
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_eth_getIfacBroardcast(char *pcIface, char* pcBoard)
{
	int ret = -1;
    int sockfd = -1;
    struct ifreq ifr;

	CHECK_RET_VAL_P(pcIface && pcBoard,-PARAM_INPUT_DATA_IS_NULL, "param input data invalid!\n");
	
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, pcIface,IFNAMSIZ);
    struct sockaddr_in *addr = (struct sockaddr_in *)&ifr.ifr_broadaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	CHECK_RET_VAL_P(sockfd > 0,-SOCKET_OPEN_FAILED,"socket faild\n");

	
	//get the broad address of this interface
	ret = ioctl(sockfd, SIOCGIFBRDADDR, &ifr);
	CHECK_RET_VAL_EXE_P(ret == 0,-SYSTEM_IOCTRL_FAILED,close(sockfd),"ioctl SIOCGIFBRDADDR faild\n");
		
    snprintf(pcBoard, 16, "%s",(char *)inet_ntoa(addr->sin_addr));
    close(sockfd);
    return WOW_SUCCESS;
}


/*brief    获取iface接口mask地址
 *param ： pcIface: 网卡接口名称
 *param ： pcMask: 存储iface的mask地址--最少16个字节
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_eth_getIfaceMask(char *pcIface, char* pcMask)
{
	int ret = -1;
    int sockfd = -1;
    struct ifreq ifr;

	CHECK_RET_VAL_P(pcIface && pcMask,-PARAM_INPUT_DATA_IS_NULL, "param input data invalid!\n");
	
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, pcIface,IFNAMSIZ);
    struct sockaddr_in *addr = (struct sockaddr_in *)&ifr.ifr_netmask;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	CHECK_RET_VAL_P(sockfd > 0,-SOCKET_OPEN_FAILED,"socket faild\n");

	//get the subnet mask of this interface
    ret = ioctl(sockfd, SIOCGIFNETMASK, &ifr);
	CHECK_RET_VAL_EXE_P(ret == 0,-SYSTEM_IOCTRL_FAILED,close(sockfd),"ioctl SIOCGIFNETMASK faild\n");
		
    snprintf(pcMask, 16, "%s",(char *)inet_ntoa(addr->sin_addr));
    close(sockfd);
    return WOW_SUCCESS;

}


/*brief    根据以太网TCP\UDP已占用端口号
 *param ： ptNPinfo: 存放获取内容缓存地址
 *return： 获取成功返回0 失败返回<0
 */
__EX_API__ int wow_eth_getOccupiedPort(NetworkPorts_T *ptNPinfo)
{
    FILE* fp;
    char line[256];
    unsigned int port;
	
	CHECK_RET_VAL_P(ptNPinfo,-PARAM_INPUT_STRUCT_IS_NULL, "param input struct invalid!\n");


    memset(ptNPinfo, 0, sizeof(NetworkPorts_T));
    fp = fopen("/proc/self/net/tcp", "r");
	CHECK_RET_VAL_P(fp,-SYSTEM_FOPEN_FAILED,"fopen  faild!\n");
	
    while (fgets(line, sizeof(line), fp)) 
	{
        if (1 == sscanf(line, " %*d: %*p:%x", &port)) 
		{
            ptNPinfo->tcp[ptNPinfo->tcp_cnt] = port;
            ptNPinfo->tcp_cnt++;
        }
    }
    fclose(fp);

    fp = fopen("/proc/self/net/udp", "r");
	CHECK_RET_VAL_P(fp,-SYSTEM_FOPEN_FAILED,"fopen faild!\n");
    while (fgets(line, sizeof(line), fp)) 
	{
        if (1 == sscanf(line, " %*d: %*p:%x", &port)) 
		{
            ptNPinfo->udp[ptNPinfo->udp_cnt] = port;
            ptNPinfo->udp_cnt++;
        }
    }
    fclose(fp);
	
    return WOW_SUCCESS;
}


/*brief    获取iface接口所用流量
 *param ： pcIface: 网卡接口名称
 *param ： psnTx: 存储iface的发送字节数
 *param ： psnRx: 存储iface的接收字节数
 *return： 成功返回0 失败返回M0
 */
__EX_API__ int wow_eth_getIfaceRtx(const char *pcIface, size_t *psnTx,size_t* psnRx)
{
	size_t read;
    size_t bytes_read;
    char str[2][16];
	char *line = NULL;

	/*文件/proc/net/dev*/
	FILE *file = fopen("/proc/net/dev", "r");
	CHECK_RET_VAL_P(file,-SYSTEM_FOPEN_FAILED,"fopen faild!\n");

    //从第三行开始读取网络接口数据
    while ((read = getline(&line, &bytes_read, file)) != -1)
    {
        if (strstr(line, pcIface) != NULL)
        {
			memset(str,0,sizeof(str));
            sscanf(line,"%*s%s%*s%*s%*s%*s%*s%*s%*s%s",str[0],str[1]);
            *psnTx = atol(str[0]);
            *psnRx = atol(str[1]);
        }
    }
    fclose(file);
	
	return WOW_SUCCESS;
}


/*收发数据包结构体*/
typedef struct{
  size_t tx_bytes;
  size_t rx_bytes;
  struct timeval rtx_time;
} EthRtxBytes_T;

/*brief    获取所有iface接口速度
 *param ： pcIface: 网卡接口名称
 *param ： pdUpload: 上行速率
 *param ： pdDnload: 下行速率
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_eth_getIfacSpeed(const char *pcIface,double *pdUpload, double *pdDnload)
{

	CHECK_RET_VAL_P(pcIface && pdUpload && pdDnload,-PARAM_INPUT_DATA_IS_NULL, "param input data invalid!\n");
	

    EthRtxBytes_T rtx0, rtx1;

    gettimeofday(&rtx0.rtx_time, NULL);
    wow_eth_getIfaceRtx(pcIface, &rtx0.tx_bytes, &rtx0.rx_bytes);
    sleep(1);
	gettimeofday(&rtx1.rtx_time, NULL);
    wow_eth_getIfaceRtx(pcIface, &rtx1.tx_bytes, &rtx1.rx_bytes);


    int64_t time_lapse = (rtx1.rtx_time.tv_sec * 1000 + rtx1.rtx_time.tv_usec / 1000) - (rtx0.rtx_time.tv_sec * 1000 + rtx0.rtx_time.tv_usec / 1000);

    *pdDnload = (rtx1.rx_bytes - rtx0.rx_bytes) * 1.0 / (1024 * time_lapse * 1.0 / 1000);
    *pdUpload = (rtx1.tx_bytes - rtx0.tx_bytes) * 1.0 / (1024 * time_lapse * 1.0 / 1000);

	return WOW_SUCCESS;
}

/*brief    获取一本机IP地址
 *param ： csIp: IP地址存储
 *return： 获取成功返回0 失败返回<0
 */
__EX_API__ int wow_eth_getLocalIp(char csIp[16])
{
    int ret = 0;
    int sockfd = -1;
    struct ifreq ifr;
    struct sockaddr_in *addr = NULL;

	StringList_T* list = wow_eth_getIfaceName();
	CHECK_RET_VAL(list && wow_stringlist_size(list) > 0,-1);

    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, wow_stringlist_data(list,0),IFNAMSIZ);

    wow_stringlist_free(&list);
	
    addr = (struct sockaddr_in *)&ifr.ifr_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	CHECK_RET_VAL_P(sockfd > 0,-SOCKET_OPEN_FAILED,"socket faild\n");
	
    ret = ioctl(sockfd, SIOCGIFADDR, &ifr);
	CHECK_RET_VAL_EXE_P(ret == 0,-SYSTEM_IOCTRL_FAILED,close(sockfd),"ioctl SIOCGIFADDR faild\n");
	
    strncpy(csIp, inet_ntoa(addr->sin_addr), 16);

    close(sockfd);

    return WOW_SUCCESS;
}


