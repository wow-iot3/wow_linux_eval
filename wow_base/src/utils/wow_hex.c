#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_keyword.h"

#include "utils/wow_hex.h"

/*brief    数组转16进制字符串
 *param ： pu8Idata:待转换数组
 *param ： snIlen:输入数组长度
 *param ： pu8Odata:存储输出字符串结果
 *param ： snOlen:输出字符串长度 
 *return： 成功返回0 失败返回-1
 	注：输出长度应大于2倍于输入数组长度
 */
__EX_API__ int ByteArray2HexStr(const uint8_t* pu8Idata, size_t snIlen, uint8_t* pu8Odata, size_t snOlen)
{
	int i;
	char hexval[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

	
	CHECK_RET_VAL_P(pu8Idata && pu8Odata,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(snOlen >= 2*snIlen,-PARAM_INPUT_ARG_INVALID,"param input size invalid!\n");

	for (i = 0; i < snIlen; i++) {
		pu8Odata[i * 2] = hexval[((pu8Idata[i] >> 4) & 0xF)];
		pu8Odata[(i * 2) + 1] = hexval[(pu8Idata[i]) & 0x0F];
	}

	return WOW_SUCCESS;
}

/*brief    16进制字符串转数组
 *param ： pu8Idata:待转换数组
 *param ： snIlen:输入数组长度
 *param ： pu8Odata:输出字符串结果
 *param ： snOlen:输出字符串长度 
 *return： 成功返回0 失败返回-1
 	注：输出长度应大于等于输入数组长度的二分之一
 */
__EX_API__ int HexStr2ByteArray(const uint8_t* pu8Idata, size_t snIlen, uint8_t* pu8Odata, size_t snOlen)
{
	int  i;
	char ch;

	CHECK_RET_VAL_P(pu8Idata && pu8Odata,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(snOlen >= snIlen/2,-PARAM_INPUT_ARG_INVALID,"param input size invalid!\n");

	for (i = 0; i < snIlen/2; i++) 
	{
		ch = pu8Idata[i * 2];
		if (ch >= '0' && ch <= '9'){
			pu8Odata[i] = (char)(ch - '0');
		}
		else if (ch >= 'A' && ch <= 'F'){
			pu8Odata[i] = (char)(ch - 'A' + 10);
		}
		else if (ch >= 'a' && ch <= 'f'){
			pu8Odata[i] = (char)(ch - 'a' + 10);
		}
		else{
			return -PARAM_INPUT_DATA_ERROR;
		}
		  
		ch = pu8Idata[i * 2 + 1];
		if (ch >= '0' && ch <= '9'){
			pu8Odata[i] = (char)((pu8Odata[i] << 4) + (ch - '0'));
		}
		else if (ch >= 'A' && ch <= 'F'){
			pu8Odata[i] = (char)((pu8Odata[i] << 4) + (ch - 'A' + 10));
		}
		else if (ch >= 'a' && ch <= 'f'){
			pu8Odata[i] = (char)((pu8Odata[i] << 4) + (ch - 'a' + 10));
		}  
		else{
			 return -PARAM_INPUT_DATA_ERROR;
		}
		 
	}

	return WOW_SUCCESS;
}


/*brief    以十六进制打印数组内容
 *param ： pu8Data:数组内容
 *param ： nLen:输入数组长度
 *return： 无
 */
__EX_API__ void wow_hex_print(uint8_t* pu8Data, int nLen)
{
	int with_space = 1;
	uint8_t *i, *j;
	uint8_t* buff = (uint8_t*)malloc(nLen * 3 + 1);
	CHECK_RET_VOID(buff);

	const char HEX_MAP[] = "0123456789abcdef";
	
	for(i = pu8Data, j = buff; i < pu8Data + nLen; i++){
		*j++ = HEX_MAP[*i >> 4 & 0xF];
		*j++ = HEX_MAP[*i & 0xF];
		if(with_space)
			*j++ = ' ';
	}
	*j = '\0';

	printf("%s\n",buff);
	
	free(buff);
}
