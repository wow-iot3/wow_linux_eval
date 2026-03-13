#define _GNU_SOURCE 		/* See feature_test_macros(7) */
#include <stdio.h>

#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <sys/types.h>
#include <execinfo.h>

#include "prefix/wow_check.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_common.h"

#include "utils/wow_singleton.h"
#include "algorithm/wow_bloomfilter.h"

#include "plugin/wow_debug.h"
#include "plugin/wow_shell.h"

#include "wow_iot_config.h"
#include "wow_iot_internal.h"

static BloomFilter_T* gpt_debugBloomFilter = NULL;

/*brief    设置debug状态
 *param ： pcMod   ：模块名称
 *param ： bEnable ：是否使能
 *return： 无
 */
void wow_debug_setStatus(const char *pcMod,bool bEnable)
{
	int ret = 0;
	
	if(bEnable){
		ret = wow_bloomfilter_set_1(gpt_debugBloomFilter,(const void*)pcMod);
	}
	else{
		ret = wow_bloomfilter_set_0(gpt_debugBloomFilter,(const void*)pcMod);
	}

	CHECK_RET_VOID_P(ret >= 0,"mod:%s wow_bloomfilter_set_%d failed!\n",pcMod,bEnable);
}

/*brief    获取debug状态
 *param ： pcMod   ：模块名称
 *return： 1-此模块已使能 0-此模块未使能 -1-无效模块
 */
int wow_debug_getStatus(const char *pcMod)
{
	return wow_bloomfilter_get(gpt_debugBloomFilter,(const void*)pcMod);
}

#define DEBUG_BUF_SIZE 2048
static const char *debug_level_str[] = {
    "DIS", "ERR", "WRN", "INF", "DBG"
};

void wow_debug(const char *mod,DebugLevel_E lev,const char *format, ...)
{
	CHECK_RET_VOID(wow_debug_getStatus((const void*)mod));
	
	va_list ap;
	char *str = NULL;;
	char buf[DEBUG_BUF_SIZE] = {0};
	
	va_start(ap, format);
	vasprintf(&str, format, ap);
	va_end(ap); 
	
	int size = snprintf(buf,DEBUG_BUF_SIZE-1,"[%s][%s]:%s",mod,debug_level_str[lev],str);
	
#if WOW_SHELL_TTY_SUPPORT
	_wow_shell_tty_write(buf,size);
#endif
	
#if WOW_SHELL_ETH_SUPPORT
	_wow_shell_eth_write(buf,size);
#endif		

	free(str);
}

static int debug_init(void)
{
	gpt_debugBloomFilter = wow_bloomfilter_init(TB_BLOOM_FILTER_PROBABILITY_0_001, MAXU16);
	CHECK_RET_VAL_P(gpt_debugBloomFilter,-1,"debug_init failed!\n");

	return 0;
}

static void debug_exit(void)
{
    wow_bloomfilter_exit(&gpt_debugBloomFilter);
}

__IN_API__ int _wow_debug_env_init(void)
{
    return wow_singleton_instance(SINGLETON_IOT_MOD_DEBUG, debug_init, debug_exit);
}

static int shell_debug(int argc, char *argv[])
{
	char modbuf[MAX_NAME_SIZE] = {0};
	char cmdbuf[8]  = {0};
				
	CHECK_RET_VAL(argc == 3, -1);
	memset(modbuf,0,sizeof(modbuf));
	strncpy(modbuf,argv[1],MIN2(strlen(argv[1]),MAX_NAME_SIZE-1));

	memset(cmdbuf,0,sizeof(cmdbuf));
	strncpy(cmdbuf,argv[2],7);
	if(!strncmp(cmdbuf,"on",2)){
		wow_debug_setStatus(modbuf,1);
	}
	else if(!strncmp(cmdbuf,"off",3)){	
		wow_debug_setStatus(modbuf,0);
	}
	else{
		return -1;
	}
	return 0;
}


#define DEBUG_INFO "debug XXX on/off  (XXX is the module name)"
WOW_DECLARE_SHELL_IN("debug" ,shell_debug,DEBUG_INFO);


