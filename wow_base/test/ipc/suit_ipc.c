#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "greatest/greatest.h"
#include "wow_test_config.h"

#include "prefix/wow_check.h"
#include "system/wow_thread.h"

#include "ipc/wow_ipc.h"

#define MOD_TAG "[ipc]"


enum ipc_cmd {
	IPC_TEST_MOD1 = 0,
	IPC_TEST_MOD2,
	IPC_TEST_MOD3,
	IPC_TEST_MOD4,
	IPC_TEST_BROADCAST,
};


#define IPC_SERVER_ID   0x00
#define IPC_CLIENT_ID1  0x01
#define IPC_CLIENT_ID2  0x02
#define IPC_CLIENT_ID3  0x03
#define IPC_CLIENT_ID4  0x04


#define IPC_TEST1 BUILD_IPC_MSG_ID(IPC_SERVER_ID,IPC_CLIENT_ID1,IPC_TEST_MOD1)
#define IPC_TEST2 BUILD_IPC_MSG_ID(IPC_SERVER_ID,IPC_CLIENT_ID2,IPC_TEST_MOD2)
#define IPC_TEST3 BUILD_IPC_MSG_ID(IPC_SERVER_ID,IPC_CLIENT_ID3,IPC_TEST_MOD3)
#define IPC_TEST4 BUILD_IPC_MSG_ID(IPC_SERVER_ID,IPC_CLIENT_ID4,IPC_TEST_MOD4)
#define IPC_TEST5 BUILD_IPC_MSG_ID(IPC_SERVER_ID,IPC_CLIENT_ID2,IPC_TEST_BROADCAST)


#define IPC_TEST1_ACK BUILD_IPC_MSG_ID(IPC_CLIENT_ID1,IPC_SERVER_ID,IPC_TEST_MOD1)
#define IPC_TEST2_ACK BUILD_IPC_MSG_ID(IPC_CLIENT_ID2,IPC_SERVER_ID,IPC_TEST_MOD2)
#define IPC_TEST3_ACK BUILD_IPC_MSG_ID(IPC_CLIENT_ID3,IPC_SERVER_ID,IPC_TEST_MOD3)
#define IPC_TEST4_ACK BUILD_IPC_MSG_ID(IPC_CLIENT_ID4,IPC_SERVER_ID,IPC_TEST_MOD4)

struct calc_args {
    int32_t left;
    int32_t right;
    int32_t opcode;
};

int server_ipc_callback1(Ipc_T* ipc,void *in_arg, size_t in_len, void* priv)
{

	struct calc_args *calc = (struct calc_args *)in_arg;
	printf(MOD_TAG"TEST1:left:%d opcode:%d right:%d\n",(int)calc->left, (int)calc->opcode, (int)calc->right);
	
	int out_arg = 12;
	
	int ret = wow_ipc_send(ipc, IPC_TEST1_ACK,&out_arg,sizeof(int));
	GREATEST_ASSERT(ret == 0);

	return 0;
}

int server_ipc_callback2(Ipc_T* ipc,void *in_arg, size_t in_len, void* priv)
{
	printf(MOD_TAG"TEST2: buf:%s in_len:%d\n",(char*)in_arg,(int)in_len);

	int ret = wow_ipc_send(ipc, IPC_TEST2_ACK,"AAAAAAAA",10);
	GREATEST_ASSERT(ret == 0);
	
	return 0;
}

//借助服务器实现转发
int server_ipc_callback3(Ipc_T* ipc,void *in_arg, size_t in_len, void* priv)
{
	printf(MOD_TAG"TEST3: buf:%s in_len:%d\n",(char*)in_arg,(int)in_len);

	int ret = wow_ipc_send(ipc, IPC_TEST4_ACK,in_arg,in_len);
	GREATEST_ASSERT(ret == 0);
	
	return 0;
}

int server_ipc_callback4(Ipc_T* ipc,void *in_arg, size_t in_len, void* priv)
{
	printf(MOD_TAG"TEST4: buf:%s in_len:%d\n",(char*)in_arg,(int)in_len);

	int ret = wow_ipc_send(ipc, IPC_TEST4_ACK,in_arg,in_len);
	GREATEST_ASSERT(ret == 0);
	
	return 0;
}

int client2_ipc_callback(Ipc_T* ipc,void *in_arg, size_t in_len, void* priv)
{
	printf(MOD_TAG"IPC_TEST2_ACK buf:%s in_len:%d\n",(char*)in_arg,(int)in_len);
	return 0;
}

