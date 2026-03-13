#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>

#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"


#include "utils/wow_str.h"
#include "utils/wow_singleton.h"
#include "network/wow_socket.h"
#include "plugin/wow_shell.h"

#include "list/wow_slist.h"
#include "system/wow_thread.h"

#include "wow_iot_internal.h"
#include "wow_iot_config.h"


#define ETH_SHELL_PORT  7260

#define SHELL_BUFFER_SIZE 256

typedef struct {
	int			 	 run;    
	pthread_idx_t    tid;
    int 		 sktfd;
	char     	 serAddr[64];
	uint16_t     sePort;
	char     	 cliAddr[64];
	uint16_t     cliPort;
}__data_aligned__ EthThread_T;

typedef struct {
	int				 run;      
	pthread_idx_t    tid;
}__data_aligned__ TtyThread_T;


extern const ShellDesc_T __start__shell_in_cmd[];
extern const ShellDesc_T __stop__shell_in_cmd[];
extern const ShellDesc_T __start__shell_ex_cmd[];
extern const ShellDesc_T __stop__shell_ex_cmd[];

static Slist_T* gpt_sheelList = NULL;

static TtyThread_T    gt_ttyThread = {0};
static EthThread_T    gt_ethThread = {0};

/*加载built_in与外部库*/
void shell_plugin_init()
{
	const ShellDesc_T *pcmd;

	gpt_sheelList = wow_slist_create();
	CHECK_RET_VOID(gpt_sheelList);

	//加载本地shell-cmd
	for(pcmd = __start__shell_in_cmd; pcmd < __stop__shell_in_cmd; pcmd++) {
		CHECK_RET_CONTINUE(pcmd && pcmd->cmd && pcmd->pfunc);
		wow_slist_insert_tail(gpt_sheelList,(void*)pcmd);
	}	

	//加载本地shell-cmd
	for(pcmd = __start__shell_ex_cmd; pcmd < __stop__shell_ex_cmd; pcmd++) {
		CHECK_RET_CONTINUE(pcmd && pcmd->cmd && pcmd->pfunc);
		wow_slist_insert_tail(gpt_sheelList,(void*)pcmd);
	}	
}

void shell_plugin_exit()
{
	wow_slist_destroy(&gpt_sheelList,NULL);
}

//输入命令与本地shell-cmd匹配
ShellDeal_F shell_find_func(const char *command)
{
	int i = 0;
	ShellDesc_T* tmp = NULL; 
	
	CHECK_RET_VAL(command && gpt_sheelList, NULL);
	
	for(i = 0;  i< wow_slist_size(gpt_sheelList); i++) 
	{
		tmp = wow_slist_peek_by_index(gpt_sheelList,i);
		CHECK_RET_CONTINUE(tmp);
		
		//dprintf("index:%d cmm(%d):%s name(%d):%s\n",i,strlen(command),command,strlen(tmp->cmd),tmp->cmd);
		if(0 == strcmp(tmp->cmd, command)){
			return tmp->pfunc;
		}
	}
	return NULL;
}
/*************************************help_shell*******************************************/
static int shell_help(int argc, char *argv[])
{
	int i = 0;
	char buf[512] = {0};
	ShellDesc_T* tmp = NULL; 
		
	CHECK_RET_VAL(gpt_sheelList, -1);
	
	for(i=0; i <  wow_slist_size(gpt_sheelList); i++) 
	{	
		tmp = wow_slist_peek_by_index(gpt_sheelList,i);
		CHECK_RET_CONTINUE(tmp);

		int size = snprintf(buf,511,"%s: \t%s\n",tmp->cmd, tmp->info);
		
#if WOW_SHELL_TTY_SUPPORT
		_wow_shell_tty_write(buf,size);
#endif

#if WOW_SHELL_ETH_SUPPORT
		_wow_shell_eth_write(buf,size);
#endif		
	}
	return 0;
}

static int shell_empty(int argc, char *argv[])
{
}

WOW_DECLARE_SHELL_IN("help",shell_help,"Show all valid cmd!");
WOW_DECLARE_SHELL_CMD("wow",shell_empty,"This is a empty command!");

/**************************串口与以太网后台调试功能**************************************/

/*brief    shell-tty数据写入
 *param ： pcStr	：写入数据内容
 *param ： snLen	：写入数据内容长度
 *return： 无
 */
__IN_API__ void _wow_shell_tty_write(const char *pcStr,size_t len)
{
	printf("%s",pcStr);
}

/*brief 串口后台获取输入方式 */
static void* shell_tty_thread(void *arg)
{
	int argc = 0;
	int size = 0;
	char** argv = NULL;
	ShellDeal_F fun;
	char command[SHELL_BUFFER_SIZE];
	char  wBuf[SHELL_BUFFER_SIZE];
	
	while(gt_ttyThread.run){
		printf("##:");

		/*获取一行内容 阻塞操作 */
		/*标准输入需要有回车键或者CTRL+D退出*/
		fgets(command, SHELL_BUFFER_SIZE-1, stdin);
		CHECK_RET_CONTINUE(strlen(command) > 1);
		//删除最后一字节换行符
		command[strlen(command)-1]='\0';
		
		//将命令与参数分离
		argv = wow_str_split(command," ",&argc);
		CHECK_RET_CONTINUE(argc > 0 && argv);
		
		//查询是否存在此shell命令
		fun = shell_find_func(argv[0]);
		if(NULL != fun) {
			//存在 调用此命令函数
			if(-1 == (*fun)(argc, argv)){
				size = snprintf(wBuf,SHELL_BUFFER_SIZE-1,"exec %s command errer!\n",argv[0]);
			}
			else{
				size = snprintf(wBuf,SHELL_BUFFER_SIZE-1,"exec %s command succeeful!\n",argv[0]);
			}
		}
		else{
			size = snprintf(wBuf,SHELL_BUFFER_SIZE-1,"can not find %s command\n",argv[0]);
		}
		_wow_shell_tty_write(wBuf,size);
		wow_str_split_free(argv);
	}

	return NULL;
}

