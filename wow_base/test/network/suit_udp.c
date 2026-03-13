#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#include "greatest/greatest.h"
#include "wow_test_config.h"

#include "prefix/wow_check.h"
#include "network/wow_udp.h"
#include "system/wow_countdown.h"
#include "network/wow_net_iface.h"

#define MOD_TAG "[udp]"
#define NTP_TIMESTAMP_DELTA 2208988800ull

TEST test_udp_client_fun1(void)
{
    Udp_T* udp;
	int ret = 0;
	uint8_t sbuf[48];
	uint8_t rbuf[48];
	
	printf(MOD_TAG"suit_udp---------test_udp_client_fun1\n");
	
    udp = wow_udp_open(NULL,0);
    GREATEST_ASSERT(udp);
	
	ret = wow_udp_connect(udp,SUIT_SOCKET_UDP_SERVER_NAME,SUIT_SOCKET_UDP_SERVER_PORT);
	GREATEST_ASSERT(ret == 0);
		
	memset(sbuf,0,48);
	sbuf[0] = 0xA3; //0xa3版本4 0x1b版本3 0x13版本2 0x0b版本1	

	CountDown_T* cdown = wow_cdown_new();
	GREATEST_ASSERT(cdown);
	wow_cdown_reset(cdown);
	wow_cdown_set(cdown,SUIT_THREAD_RUN_TIMES_MS);
	
	while(!wow_cdown_is_expired(cdown))
	{
		ret = wow_udp_write(udp,sbuf,48);
		GREATEST_ASSERT(ret == 48);

		ret = wow_udp_read_timeout(udp,rbuf,48,1000);
		if(ret > 0) break;
	}

	wow_cdown_free(&cdown);
	
	GREATEST_ASSERT(ret > 0);

	wow_udp_close(udp);
	
	uint32_t ntpTime = 0;
	ntpTime = (rbuf[40])<<24 | rbuf[41]<<16 | rbuf[42]<<8 | rbuf[43];
	ntpTime -= NTP_TIMESTAMP_DELTA;		
	printf(MOD_TAG"udp ntpTime:%d\n",ntpTime);

	PASS();
}


TEST test_udp_server_fun(void)
{
	Udp_T* udp;
	char ip[64];
	uint16_t port;
	int size = 0;
	uint8_t buf[156];
	int count = 0;
		
	printf(MOD_TAG"suit_udp---------test_udp_server_fun2\n");

	int ret =  wow_eth_getLocalIp(ip);
	GREATEST_ASSERT(ret == 0);

    udp = wow_udp_open(ip,SUIT_UDP_LOCAL_PORT_02);
    GREATEST_ASSERT(udp);

	CountDown_T* cdown = wow_cdown_new();
	GREATEST_ASSERT(cdown);
	wow_cdown_reset(cdown);
	wow_cdown_set(cdown,SUIT_THREAD_RUN_TIMES_MS);

	while(!wow_cdown_is_expired(cdown))
	{
		memset(buf,0,156);
		size = wow_udp_read_timeout(udp,buf,156,1000);
		CHECK_RET_CONTINUE(size > 0);
		wow_udp_peer(udp,ip,&port);
		printf("recv %s:%d data(%d):%s\n",ip,port,size,buf);
	
		wow_udp_write(udp,buf,size);
		if(count++ > 5) break;
	} 
	
	wow_cdown_free(&cdown);
	wow_udp_close(udp);

	PASS();
}


SUITE(suit_udp)
{
    RUN_TEST(test_udp_client_fun1);
	RUN_TEST(test_udp_server_fun);
}