int client3_ipc_callback(Ipc_T* ipc,void *in_arg, size_t in_len, void* priv)
{
	printf(MOD_TAG"IPC_TEST3_ACK buf:%s in_len:%d\n",(char*)in_arg,(int)in_len);
	return 0;
}

int client4_ipc_callback(Ipc_T* ipc,void *in_arg, size_t in_len, void* priv)
{
	printf(MOD_TAG"IPC_TEST4_ACK buf:%s in_len:%d\n",(char*)in_arg,(int)in_len);
	return 0;
}


int server_ipc_callback_all(Ipc_T* ipc,uint32_t func_id,void *in_arg, size_t in_len, void* priv)
{
	int ret = 0;

	//printf("server_ipc_callback_all func_id:%08x\n",func_id);

	uint16_t  mod = GET_IPC_MSG_CMD(func_id); 
	switch(mod){
		case IPC_TEST_BROADCAST:
			printf(MOD_TAG"TEST5: buf:%s in_len:%d\n",(char*)in_arg,(int)in_len);
			wow_ipc_broadcast(ipc,IPC_TEST_BROADCAST,in_arg, in_len);
			return 0;
		default:
			break;
	}
	
	switch(func_id){
		case IPC_TEST1_ACK:
			{
				struct calc_args *calc = (struct calc_args *)in_arg;
				printf(MOD_TAG"TEST1:left:%d opcode:%d right:%d\n",(int)calc->left, (int)calc->opcode, (int)calc->right);
				int out_arg = 12;
				ret = wow_ipc_send(ipc, IPC_TEST1_ACK,&out_arg,sizeof(int));
				GREATEST_ASSERT(ret == 0);

			}
			break;
		case IPC_TEST2_ACK:
			{
				printf(MOD_TAG"TEST2: buf:%s in_len:%d\n",(char*)in_arg,(int)in_len);
				ret = wow_ipc_send(ipc, IPC_TEST2_ACK,"AAAAAAAA",10);
				GREATEST_ASSERT(ret == 0);
			}
			break;
		case IPC_TEST3_ACK:
			{
				printf(MOD_TAG"TEST3: buf:%s in_len:%d\n",(char*)in_arg,(int)in_len);
				ret = wow_ipc_send(ipc, IPC_TEST4_ACK,in_arg,in_len);
				GREATEST_ASSERT(ret == 0);
			}
			break;
		case IPC_TEST4_ACK:
			{
				printf(MOD_TAG"TEST4: buf:%s in_len:%d\n",(char*)in_arg,(int)in_len);
				ret = wow_ipc_send(ipc, IPC_TEST4_ACK,in_arg,in_len);
				GREATEST_ASSERT(ret == 0);
			}
			break;
	}


	return 0;
}

int client2_ipc_callback_all(Ipc_T* ipc,uint32_t func_id,void *in_arg, size_t in_len, void* priv)
{
	uint16_t  mod = GET_IPC_MSG_CMD(func_id); 
	switch(mod){
		case IPC_TEST_MOD2:
			printf(MOD_TAG"IPC_TEST2_ACK buf:%s in_len:%d\n",(char*)in_arg,(int)in_len);
			break;
		case IPC_TEST_BROADCAST:
			printf(MOD_TAG"client2 broadcast buf:%s in_len:%d\n",(char*)in_arg,(int)in_len);
			break;
		}

	return 0;
}

int client3_ipc_callback_all(Ipc_T* ipc,uint32_t func_id,void *in_arg, size_t in_len, void* priv)
{
	uint16_t  mod = GET_IPC_MSG_CMD(func_id); 
	switch(mod){
		case IPC_TEST_MOD3:
			printf(MOD_TAG"IPC_TEST3_ACK buf:%s in_len:%d\n",(char*)in_arg,(int)in_len);
			break;
		case IPC_TEST_BROADCAST:
			printf(MOD_TAG"client3 broadcast buf:%s in_len:%d\n",(char*)in_arg,(int)in_len);
			break;
		}

	return 0;
}

int client4_ipc_callback_all(Ipc_T* ipc,uint32_t func_id,void *in_arg, size_t in_len, void* priv)
{

	uint16_t  mod = GET_IPC_MSG_CMD(func_id); 
	switch(mod){
		case IPC_TEST_MOD4:
			printf(MOD_TAG"IPC_TEST4_ACK buf:%s in_len:%d\n",(char*)in_arg,(int)in_len);
			break;
		case IPC_TEST_BROADCAST:
			printf(MOD_TAG"client4 broadcast buf:%s in_len:%d\n",(char*)in_arg,(int)in_len);
			break;
		}

	return 0;
}

