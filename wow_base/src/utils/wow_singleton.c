#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "prefix/wow_errno.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_check.h"

#include "system/wow_atomic.h"
#include "utils/wow_singleton.h"

#include "wow_base_internal.h"

typedef struct{
    singleton_exit_func_t  exit;	 
    atomic_t 	           instance;    				 
}__data_aligned__ Singleton_T;

static bool gb_singletonInited = false;
static Singleton_T gst_singletons[SINGLETON_TYPE_MAXN] = {{0}};

__IN_API__ int _wow_singleton_init(void)
{
	if(gb_singletonInited == false){
		memset(&gst_singletons, 0, sizeof(gst_singletons));	
		gb_singletonInited = true;
	}

	return WOW_SUCCESS;
}

__IN_API__ int _wow_singleton_exit(void)
{
    size_t i = SINGLETON_TYPE_MAXN;
    while (i--)
    {
        if (gst_singletons[i].exit){
            int instance = atomic_get(&gst_singletons[i].instance);
            if (instance && instance == 1 && gst_singletons[i].exit) {	
                gst_singletons[i].exit();
            }
        }
    }
    memset(&gst_singletons, 0, sizeof(gst_singletons));

	return WOW_SUCCESS;
}


/*brief    执行实例      模块未被初始化，执行fInit函数，执行成功标识已初始化
 *param ： eIndex    ：模块执行标识
 *param ： fInit     ：初始化函数指针	---不可为NULL
 *param ： fExit     ：退出函数指针         ---可为NULL
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_singleton_instance(SingletonType_E eIndex, singleton_init_func_t fInit, singleton_exit_func_t fExit)
{
    CHECK_RET_VAL_P(eIndex < SINGLETON_TYPE_MAXN, -PARAM_INPUT_ARG_INVALID, "param input arg invalid!\n");
    CHECK_RET_VAL_P(fInit, -PARAM_INPUT_FUNC_IS_NULL, "param input func invalid!\n");

	 
    bool instance = atomic_compare_and_swap_bool(&gst_singletons[eIndex].instance, 0, 1);
	CHECK_RET_VAL(instance && gst_singletons[eIndex].instance == 1,0);

    CHECK_RET_VAL_P(fInit() == 0, -WOW_CB_FAILED,"eIndex(%d) init failed!\n",eIndex);

	atomic_set(&gst_singletons[eIndex].instance,1);
	gst_singletons[eIndex].exit = fExit;

    return WOW_SUCCESS;
}
