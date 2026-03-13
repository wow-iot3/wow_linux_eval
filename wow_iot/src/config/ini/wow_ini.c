#include <stdarg.h>

#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"
#include "file/wow_hfile.h"
#include "config/wow_ini.h"

#include "iniparser.h"
#include "wow_iot_errno.h"


/* brief    加载ini配置文件
 * param :  file:文件路径
 * return:  成功返回ini操作符 失败返回-1
 */
Ini_PT wow_ini_init(const char* pFile)
{
	CHECK_RET_VAL_ERRNO_P(pFile, -PARAM_INPUT_DATA_IS_NULL, "param input data invalid!\n");
	
    dictionary *ini = iniparser_load(pFile);
	CHECK_RET_VAL_ERRNO_P(ini, -INI_PARSER_LOAD_ERROR, "iniparser_load %s failed!\n", pFile);

    return (Ini_PT)ini;
}



/* brief    退出ini配置文件
 * param :  ptIni:ini操作符
 * return:  无
 */
void wow_ini_exit(Ini_PT* ptIni)
{
	CHECK_RET_VOID(ptIni && *ptIni);
	dictionary *ini = (dictionary *)*ptIni;
	
    iniparser_freedict(ini);
	*ptIni = NULL;
}

/* brief    获取key对应的string格式内容
 * param :  ptIni:ini操作符
 * param :  pcKey:key值
 * param :  pccVal:value值缓存
 * return:  成功返回0 失败返回< 0 
 */
int wow_ini_get_string(Ini_PT ptIni,const char * pcKey,char** pccVal)
{
	dictionary *ini = (dictionary *)ptIni;
	CHECK_RET_VAL_P(ini, -PARAM_INPUT_STRUCT_IS_NULL, "param input strcut invalid!\n");
	CHECK_RET_VAL_P(pcKey && pccVal, -PARAM_INPUT_DATA_IS_NULL, "param input data invalid!\n");

	char* value = iniparser_getstring(ini, pcKey, NULL);
	CHECK_RET_VAL_P(value, -INI_PARSER_GETSTRING_ERROR, "can not find %s value!\n",pcKey);

	*pccVal = value;
	
	return 0;
}

/* brief    获取key对应的int格式内容
 * param :  ptIni:ini操作符
 * param :  pcKey:key值
 * param :  pnVal:value值缓存
 * return:  成功返回0 失败返回< 0 
 */
int wow_ini_get_int(Ini_PT ptIni,const char * pcKey,int* pnVal)
{
	dictionary *ini = (dictionary *)ptIni;
	CHECK_RET_VAL_P(ini , -PARAM_INPUT_STRUCT_IS_NULL, "param input struct invalid!\n");
	CHECK_RET_VAL_P(pcKey && pnVal, -PARAM_INPUT_DATA_IS_NULL, "param input data invalid!\n");

    int value = iniparser_getint(ini, pcKey, -13765);
	CHECK_RET_VAL_P(value != -13765, -INI_PARSER_GETINT_ERROR, "can not find %s value!\n",pcKey);

	*pnVal = value;
	return 0;
}

/* brief    获取key对应的double格式内容
 * param :  ptIni:ini操作符
 * param :  pcKey:key值
 * param :  pdVal:value值缓存
 * return:  成功返回0 失败返回< 0 
 */
int wow_ini_get_double(Ini_PT ptIni,const char * pcKey,double* pdVal)
{
	dictionary *ini = (dictionary *)ptIni;
	CHECK_RET_VAL_P(ini , -PARAM_INPUT_STRUCT_IS_NULL, "param input struct invalid!\n");
	CHECK_RET_VAL_P(pcKey && pdVal, -PARAM_INPUT_DATA_IS_NULL, "param input data invalid!\n");

    double value = iniparser_getdouble(ini, pcKey, -137.65);
	CHECK_RET_VAL_P(value != -137.65, -INI_PARSER_GETDOUBLE_ERROR, "can not find %s value!\n",pcKey);

	*pdVal = value;
	return 0;
}

/* brief    获取key对应的bool格式内容
 * param :  ptIni:ini操作符
 * param :  pcKey:key值
 * param :  pbVal:value值缓存
 * return:  成功返回0 失败返回< 0 
 */
int wow_ini_get_bool(Ini_PT ptIni,const char * pcKey,bool* pbVal)
{
	dictionary *ini = (dictionary *)ptIni;
	CHECK_RET_VAL_P(ini , -PARAM_INPUT_STRUCT_IS_NULL, "param input struct invalid!\n");
	CHECK_RET_VAL_P(pcKey && pbVal, -PARAM_INPUT_DATA_IS_NULL, "param input data invalid!\n");

				
    int value = iniparser_getboolean(ini, pcKey, -1);
	CHECK_RET_VAL_P(value != -1, -INI_PARSER_GETBOOL_ERROR, "can not find %s value!\n",pcKey);

	*pbVal = (bool)value;
	return 0;
}

/* brief    设置key-value内容
 * param :  ptIni:ini操作符
 * param :  pcKey:key值
 * param :  pcVal:value值缓存
 * return:  成功返回0 失败返回< 0 
 */
int wow_ini_set_string(Ini_PT ptIni,const char * pcKey,const char* pcVal)
{
	dictionary *ini = (dictionary *)ptIni;
	CHECK_RET_VAL_P(ini , -PARAM_INPUT_STRUCT_IS_NULL, "param input struct invalid!\n");
	CHECK_RET_VAL_P(pcKey, -PARAM_INPUT_DATA_IS_NULL, "param input data invalid!\n");


    return iniparser_set(ini, pcKey, pcVal);
}

/* brief    获取key-value内容
 * param :  ptIni:ini操作符
 * param :  pcKey:key值
 * return:  成功返回0 失败返回< 0 
 */
int wow_ini_del(Ini_PT ptIni, const char *pcKey)
{
	dictionary *ini = (dictionary *)ptIni;
	CHECK_RET_VAL_P(ini , -PARAM_INPUT_STRUCT_IS_NULL, "param input struct invalid!\n");
	CHECK_RET_VAL_P(pcKey, -PARAM_INPUT_DATA_IS_NULL, "param input data invalid!\n");

    iniparser_unset(ini, pcKey);
	return 0;
}

/* brief    报文init文件
 * param :  ptIni:ini操作符
 * param :  pcFile:存储文件名称
 * return:  成功返回0 失败返回< 0 
 */
int wow_ini_save(Ini_PT ptIni,const char* pcFile)
{
	dictionary *ini = (dictionary *)ptIni;
	CHECK_RET_VAL_P(ini , -PARAM_INPUT_STRUCT_IS_NULL, "param input struct invalid!\n");
	CHECK_RET_VAL_P(pcFile, -PARAM_INPUT_DATA_IS_NULL, "param input data invalid!\n");

    FILE *f = fopen(pcFile, "w+");
	CHECK_RET_VAL_P_A(f,-SYSTEM_FOPEN_FAILED,"fopen %s false!\n",pcFile);

    iniparser_dump_ini(ini, f);
	
    fclose(f);
	
    return 0;
}