static void ipc_client_test0(Ipc_T* ipc)
{
	int i = 0;
	int ret = -1;
    struct calc_args calc;
    calc.left = 123;
    calc.right = 321;
    calc.opcode = '+';
    int out = 0;
	
	printf(MOD_TAG"---------------ipc_client0_test\n");	
	for(i = 0; i < 3; i++)
	{
		ret = wow_ipc_send(ipc, IPC_TEST1,&calc, sizeof(calc));
		CHECK_RET_CONTINUE_P(ret == sizeof(calc),"wow_ipc_send %08x failed!\n", IPC_TEST1);
		do{
			usleep(10*1000);
			ret = wow_ipc_recv(ipc,IPC_TEST1,&out,sizeof(int));
		}while(ret < 0);
		printf(MOD_TAG"IPC_TEST1 out(%d):%d\n",ret,out);
	}
}

static void ipc_client_test1(Ipc_T* ipc)
{
	int i = 0;
	int ret = -1;
    struct calc_args calc;
    calc.left = 123;
    calc.right = 321;
    calc.opcode = '+';
    int out;
	
	printf(MOD_TAG"---------------ipc_client1_test\n");	
	for(i = 0; i < 3; i++)
	{
		ret = wow_ipc_send_wait(ipc, IPC_TEST1,2000,&calc, sizeof(calc), &out, sizeof(out));
		CHECK_RET_CONTINUE_P(ret > 0,"wow_ipc_send %08x failed!\n", IPC_TEST1);
		printf(MOD_TAG"IPC_TEST1 out(%d):%d\n",ret,out);
		sleep(1);
	}
}

static void ipc_client_test2(Ipc_T* ipc)
{
	int i = 0;
	int ret = -1;
	printf(MOD_TAG"---------------ipc_client2_test\n");
	for(i = 0; i < 3; i++)
	{
		ret = wow_ipc_send(ipc,IPC_TEST2,"2222222222", 10);
		CHECK_RET_CONTINUE_P(ret == 10,"wow_ipc_send %08x failed!\n", IPC_TEST2);
		sleep(2);

	}
}

static void ipc_client_test3(Ipc_T* ipc)
{
	int i = 0;
	int ret = -1;
	
	printf(MOD_TAG"---------------ipc_client3_test\n");
	for(i = 0; i < 3; i++)
	{
		ret = wow_ipc_send(ipc,IPC_TEST3,"3333333333", 10);
		CHECK_RET_CONTINUE_P(ret == 10,"wow_ipc_send %08x failed!\n", IPC_TEST3);
		sleep(2);

	}
}

static void ipc_client_test4(Ipc_T* ipc)
{
	int i = 0;
	int ret = -1;
	printf(MOD_TAG"---------------ipc_client4_test\n");

	for(i = 0; i < 3; i++)
	{
		ret = wow_ipc_send(ipc,IPC_TEST4,"4444444444", 10);
		CHECK_RET_CONTINUE_P(ret == 10,"wow_ipc_send %08x failed!\n", IPC_TEST4);
		sleep(2);

	}
}

#if SUIT_RUN_ERROR_TEST
TEST test_ipc_error(void)
{
	printf(MOD_TAG"suit_ipc---------------test_ipc_error\n");

	int ret = 0;
    Ipc_T* test_ipc_00 = NULL;
	Ipc_T* test_ipc_01 = NULL;

    test_ipc_00 = wow_ipc_create_server(0);
    GREATEST_ASSERT(test_ipc_00);
    test_ipc_01 = wow_ipc_create_client(0,1);
    GREATEST_ASSERT(test_ipc_01);

	//应用时不可用此方式 将上述客户端1作为2的服务器
    //test_ipc_02 = wow_ipc_create_client(1,2);
    //GREATEST_ASSERT(test_ipc_02);

	ret = wow_ipc_set_notify(test_ipc_01,0,NULL,NULL);
	GREATEST_ASSERT(ret != 0);
	ret = wow_ipc_send(test_ipc_01,0,"AAAAAA",0);
	GREATEST_ASSERT(ret != 0);
	ret = wow_ipc_send(test_ipc_01,0,NULL,1);
	GREATEST_ASSERT(ret != 0);
	uint8_t test;
	ret = wow_ipc_send_wait(test_ipc_01,0,0,NULL,1,&test,1);
	GREATEST_ASSERT(ret != 0);
	ret = wow_ipc_send_wait(test_ipc_01,0,0,"A",1,NULL,1);
	GREATEST_ASSERT(ret != 0);


	wow_ipc_destroy(&test_ipc_00);
    wow_ipc_destroy(&test_ipc_01);

	PASS();
}
#endif

