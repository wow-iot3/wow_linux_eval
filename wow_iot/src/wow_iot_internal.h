#ifndef __WOW_IOT_INTERNAL_H_
#define __WOW_IOT_INTERNAL_H_

#include "utils/wow_singleton.h"

typedef enum{
    SINGLETON_TYPE_MOD_DEBUG = SINGLETON_TYPE_IOT,
    SINGLETON_IOT_MOD_SHELL,
    SINGLETON_IOT_MOD_DEBUG,
    SINGLETON_IOT_MOD_LOG,
    SINGLETON_IOT_MOD_SQLITE,
}IotSingletonIndex_E;

int _wow_shell_env_init(void);
int _wow_debug_env_init(void);
int _wow_log_env_init(void);

int _wow_sqlite3_library_init(void);

#define WOW_DECLARE_SHELL_IN(cmdname, func, helpinfo) \
    static ShellDesc_T _shellcmd_##func \
    __attribute__((used,section("_shell_in_cmd")))={ \
	.cmd = cmdname,.pfunc = func,.info = helpinfo\
	};

/*brief    shell-tty数据写入
 *param ： pcStr	：写入数据内容
 *param ： snLen	：写入数据内容长度
 *return： 无
 */
void _wow_shell_tty_write(const char *pcStr,size_t snLen);


/*brief    shell-eth数据写入
 *param ： pcStr	：写入数据内容
 *param ： snLen	：写入数据内容长度
 *return： 无
 */
void _wow_shell_eth_write(const char *pcStr,size_t snLen);

#endif
