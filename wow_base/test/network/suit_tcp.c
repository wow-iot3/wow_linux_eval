#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#include "greatest/greatest.h"
#include "wow_test_config.h"

#include "prefix/wow_check.h"

#include "system/wow_thread.h"

#include "network/wow_tcp.h"
#include "network/wow_net_iface.h"

#define MOD_TAG "[tcp]"

int tcp_client_read_callback(Tcp_T* tcp,void* arg)
{
	uint8_t rbuf[48] = {0};

	int rlen = 0;
	
	rlen = wow_tcp_read(tcp,rbuf,48);
	CHECK_RET_VAL_P(rlen >= 0,-1,"wow_tcp_read failed.");
	CHECK_RET_VAL(rlen > 0,0);

	printf(MOD_TAG"22-client-tcp read(%d):%s\n",rlen,rbuf);
	return 0;
}

static Tcp_T* g_tcp[16] = {0};
int tcp_server_read_callback(Tcp_T* tcp,void* arg)
{
	uint8_t rbuf[48];

	int ret = 0;
	int rlen = 0;

	rlen = wow_tcp_read(tcp,rbuf,48);
	CHECK_RET_VAL_P(rlen >= 0,-1,"wow_tcp_read failed.");
	CHECK_RET_VAL(rlen > 0,0);

	printf(MOD_TAG"11-server-tcp read(%d):%s\n",rlen,rbuf);

	ret = wow_tcp_write(tcp,rbuf,rlen);
    CHECK_RET_VAL_P(ret == rlen,0,"wow_tcp_write failed.\n");

	return 0;
}

int tcp_server_aceept_callback(Tcp_T* tcp,void* arg)
{
	int ret = 0;
	static int count = 0;
	
	ret = wow_tcp_read_cb(tcp, tcp_server_read_callback,NULL);
	GREATEST_ASSERT(ret == 0);

	g_tcp[count++] = tcp;
	return 0;
}


TEST test_tcp_server_fun1(void)
{
	int ret = 0;
	int i = 0;
	char ip[16] = {0};
	uint8_t sbuf[16] = {0};
	uint8_t rbuf[48] = {0};

	Tcp_T* server_tcp = NULL;
	Tcp_T* client1_tcp = NULL;
	Tcp_T* client2_tcp = NULL;
	
	printf(MOD_TAG"suit_tcp---------test_tcp_server_fun1\n");

	ret =  wow_eth_getLocalIp(ip);
	GREATEST_ASSERT(ret == 0);
	
	/*创建服务器*/
    server_tcp = wow_tcp_open(ip,SUIT_TCP_LOCAL_PORT_01);
    GREATEST_ASSERT(server_tcp > 0);
	ret = wow_tcp_listen(server_tcp,16);
	GREATEST_ASSERT(ret == 0);
	ret = wow_tcp_accpet_cb(server_tcp,tcp_server_aceept_callback,NULL);
	GREATEST_ASSERT(ret == 0);
	

	/*创建客户端1*/
	printf("-----------------0----------------\n");
	client1_tcp = wow_tcp_open(NULL,0);
	GREATEST_ASSERT(client1_tcp);
	ret = wow_tcp_connect(client1_tcp,ip,SUIT_TCP_LOCAL_PORT_01);
	GREATEST_ASSERT(ret == 0);

	for(i = 0; i < 10; i++){
		memset(sbuf,0x30+i,15);
		ret = wow_tcp_write(client1_tcp,sbuf,16);
		GREATEST_ASSERT(ret == 16);

		ret = wow_tcp_read_timeout(client1_tcp,rbuf,48,1000);
		if(ret > 0){
			printf(MOD_TAG"11-client-tcp read(%d):%s\n",ret,rbuf);
		}
		sleep(1);
	}


	/*创建客户端2*/
	printf("-----------------1----------------\n");
	client2_tcp = wow_tcp_open(NULL,0);
	GREATEST_ASSERT(client2_tcp);
	ret = wow_tcp_connect(client2_tcp,ip,SUIT_TCP_LOCAL_PORT_01);
	GREATEST_ASSERT(ret == 0);
	
	ret = wow_tcp_read_cb(client2_tcp, tcp_client_read_callback,NULL);
	GREATEST_ASSERT(ret == 0);

	for(i = 0; i < 10; i++){
		memset(sbuf,0x30+i,15);
		ret = wow_tcp_write(client2_tcp,sbuf,16);
		GREATEST_ASSERT(ret == 16);
		sleep(1);
	}

	wow_tcp_close(client2_tcp);
	wow_tcp_close(client1_tcp);
	wow_tcp_close(server_tcp);
	for(i = 0; i < 16; i++){
		if(g_tcp[i]) wow_tcp_close(g_tcp[i]);
	}
	
	
	PASS();
}

static int g_tcp_run_flag = 1;
static void* tcp_recv_thread(void *arg) 
{
	
	int ret = 0;
	int size = 0;
	Tcp_T* tcp = (Tcp_T*)arg;
	uint8_t buf[1024];

	while(g_tcp_run_flag)
	{
		size = wow_tcp_read_timeout(tcp,buf,48,100);

		CHECK_RET_CONTINUE(size > 0);
		printf(MOD_TAG"22--server-recv data(%d):%s\n",size,buf);

		ret = wow_tcp_write(tcp,buf,size);
		CHECK_RET_BREAK_P(ret == size,"wow_tcp_write failed.");
	} 
	
	wow_tcp_close(tcp);  

	return NULL;
}

