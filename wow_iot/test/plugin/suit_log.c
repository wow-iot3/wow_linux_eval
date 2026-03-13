#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "plugin/wow_log.h"
#include "plugin/wow_debug.h"
#include "system/wow_thread.h"

#include "greatest/greatest.h"
#include "wow_test_config.h"


#define MOD_TAG       "log"
#define LOG_FILE_PATH "log/"

typedef struct{
	char mod[10];
	pthread_idx_t thread;
}LogModInfo_T;

static LogModInfo_T logMod[3]={};

void* moduleLogThread(void *arg)
{
	int count = 20;
	LogModInfo_T* logMod = (LogModInfo_T*)arg;

	do{
		//错误log 保存条目附有ERROR字样
		ModuleErrorLog(logMod->mod,"Test-111:%d\n",count);
		//告警log 保存条目附有WARN字样
		ModuleWarnLog(logMod->mod,"Test-222:%d\n",count);
		//提示log 保存条目附有INFO字样
		ModuleInfoLog(logMod->mod,"Test-333:%d\n",count);
		//调试log 保存条目附有DEBUG字样
		ModuleDebugLog(logMod->mod,"Test-444:%d\n",count);

		sleep(1);
	}while(count--);
	
	return NULL;
}	


TEST test_log_module1(void)
{
	int ret = 0;
	
	ModDebug_D(MOD_TAG,"suit_log---------test_log_module1\n");	
		
	strncpy(logMod[0].mod, "module1",9);

	ret = wow_log_set_module_path(logMod[0].mod,WOW_TEST_FILE_STRING(LOG_FILE_PATH));
	
	ret = wow_log_set_module_level(logMod[0].mod,LOG_LEV_DEBUG);
	GREATEST_ASSERT(ret == 0);
	
	ret = wow_log_set_module_flag(logMod[0].mod,LOG_ATT_TIMESTAMP);
	GREATEST_ASSERT(ret == 0);

	logMod[0].thread = wow_thread_create(MOD_TAG,moduleLogThread,(void *)&logMod[0]);
	GREATEST_ASSERT(logMod[0].thread);
	
	PASS();
}

TEST test_log_module2(void)
{
	int ret = 0;

	ModDebug_D(MOD_TAG,"suit_log---------test_log_module2\n");
	
	strncpy(logMod[1].mod, "module2",9);
	ret = wow_log_set_module_level(logMod[1].mod,LOG_LEV_ERR);
	GREATEST_ASSERT(ret == 0);

	logMod[1].thread  = wow_thread_create(MOD_TAG,moduleLogThread,(void *)&logMod[1]);
	GREATEST_ASSERT(logMod[1].thread);
	
	PASS();
}


TEST test_log_module3(void)
{
	int ret = 0;

	ModDebug_D(MOD_TAG,"suit_log---------test_log_module3\n");
	
	strncpy(logMod[2].mod, "module3",9);
	ret = wow_log_set_module_level(logMod[2].mod,LOG_LEV_ERR);
	GREATEST_ASSERT(ret == 0);

	ret = wow_log_set_module_file(logMod[2].mod,512,3);
	GREATEST_ASSERT(ret == 0);

	ret = wow_log_set_module_flag(logMod[2].mod,LOG_ATT_TIMESTAMP |LOG_ATT_PIDTID |LOG_ATT_FUNCLINE);
	GREATEST_ASSERT(ret == 0);
	
	logMod[2].thread = wow_thread_create(MOD_TAG,moduleLogThread,(void *)&logMod[2]);
	GREATEST_ASSERT(logMod[2].thread);
	
	PASS();
}



SUITE(suit_log)
{
	wow_debug_setStatus(MOD_TAG,SUIT_RUN_DEBUG_FLAG);
	
    RUN_TEST(test_log_module1);
    RUN_TEST(test_log_module2);
	RUN_TEST(test_log_module3);
}

