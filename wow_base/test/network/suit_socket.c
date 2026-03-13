#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#include "greatest/greatest.h"
#include "wow_test_config.h"

#include "prefix/wow_check.h"

#include "network/wow_socket.h"
#include "system/wow_countdown.h"
#include "network/wow_net_iface.h"

#define MOD_TAG "[socket]"

void *sock_tcp_noblock_thread(void *arg) 
{
	uint8_t buf[1024];
	int size = 0;
	char ip[64];
	uint16_t port;
	int datafd = *(int*)arg;

	CountDown_T* cdown = wow_cdown_new();
	CHECK_RET_VAL_P(cdown,NULL,"wow_cdown_new failed!\n");
	wow_cdown_reset(cdown);
	wow_cdown_set(cdown,SUIT_THREAD_RUN_TIMES_MS);
	
	while(!wow_cdown_is_expired(cdown))
	{
		usleep(100000);
		size = wow_socket_trecv(datafd,buf,1024);
		CHECK_RET_CONTINUE(size > 0);
		printf("tcp_noblock_thread--recv data(%d):%s\n",size,buf);

		/*获取ip信息*/
		wow_socket_peerName(datafd,ip,&port);
		printf("tcp_noblock_thread--peer info %s:%d\n",ip,port);
		wow_socket_localName(datafd,ip,&port);
		printf("tcp_noblock_thread--local info %s:%d\n",ip,port);

		wow_socket_tsend(datafd,buf,size);
	} 

	wow_socket_close(datafd);
	wow_cdown_free(&cdown);
	
	return ((void *)0); 
}

void *sock_tcp_block_thread(void *arg) 
{
	uint8_t buf[1024];
	int size = 0;
	char ip[64];
	uint16_t port;
	int datafd = *(int*)arg;
	
	CountDown_T* cdown = wow_cdown_new();
	CHECK_RET_VAL_P(cdown,NULL,"wow_cdown_new failed!\n");
	wow_cdown_reset(cdown);
	wow_cdown_set(cdown,SUIT_THREAD_RUN_TIMES_MS);

	
	while(!wow_cdown_is_expired(cdown))
	{
		size = wow_socket_trecv(datafd,buf,1024);
		CHECK_RET_CONTINUE(size > 0);
		printf("tcp_block_thread--recv data(%d):%s\n",size,buf);

		/*获取ip信息*/
		wow_socket_peerName(datafd,ip,&port);
		printf("tcp_block_thread--peer info %s:%d\n",ip,port);
		wow_socket_localName(datafd,ip,&port);
		printf("tcp_block_thread--local info %s:%d\n",ip,port);

		wow_socket_tsend(datafd,buf,size);
	} 

	wow_socket_close(datafd);
	wow_cdown_free(&cdown);
	
	return ((void *)0);  
}