TEST test_ipc_fun_01(void)
{
	int ret = -1;
	Ipc_T* ipc_server;
	Ipc_T* ipc_client1;

	printf(MOD_TAG"suit_ipc---------------test_ipc_fun_01\n");

	ipc_server = wow_ipc_create_server(IPC_SERVER_ID);
	GREATEST_ASSERT(ipc_server);

	ipc_client1 = wow_ipc_create_client(IPC_SERVER_ID,IPC_CLIENT_ID1);
	GREATEST_ASSERT(ipc_client1);

	ret = wow_ipc_set_notify_all(ipc_server,server_ipc_callback_all,NULL);
	GREATEST_ASSERT(ret == 0);
	
	ipc_client_test0(ipc_client1);
	ipc_client_test1(ipc_client1);

	sleep(1);
	wow_ipc_destroy(&ipc_client1);
	wow_ipc_destroy(&ipc_server);

	PASS();
}

TEST test_ipc_fun_02(void)
{
	int ret = -1;
	Ipc_T* ipc_server;
	Ipc_T* ipc_client1;
	Ipc_T* ipc_client2;
	Ipc_T* ipc_client3;
	Ipc_T* ipc_client4;

	printf(MOD_TAG"suit_ipc---------------test_ipc_fun_02\n");

    ipc_server = wow_ipc_create_server(IPC_SERVER_ID);
	GREATEST_ASSERT(ipc_server);
	ipc_client1 = wow_ipc_create_client(IPC_SERVER_ID,IPC_CLIENT_ID1);
	GREATEST_ASSERT(ipc_client1);
	ipc_client2 = wow_ipc_create_client(IPC_SERVER_ID,IPC_CLIENT_ID2);
	GREATEST_ASSERT(ipc_client2);
	ipc_client3 = wow_ipc_create_client(IPC_SERVER_ID,IPC_CLIENT_ID3);
	GREATEST_ASSERT(ipc_client3);
	ipc_client4 = wow_ipc_create_client(IPC_SERVER_ID,IPC_CLIENT_ID4);
	GREATEST_ASSERT(ipc_client4);

	//用于反馈IPC_TEST1的数据响应
	ret = wow_ipc_set_notify(ipc_server,IPC_TEST1,server_ipc_callback1,NULL);
	GREATEST_ASSERT(ret == 0);
	//用于反馈IPC_TEST2的数据响应
	ret = wow_ipc_set_notify(ipc_server,IPC_TEST2,server_ipc_callback2,NULL);
	GREATEST_ASSERT(ret == 0);
	//用于反馈IPC_TEST3的数据响应--转发到client4
	ret = wow_ipc_set_notify(ipc_server,IPC_TEST3,server_ipc_callback3,NULL);
	GREATEST_ASSERT(ret == 0);
	ret = wow_ipc_set_notify(ipc_server,IPC_TEST4,server_ipc_callback4,NULL);
	GREATEST_ASSERT(ret == 0);

	//用于接收IPC_TEST2_ACK的信息
	ret = wow_ipc_set_notify(ipc_client2,IPC_TEST2_ACK,client2_ipc_callback,NULL);
	GREATEST_ASSERT(ret == 0);
	//用于接收IPC_TEST3_ACK的信息
	ret = wow_ipc_set_notify(ipc_client3,IPC_TEST3_ACK,client3_ipc_callback,NULL);
	GREATEST_ASSERT(ret == 0);
	//用于接收IPC_TEST4_ACK的信息
	ret = wow_ipc_set_notify(ipc_client4,IPC_TEST4_ACK,client4_ipc_callback,NULL);
	GREATEST_ASSERT(ret == 0);

	ipc_client_test1(ipc_client1);
	ipc_client_test2(ipc_client2);
	ipc_client_test3(ipc_client3);
	ipc_client_test4(ipc_client4);

	wow_ipc_destroy(&ipc_client1);
	wow_ipc_destroy(&ipc_client2);
	wow_ipc_destroy(&ipc_client3);
	wow_ipc_destroy(&ipc_client4);
    wow_ipc_destroy(&ipc_server);

    PASS();
}

