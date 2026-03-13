#include <errno.h>
#include <dlfcn.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>

#include "prefix/wow_check.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_common.h"

#include "utils/wow_str.h"
#include "utils/wow_singleton.h"
#include "file/wow_dir.h"
#include "list/wow_slist.h"
#include "list/wow_string_list.h"

#include "plugin/wow_plugin.h"


static List_T* gpt_plugins_list = NULL;

typedef struct {
	int active;
	void* handle;
	const PluginDesc_T *desc;
}__data_aligned__ PluginInfo_T;


static int plugin_add(const PluginDesc_T *desc,void* handle)
{
	CHECK_RET_VAL(desc && gpt_plugins_list, -1);

	PluginInfo_T *plugin  = CALLOC(1,PluginInfo_T);
	CHECK_RET_VAL_P(plugin, -SYSTEM_MALLOC_FAILED,"malloc PluginInfo_T failed!\n");
	
	plugin->active = 0;
	plugin->handle = handle;
	plugin->desc   = desc;

	int i = 0;
	int size = wow_slist_size(gpt_plugins_list);
	for(i = 0; i < size; i++){
		PluginInfo_T *tmp = wow_slist_peek_by_index(gpt_plugins_list,i);
		if(desc->priority >= tmp->desc->priority ){
			return wow_slist_insert_by_index(gpt_plugins_list,i,plugin);
		}
	}
	return wow_slist_insert_tail(gpt_plugins_list,plugin);
}

static int plugin_enable(const char *module, char **cli_enable)
{
	int i;
	for (i = 0; cli_enable[i] != NULL; i++){
		if(StringEqual(cli_enable[i], module) == 1){
			return 0;
		}
	}
	return -1;
}

extern const PluginDesc_T __start__plugin_def[];
extern const PluginDesc_T __stop__plugin_def[];

__EX_API__ int wow_plugin_init(const char *pcModule)
{
	int i = 0;
	int ret = 0;
	int size = 0;
	
	CHECK_RET_VAL_P(gpt_plugins_list, -PLUGIN_NOT_INITED, "plugin not inited!\n!\n");
	CHECK_RET_VAL_P(pcModule, -PARAM_INPUT_DATA_IS_NULL, "param input data invalid!\n");
	
	StringList_T* file_list = NULL;
	char *filemodule = NULL;
	void *handle   = NULL;
	const PluginDesc_T *desc = NULL;
	char** cli_enabled  = NULL;
	
	//分离使能plugin的名称
	cli_enabled = wow_str_split(pcModule, "@", &size);
	CHECK_RET_VAL_P(cli_enabled && size > 0,-WOW_FAILED,"praram exec arg invalid!\n");

	/***********内部接口处理**************/
	for (desc = __start__plugin_def; desc < __stop__plugin_def; desc++)
	{
		CHECK_RET_CONTINUE(desc && desc->module);
		//printf("-----in-module:%s\n",desc->module);
		ret = plugin_enable(desc->module, cli_enabled);
		CHECK_RET_CONTINUE(ret == 0);

		plugin_add(desc,NULL);
	}
	
	/***********外部接口处理**************/
	file_list = wow_dir_fileList(WOW_BASE_PLUGIN_DIR);
	if(file_list){
		size = wow_stringlist_size(file_list);
		for( i = 0; i < size; i++){
			filemodule = wow_stringlist_data(file_list,i);
			CHECK_RET_CONTINUE(filemodule);
			if(wow_str_has_suffix(filemodule, ".so") == 1){
				handle = dlopen(filemodule, RTLD_LAZY);
				CHECK_RET_CONTINUE_P(handle,"dlopen failed!\n");

				desc = dlsym(handle, "wow_plugin_desc");
				CHECK_RET_GOTO_ERRNO_P(desc,here,-SYSTEM_DLSYM_FAILED,"--dlsym failed!\n");

				//printf("-----ex-module:%s\n",desc->module);
				ret = plugin_enable(desc->module, cli_enabled);
				CHECK_RET_GOTO_ERRNO_P(ret == 0,here,PLUGIN_ENABLE_FAILED,"plugin_enable failed!\n");

				ret = plugin_add(desc,handle);
				CHECK_RET_CONTINUE(ret != 0);
here:
				dlclose(handle);
			}
		}
	}
	wow_stringlist_free(&file_list);
	
	/***********对使能plugin初始化**************/
	size = wow_slist_size(gpt_plugins_list);
	for(i = 0; i < size; i++){
		PluginInfo_T *plugin = wow_slist_peek_by_index(gpt_plugins_list,i);
		CHECK_RET_CONTINUE(plugin && plugin->desc && plugin->desc->init);
		
		ret = plugin->desc->init();
		CHECK_RET_CONTINUE(ret == 0);

		plugin->active = 1;
	
	}

	wow_str_split_free(cli_enabled);
	
	return 0;
}

static void plugin_list_destroy(void* data)
{
	PluginInfo_T *plugin  = (PluginInfo_T*)data;
	if(plugin->active ==1 && plugin->desc && plugin->desc->exit ){
		plugin->desc->exit();
		if(plugin->handle){
			dlclose(plugin->handle);
		}
	}
	
	FREE(data);
}
static bool plugin_list_match(const void *data, const void *match_data)
{
	PluginInfo_T *plugin  = (PluginInfo_T*)data;
	CHECK_RET_VAL(plugin && plugin->desc && plugin->desc->module,false);

	return (strcmp(plugin->desc->module, (const char*)match_data) == 0)?true:false;
}


__EX_API__ void wow_plugin_exit(void)
{
	wow_slist_clear(gpt_plugins_list,plugin_list_destroy);
}

__EX_API__ void* wow_plugin_get_user(const char* pcModule)
{
	CHECK_RET_VAL_ERRNO_P(pcModule, -PARAM_INPUT_DATA_IS_NULL, "param input data invalid!\n");

	PluginInfo_T *plugin  = (PluginInfo_T*)wow_slist_peek_by_match(gpt_plugins_list,plugin_list_match,pcModule);
	CHECK_RET_VAL_ERRNO_P(plugin,-PLUGIN_FIND_FAILED,"find module:%s failed!\n",pcModule);
	
	return plugin->desc->user;
}


static int plugin_init(void)
{
	gpt_plugins_list = wow_slist_create();
	
	return 0;
}

static void plugin_exit(void)
{
    wow_slist_destroy(&gpt_plugins_list,NULL);
}

__IN_API__ int _wow_plugin_env_init(void)
{
    return wow_singleton_instance(SINGLETON_BASE_MOD_SHELL, plugin_init, plugin_exit);
}

DECLARE_PLUGIN_BUILDIN(in_plugin_wow_bug,WOW_PLUGIN_PRIORITY_HIGH,NULL, NULL)

