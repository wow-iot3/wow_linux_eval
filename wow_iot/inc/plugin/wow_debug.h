#ifndef __WOW_IOT_DEBUG_WOW_DEBUG_H_
#define __WOW_IOT_DEBUG_WOW_DEBUG_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum{
	DEBUG_LEVEL_DISABLE	= 0,
	DEBUG_LEVEL_ERROR 	= 1,
	DEBUG_LEVEL_WARN	= 2,
	DEBUG_LEVEL_INFO	= 3,
	DEBUG_LEVEL_DEBUG 	= 4
}DebugLevel_E;


#define DEBUG_DEFAULT_LEVEL		DEBUG_LEVEL_DEBUG


//建议用户标注APP-进程名称—模块名称样式
void wow_debug(const char *mod,DebugLevel_E lev,const char *format, ...);
#if (DEBUG_DEFAULT_LEVEL >= DEBUG_LEVEL_DEBUG)
	#define ModDebug_D(mod,...)   wow_debug(mod,DEBUG_LEVEL_DEBUG,__VA_ARGS__);
#else
	#define ModDebug_D(mod,...)
#endif

#if (DEBUG_DEFAULT_LEVEL >= DEBUG_LEVEL_INFO)
	#define ModDebug_I(mod,...)   wow_debug(mod,DEBUG_LEVEL_INFO,__VA_ARGS__);
#else
	#define ModDebug_I(mod,...)
#endif

#if (DEBUG_DEFAULT_LEVEL >= DEBUG_LEVEL_WARN)
	#define ModDebug_W(mod,...)   wow_debug(mod,DEBUG_LEVEL_WARN,__VA_ARGS__);
#else
	#define ModDebug_W(mod,...)
#endif

#if (DEBUG_DEFAULT_LEVEL >= DEBUG_LEVEL_ERROR)
	#define ModDebug_E(mod,...)   wow_debug(mod,DEBUG_LEVEL_ERROR,__VA_ARGS__);
#else
	#define ModDebug_E(mod,...)
#endif

/*brief    设置debug状态
 *param ： pcMod   ：模块名称
 *param ： bEnable ：是否使能
 *return： 无
 */
void wow_debug_setStatus(const char* pcMod,bool  bEnable);

/*brief    获取debug状态
 *param ： pcMod   ：模块名称
 *return： 1-此模块已使能 0-此模块未使能 -1-无效模块
 */
int wow_debug_getStatus(const char* pcMod);


#ifdef __cplusplus
}
#endif

#endif
