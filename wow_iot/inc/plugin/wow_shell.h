#ifndef __WOW_BASE_PLUGIN_WOW_SHEEL_H_
#define __WOW_BASE_PLUGIN_WOW_SHEEL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "prefix/wow_keyword.h"

/*shell管理初始化(实现tty与telnet的控制台语句) 默认已加载"help"、"debug"
 *其中tty实现方式为接收标准输入内容 其中eth实现方式为udp-client方式连接
 */
typedef int (*ShellDeal_F)(int argc, char *argv[]);

typedef struct{
    const char *cmd;
	ShellDeal_F pfunc;
    const char *info;
	const char *priv;
}__data_aligned__ ShellDesc_T;

//声明shell函数功能-即自定义shell语句 可通过控制台获取命名 
//libbase库中已添加"help"命令
//cmdname-shell命令(字符串形式) func-cmdname命令执行函数 helpinfo-shell命令说明(字符串形式)
#define WOW_DECLARE_SHELL_CMD(cmdname, func, helpinfo) \
    static ShellDesc_T _shellcmd_##func \
    __attribute__((used,section("_shell_ex_cmd")))={ \
	.cmd = cmdname,.pfunc = func,.info = helpinfo\
	};

	

#ifdef __cplusplus
}
#endif

#endif