static void *tcp_accept_thread(void *arg)  
{  
	int serverfd = *(int*)arg;
	int count   = 0;
	int datafd  = 0;
	int ret = 0;

	char ip[64];
	uint16_t port;
	pthread_t ntid;
	
	CountDown_T* cdown = wow_cdown_new();
	CHECK_RET_VAL_P(cdown,NULL,"wow_cdown_new failed!\n");
	wow_cdown_reset(cdown);
	wow_cdown_set(cdown,SUIT_THREAD_RUN_TIMES_MS);

	while(!wow_cdown_is_expired(cdown))
	{
		datafd = wow_socket_accept(serverfd,ip,&port);
		CHECK_RET_CONTINUE(datafd > 0);
		printf("tcp socket accept info %s:%d\n",ip,port);

		count++;
		if(count == 1){
			int block = -1;
			
			ret = wow_socket_setopt(datafd,SOCKET_CTRL_GET_BLOCK,&block);
			CHECK_RET_BREAK_P(ret == 0,"socket ctrl faild!\n");
			printf("tcp socket get block:%d\n",block);
			
			ret = wow_socket_setopt(datafd,SOCKET_CTRL_SET_BLOCK,0);
			CHECK_RET_BREAK_P(ret == 0,"socket ctrl faild!\n");
			pthread_create(&ntid,NULL,sock_tcp_noblock_thread,(void *)&datafd);
		}else if(count == 2){
			int size = -1;

			ret = wow_socket_setopt(datafd,SOCKET_CTRL_GET_RECV_BUFF_SIZE,&size);
			CHECK_RET_BREAK_P(ret == 0,"socket ctrl faild!\n");
			printf("tcp socket get default recv size:%d\n",size);
			
			ret = wow_socket_setopt(datafd,SOCKET_CTRL_SET_RECV_BUFF_SIZE,32*1024);
			CHECK_RET_BREAK_P(ret == 0,"socket ctrl faild!\n");
			ret = wow_socket_setopt(datafd,SOCKET_CTRL_GET_RECV_BUFF_SIZE,&size);
			CHECK_RET_BREAK_P(ret == 0,"socket ctrl faild!\n");
			printf("tcp socket set now recv size:%d\n",size);

			
			ret = wow_socket_setopt(datafd,SOCKET_CTRL_GET_SEND_BUFF_SIZE,&size);
			CHECK_RET_BREAK_P(ret == 0,"socket ctrl faild!\n");
			printf("tcp socket get default send size:%d\n",size);
			
			ret = wow_socket_setopt(datafd,SOCKET_CTRL_SET_SEND_BUFF_SIZE,32*1024);
			CHECK_RET_BREAK_P(ret == 0,"socket ctrl faild!\n");
			ret = wow_socket_setopt(datafd,SOCKET_CTRL_GET_SEND_BUFF_SIZE,&size);
			CHECK_RET_BREAK_P(ret == 0,"socket ctrl faild!\n");
			printf("tcp socket get now send size:%d\n",size);	
			
			pthread_create(&ntid,NULL,sock_tcp_block_thread,(void *)&datafd);
		}else{
			pthread_create(&ntid,NULL,sock_tcp_block_thread,(void *)&datafd);
		}
	} 

	wow_socket_close(datafd);
	wow_cdown_free(&cdown);
	
    return ((void *)0);  
} 

TEST test_socket_tcp_server(void)
{
    int sktfd = 0;
	int ret = 0;
	pthread_t ntid;
	char ip[16] = {0};

	printf(MOD_TAG"suit_socket---------test_socket_tcp_server\n");	
	
	ret =  wow_eth_getLocalIp(ip);
	GREATEST_ASSERT(ret == 0);

    sktfd = wow_socket_open(SOCKET_TYPE_TCP);
    GREATEST_ASSERT(sktfd > 0);
	
	ret = wow_socket_setopt(sktfd,SOCKET_CTRL_SET_BLOCK,0);
	GREATEST_ASSERT(ret == 0);

	ret = wow_socket_bind(sktfd,ip,SUIT_SOCKET_TCP_LOCAL_PORT);
	GREATEST_ASSERT(ret == 0);
	
	ret = wow_socket_listen(sktfd,3);
	GREATEST_ASSERT(ret == 0);

	/*创建线程 实现服务器接收多客户端*/
	ret = pthread_create(&ntid,NULL,tcp_accept_thread,(void *)&sktfd);
	GREATEST_ASSERT(ret == 0);

	PASS();
}

static void *udp_recv_thread(void *arg)  
{  
	int serverfd = *(int*)arg;
	uint8_t buf[1024];
	int size = 0;
	char ip[64];
	uint16_t port;
	
	CountDown_T* cdown = wow_cdown_new();
	CHECK_RET_VAL_P(cdown,NULL,"wow_cdown_new failed!\n");
	wow_cdown_reset(cdown);
	wow_cdown_set(cdown,SUIT_THREAD_RUN_TIMES_MS);

	while(!wow_cdown_is_expired(cdown))
	{
		usleep(100000);
		size = wow_socket_urecv(serverfd,ip,&port,buf,1024);
		CHECK_RET_CONTINUE(size > 0);
		printf("recv data(%d):%s\n",size,buf);
		
		wow_socket_usend(serverfd,ip,port,buf,size);
	} 
	
	wow_socket_close(serverfd);
	wow_cdown_free(&cdown);
	
    return ((void *)0);  
} 




