#ifndef __WOW_PROTO_DATA_H_
#define __WOW_PROTO_DATA_H_

#include <string.h>

typedef enum{
	DATA_BOOL  = 0,
	DATA_UINT_8,
	DATA_SINT_8,
	DATA_UINT_16,
	DATA_SINT_16,
	DATA_UINT_32,
	DATA_SINT_32,
	DATA_UINT_64,
	DATA_SINT_64,
	DATA_FLOAT,
	DATA_DOUBLE,
	DATA_FUNC,
	DATA_MAX,
}DevDataType_E;
	
typedef union{
	int8_t		s8;
	uint8_t 	u8;
	int16_t 	s16;
	uint16_t	u16;
	int32_t 	s32;
	uint32_t	u32;
	int64_t 	s64;
	uint64_t	u64;	
	float		f;
	double		d;
	char		data[8];
}DevValue_U;


typedef struct{
	DevDataType_E type;
	DevValue_U    value;
}DevValue_T;

/*brief    获取数据类型
 *param ： pcTypeStr: 数据类型字符串
 *return： 成功返回数据类型 失败返回DATA_MAX
 */
DevDataType_E wow_data_get_type(const char* pcTypeStr);


/*brief    获取数据类型长度
 *param ： eType: 数据类型
 *return： 成功返回数据类型长度 失败返回<0
 */
int wow_data_get_len(DevDataType_E eType);

/*brief    数据字符串转数据结构存储
 *param ： eType: 数据类型
 *param ： pcStr: 数据字符串
 *param ： ptVal: 数据处理储存指针
 *return： 成功返回0 失败返回<0
 */
int wow_data_str_to_val(DevDataType_E eType,const char* pcStr,DevValue_U* ptVal);

/*brief    数据结构存储转字符串
 *param ： eType: 数据类型
 *param ： ptVal: 数据处理指针
 *param ： pcStr: 数据字符串储存指针
 *return： 成功返回0 失败返回<0
 */
int wow_data_val_to_str(DevDataType_E eType,DevValue_U* ptVal,char* pcStr);

/*brief    按照数据类型完成数组到指定类型的转换
 *param ： eType: 数据类型
 *param ： pu8Data: 数据储存指针
 *param ： nBulk: 数据长度
 *param ： pDest: 存储长度
 *return： 成功返回0 失败返回<0
 */
int wow_data_byte_to_other(DevDataType_E eType, uint8_t* pu8Data ,uint32_t nBulk,void *pDest);
int wow_data_byte_to_other_offset(DevDataType_E eType, uint8_t* pu8Data,uint32_t nBulk,void *pDest,uint32_t nPos);

/*brief    按照数据类型完成指定类型的转换为数组
 *param ： eType: 数据类型
 *param ： pData: 数据储存指针
 *param ： nBulk: 数据长度
 *param ： pu8Dest: 转换数据存储指针
 *return： 成功返回0 失败返回<0
 */										
int wow_data_other_to_byte(DevDataType_E eType,void *pData,uint32_t nBulk, uint8_t* pu8Dest);
int wow_data_other_to_byte_offset(DevDataType_E eType,void *pData,uint32_t nPos,uint32_t nBulk, uint8_t* pu8Dest);

#endif
