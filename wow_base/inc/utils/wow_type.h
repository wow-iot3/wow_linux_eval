#ifndef __WOW_BASE_PREFIX_WOW_TYPE_H_ 
#define __WOW_BASE_PREFIX_WOW_TYPE_H_ 

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>

///< 数据类型约束使用以下字段 
///<int64_t/int32_t/int16_t/int8_t
///<uint64_t/uint32_t/uint16_t/uint8_t
///<float/double/bool/char

///< 数据类型枚举类型(当数据类型存在多种公用一个参数的情况使用)
typedef union{
    uint8_t                      u8;
    int8_t                       s8;
    bool                         b;
    uint16_t                     u16;
    int16_t                      s16;
    uint32_t                     u32;
    int32_t                      s32;
    uint64_t                     u64;
    int64_t                      s64;
    size_t                       ul;
	char*                        str;
	uint8_t*                     sstr;
	void*                        ptr;
    float                        f;
    double                       d;
}Value_U, *Value_PU;


///<强制类型转换
#define p2b(x)                       ((bool)(size_t)(x))
#define p2u8(x)                      ((uint8_t)(size_t)(x))
#define p2u16(x)                     ((uint16_t)(size_t)(x))
#define p2u32(x)                     ((uint32_t)(size_t)(x))
#define p2u64(x)                     ((uint64_t)(size_t)(x))
#define p2s8(x)                      ((int8_t)(size_t)(x))
#define p2s16(x)                     ((int16_t)(size_t)(x))
#define p2s32(x)                     ((int32_t)(size_t)(x))
#define p2s64(x)                     ((int64_t)(size_t)(x))
#define u2p(x)                       ((void*)(size_t)(x))
#define i2p(x)                       ((void*)(ssize_t)(x))




/***********************字符串转数据类型 --支持各进制(2\8\10\16)***********************************/
/*brief    二进制字符串转int16
 *param ： pcStr:二进制字符
 *param ： pu16Data:int16数据指针
 *return： 成功返回0 失败返回<0
 */
int S2toU16(const char* pcStr,uint16_t* pu16Data);
/*brief    十进制字符串转int16
 *param ： pcStr:十进制字符
 *param ： pu16Data:int16数据指针
 *return： 成功返回0 失败返回<0
 */
int S10toU16(const char* pcStr,uint16_t* pu16Data);
/*brief    十六进制字符串转int16
 *param ： pcStr:十六进制字符
 *param ： pu16Data:int16数据指针
 *return： 成功返回0 失败返回<0
 */
int S16toU16(const char* pcStr,uint16_t* pu16Data);

/*brief    二进制字符串转int32
 *param ： pcStr:二进制字符
 *param ： pu32Data:int32数据指针
 *return： 成功返回0 失败返回<0
 */
int S2toU32(const char* pcStr,uint32_t* pu32Data);

/*brief    十进制字符串转int32
 *param ： pcStr:十进制字符
 *param ： pu32Data:int32数据指针
 *return： 成功返回0 失败返回<0
 */
int S10toU32(const char* pcStr,uint32_t* pu32Data);

/*brief    十六进制字符串转int32
 *param ： pcStr:十六进制字符
 *param ： pu32Data:int32数据指针
 *return： 成功返回0 失败返回<0
 */
int S16toU32(const char* pcStr,uint32_t* pu32Data);

/*brief    二进制字符串转int64
 *param ： pcStr:二进制字符
 *param ： pu64Data:int64数据指针
 *return： 成功返回0 失败返回<0
 */
 int S2toU64(const char* pcStr,uint64_t* pu64Data);

/*brief    十进制字符串转int64
 *param ： pcStr:十进制字符
 *param ： pu64Data:int64数据指针
 *return： 成功返回0 失败返回<0
 */
 int S10toU64(const char* pcStr,uint64_t* pu64Data);

/*brief    十六进制字符串转int64
 *param ： pcStr:十六进制字符
 *param ： pu64Data:int64数据指针
 *return： 成功返回0 失败返回<0
 */
int S16toU64(const char* pcStr,uint64_t* pu64Data);

/*brief    字符串转int64
 *param ： pcStr:十六进制字符
 *param ： pu64Data:int64数据指针
 *return： 成功返回0 失败返回<0
           注：如果未标识0x或0b则标识使用十进制
 */
int str2u64(const char* pcStr,uint64_t* pu64Data);

/*brief    字符串转double
 *param ： pcStr:十六进制字符
 *param ： pdData:double数据指针
 *return： 成功返回0 失败返回<0
 */
int str2double(const char* pcStr,double* pdData);


