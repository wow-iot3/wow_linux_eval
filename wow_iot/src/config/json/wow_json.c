#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"

#include "file/wow_hfile.h"
#include "file/wow_file.h"
#include "config/wow_json.h"
#include "wow_iot_errno.h"

#include "cJSON.h"

static cJSON* json_item_create(JsonType_E type,void* value)
{
	CHECK_RET_VAL_P(type < JSON_TYPE_MAX,NULL,"param input arg invalid!\n");
	switch (type){
		case JSON_TYPE_BOOL:
			return cJSON_CreateBool(*(bool*)value);	
		case JSON_TYPE_INT8:
			return cJSON_CreateNumber(*(char*)value);
		case JSON_TYPE_INT16:
			return cJSON_CreateNumber(*(short*)value);
		case JSON_TYPE_INT32:
			return cJSON_CreateNumber(*(int*)value);
		case JSON_TYPE_FLOAT:	
			return cJSON_CreateNumber(*(float*)value);
		case JSON_TYPE_INT64:
			return cJSON_CreateNumber(*(long long*)value);
		case JSON_TYPE_DOUBLE:
			return cJSON_CreateNumber(*(double*)value);
		case JSON_TYPE_STRING:
			return cJSON_CreateString((char*)value);
		case JSON_TYPE_ARRARY:
		case JSON_TYPE_OBJECT:
			return (cJSON*)value;
		default:
			return NULL;
	}
}

static void* json_item_value(cJSON* json)
{
	if(cJSON_IsTrue(json)){
		json->valuedouble = 1;
	    return (void*)&json->valuedouble;
	}
	else if(cJSON_IsFalse(json)){
		json->valuedouble = 0;
		return (void*)&json->valuedouble;
	}
	else if(cJSON_IsNumber(json)){
		return (void*)&json->valuedouble;
	}
	else if(cJSON_IsString(json)){
		return (void*)json->valuestring;
	}
    else if(cJSON_IsObject(json)){
		return (void*)json;
    }
    else if(cJSON_IsArray(json)){
		return (void*)json;
    }else{
		return NULL;
	}
}

/*brief    创建json格式对象
 *return： 成功返回对象操作符 失败返回NULL
 */
Json_PT wow_json_object_new()
{
	return (Json_PT)cJSON_CreateObject();
}

/*brief    向json对象中插入数据，存在则替换
 *param ： ptObject:对象操作符
 *param ： pcKey   :插入对象键
 *param ： eType  :插入对象键类型
 *param ： pValue :插入对象键值
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_json_object_add(Json_PT ptObject, const char* pcKey,JsonType_E eType,void* pValue)
{
	cJSON *object = (cJSON *)ptObject;
	CHECK_RET_VAL_P(object,-PARAM_INPUT_STRUCT_IS_NULL, "param input struct invalid!\n");
	CHECK_RET_VAL_P(pcKey && pValue,-PARAM_INPUT_DATA_IS_NULL, "param input data invalid!\n");
	CHECK_RET_VAL_P(cJSON_IsObject(object) ,-JSON_IS_NOT_OBJECT, "param input invalid!\n");

	cJSON *item = json_item_create(eType,pValue);
	CHECK_RET_VAL_P(item ,-JSON_CREATE_ERROR, "json_item_create failed!\n");
		
	if(cJSON_HasObjectItem(object, pcKey)){
		cJSON_ReplaceItemInObject(object, pcKey, item);
	}else{
		cJSON_AddItemToObject(object, pcKey, item);
	}

	return 0;
}

/*brief    删除json对象
 *param ： ptObject:对象操作符
 *param ： pcKey   :获取对象键
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_json_object_del(Json_PT ptObject, const char* pcKey)
{
	cJSON *object = (cJSON *)ptObject;
	CHECK_RET_VAL_P(object,-PARAM_INPUT_STRUCT_IS_NULL, "param input struct invalid!\n");
	CHECK_RET_VAL_P(pcKey,-PARAM_INPUT_DATA_IS_NULL, "param input data invalid!\n");

	if(cJSON_HasObjectItem(object, pcKey)){
		 cJSON_DeleteItemFromObject(object, pcKey);
	}
	return 0;
}

/*brief    获取json对象中键值
 *param ： ptObject:对象操作符
 *param ： pcKey   :获取对象键
 *param ： pValue :获取对象键存放信息
 *return： 成功返回获取值 失败返回NULL
 */
