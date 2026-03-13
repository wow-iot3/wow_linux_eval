#ifndef __WOW_IOT_LOG_WOW_LOG_H_
#define __WOW_IOT_LOG_WOW_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

/*使用说明
 *由于日志模块已文件的方式记录，约束不存在重复的模块名称
 *建议用户标注APP-进程名称—模块名称样式
 *日志默认存储路径 /home/log/
 */
 
typedef enum{
	LOG_LEV_EMERG   = 0,  /* system is unusable */
	LOG_LEV_ALERT   = 1,  /* action must be taken immediately */
	LOG_LEV_CRIT    = 2,  /* critical conditions */
	LOG_LEV_ERR     = 3,  /* error conditions */
	LOG_LEV_WARNING = 4,  /* warning conditions */
	LOG_LEV_NOTICE  = 5,  /* normal but significant condition */
	LOG_LEV_INFO    = 6,  /* informational */
	LOG_LEV_DEBUG   = 7,  /* debug-level messages */
	LOG_LEV_VERB    = 8,  /* verbose messages */
}LogLevel_E;
	
typedef enum{
	LOG_ATT_TIMESTAMP   = 1, 
	LOG_ATT_PIDTID      = 2,  
	LOG_ATT_FUNCLINE    = 4, 
}LogAttr_E;


/*brief    设置日志文件级别
 *param ： eLevel: 打印级别			//默认LOG_LEV_WARNING
 *return： 成功返回0 失败返回-1
 */
int wow_log_set_default_level(LogLevel_E eLevel);

/*brief    设置日志文件目录打印特性
 *param ： eAttr: 打印特性   //默认0
 *return： 成功返回0 失败返回-1
 */
int wow_log_set_default_flag(LogAttr_E eAttr);

/*brief    设置日志文件信息
 *param ： u32Size: 文件存储大小  //256K
 *param ： u32Num:  文件存储个数  //默认1
 *return： 成功返回0 失败返回-1
 */
int wow_log_set_default_file(uint32_t u32Size, uint32_t u32Num);

/*brief    设置日志文件默认目录
 *param ： pcDir: 文件存储路径 "/home/tmp/log/"
 *return： 成功返回0 失败返回-1
 */
int wow_log_set_default_path(const char* pcDir);


/*brief    设置日志文件级别
 *param ： pcMod:  功能模块名称
 *param ： eLevel: 打印级别			//默认LOG_LEV_WARNING
 *return： 成功返回0 失败返回-1
 */
int wow_log_set_module_level(const char* pcMod,LogLevel_E eLevel);

/*brief    设置日志文件打印特性
 *param ： pcMod:  功能模块名称
 *param ： eAttr:  打印特性   //默认0
 *return： 成功返回0 失败返回-1
 */
int wow_log_set_module_flag(const char* pcMod,LogAttr_E eAttr);

/*brief    设置日志文件信息
 *param ： pcMod:  功能模块名称
 *param ： u32Size:  文件存储大小  //256K
 *param ： u32Num:  文件存储个数  //默认1
 *return： 成功返回0 失败返回-1
 */
int wow_log_set_module_file(const char* pcMod,uint32_t u32Size, uint32_t u32Num);

/*brief    设置日志文件存储目录
 *param ： pcMod:  功能模块名称
 *param ： pcDir: 文件存储路径 //默认"/home/tmp/log/"
 *return： 成功返回0 失败返回-1
 */
int wow_log_set_module_path(const char* pcMod,char* pcDir);



void log_print(const char* mod,int lvl, const char *file, int line,const char *fmt, ...);