/************************************数据类型转字符串****************************************************/
#define StrtoS8(str,v)   		do {sscanf(str,"%hhd", v); } while (0);
#define StrtoU8(str,v)   		do {sscanf(str,"%hhu", v); } while (0);
#define StrtoS16(str,v)   		do {sscanf(str,"%hd", v); } while (0);
#define StrtoU16(str,v)   		do {sscanf(str,"%hu", v); } while (0);
#define StrtoS32(str,v)   		do {sscanf(str,"%d", v); } while (0);
#define StrtoU32(str,v)   		do {sscanf(str,"%u", v); } while (0);
#define StrtoS64(str,v)   		do {sscanf(str,"%"PRId64, v); } while (0);
#define StrtoU64(str,v)   		do {sscanf(str,"%"PRIu64, v); } while (0);
#define StrtoF32(str,v)   		do {sscanf(str,"%f", v); } while (0);
#define StrtoD64(str,v)   		do {sscanf(str,"%lf", v); } while (0);

#define S8toStr(v, buf,n)   		do {snprintf(buf, n,"%hhd", v); } while (0);
#define U8toStr(v, buf,n)   		do {snprintf(buf, n,"%hhu", v); } while (0);
#define S16toStr(v, buf,n)   		do {snprintf(buf, n,"%hd", v); } while (0);
#define U16toStr(v, buf,n)   		do {snprintf(buf, n,"%hu", v); } while (0);
#define S32toStr(v, buf,n)   		do {snprintf(buf, n,"%d", v); } while (0);
#define U32toStr(v, buf,n)   		do {snprintf(buf, n,"%u", v); } while (0);
#define S64toStr(v, buf,n)   		do {snprintf(buf, n,"%"PRId64, v); } while (0);
#define U64toStr(v, buf,n)   		do {snprintf(buf, n,"%"PRIu64, v); } while (0);
#define D64toStr(v, buf,n)   	    do {snprintf(buf, n,"%.17g", v); } while (0);
#define F32toStr(v, buf,n)   		do {snprintf(buf, n,"%.8g", v); } while (0);

/************************************数据类型与数组相互转换****************************************************/
/*brief    short类型转数组
 *param ： pu16Data:short数据
 *param ： nSlen:short数据长度
 *param ： pu8Data:数组指针
 *param ： nDlen:数组数据长度
 *return： 成功返回0 失败返回<0
 */
int short2char(uint16_t* pu16Data,int nSlen, uint8_t *pu8Data,int nDlen);
int short2char_le(uint16_t* pu16Data,int nSlen, uint8_t *pu8Data,int nDlen);
int short2char_be(uint16_t* pu16Data,int nSlen, uint8_t *pu8Data,int nDlen);

/*brief    数组转short类型
 *param ： pu8Data:数组指针
 *param ： nSlen:数组数据长度
 *param ： pu16Data:short数据
 *param ： nDlen:short数据长度
 *return： 成功返回0 失败返回<0
 */
int char2short(uint8_t *pu8Data,int nSlen,uint16_t *pu16Data,int nDlen);
int char2short_le(uint8_t *pu8Data,int nSlen,uint16_t *pu16Data,int nDlen);
int char2short_be(uint8_t *pu8Data,int nSlen,uint16_t *pu16Data,int nDlen);

/*brief    int类型转数组
 *param ： pu32Data:int数据
 *param ： nSlen:int数据长度
 *param ： pu8Data:数组指针
 *param ： nDlen:数组数据长度
 *return： 成功返回0 失败返回<0
 */
int int2char(uint32_t* pu32Data,int nSlen,uint8_t* pu8Data,int nDlen);
int int2char_le(uint32_t* pu32Data,int nSlen,uint8_t* pu8Data,int nDlen);
int int2char_be(uint32_t* pu32Data,int nSlen,uint8_t* pu8Data,int nDlen);

/*brief    数组转int类型
 *param ： pu8Data:数组指针
 *param ： nSlen:数组指针长度
 *param ： pu32Data:int数据指针
 *param ： nDlen:数组长度
 *return： 成功返回0 失败返回<0
 */
int char2int(uint8_t* pu8Data,int nSlen,uint32_t *pu32Data,int Dlen);
int char2int_le(uint8_t* pu8Data,int nSlen,uint32_t *pu32Data,int Dlen);
int char2int_be(uint8_t* pu8Data,int nSlen,uint32_t *pu32Data,int Dlen);

/*brief    long类型转数组
 *param ： pu64Data:long数据
 *param ： nSlen:long数据长度
 *param ： pu8Data:数组指针
 *param ： nDlen:数组数据长度
 *return： 成功返回0 失败返回<0
 */
