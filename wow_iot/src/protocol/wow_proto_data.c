#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_keyword.h"
#include "hash/wow_hash.h"
#include "utils/wow_type.h"

#include "wow_iot_errno.h"
#include "protocol/wow_proto_data.h"

/*brief    获取数据类型
 *param ： pcTypeStr: 数据类型字符串
 *return： 成功返回数据类型 失败返回DATA_MAX
 */
DevDataType_E wow_data_get_type(const char* pcTypeStr)
{
	CHECK_RET_VAL_P(pcTypeStr,-PARAM_INPUT_DATA_IS_NULL,"param data input invalid!\n");
	uint32_t hash = (uint32_t)wow_hash_data(HASH_ARRAY_RS,(uint8_t const*)pcTypeStr,strlen(pcTypeStr));

	switch(hash){
		case 0xd08706ed:///<bool_t
			return DATA_BOOL;
		case 0x18f3b533:///<sint8_t
			return DATA_SINT_8;
		case 0x761398d8:///<sint16_t
			return DATA_SINT_16;
		case 0x3237968e:///<sint32_t
			return DATA_SINT_32;
		case 0x532fdf57:///<int64_t
			return DATA_SINT_64;
		case 0x6bd44a81:///<uint8_t
			return DATA_UINT_8;
		case 0x0c16310a:///<uint16_t
			return DATA_UINT_16;
		case 0xc83a2ec0:///<uint32_t
			return DATA_UINT_32;
		case 0xe9327789:///<uint64_t
			return DATA_UINT_64;
		case 0x8812672b:///<float_t
			return DATA_FLOAT;
		case 0x756f291e:///<double_t
			return DATA_DOUBLE;
	    case 0x1356c9f7:///<func_t
			return DATA_FUNC;
		default:
			return DATA_MAX; 
	}
}

/*brief    数据字符串转数据结构存储
 *param ： eType: 数据类型
 *param ： pcStr: 数据字符串
 *param ： ptVal: 数据处理储存指针
 *return： 成功返回0 失败返回-1
 */
