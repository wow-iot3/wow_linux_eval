#ifndef __WOW_BASE_UTILS_WOW_HEX_H_
#define __WOW_BASE_UTILS_WOW_HEX_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

/*brief    数组转16进制字符串
 *param ： pu8Idata:待转换数组
 *param ： snIlen:输入数组长度
 *param ： pu8Odata:存储输出字符串结果
 *param ： snOlen:输出字符串长度 
 *return： 成功返回0 失败返回-1
 	注：输出长度应大于等于2倍于输入数组长度
 */
int ByteArray2HexStr(const uint8_t* pu8Idata, size_t snIlen, uint8_t* pu8Odata, size_t snOlen);

/*brief    16进制字符串转数组
 *param ： pu8Idata:待转换数组
 *param ： snIlen:输入数组长度
 *param ： pu8Odata:输出字符串结果
 *param ： snOlen:输出字符串长度 
 *return： 成功返回0 失败返回-1
 	注：输出长度应大于等于输入数组长度的二分之一
 */
int HexStr2ByteArray(const uint8_t* pu8Idata, size_t snIlen, uint8_t* pu8Odata, size_t snOlen);


/*brief    以十六进制打印数组内容
 *param ： data:数组内容
 *param ： len:输入数组长度
 *return： 无
 */
void wow_hex_print(uint8_t* pu8Data, int nLen);

#ifdef __cplusplus
}
#endif

#endif