int long2char(uint64_t* pu64Data,int nSlen,uint8_t* pu8Data,int nDlen);
int long2char_le(uint64_t* pu64Data,int nSlen,uint8_t* pu8Data,int nDlen); 
int long2char_be(uint64_t* pu64Data,int nSlen,uint8_t* pu8Data,int nDlen); 

/*brief    数组转long类型
 *param ： pu8Data:数组指针
 *param ： nSlen:数组指针长度
 *param ： pu64Data:long数据指针
 *param ： nDelen:long数据长度
 *return： 成功返回0 失败返回<0
 */
int char2long(uint8_t* pu8Data,int nSlen,uint64_t *pu64Data,int nDlen);
int char2long_le(uint8_t* pu8Data,int nSlen,uint64_t *pu64Data,int nDlen);
int char2long_be(uint8_t* pu8Data,int nSlen,uint64_t *pu64Data,int nDlen);

/*brief    数组转float类型
 *param ： pu8Data:数组指针
 *param ： nSlen:数组指针长度
 *param ： pfData:float数据指针
 *param ： nDlen:float数据长度
 *return： 成功返回0 失败返回<0
 */
int char2float(uint8_t* pu8Data,int nSlen,float *pfData,int nDlen);
int char2float_le(uint8_t* pu8Data,int nSlen,float *pfData,int nDlen);
int char2float_be(uint8_t* pu8Data,int nSlen,float *pfData,int nDlen);

/*brief    float类型转数组
 *param ： pfData:float数据
 *param ： nSlen:float数据长度
 *param ： pu8Data:数组指针
 *param ： nDlen:数组指针长度
 *return： 成功返回0 失败返回<0
 */
int float2char(float* pfData, int nSlen,uint8_t *pu8Data,int nDlen);
int float2char_le(float* pfData, int nSlen,uint8_t *pu8Data,int nDlen);
int float2char_be(float* pfData, int nSlen,uint8_t *pu8Data,int nDlen);


/************************************BCD码与字符串相互转换****************************************************/
#define _bcd2bin(val) (val & 0x0f) + (val >> 4) * 10
#define _bin2bcd(val) ((val / 10) << 4) + val % 10

/*brief    将8421BCD码转换为十进制数字串
 *param ： pu8BcdData:BCD码数据指针
 *param ： nnBcdLen:BCD码数据长度
 *param ： pu8StrData:转码数据指针
 *param ： nStrLen:转码数据长度
 *return： 成功返回0 失败返回<0
 */
int BCD2String(uint8_t *pu8BcdData, int nBcdLen, uint8_t *pu8StrData, int nStrLen);

/*brief    将十进制数字串转换为8421BCD码
 *param ： pu8StrData:原数据指针
 *param ： nStrLen:原数据长度
 *param ： pu8BcdData:BCD码数据指针
 *param ： nnBcdLen:BCD码数据长度
 *return： 成功返回0 失败返回<0
 */
int String2BCD(const uint8_t *pu8StrData, int nStrLen, uint8_t *pu8BcdData, int nBcdLen);


/************************************modbus 转换****************************************************/
int uint32_to_uint16(uint32_t* pu32Value,int nSlen, uint16_t *pu16Data,int nDlen);
int uint32_to_uint16_le(uint32_t* pu32Value,int nSlen, uint16_t *pu16Data,int nDlen);
int uint32_to_uint16_be(uint32_t* pu32Value,int nSlen, uint16_t *pu16Data,int nDlen);

int uint16_to_uint32(uint16_t *pu16Data,int nSlen, uint32_t *pu32Value,int nDlen);
int uint16_to_uint32_le(uint16_t *pu16Data,int nSlen, uint32_t *pu32Value,int nDlen);
int uint16_to_uint32_be(uint16_t *pu16Data,int nSlen, uint32_t *pu32Value,int nDlen);

int uint16_to_float(uint16_t *pu16Data,int nSlen, float *pfValue,int nDlen);
int uint16_to_float_le(uint16_t *pu16Data,int nSlen, float *pfValue,int nDlen);
int uint16_to_float_be(uint16_t *pu16Data,int nSlen, float *pfValue,int nDlen);

int float_to_uint16(float* pfValue, int nSlen,uint16_t *pu16Data,int nDlen);
int float_to_uint16_le(float* pfValue,int nSlen, uint16_t *pu16Data,int nDlen);
int float_to_uint16_be(float* pfValue,int nSlen, uint16_t *pu16Data,int nDlen);

#ifdef __cplusplus
}
#endif

#endif