///<错误信息log存储	emerg
#define ModuleEmergLog(mod,...)do{ \
	log_print(mod,LOG_LEV_EMERG, __FILE__, __LINE__, __VA_ARGS__); \
}while(0);
///<错误信息log存储	alert
#define ModuleAlertLog(mod,...)do{ \
	log_print(mod,LOG_LEV_ALERT, __FILE__, __LINE__, __VA_ARGS__); \
}while(0);
///<错误信息log存储	crit
#define ModuleCritLog(mod,...)do{ \
	log_print(mod,LOG_LEV_CRIT, __FILE__, __LINE__, __VA_ARGS__); \
}while(0);
///<错误信息log存储	error
#define ModuleErrorLog(mod,...)do{ \
	log_print(mod,LOG_LEV_ERR, __FILE__, __LINE__, __VA_ARGS__); \
}while(0);
///<告警信息log存储 warn
#define ModuleWarnLog(mod,...)do{ \
	log_print(mod,LOG_LEV_WARNING, __FILE__, __LINE__, __VA_ARGS__); \
}while(0);	
///<告警信息log存储 notice
#define ModuleNoticeLog(mod,...)do{ \
	log_print(mod,}while(0);LOG_LEV_NOTICE, __FILE__, __LINE__, __VA_ARGS__); \
}while(0);
///<提示信息log存储 Info
#define ModuleInfoLog(mod,...)do{ \
	log_print(mod,LOG_LEV_INFO, __FILE__, __LINE__, __VA_ARGS__); \
}while(0);
///<调试信息log存储 debug
#define ModuleDebugLog(mod,...)do{ \
	log_print(mod,LOG_LEV_DEBUG, __FILE__, __LINE__, __VA_ARGS__); \
}while(0);
///<全信息打印
#define ModuleVerbLog(mod,...)do{ \
	log_print(mod,LOG_LEV_VERB, __FILE__, __LINE__,__VA_ARGS__); \
}while(0);


/*********************程序LOG存储显示********************************/
#define CHECK_RET_GOTO_L(mod,x,out,args...)		\
do {							    \
	if (!(x)) {						\
		ModuleErrorLog(mod,args);			\
		goto out;				\
	}								\
} while(0);

#define CHECK_RET_GOTO_EXE_L(mod,x,out,exe,args...)		\
do {							    \
	if (!(x)) {						\
		ModuleErrorLog(mod,args);			\
		exe;					\
		goto out;				\
	}								\
} while(0);

#define CHECK_RET_VAL_L(mod,x,v,args...)		\
do {							    \
	if (!(x)) {						\
		ModuleErrorLog(mod,args);			\
		return v;				\
	}								\
} while(0);
#define CHECK_RET_VAL_EXE_L(mod,x,v,exe,args...)		\
	do {								\
		if (!(x)) { 					\
			ModuleErrorLog(mod,args);			\
			exe;					\
			return v;				\
		}								\
	} while(0);
		

#define CHECK_RET_VOID_L(mod,x,args...)		\
do {							    \
	if (!(x)) {						\
		ModuleErrorLog(mod,args);		\
		return;				\
	}								\
} while(0);
#define CHECK_RET_VOID_EXE_L(mod,x,exe,args...)		\
		if (!(x)) { 					\
			ModuleErrorLog(mod,args);			\
			exe;					\
			return;				\
		}	

#define CHECK_RET_CONTINUE_L(mod,x,args...)		\
	if (!(x)) {						\
		ModuleErrorLog(mod,args);			\
		continue;				\
	}		
#define CHECK_RET_CONTINUE_EXE_L(mod,x,exe,args...)		\
	if (!(x)) {						\
		ModuleErrorLog(mod,args);			\
		exe;					\
		continue;				\
	}	
	
#define CHECK_RET_BREAK_L(mod,x,args...)		\
	if (!(x)) {						\
		ModuleErrorLog(mod,args);		\
		break;				\
	}								
#define CHECK_RET_BREAK_EXE_L(mod,x,exe,args...)		\
	if (!(x)) {						\
		ModuleErrorLog(mod,args);		\
		exe;				\
		break;				\
	}		
	
#define CHECK_RET_STREQ_L(mod,STR1, STR2,v,args...) \
do { \
	if (strcmp(STR1, STR2) != 0) { \
		ModuleErrorLog(mod,args);		\
		return v;				\
	} \
} while (0);

#ifdef __cplusplus
}
#endif

#endif