__EX_API__ void* wow_json_object_value(Json_PT ptObject, const char* pcKey)
{
	cJSON *object = (cJSON *)ptObject;
	CHECK_RET_VAL_ERRNO_P(object,-PARAM_INPUT_STRUCT_IS_NULL, "param input struct invalid!\n");
	CHECK_RET_VAL_ERRNO_P(pcKey,-PARAM_INPUT_DATA_IS_NULL, "param input data invalid!\n");
	CHECK_RET_VAL(cJSON_HasObjectItem(object, pcKey),NULL);

	cJSON *item = cJSON_GetObjectItem(object,pcKey);
	CHECK_RET_VAL_P(item,NULL, "cJSON_GetArrayItem failed!\n");
	
	return json_item_value(item);
}

/*brief    获取json对象中键值 
 *param ： ptObject:create创建返回的操作符
 *param ： pVarfmt:占位符 s对应字符串和i整数
 *return： 成功返回获取值 失败返回NULL
 */
__EX_API__ void*  wow_json_object_value_ex(Json_PT ptObject,const char *pVarfmt,...)
{
	cJSON *json = (cJSON *)ptObject;

	CHECK_RET_VAL_ERRNO_P(json,-PARAM_INPUT_STRUCT_IS_NULL, "param input struct invalid!\n");
	CHECK_RET_VAL_ERRNO_P(pVarfmt,-PARAM_INPUT_DATA_IS_NULL, "param input data invalid!\n");

	va_list vl;
	va_start(vl, pVarfmt);
	const char *type = pVarfmt;
	while (*type) {
		switch (*type++) {
		case 's': json = cJSON_GetObjectItem(json, va_arg(vl, char*)); break;
		case 'i': json = cJSON_GetArrayItem(json, va_arg(vl, int)); break;
		default: json = NULL;break;
		}
		CHECK_RET_BREAK(json);
	}
	va_end(vl);
	
	return json_item_value(json);

}

/*brief    创建json格式数组
 *return： 成功返回数组操作符 失败返回NULL
 */
Json_PT wow_json_arrary_new()
{
	return (Json_PT)cJSON_CreateArray();
}


/*brief    获取json数组内容个数
 *param ： ptObject:数组操作符
 *return： 成功返回个数失败返回-1
 */
__EX_API__ int wow_json_array_size(Json_PT ptArray)
{
	cJSON *array = (cJSON *)ptArray;
	CHECK_RET_VAL_P(array,-PARAM_INPUT_STRUCT_IS_NULL, "param input struct invalid!\n");
	CHECK_RET_VAL_P(cJSON_IsArray(array) ,-JSON_IS_NOT_ARRAY, "param input data invalid!\n");
	
	return cJSON_GetArraySize(array);
}

/*brief    向json数组中追加数据
 *param ： ptObject:数组操作符
 *param ： eType  :插入对象键类型
 *param ： pValue :插入对象键值
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_json_array_add(Json_PT ptArray,JsonType_E eType,void* pValue)
{
	cJSON *array = (cJSON *)ptArray;
	CHECK_RET_VAL_P(array,-PARAM_INPUT_STRUCT_IS_NULL, "param input struct invalid!\n");
	CHECK_RET_VAL_P(pValue,-PARAM_INPUT_DATA_IS_NULL, "param input data invalid!\n");
	CHECK_RET_VAL_P(cJSON_IsArray(array) ,-JSON_IS_NOT_ARRAY, "param input data invalid!\n");

	cJSON *item = json_item_create(eType,pValue);
	CHECK_RET_VAL_P(item ,-JSON_CREATE_ERROR, "json_item_create failed!\n");

	cJSON_AddItemToArray(array, item);
	
	return 0;
}


/*brief    删除json数组内容
 *param ： ptObject:数组操作符
 *param ： nIndex :内容index标识
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_json_array_del(Json_PT ptArray, uint32_t nIndex)
{
	cJSON *array = (cJSON *)ptArray;
	CHECK_RET_VAL_P(array,-PARAM_INPUT_STRUCT_IS_NULL, "param input struct invalid!\n");
	CHECK_RET_VAL_P(cJSON_IsArray(array) ,-JSON_IS_NOT_ARRAY, "param input data invalid!\n");
	
	int size = cJSON_GetArraySize(array);
	CHECK_RET_VAL_P(size > 0 && nIndex < size,-JSON_GET_ARRARY_SIZE_ERROR, "param input nIndex invalid!\n");

	cJSON_DeleteItemFromArray(array, nIndex);

	return 0;
}

/*brief    获取json数组中键值
 *param ： ptObject:数组操作符
 *param ： nIndex :内容index标识
 *param ： pValue :获取对象键存放信息
 *return： 成功返回获取值 失败返回NULL
 */
