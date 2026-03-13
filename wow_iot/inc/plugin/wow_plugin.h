#ifndef __WOW_BASE_PLUGIN_WOW_PLUGIN_H_
#define __WOW_BASE_PLUGIN_WOW_PLUGIN_H_

#ifdef __cplusplus
extern "C" {
#endif

#define WOW_PLUGIN_PRIORITY_LOW      -100
#define WOW_PLUGIN_PRIORITY_DEFAULT     0
#define WOW_PLUGIN_PRIORITY_HIGH      100

typedef struct  {
	const char *module;
	const char *version;
	int  priority;
	int  (*init) (void);
	void (*exit) (void);
	void* (*user) (void* arg);
}__attribute__((aligned(32))) PluginDesc_T;


#define DECLARE_PLUGIN_BUILDIN(module,priority, finit, fexit) \
    static PluginDesc_T _plugin_builtin_##module \
    __attribute__((used,section("_plugin_def")))={ \
		 #module,NULL,priority,finit,fexit\
	};

#define DECLARE_PLUGIN_BUILDEX(module,version, priority, finit, fexit) \
	 PluginDesc_T wow_plugin_desc = { \
		#module,version,priority,finit,fexit\
	};	


/*brief    插件功能初始化
 *param ： pcModule:使能模块名称 多模块使用@字符作为间隔符
 *return： 成功返回0 失败返回<0
 */
int   wow_plugin_init(const char* pcModule);

/*brief    插件功能退出
 *return： 无
 */
void  wow_plugin_exit(void);


/*brief    插件功能初始化
 *param ： pcModule:使能模块名称 多模块使用@字符作为间隔符
 *return： 成功返回0 失败返回<0
 */
void* wow_plugin_get_user(const char* pcModule);

#ifdef __cplusplus
}
#endif

#endif