TEST test_ipc_fun_03(void)
{
	int ret = -1;
	Ipc_T* ipc_server;
	Ipc_T* ipc_client1;
	Ipc_T* ipc_client2;
	Ipc_T* ipc_client3;
	Ipc_T* ipc_client4;

	printf(MOD_TAG"suit_ipc---------------test_ipc_fun_03\n");

    ipc_server = wow_ipc_create_server(IPC_SERVER_ID);
	GREATEST_ASSERT(ipc_server);

	ipc_client1 = wow_ipc_create_client(IPC_SERVER_ID,IPC_CLIENT_ID1);
	GREATEST_ASSERT(ipc_client1);
	ipc_client2 = wow_ipc_create_client(IPC_SERVER_ID,IPC_CLIENT_ID2);
	GREATEST_ASSERT(ipc_client2);
	ipc_client3 = wow_ipc_create_client(IPC_SERVER_ID,IPC_CLIENT_ID3);
	GREATEST_ASSERT(ipc_client3);
	ipc_client4 = wow_ipc_create_client(IPC_SERVER_ID,IPC_CLIENT_ID4);
	GREATEST_ASSERT(ipc_client4);

	ret = wow_ipc_set_notify_all(ipc_server,server_ipc_callback_all,NULL);
	GREATEST_ASSERT(ret == 0);
	ret = wow_ipc_set_notify_all(ipc_client2,client2_ipc_callback_all,NULL);
	GREATEST_ASSERT(ret == 0);
	ret = wow_ipc_set_notify_all(ipc_client3,client3_ipc_callback_all,NULL);
	GREATEST_ASSERT(ret == 0);
	ret = wow_ipc_set_notify_all(ipc_client4,client4_ipc_callback_all,NULL);
	GREATEST_ASSERT(ret == 0);

	ipc_client_test1(ipc_client1);
	ipc_client_test2(ipc_client2);
	ipc_client_test3(ipc_client3);
	ipc_client_test4(ipc_client4);

	wow_ipc_destroy(&ipc_client1);
	wow_ipc_destroy(&ipc_client2);
	wow_ipc_destroy(&ipc_client3);
	wow_ipc_destroy(&ipc_client4);
    wow_ipc_destroy(&ipc_server);

    PASS();
}

TEST test_ipc_fun_04(void)
{
	int ret = -1;
	Ipc_T* ipc_server;
	Ipc_T* ipc_client1;
	Ipc_T* ipc_client2;
	Ipc_T* ipc_client3;
	Ipc_T* ipc_client4;

	printf(MOD_TAG"suit_ipc---------------test_ipc_fun_04\n");

	ipc_server = wow_ipc_create_server(IPC_SERVER_ID);
	GREATEST_ASSERT(ipc_server);

	ipc_client1 = wow_ipc_create_client(IPC_SERVER_ID,IPC_CLIENT_ID1);
	GREATEST_ASSERT(ipc_client1);
	ipc_client2 = wow_ipc_create_client(IPC_SERVER_ID,IPC_CLIENT_ID2);
	GREATEST_ASSERT(ipc_client2);
	ipc_client3 = wow_ipc_create_client(IPC_SERVER_ID,IPC_CLIENT_ID3);
	GREATEST_ASSERT(ipc_client3);
	ipc_client4 = wow_ipc_create_client(IPC_SERVER_ID,IPC_CLIENT_ID4);
	GREATEST_ASSERT(ipc_client4);

	ret = wow_ipc_set_notify_all(ipc_server,server_ipc_callback_all,NULL);
	GREATEST_ASSERT(ret == 0);
	ret = wow_ipc_set_notify_all(ipc_client2,client2_ipc_callback_all,NULL);
	GREATEST_ASSERT(ret == 0);
	ret = wow_ipc_set_notify_all(ipc_client3,client3_ipc_callback_all,NULL);
	GREATEST_ASSERT(ret == 0);
	ret = wow_ipc_set_notify_all(ipc_client4,client4_ipc_callback_all,NULL);
	GREATEST_ASSERT(ret == 0);

	char data[10] = "*********";
	ret = wow_ipc_send(ipc_client2,IPC_TEST5,data, 10);
	GREATEST_ASSERT(ret == 10);

	sleep(1);
	wow_ipc_destroy(&ipc_client1);
	wow_ipc_destroy(&ipc_client2);
	wow_ipc_destroy(&ipc_client3);
	wow_ipc_destroy(&ipc_client4);
	wow_ipc_destroy(&ipc_server);

	PASS();
}

SUITE(suit_ipc)
{
#if SUIT_RUN_ERROR_TEST
    RUN_TEST(test_ipc_error);
#endif
    RUN_TEST(test_ipc_fun_01);
	RUN_TEST(test_ipc_fun_02);
	RUN_TEST(test_ipc_fun_03); //推荐此方式
	RUN_TEST(test_ipc_fun_04);
}




