#ifndef __WOW_IOT_CONFIG_WOW_JSON_H_
#define __WOW_IOT_CONFIG_WOW_JSON_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum{
	JSON_TYPE_BOOL,
	JSON_TYPE_INT8,
	JSON_TYPE_INT16,
	JSON_TYPE_INT32,
	JSON_TYPE_INT64,
	JSON_TYPE_FLOAT,
	JSON_TYPE_DOUBLE,		
	JSON_TYPE_STRING,			
	JSON_TYPE_ARRARY,			
	JSON_TYPE_OBJECT,
	JSON_TYPE_MAX
}JsonType_E;

typedef void* Json_PT;

/*brief    解析json文件内容
 *param ： pcFile:json文件路径
 *return： 成功返回操作对象 失败返回NULL
 */
Json_PT wow_json_init_by_file(const char* pcFile);


/*brief    解析json字段内容
 *param ： pcStr:json字段
 *return： 成功返回操作对象 失败返回NULL
 */
Json_PT wow_json_init_by_str(const char* pcStr);

/*brief    删除json对象内容
 *param ： ptItem:json对象
 *return： 无
 */
void wow_json_destory(Json_PT* ptItem);

/************************************json对象操作*****************************/
/*brief    创建json格式对象
 *return： 成功返回对象操作符 失败返回NULL
 */
Json_PT wow_json_object_new();

/*brief    向json对象中插入数据，存在则替换
 *param ： ptObject:对象操作符
 *param ： pcKey   :插入对象键
 *param ： eType  :插入对象键类型
 *param ： pValue :插入对象键值
 *return： 成功返回0 失败返回< 0
 */
int wow_json_object_add(Json_PT ptObject, const char* pcKey,JsonType_E eType,void* pValue);

/*brief    删除json对象
 *param ： ptObject:对象操作符
 *param ： pcKey   :获取对象键
 *return： 成功返回0 失败返回< 0
 */
int wow_json_object_del(Json_PT ptObject, const char* pcKey);

/*brief    获取json对象中键值
 *param ： ptObject:对象操作符
 *param ： pcKey   :获取对象键
 *return： 成功返回获取值 失败返回NULL
 注：浮点型使用*(double*)样式 整数使用(int)*(double*)样式 字符型使用(char*)
 */
void* wow_json_object_value(Json_PT ptObject, const char* pcKey);

/*brief    获取json对象中键值 
 *param ： ptObject:create创建返回的操作符
 *param ： pVarfmt:占位符 s对应字符串和i整数
 *return： 成功返回获取值 失败返回NULL
 */
void*  wow_json_object_value_ex(Json_PT ptObject,const char *pVarfmt,...);


/************************************json数组操作*****************************/
/*brief    创建json格式数组
 *return： 成功返回数组操作符 失败返回NULL
 */
Json_PT wow_json_arrary_new();

/*brief    向json数组中追加数据
 *param ： ptObject:数组操作符
 *param ： eType  :插入对象键类型
 *param ： pValue :插入对象键值
 *return： 成功返回0 失败返回< 0
 */
int wow_json_array_add(Json_PT ptArray,JsonType_E eType,void* pValue);

/*brief    删除json数组内容
 *param ： ptObject:数组操作符
 *param ： nIndex :内容index标识
 *return： 成功返回0 失败返回< 0
 */
int wow_json_array_del(Json_PT ptArray, uint32_t nIndex);

/*brief    获取json数组内容个数
 *param ： ptObject:数组操作符
 *return： 成功返回个数失败返回< 0
 */
int wow_json_array_size(Json_PT ptArray);

/*brief    获取json数组中键值
 *param ： ptObject:数组操作符
 *param ： nIndex :内容index标识
 *param ： pValue :获取对象键存放信息
 *return： 成功返回获取值 失败返回NULL
 */
void* wow_json_array_value(Json_PT ptArray, uint32_t nIndex);

/*brief    获取json对象内容 
 *param ： ptItem :json对象
 *return： 成功返回内容 失败返回NULL
 注：必须使用释放内存防止内存泄漏
 */
char* wow_json_dump_new(Json_PT ptItem);
void wow_json_dump_free(char* pData);

/*brief    保存json对象内容
 *param ： ptItem:json对象
 *param ： pFile:保存json文件路径
 *return： 返回操作对象 失败返回< 0
 */
int wow_json_save_file(Json_PT ptItem,const char* pFile);


/**************************json-struct转换*********************************/
#define wow_json_object_JSON_TYPE_INT8_value(src,dest) \
		dest = (char)*(double*)src;
#define wow_json_object_JSON_TYPE_INT16_value(src,dest) \
		dest = (short)*(double*)src;
#define wow_json_object_JSON_TYPE_INT32_value(src,dest) \
		dest = (int)*(double*)src;
#define wow_json_object_JSON_TYPE_FLOAT_value(src,dest) \
		dest = (float)*(double*)src;
#define wow_json_object_JSON_TYPE_DOUBLE_value(src,dest) \
		dest = (float)*(double*)src;
#define wow_json_object_JSON_TYPE_STRING_value(src,dest) \
		strncpy(dest, (char*)src,strlen((char*)src));

#define wow_json_object_add_array(tjson,item,type,value, num) \
	int idx_##item = 0 ; \
	Json_PT arr_##item = wow_json_arrary_new(); \
	for(idx_##item = 0 ; idx_##item < num; idx_##item++){ \
			wow_json_array_add(arr_##item,type,value[idx_##item]); \
	}\
	wow_json_object_add(tjson,#item,JSON_TYPE_ARRARY,arr_##item);\

#define wow_json_object_value_array(fjson,item,type,value)\
		int idx_##item = 0 ; \
		void* value_##item; \
		Json_PT arr_##item = wow_json_object_value(fjson,##item); \
		for(idx_##item = 0 ; idx_##item < wow_json_array_size(arr_##item); idx_##item++){ \
				value_##item = wow_json_array_value(arr_##item,idx_##item); \
				wow_json_object_##type##_value(value_##item,value[idx_##item]); \
		}

#ifdef __cplusplus
}
#endif

#endif
