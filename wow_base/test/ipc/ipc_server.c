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
				CHECK_RET_VAL_P(ret == sizeof(int),-1,"wow_ipc_send failed.\n");

			}
			break;
		case IPC_TEST2_ACK:
			{
				printf(MOD_TAG"TEST2: buf:%s in_len:%d\n",(char*)in_arg,(int)in_len);
				ret = wow_ipc_send(ipc, IPC_TEST2_ACK,"AAAAAAAA",10);
				CHECK_RET_VAL_P(ret == in_len,-1,"wow_ipc_send failed.\n");
			}
			break;
		case IPC_TEST3_ACK:
			{
				printf(MOD_TAG"TEST3: buf:%s in_len:%d\n",(char*)in_arg,(int)in_len);
				ret = wow_ipc_send(ipc, IPC_TEST4_ACK,in_arg,in_len);
				CHECK_RET_VAL_P(ret == in_len,-1,"wow_ipc_send failed.\n");
			}
			break;
		case IPC_TEST4_ACK:
			{
				printf(MOD_TAG"TEST4: buf:%s in_len:%d\n",(char*)in_arg,(int)in_len);
				ret = wow_ipc_send(ipc, IPC_TEST4_ACK,in_arg,in_len);
				CHECK_RET_VAL_P(ret == in_len,-1,"wow_ipc_send failed.\n");
			}
			break;
	}


	return 0;
}



int main(int argc, char *argv[])
{
	int ret = -1;

    Ipc_T* ipc_server = wow_ipc_create_server(IPC_SERVER_ID);
	CHECK_RET_VAL_P(ipc_server,-1,"wow_ipc_create_server failed.\n");

	ret = wow_ipc_set_notify_all(ipc_server,server_ipc_callback_all,NULL);
	CHECK_RET_VAL_P(ret == 0,-1,"wow_ipc_set_notify_all failed.\n");

	while(1) {sleep(1);}

    wow_ipc_destroy(&ipc_server);

    return 0;
}