static void* tcp_accept_thread(void *arg)  
{  
	Tcp_T* tcp = (Tcp_T*)arg;
	int count = 0;

	while(1)
	{
		Tcp_T* ntcp = wow_tcp_accpet(tcp);
		CHECK_RET_CONTINUE(ntcp);
		
		wow_thread_create(MOD_TAG,tcp_recv_thread,(void *)ntcp);
		if(count++ == 4){
			g_tcp_run_flag = 0;
			break;
		} 
	} 
	
	g_tcp_run_flag = 0;
	wow_tcp_close(tcp);

	return NULL;
} 

TEST test_tcp_server_fun2(void)
{
	int ret = 0;
	int i   = 0;
	pthread_idx_t tid;
	char ip[16] = {0};
	uint8_t sbuf[16] = {0};
	uint8_t rbuf[48] = {0};

    Tcp_T* server_tcp = NULL;
	Tcp_T* client1_tcp = NULL;
	Tcp_T* client2_tcp = NULL;

	
	printf(MOD_TAG"suit_tcp---------test_tcp_server_fun2\n");	

	ret =  wow_eth_getLocalIp(ip);
	GREATEST_ASSERT(ret == 0);

	/*创建服务器*/
	server_tcp = wow_tcp_open(ip,SUIT_TCP_LOCAL_PORT_02);
	GREATEST_ASSERT(server_tcp);
	
	ret = wow_tcp_listen(server_tcp,16);
    GREATEST_ASSERT(ret == 0);
	tid = wow_thread_create(MOD_TAG,tcp_accept_thread,(void *)server_tcp);
	GREATEST_ASSERT(tid);

	printf("-----------------0----------------\n");
	/*创建客户端1*/
	client1_tcp = wow_tcp_open(NULL,0);
	GREATEST_ASSERT(client1_tcp);
	ret = wow_tcp_connect(client1_tcp,ip,SUIT_TCP_LOCAL_PORT_02);
	GREATEST_ASSERT(ret == 0);

	for(i = 0; i < 10; i++){
		memset(sbuf,0x30+i,15);
		ret = wow_tcp_write(client1_tcp,sbuf,16);
		GREATEST_ASSERT(ret == 16);

		ret = wow_tcp_read_timeout(client1_tcp,rbuf,48,1000);
		if(ret > 0){
			printf(MOD_TAG"11-client-tcp read(%d):%s\n",ret,rbuf);
		}
		sleep(1);
	}

	printf("----------------1-----------------\n");

	/*创建客户端2*/
    client2_tcp = wow_tcp_open(NULL,0);
    GREATEST_ASSERT(client2_tcp);
	ret = wow_tcp_connect(client2_tcp,ip,SUIT_TCP_LOCAL_PORT_02);
	GREATEST_ASSERT(ret == 0);
	
	ret = wow_tcp_read_cb(client2_tcp, tcp_client_read_callback, NULL);
	GREATEST_ASSERT(ret == 0);

	for(i = 0; i < 10; i++){
		memset(sbuf,0x30+i,15);
		ret = wow_tcp_write(client2_tcp,sbuf,16);
		GREATEST_ASSERT(ret == 16);

		sleep(1);
	}
	
	wow_thread_cancel(tid);
	g_tcp_run_flag = 0;

	printf("----------------2-----------------\n");
	wow_tcp_close(client2_tcp);
	wow_tcp_close(client1_tcp);
	wow_tcp_close(server_tcp);

	PASS();
}

//http://tcplab.openluat.com/ 提供公网tcp测试
//http://47.107.99.203:8088/  提供公网tcp测试
TEST test_tcp_remote1(void)
{
	int ret = 0;
	int i   = 0;
	uint8_t sbuf[16] = {0};
	uint8_t rbuf[48] = {0};

	Tcp_T* client1_tcp = NULL;
	Tcp_T* client2_tcp = NULL;

	printf(MOD_TAG"suit_tcp---------test_tcp_remote1\n");	

	/*创建客户端1*/
	client1_tcp = wow_tcp_open(NULL,0);
	GREATEST_ASSERT(client1_tcp);
	ret = wow_tcp_connect(client1_tcp,SUIT_TCP_LOCAL_SERVER_IP_01,SUIT_TCP_LOCAL_SERVER_PORT_01);
	CHECK_RET_GOTO(ret == 0,out);
	
	for(i = 0; i < 10; i++){
		memset(sbuf,0x30+i,16);
		ret = wow_tcp_write(client1_tcp,sbuf,16);
		GREATEST_ASSERT(ret == 16);

		ret = wow_tcp_read_timeout(client1_tcp,rbuf,48,3000);
		if(ret > 0){
			printf(MOD_TAG"11-client-tcp read(%d):%s\n",ret,rbuf);
		}
		sleep(1);
	}

	/*创建客户端2*/
    client2_tcp = wow_tcp_open(NULL,0);
    CHECK_RET_GOTO(ret == 0,out);
	ret = wow_tcp_connect(client1_tcp,SUIT_TCP_LOCAL_SERVER_IP_02,SUIT_TCP_LOCAL_SERVER_PORT_02);
	CHECK_RET_GOTO(ret == 0,out);

out: ///<断言加入此处 connect外网失败时 未释放相关资源	
	wow_tcp_close(client2_tcp);
	wow_tcp_close(client1_tcp);
	GREATEST_ASSERT(ret == 0);

	PASS();
}



SUITE(suit_tcp)
{
	RUN_TEST(test_tcp_server_fun1);
	RUN_TEST(test_tcp_server_fun2);
	RUN_TEST(test_tcp_remote1);
}