TEST test_socket_udp_server(void)
{
    int sktfd = 0;
	int ret = 0;
	pthread_t ntid;
	char ip[16] = {0};

	printf(MOD_TAG"suit_socket---------test_socket_udp_server\n");

	ret =  wow_eth_getLocalIp(ip);
	GREATEST_ASSERT(ret == 0);

    sktfd = wow_socket_open(SOCKET_TYPE_UDP);
    GREATEST_ASSERT(sktfd > 0);

	ret = wow_socket_setopt(sktfd,SOCKET_CTRL_SET_BLOCK,0);
	GREATEST_ASSERT(ret == 0);

	ret = wow_socket_bind(sktfd,ip,SUIT_SOCKET_UDP_LOCAL_PORT);
	GREATEST_ASSERT(ret == 0);

	/*创建线程 实现服务器接收多客户端*/
	ret = pthread_create(&ntid,NULL,udp_recv_thread,(void *)&sktfd);
	GREATEST_ASSERT(ret == 0);

	PASS();
}


#define SOCKET_CONNECT_TIMEOUT (3000)
#define NTP_TIMESTAMP_DELTA 2208988800ull
TEST test_socket_udp_client(void)
{
    int sktfd = 0;
	int ret = 0;
	char ip[64];
	uint16_t port;
	uint8_t sbuf[48];
	uint8_t rbuf[48];
	
	printf(MOD_TAG"suit_socket---------test_socket_udp_client\n");
	
    sktfd = wow_socket_open(SOCKET_TYPE_UDP);
    GREATEST_ASSERT(sktfd > 0);

	ret = wow_socket_connect(sktfd,SUIT_SOCKET_UDP_SERVER_NAME,SUIT_SOCKET_UDP_SERVER_PORT,SOCKET_CONNECT_TIMEOUT);
	GREATEST_ASSERT(ret == 0);
	
	/*获取ip信息*/
	wow_socket_peerName(sktfd,ip,&port);
		
	memset(sbuf,0,48);
	sbuf[0] = 0xA3; //0xa3版本4 0x1b版本3 0x13版本2 0x0b版本1
	
	CountDown_T* cdown = wow_cdown_new();
	GREATEST_ASSERT(cdown);
	wow_cdown_reset(cdown);
	wow_cdown_set(cdown,SUIT_THREAD_RUN_TIMES_MS);
	
	while(!wow_cdown_is_expired(cdown))
	{
		ret = wow_socket_usend(sktfd,ip,port,sbuf,48);
		GREATEST_ASSERT(ret == 48);
	
		sleep(1);
		ret = wow_socket_urecv(sktfd,ip,&port,rbuf,48);
		if(ret > 0) break;
	}

	wow_cdown_free(&cdown);
	GREATEST_ASSERT(ret > 0);
	
	wow_socket_close(sktfd);
	
	uint32_t ntpTime = 0;
	ntpTime = (rbuf[40])<<24 | rbuf[41]<<16 | rbuf[42]<<8 | rbuf[43];
	ntpTime -= NTP_TIMESTAMP_DELTA;		
	printf(MOD_TAG"--socket ntpTime:%d\n",ntpTime);

	PASS();
}


TEST test_socket_tcp_client(void)
{
	int i = 0;
    int sktfd = 0;
	int ret = 0;
	uint8_t rbuf[48];
	
	printf(MOD_TAG"suit_socket---------test_socket_tcp_client\n");
	
    sktfd = wow_socket_open(SOCKET_TYPE_TCP);
    GREATEST_ASSERT(sktfd > 0);

	ret = wow_socket_connect(sktfd,SUIT_SOCKET_TCP_SERVER_IP,SUIT_SOCKET_TCP_SERVER_PORT,SOCKET_CONNECT_TIMEOUT);
	GREATEST_ASSERT(ret == 0);
	
	ret = wow_socket_tsend(sktfd,(uint8_t*)"AAAAAAAAAAAAAAAAAAAAAA",strlen("AAAAAAAAAAAAAAAAAAAAAA"));
	GREATEST_ASSERT(ret == strlen("AAAAAAAAAAAAAAAAAAAAAA"));

	for(i = 0; i < 10; i++){
		ret = wow_socket_trecv(sktfd,rbuf,48);
		if(ret == 48) break;
		sleep(1);

	}

	GREATEST_ASSERT(ret > 0);
		
	wow_socket_close(sktfd);

	PASS();
}



SUITE(suit_socket)
{
    RUN_TEST(test_socket_tcp_server);
	RUN_TEST(test_socket_udp_server);
	RUN_TEST(test_socket_tcp_client);
	RUN_TEST(test_socket_udp_client);
}