static int shell_tty_init(void)
{
	memset(&gt_ttyThread,0,sizeof(TtyThread_T));

	gt_ttyThread.run = 1;
	gt_ttyThread.tid = wow_thread_create("shell_tty",shell_tty_thread, NULL);
	CHECK_RET_VAL_P(gt_ttyThread.tid > 0, -1, "wow_thread_create failed!\n");
	
	return  0;
}


static void shell_tty_exit(void)
{
	gt_ttyThread.run = 0;
	wow_thread_cancel(gt_ttyThread.tid);
	usleep(100*1000);
}



/*brief    shell-eth数据写入
 *param ： pcStr	：写入数据内容
 *param ： snLen	：写入数据内容长度
 *return： 无
 */
__IN_API__ void _wow_shell_eth_write(const char *pcStr,size_t snLen)
{
	CHECK_RET_VOID(gt_ethThread.sktfd > 0 && gt_ethThread.cliPort > 0);
	wow_socket_usend(gt_ethThread.sktfd,gt_ethThread.cliAddr,gt_ethThread.cliPort,(const uint8_t*)pcStr,snLen);
}

static void* shell_eth_thread(void *arg)  
{  
	int serverfd = *(int*)arg;
	int argc = 0;
	int size = 0;
	char** argv = NULL;
	ShellDeal_F pfunc;
	char  wBuf[SHELL_BUFFER_SIZE];
	char  rBuf[SHELL_BUFFER_SIZE];	

	while(gt_ethThread.run)
	{
		usleep(100000);
		size = wow_socket_urecv(serverfd,gt_ethThread.cliAddr,&gt_ethThread.cliPort,
									(uint8_t*)rBuf,SHELL_BUFFER_SIZE);
		CHECK_RET_CONTINUE(size > 0);
		//dprintf("recv data(%d):%s\n",size,rBuf);
		
		//将命令与参数分离
		argv = wow_str_split(rBuf," ",&argc);
		CHECK_RET_CONTINUE(argc > 0);
		
		//查询是否存在此shell命令
		pfunc = shell_find_func(argv[0]);
		if(NULL != pfunc) {
			//存在 调用此命令函数
			if(-1 == (*pfunc)(argc, argv)){
				size = snprintf(wBuf,SHELL_BUFFER_SIZE-1,"exe %s command errer!\r\n##:",argv[0]);
			}
			else{
				size = snprintf(wBuf,SHELL_BUFFER_SIZE-1,"exe %s command succeeful!\r\n##:",argv[0]);
			}
		}
		else{
			size = snprintf(wBuf,SHELL_BUFFER_SIZE-1,"can not find %s command\r\n##:",argv[0]);
		}

		
		_wow_shell_eth_write(wBuf,size);
		wow_str_split_free(argv);

		memset(wBuf,0,SHELL_BUFFER_SIZE);
		memset(rBuf,0,SHELL_BUFFER_SIZE);
	} 
	
	wow_socket_close(gt_ethThread.sktfd); 

	return NULL;
} 

static int shell_eth_init(void)
{
	memset(&gt_ethThread,0,sizeof(TtyThread_T));

	gt_ethThread.sktfd = wow_socket_open(SOCKET_TYPE_UDP);
	CHECK_RET_VAL_P(gt_ethThread.sktfd > 0,-1,"wow_socket_open failed!\n");

	int ret = wow_socket_bind(gt_ethThread.sktfd,NULL,ETH_SHELL_PORT);
	CHECK_RET_GOTO_P(ret == 0,out, "wow_socket_bind failed!\n");

	/*创建线程 实现服务器接收多客户端*/
	gt_ethThread.run = 1;
	gt_ethThread.tid = wow_thread_create("shell_eth",shell_eth_thread, (void *)&gt_ethThread.sktfd);
	CHECK_RET_GOTO_P(gt_ttyThread.tid > 0, out, "wow_thread_create failed!\n");

	return 0;
out:
	wow_socket_close(gt_ethThread.sktfd);
	return -1;
}

static void shell_eth_exit(void)
{
	gt_ethThread.run = 0;
	wow_thread_cancel(gt_ethThread.tid);
	usleep(100*1000);
}

//shell自定义命令后台管理
static int shell_init(void)
{
	//获取全部自定义shell-cmd
	shell_plugin_init();

#if WOW_SHELL_TTY_SUPPORT
	shell_tty_init();
#endif
	
#if WOW_SHELL_ETH_SUPPORT
	shell_eth_init();
#endif	

	return 0;
}

static void shell_exit(void)
{
	shell_plugin_exit();
	
#if WOW_SHELL_TTY_SUPPORT
	shell_tty_exit();
#endif
		
#if WOW_SHELL_ETH_SUPPORT
	shell_eth_exit();
#endif	

}

/*******************************内部接口***********************************************/
__IN_API__ int _wow_shell_env_init(void)
{
    return wow_singleton_instance(SINGLETON_IOT_MOD_SHELL, shell_init, shell_exit);
}

