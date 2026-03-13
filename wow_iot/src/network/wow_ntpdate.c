#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <syslog.h>

#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_common.h"

#include "network/wow_udp.h"
#include "network/wow_ntpdate.h"

#define UTC_NTP 2208988800U /* 1970 - 1900 */
/*
                       1                   2                    3
   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9  0 1
  +---+-----+-----+---------------+---------------+----------------+
  |LI | VN  |Mode |    Stratum    |     Poll      |   Precision    | 0
  +---+-----+-----+---------------+---------------+----------------+
  |                          Root  Delay                           | 1
  +----------------------------------------------------------------+
  |                       Root  Dispersion                         | 2
  +----------------------------------------------------------------+
  |                     Reference Identifier                       | 3
  +----------------------------------------------------------------+
  |                    Reference Timestamp (64)                    | 4
  +----------------------------------------------------------------+
  |                    Originate Timestamp (64)                    | 6
  +----------------------------------------------------------------+
  |                     Receive Timestamp (64)                     | 8
  +----------------------------------------------------------------+
  |                     Transmit Timestamp (64)                    | 10
  +----------------------------------------------------------------+
  |                 Key Identifier (optional) (32)                 | 12
  +----------------------------------------------------------------+
  |                 Message Digest (optional) (128)                | 13+
  +----------------------------------------------------------------+
timestamp:
	since 1900
1970-1900:
	25,567 days can be converted to one of these units:
		* 2,208,988,800 seconds
		* 36,816,480 minutes
		* 613,608 hours
		* 3652 weeks (rounded down)
refs:
	http://www.faqs.org/rfcs/rfc2030.html
	http://www.ntp.org/ntpfaq/NTP-s-algo.htm
*/

#define I_MISC		0
#define I_ORTIME	6
#define	I_TXTIME	10

#define TIMEFIX		2208988800UL


/* // 常用的 NTP 服务器地址列表
   "1.cn.pool.ntp.org"
   "2.cn.pool.ntp.org"
   "3.cn.pool.ntp.org"
   "0.cn.pool.ntp.org"
   "cn.pool.ntp.org"
   "tw.pool.ntp.org"
   "0.tw.pool.ntp.org"
   "1.tw.pool.ntp.org"
   "2.tw.pool.ntp.org"
   "3.tw.pool.ntp.org"
   "pool.ntp.org"	
   "time.windows.com"
   "time.nist.gov"	
   "time-nw.nist.gov"
   "asia.pool.ntp.org"
   "europe.pool.ntp.org"
   "oceania.pool.ntp.org"
   "north-america.pool.ntp.org"
   "south-america.pool.ntp.org"
   "africa.pool.ntp.org"
   "ca.pool.ntp.org"
   "uk.pool.ntp.org"
   "us.pool.ntp.org"
   "au.pool.ntp.org"
*/
int wow_ntpdate_exec(void)
{
	int ret = -1;
	uint32_t packet[12];
	char s[64], q[128];
	struct timeval  tv;
	struct timeval txtime;
	struct timeval rxtime;


	Udp_T* udp = wow_udp_open(NULL,0);
	CHECK_RET_VAL_P(udp ,-UDP_CREATE_CLIENT_FAILED,"wow_udp_create_client failed.\n");

	ret = wow_udp_connect(udp,"1.cn.pool.ntp.org",123);
	CHECK_RET_VAL_P(ret == 0 ,-UDP_CONNECT_FAILED,"wow_udp_create_client failed.\n");

	/*发送时间同步帧*/
	memset(&packet, 0, sizeof(packet));
	packet[I_MISC] = htonl((4 << 27) | (3 << 24));	// VN=v4 | mode=3 (client)
	//packet[I_MISC] = htonl((3 << 27) | (3 << 24));	// VN=v3 | mode=3 (client)
	gettimeofday(&txtime, NULL);
	packet[I_TXTIME] =  htonl(txtime.tv_sec + TIMEFIX);

	ret = wow_udp_write_timeout(udp, (uint8_t*)packet, sizeof(packet), 3*1000);
	CHECK_RET_VAL_P(ret == 48,-UDP_WRITE_FAILED,"send ntp frame false!\n");
	
	ret= wow_udp_read_timeout(udp,  (uint8_t*)packet, sizeof(packet), 3*1000);
	CHECK_RET_VAL_P(ret == 48,-UDP_READ_FAILED,"Invalid packet size!\n");

	/*解析同步时间*/
	gettimeofday(&rxtime, NULL);

	uint32_t u = ntohl(packet[0]);
#if 1
	printf("u = 0x%08x\n", u);
	printf("LI = %u\n", u >> 30);
	printf("VN = %u\n", (u >> 27) & 0x07);
	printf("mode = %u\n", (u >> 24) & 0x07);
	printf("stratum = %u\n", (u >> 16) & 0xFF);
	printf("poll interval = %u\n", (u >> 8) & 0xFF);
	printf("precision = %u\n", u & 0xFF);
#endif
	// mode != 4 (server)
	CHECK_RET_VAL_P((u & 0x07000000) == 0x04000000,-WOW_FAILED,"Invalid response!\n");


	time_t  ntpt = ntohl(packet[I_TXTIME]) - TIMEFIX;
	time_t  t = (rxtime.tv_sec - txtime.tv_sec) >> 1;
	time_t  diff = (ntpt - rxtime.tv_sec) + t;
#if 1
	printf("txtime = %ld\n", txtime.tv_sec);
	printf("rxtime = %ld\n", rxtime.tv_sec);
	printf("ntpt	 = %ld\n", ntpt);
	printf("rtt/2  = %ld\n", t);
	printf("diff	 = %ld\n", diff);
#endif

	if (diff != 0) {
		gettimeofday(&tv, NULL);
		tv.tv_sec  += diff;
		settimeofday(&tv, NULL);
		strftime(s, sizeof(s), "%a, %d %b %Y %H:%M:%S %z", localtime(&tv.tv_sec));
		snprintf(q,127, "Time Updated: %s [%s%lds]", s, diff > 0 ? "+" : "", diff);
	}
	else {
		t = time(0);
		strftime(s, sizeof(s), "%a, %d %b %Y %H:%M:%S %z", localtime(&t));
		snprintf(q,127, "Time: %s, no change was needed.", s);
	}
	//printf("\n\n%s\n", q);
	syslog(LOG_INFO,"%s",q);

	
	return 0;
}



