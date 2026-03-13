#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "prefix/wow_check.h"

#include "prefix/wow_check.h"
#include "system/wow_thread.h"

#include "ipc/wow_ipc.h"
#include "ipc_include.h"

#define MOD_TAG "[ipc]"



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


int main(int argc, char *argv[])
{
	int ret = -1;
	Ipc_T* ipc_client1;
	Ipc_T* ipc_client2;
	Ipc_T* ipc_client3;
	Ipc_T* ipc_client4;

	ipc_client1 = wow_ipc_create_client(IPC_SERVER_ID,IPC_CLIENT_ID1);
	CHECK_RET_VAL_P(ipc_client1,-1,"wow_ipc_create_client failed.\n");
	ipc_client2 = wow_ipc_create_client(IPC_SERVER_ID,IPC_CLIENT_ID2);
	CHECK_RET_VAL_P(ipc_client2,-1,"wow_ipc_create_client failed.\n");
	ipc_client3 = wow_ipc_create_client(IPC_SERVER_ID,IPC_CLIENT_ID3);
	CHECK_RET_VAL_P(ipc_client3,-1,"wow_ipc_create_client failed.\n");
	ipc_client4 = wow_ipc_create_client(IPC_SERVER_ID,IPC_CLIENT_ID4);
	CHECK_RET_VAL_P(ipc_client4,-1,"wow_ipc_create_client failed.\n");

	ret = wow_ipc_set_notify_all(ipc_client2,client2_ipc_callback_all,NULL);
	CHECK_RET_VAL_P(ret == 0,-1,"wow_ipc_set_notify_all failed.\n");
	ret = wow_ipc_set_notify_all(ipc_client3,client3_ipc_callback_all,NULL);
	CHECK_RET_VAL_P(ret == 0,-1,"wow_ipc_set_notify_all failed.\n");
	ret = wow_ipc_set_notify_all(ipc_client4,client4_ipc_callback_all,NULL);
	CHECK_RET_VAL_P(ret == 0,-1,"wow_ipc_set_notify_all failed.\n");


	ipc_client_test1(ipc_client1);
	ipc_client_test2(ipc_client2);
	ipc_client_test3(ipc_client3);
	ipc_client_test4(ipc_client4);

	wow_ipc_destroy(&ipc_client1);
	wow_ipc_destroy(&ipc_client2);
	wow_ipc_destroy(&ipc_client3);
	wow_ipc_destroy(&ipc_client4);

    return 0;
}