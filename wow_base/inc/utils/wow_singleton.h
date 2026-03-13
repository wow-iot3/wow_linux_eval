#ifndef __WOW_BASE_UTILS_WOW_SINGLETON_H_
#define __WOW_BASE_UTILS_WOW_SINGLETON_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum{
	SINGLETON_TYPE_BASE  = 0x00,
	SINGLETON_TYPE_HAL   = 0x30,
	SINGLETON_TYPE_IOT   = 0x60,
	SINGLETON_TYPE_APP   = 0x90,
    SINGLETON_TYPE_MAXN  = 0xFF,
}SingletonType_E;

/*约束初始化函数 成功返回0，失败返回-1*/
typedef int  (*singleton_init_func_t)(void);
typedef void (*singleton_exit_func_t)(void);

/*brief    执行实例      模块未被初始化，执行fInit函数，执行成功标识已初始化
 *param ： eIndex    ：模块执行标识
 *param ： fInit     ：初始化函数指针	---不可为NULL
 *param ： fExit     ：退出函数指针         ---可为NULL
 *return： 成功返回0 失败返回<0
 */
int  wow_singleton_instance(SingletonType_E eIndex, singleton_init_func_t fInit, singleton_exit_func_t fExit);

#ifdef __cplusplus
}
#endif

#endif

