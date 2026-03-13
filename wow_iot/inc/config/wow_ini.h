#ifndef __WOW_IOT_CONFIG_WOW_INI_H_
#define __WOW_IOT_CONFIG_WOW_INI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

typedef void* Ini_PT;

/* brief    加载ini配置文件
 * param :  pcFile:文件路径
 * return:  成功返回ini操作符 失败返回-1
 */
Ini_PT wow_ini_init(const char* pcFile);


/* brief    退出ini配置文件
 * param :  ptIni:ini操作符
 * return:  无
 */
void wow_ini_exit(Ini_PT* ptIni);

/* brief    获取key对应的string格式内容
 * param :  ptIni:ini操作符
 * param :  pcKey:key值
 * param :  pccVal:value值缓存
 * return:  成功返回0 失败返回-1
 */
int wow_ini_get_string(Ini_PT ptIni,const char * pcKey,char** pccVal);

/* brief    获取key对应的int格式内容
 * param :  ptIni:ini操作符
 * param :  pcKey:key值
 * param :  pnVal:value值缓存
 * return:  成功返回0 失败返回-1
 */
int wow_ini_get_int(Ini_PT ptIni,const char * pcKey,int* pnVal);

/* brief    获取key对应的double格式内容
 * param :  ptIni:ini操作符
 * param :  pcKey:key值
 * param :  pVal:value值缓存
 * return:  成功返回0 失败返回-1
 */
int wow_ini_get_double(Ini_PT ptIni,const char * pcKey,double* pVal);

/* brief    获取key对应的bool格式内容
 * param :  ptIni:ini操作符
 * param :  pcKey:key值
 * param :  pVal:value值缓存
 * return:  成功返回0 失败返回-1
 */
int wow_ini_get_bool(Ini_PT ptIni,const char * pcKey,bool* pVal);

/* brief    设置key-value内容
 * param :  ptIni:ini操作符
 * param :  pcKey:key值
 * param :  pVal:value值缓存
 * return:  成功返回0 失败返回-1
 */
int wow_ini_set_string(Ini_PT ptIni,const char * pcKey,const char* pVal);

/* brief    获取key-value内容
 * param :  ptIni:ini操作符
 * param :  pcKey:key值
 * return:  成功返回0 失败返回-1
 */
int  wow_ini_del(Ini_PT ptIni, const char *pcKey);

/* brief    报文init文件
 * param :  ptIni:ini操作符
 * param :  pcFile:存储文件名称
 * return:  成功返回0 失败返回-1
 */
int wow_ini_save(Ini_PT ptIni,const char* pcFile);

#ifdef __cplusplus
}
#endif

#endif