int wow_data_str_to_val(DevDataType_E eType,const char* pcStr,DevValue_U* ptVal)
{
	CHECK_RET_VAL_P(pcStr && ptVal,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

	switch(eType){
		case DATA_BOOL:
		case DATA_UINT_8:
			StrtoU8(pcStr,&ptVal->u8);
			break;
		case DATA_SINT_8:
			StrtoS8(pcStr,&ptVal->s8);
			break;
		case DATA_UINT_16:
			StrtoU16(pcStr,&ptVal->u16);
			break;
		case DATA_SINT_16:
			StrtoS16(pcStr,&ptVal->s16);
			break;
		case DATA_UINT_32:
			StrtoU32(pcStr,&ptVal->u32);
			break;
		case DATA_SINT_32:
			StrtoS32(pcStr,&ptVal->s32);
			break;
		case DATA_FLOAT:
			StrtoF32(pcStr,&ptVal->f);
			break;
		case DATA_UINT_64:
			StrtoU64(pcStr,&ptVal->u64);
			break;
		case DATA_SINT_64:
			StrtoS64(pcStr,&ptVal->s64);
			break;
		case DATA_DOUBLE:
			StrtoD64(pcStr,&ptVal->d);
			break;
		case DATA_FUNC:
		default:
			return PARAM_INPUT_ARG_INVALID; 	
	}

	return 0;
}


/*brief    数据结构存储转字符串
 *param ： eType: 数据类型
 *param ： pcStr: 数据字符串储存指针
 *param ： ptVal: 数据处理指针
 *return： 成功返回0 失败返回-1
 */
int wow_data_val_to_str(DevDataType_E eType,DevValue_U* ptVal,char* pcStr)
{
	CHECK_RET_VAL_P(pcStr && ptVal,-PARAM_INPUT_DATA_IS_NULL,"param input invalid!\n");

	switch(eType){
		case DATA_BOOL:
		case DATA_UINT_8:
			U8toStr(ptVal->u8,pcStr,MAXSTR8);
			break;
		case DATA_SINT_8:
			S8toStr(ptVal->s8,pcStr,MAXSTR8);
		case DATA_UINT_16:
			U16toStr(ptVal->u16,pcStr,MAXSTR16);
			break;
		case DATA_SINT_16:
			S16toStr(ptVal->s16,pcStr,MAXSTR16);
			break;
		case DATA_UINT_32:
			U32toStr(ptVal->u32,pcStr,MAXSTR32);
			break;
		case DATA_SINT_32:
			S32toStr(ptVal->s32,pcStr,MAXSTR32);
			break;
		case DATA_FLOAT:
			F32toStr(ptVal->f,pcStr,MAXSTRFLOAT);
			break;
		case DATA_UINT_64:
			U64toStr(ptVal->u64,pcStr,MAXSTR64);
			break;
		case DATA_SINT_64:
			S64toStr(ptVal->s64,pcStr,MAXSTR64);
			break;
		case DATA_DOUBLE:
			D64toStr(ptVal->d,pcStr,MAXSTRDOUBLE);
			break;
		case DATA_FUNC:
		default:
			return PARAM_INPUT_ARG_INVALID; 
	}

	return 0;
}

/*brief    获取数据类型长度
 *param ： eType: 数据类型
 *return： 成功返回数据类型长度 失败返回<0
 */
int wow_data_get_len(DevDataType_E eType)
{	
	switch(eType){
		case DATA_BOOL:
		case DATA_UINT_8:
		case DATA_SINT_8:
			return 1;
		case DATA_UINT_16:
		case DATA_SINT_16:
			return 2;
		case DATA_UINT_32:
		case DATA_SINT_32:
		case DATA_FLOAT:
			return 4;
		case DATA_UINT_64:
		case DATA_SINT_64:
		case DATA_DOUBLE:
			return 8;
		case DATA_FUNC:
		default:
			return PARAM_INPUT_ARG_INVALID;
	}

	return 0;
}

/*brief    按照数据类型完成数组到指定类型的转换
 *param ： eType: 数据类型
 *param ： pu8Data: 数据储存指针
 *param ： nBulk: 数据长度
 *param ： pDest: 转换数据存储指针
 *return： 成功返回0 失败返回<0
 */

int wow_data_byte_to_other_offset(DevDataType_E eType, uint8_t* pu8Data,uint32_t nBulk,void *pDest,uint32_t nPos)
{
	CHECK_RET_VAL_P(pu8Data && pDest,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid.");
	CHECK_RET_VAL_P(eType < DATA_FUNC,-PARAM_INPUT_ARG_INVALID,"param input arg invalid.");

	switch(eType){
		case DATA_UINT_16:
			return char2short_be(pu8Data,nBulk,(uint16_t*)pDest + nPos,nBulk/2);
		case DATA_UINT_32:
			return char2int_be(pu8Data,nBulk,(uint32_t*)pDest+ nPos,nBulk/4);
		case DATA_FLOAT:
			return char2float_be(pu8Data,nBulk,(float*)pDest+ nPos,nBulk/4);
		default:
			return -PROTO_DATA_TYPE_NO_FUNC;
	}

	return 0;
}
int wow_data_byte_to_other(DevDataType_E eType, uint8_t* pu8Data ,uint32_t nBulk,void *pDest)
{
	CHECK_RET_VAL_P(pu8Data && pDest,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid.");
	CHECK_RET_VAL_P(eType < DATA_FUNC,-PARAM_INPUT_ARG_INVALID,"param input arg invalid.");

	return wow_data_byte_to_other_offset(eType,pu8Data,nBulk,pDest,0);
}

/*brief    按照数据类型完成指定类型的转换为数组
 *param ： eType: 数据类型
 *param ： pData: 数据储存指针
 *param ： nBulk: 数据长度
 *param ： pu8Dest: 转换数据存储指针
 *return： 成功返回0 失败返回<0
 */										
int wow_data_other_to_byte_offset(DevDataType_E eType,void *pData,uint32_t nPos,uint32_t nBulk, uint8_t* pu8Dest)
{
	CHECK_RET_VAL_P(pData && pu8Dest,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid.");
	CHECK_RET_VAL_P(eType < DATA_FUNC,-PARAM_INPUT_ARG_INVALID,"param input arg invalid.");

	switch(eType){
		case DATA_UINT_16:
			return short2char_be((uint16_t*)pData + nPos,nBulk,pu8Dest,nBulk*2);
		case DATA_UINT_32:
			return int2char_be((uint32_t*)pData + nPos,nBulk,pu8Dest,nBulk*4);
		case DATA_FLOAT:
			return float2char_be((float*)pData + nPos,nBulk,pu8Dest,nBulk*4);
		default:
			return -PROTO_DATA_TYPE_NO_FUNC;
	}

	return 0;
}
int wow_data_other_to_byte(DevDataType_E eType,void *pData,uint32_t nBulk, uint8_t* pu8Dest)
{
	return wow_data_other_to_byte_offset(eType,pData,0,nBulk,pu8Dest);
}