__EX_API__ void* wow_json_array_value(Json_PT ptArray, uint32_t nIndex)
{
	cJSON *array = (cJSON *)ptArray;
	CHECK_RET_VAL_ERRNO_P(array,-PARAM_INPUT_STRUCT_IS_NULL, "param input struct invalid!\n");
	CHECK_RET_VAL_ERRNO_P(cJSON_IsArray(array) ,-JSON_IS_NOT_ARRAY, "param input data invalid!\n");
	
	int size = cJSON_GetArraySize(array);
	CHECK_RET_VAL_P(size > 0 && nIndex < size,NULL, "param input nIndex invalid!\n");

	cJSON *item = cJSON_GetArrayItem(array, nIndex);
	CHECK_RET_VAL_P(item,NULL, "cJSON_GetArrayItem failed!\n");

	return json_item_value(item);
}




/*brief    解析json文件内容
 *param ： pcFile:json文件路径
 *return： 成功返回操作对象 失败返回NULL
 */
__EX_API__ Json_PT wow_json_init_by_file(const char* pcFile)
{
	CHECK_RET_VAL_P(pcFile,NULL,"param input data invalid!\n");

	int ret = 0;
	int size = 0;

	ret = wow_file_size(pcFile,&size);
	CHECK_RET_VAL_P(ret == 0 && size > 0,NULL,"file size invaild!\n");

	char* content = (char*)MALLOC(size + 1);
	CHECK_RET_VAL_P(content, NULL,"malloc size(%d) failed!\n",(size + 1));
	
	File_T* file = wow_hfile_open(FILE_FIO,pcFile,FILE_RDWR);
	CHECK_RET_GOTO_P(file,out,"wow_hfile_open failed!\n");

	ret = wow_hfile_read(file,content,size);
	wow_hfile_close(file);
	CHECK_RET_GOTO_P(ret ==size ,out,"wow_hfile_read failed!\n");

	content[ret] = '\0';
	
	cJSON* json = cJSON_Parse(content);
	CHECK_RET_GOTO_P(json ,out,"cJSON_Parse failed!\n");
	
	FREE(content);
	return (void*)json;

out:
	FREE(content);
	return NULL;
}

/*brief    解析json字段内容
 *param ： pcStr:json字段
 *return： 返回操作对象 失败返回NULL
 */
__EX_API__ Json_PT wow_json_init_by_str(const char* pcStr)
{
	CHECK_RET_VAL_P(pcStr,NULL,"param input data invalid!\n");
	
	cJSON* json = cJSON_Parse(pcStr);
	CHECK_RET_VAL_P(json ,NULL,"cJSON_Parse failed!\n");


	return (void*)json;
}

/*brief    删除json对象内容
 *param ： ptItem:json对象
 *return： 无
 */
__EX_API__ void wow_json_destory(Json_PT* ptItem)
{
    cJSON *json = (cJSON *)*ptItem;
	CHECK_RET_VOID(json);

	cJSON_Delete(json);
	*ptItem = NULL;
}


/*brief    获取json对象内容 使用后必须free此字段
 *param ： ptItem :json对象
 *return： 成功返回内容 失败返回NULL
 */
__EX_API__ char* wow_json_dump_new(Json_PT ptItem)
{
    cJSON *json = (cJSON *)ptItem;
	CHECK_RET_VAL(json,NULL);
	
    return  cJSON_Print(json);
}
__EX_API__ void wow_json_dump_free(char* pData)
{
	CHECK_RET_VOID(pData);

	cJSON_free(pData);
}

/*brief    保存json对象内容
 *param ： ptItem:json对象
 *param ： pFile:保存json文件路径
 *return： 返回操作对象 失败返回<0
 */
__EX_API__ int wow_json_save_file(Json_PT ptItem,const char* pFile)
{
    cJSON *json = (cJSON *)ptItem;
	CHECK_RET_VAL_P(json,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	
    char *data = cJSON_Print(json);
	CHECK_RET_VAL_P(data,-JSON_PRINT_ERROR,"cJSON_Print failed!\n");

	
	wow_file_remove(pFile);
	File_T* file = wow_hfile_open(FILE_FIO,pFile,FILE_CREATE|FILE_RDWR);
	CHECK_RET_GOTO_P(file,out,"wow_hfile_open false!\n");

	wow_hfile_write(file,data,strlen(data));
	wow_hfile_close(file);

	cJSON_free(data);
	return 0;
out:
	free(data);
	return -FILE_OPEN_FAILED;
}




