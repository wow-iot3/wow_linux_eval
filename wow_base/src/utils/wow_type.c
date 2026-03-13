#include <ctype.h>
#include <string.h>

#include "prefix/wow_check.h"
#include "prefix/wow_byte.h"
#include "prefix/wow_common.h"
#include "prefix/wow_cpu.h"
#include "prefix/wow_errno.h"

#include "utils/wow_type.h"


 static uint64_t _S2toU64(const char* pcStr)
 {
    while (isspace(*pcStr)) pcStr++;

    int sign = 0;
    if (*pcStr == '-') {
        sign = 1;
        pcStr++;
    }else if(*pcStr == '+'){
		pcStr++;
	} 

    if (pcStr[0] == '0' && (pcStr[1] == 'b' || pcStr[1] == 'B')){
		 pcStr += 2;
	}
       
    while ((*pcStr) == '0') pcStr++;

    uint32_t val = 0;
    while (*pcStr){
        char ch = *pcStr;
        CHECK_RET_BREAK(isdigit2(ch));

        val = (val << 1) + (ch - '0');
        pcStr++;
    }


    return (sign)?(~val + 1):val;
 }

 static uint64_t _S10toU64(const char* pcStr)
{	
	while (isspace(*pcStr)) pcStr++;
	
    int sign = 0;
	if (*pcStr == '-') {
		sign = 1;
		pcStr++;
	}else if(*pcStr == '+'){
		pcStr++;
	} 
	while ((*pcStr) == '0') pcStr++;

    uint64_t val = 0;
    while (*pcStr){
        char ch = *pcStr;
        CHECK_RET_BREAK(isdigit10(ch));

        val = val * 10 + (ch - '0');
        pcStr++;
    }

    return (sign)?(~val + 1):val;
}

static uint64_t _S16toU64(const char* pcStr)
{
	while (isspace(*pcStr)) pcStr++;
	
    int sign = 0;
	if (*pcStr == '-') {
		sign = 1;
		pcStr++;
	}else if(*pcStr == '+'){
		pcStr++;
	} 

    if (pcStr[0] == '0' && (pcStr[1] == 'x' ||pcStr[1] == 'X')) {
		pcStr += 2;
	}
    while ((*pcStr) == '0')pcStr++;
	
    uint64_t val = 0;
    while (*pcStr){
        char ch = *pcStr;
        if (isdigit10(ch))
            val = (val << 4) + (ch - '0');
        else if (ch > ('a' - 1) && ch < ('f' + 1))
            val = (val << 4) + (ch - 'a') + 10;
        else if (ch > ('A' - 1) && ch < ('F' + 1))
            val = (val << 4) + (ch - 'A') + 10;
        else break;
        pcStr++;
    }

    return (sign)?(~val + 1):val;
}


/*brief    二进制字符串转int64
 *param ： pcStr:二进制字符
 *param ： pu64Data:int64数据指针
 *return： 成功返回0 失败返回<0
 */
 int S2toU64(const char* pcStr,uint64_t* pu64Data)
{
    CHECK_RET_VAL_P(pcStr && pu64Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	
    *pu64Data = _S2toU64(pcStr);

    return WOW_SUCCESS;
}

/*brief    十进制字符串转int64
 *param ： pcStr:十进制字符
 *param ： pu64Data:int64数据指针
 *return： 成功返回0 失败返回<0
 */
 int S10toU64(const char* pcStr,uint64_t* pu64Data)
{
    CHECK_RET_VAL_P(pcStr && pu64Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

    *pu64Data = _S10toU64(pcStr);

    return WOW_SUCCESS;
}

/*brief    十六进制字符串转int64
 *param ： pcStr:十六进制字符
 *param ： pu64Data:int64数据指针
 *return： 成功返回0 失败返回<0
 */
int S16toU64(const char* pcStr,uint64_t* pu64Data)
{
    CHECK_RET_VAL_P(pcStr && pu64Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

    *pu64Data = _S16toU64(pcStr);

    return WOW_SUCCESS;
}

/*brief    字符串转int64
 *param ： pcStr:十六进制字符
 *param ： pu64Data:int64数据指针
 *return： 成功返回0 失败返回<0
           注：如果未标识0x或0b则标识使用十进制
 */
int str2u64(const char* pcStr,uint64_t* pu64Data)
{
	CHECK_RET_VAL_P(pcStr && pu64Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

	char const* p = pcStr;
	while (isspace(*p)) p++;

	if (*p == '-' || *p == '+') p++;

	if (*p++ == '0'){
		if (*p == 'x' || *p == 'X'){
			return S16toU64(pcStr,pu64Data);
		}	
		else if (*p == 'b' || *p == 'B'){
			return S2toU64(pcStr,pu64Data);
		}
		else {
			return S10toU64(pcStr,pu64Data);
		} 
	}
	else{
		return S10toU64(pcStr,pu64Data);
	}
}


/*brief    字符串转double
 *param ： pcStr:十六进制字符
 *param ： pdData:double数据指针
 *return： 成功返回0 失败返回<0
 */
int str2double(const char* pcStr,double* pdData)
{
	CHECK_RET_VAL_P(pcStr && pdData,0,"param pcStr is NULL!\n");

    while (isspace(*pcStr)) pcStr++;

    int sign = 0;
    if (*pcStr == '-') {
        sign = 1;
        pcStr++;
    }else if (*pcStr == '+'){
		 pcStr++;
	}
    while ((*pcStr) == '0') pcStr++;

    // compute double: lhs.rhs
    int    dec = 0;
    uint64_t lhs = 0;
    double rhs = 0.;
    int      zeros = 0;
    int8_t   decimals[256];
    int8_t*  d = decimals;
    int8_t*  e = decimals + 256;
    while (*pcStr)
    {
        char ch = *pcStr;
        if (ch == '.'){
            if (!dec) {
                dec = 1;
                pcStr++;
                continue ;
            }else{
				break;
			}
        }

        if (isdigit10(ch)){
            if (dec) {
                if (d < e){
                    if (ch != '0'){
                       
                        while (zeros--) *d++ = 0; ///< fill '0'
                        zeros = 0;
                        
                        *d++ = ch - '0';///< save decimal
                    }else{
						zeros++;
					} 
                }
            } else {
				lhs = lhs * 10 + (ch - '0');
			}
        }else{
			break;
		} 			
    
        pcStr++;
    }

    CHECK_RET_VAL(d <= decimals + 256, -PARAM_INPUT_DATA_ERROR);

    // compute decimal
    while (d-- > decimals) 
		rhs = (rhs + *d) / 10;

    *pdData =  (sign? ((double)lhs + rhs) * -1. : ((double)lhs + rhs));
    return WOW_SUCCESS;
}

/*brief    二进制字符串转int16
 *param ： pcStr:二进制字符
 *param ： pu16Data:int16数据指针
 *return： 成功返回0 失败返回<0
 */
int S2toU16(const char* pcStr,uint16_t* pu16Data)
 {      
    CHECK_RET_VAL_P(pcStr && pu16Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

    *pu16Data = (uint16_t)_S2toU64(pcStr);
    return WOW_SUCCESS;
 }

/*brief    十进制字符串转int16
 *param ： pcStr:十进制字符
 *param ： pu16Data:int16数据指针
 *return： 成功返回0 失败返回<0
 */
int S10toU16(const char* pcStr,uint16_t* pu16Data)
 {      
    CHECK_RET_VAL_P(pcStr && pu16Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

    *pu16Data = (uint16_t)_S10toU64(pcStr);
    return WOW_SUCCESS;
 }

/*brief    十六进制字符串转int16
 *param ： pcStr:十六进制字符
 *param ： pu16Data:int16数据指针
 *return： 成功返回0 失败返回<0
 */
int S16toU16(const char* pcStr,uint16_t* pu16Data)
 {      
    CHECK_RET_VAL_P(pcStr && pu16Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

    *pu16Data = (uint16_t)_S16toU64(pcStr);
    return WOW_SUCCESS;
 }

/*brief    二进制字符串转int32
 *param ： pcStr:二进制字符
 *param ： pu32Data:int32数据指针
 *return： 成功返回0 失败返回<0
 */
int S2toU32(const char* pcStr,uint32_t* pu32Data)
 {      
    CHECK_RET_VAL_P(pcStr && pu32Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

    *pu32Data = (uint32_t)_S2toU64(pcStr);
    return WOW_SUCCESS;
 }

/*brief    十进制字符串转int32
 *param ： pcStr:十进制字符
 *param ： pu32Data:int32数据指针
 *return： 成功返回0 失败返回<0
 */
int S10toU32(const char* pcStr,uint32_t* pu32Data)
 {      
    CHECK_RET_VAL_P(pcStr && pu32Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

    *pu32Data = (uint32_t)_S10toU64(pcStr);
    return WOW_SUCCESS;
 }

/*brief    十六进制字符串转int32
 *param ： pcStr:十六进制字符
 *param ： pu32Data:int32数据指针
 *return： 成功返回0 失败返回<0
 */
int S16toU32(const char* pcStr,uint32_t* pu32Data)
 {      
    CHECK_RET_VAL_P(pcStr && pu32Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

    *pu32Data = (uint32_t)_S16toU64(pcStr);
    return WOW_SUCCESS;
 }


/*brief    short类型转数组
 *param ： pu16Data:short数据
 *param ： nSlen:short数据长度
 *param ： pu8Data:数组指针
 *param ： nDlen:数组数据长度
 *return： 成功返回0 失败返回<0
 */
int short2char(uint16_t* pu16Data,int nSlen, uint8_t *pu8Data,int nDlen) 
{
	CHECK_RET_VAL_P(pu16Data && pu8Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
    CHECK_RET_VAL_P(nDlen >= nSlen*2,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    int i = 0;
    if(WOW_BIG_ENDIAN == 1){
        for(i = 0;i < nSlen;i++){
            byte_set_16_be_impl(pu8Data + 2*i,pu16Data[i]);         
        }
    }else if(WOW_LITTLE_ENDIAN == 1){
        for(i = 0;i < nSlen;i++){
            byte_set_16_le_impl(pu8Data + 2*i,pu16Data[i]);         
        }
    }else{
        for(i = 0;i < nSlen;i++){
            byte_set_16_ne_impl(pu8Data + 2*i,pu16Data[i]);         
        }  
    }
	
    return WOW_SUCCESS;
}

int short2char_le(uint16_t* pu16Data,int nSlen, uint8_t *pu8Data,int nDlen) 
{
	CHECK_RET_VAL_P(pu16Data && pu8Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
    CHECK_RET_VAL_P(nDlen >= nSlen*2,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    int i = 0;
    for(i = 0;i < nSlen;i++){
        byte_set_16_le_impl(pu8Data + 2*i,pu16Data[i]);         
    }

    return WOW_SUCCESS;
}

int short2char_be(uint16_t* pu16Data,int nSlen, uint8_t *pu8Data,int nDlen) 
{
	CHECK_RET_VAL_P(pu16Data && pu8Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
    CHECK_RET_VAL_P(nDlen >= nSlen*2,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    int i = 0;
    for(i = 0;i < nSlen;i++){
        byte_set_16_be_impl(pu8Data + 2*i,pu16Data[i]);         
    }

    return WOW_SUCCESS;
}

/*brief    数组转short类型
 *param ： pu8Data:数组指针
 *param ： nSlen:数组数据长度
 *param ： pu16Data:short数据
 *param ： nDlen:short数据长度
 *return： 成功返回0 失败返回<0
 */
int char2short(uint8_t *pu8Data,int nSlen,uint16_t *pu16Data,int nDlen) 
{
    CHECK_RET_VAL_P(pu8Data && pu16Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(nDlen >= nSlen/2,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    int  i = 0;
    if(WOW_BIG_ENDIAN == 1){
        for(i = 0;i < nSlen/2;i++){
            pu16Data[i] = byte_get_16_be_impl(pu8Data + 2*i);   
        }
    }else if(WOW_LITTLE_ENDIAN == 1){
        for(i = 0;i < nSlen/2;i++){
            pu16Data[i] = byte_get_16_le_impl(pu8Data + 2*i);   
        }
    }else{
         for(i = 0;i < nSlen/2;i++){
            pu16Data[i] = byte_get_16_ne_impl(pu8Data + 2*i);   
        }
    }
	
    return WOW_SUCCESS;
}
int char2short_le(uint8_t *pu8Data,int nSlen,uint16_t *pu16Data,int nDlen) 
{
    CHECK_RET_VAL_P(pu8Data && pu16Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(nDlen >= nSlen/2,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    int  i = 0;
    for(i = 0;i < nSlen/2;i++){
        pu16Data[i] = byte_get_16_le_impl(pu8Data + 2*i);   
    }

    return WOW_SUCCESS;
}
int char2short_be(uint8_t *pu8Data,int nSlen,uint16_t *pu16Data,int nDlen) 
{
    CHECK_RET_VAL_P(pu8Data && pu16Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(nDlen >= nSlen/2,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    int i = 0;
    for(i = 0;i < nSlen/2;i++){
        pu16Data[i] = byte_get_16_be_impl(pu8Data + 2*i);   
    }

    return WOW_SUCCESS;
}
/*brief    int类型转数组
 *param ： pu32Data:int数据
 *param ： nSlen:int数据长度
 *param ： pu8Data:数组指针
 *param ： nDlen:数组数据长度
 *return： 成功返回0 失败返回<0
 */
int int2char(uint32_t* pu32Data,int nSlen,uint8_t* pu8Data,int nDlen) 
{
	CHECK_RET_VAL_P(pu32Data && pu8Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
    CHECK_RET_VAL_P(nDlen >= 4*nSlen,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    int i = 0;
    if(WOW_BIG_ENDIAN == 1){
        for(i = 0;i < nSlen;i++){
            byte_set_32_be_impl(pu8Data + 4*i,pu32Data[i]);
        }    
    }else if(WOW_LITTLE_ENDIAN == 1){
	    for(i = 0;i < nSlen;i++){
            byte_set_32_le_impl(pu8Data + 4*i,pu32Data[i]);
        }   
    }else{
	    for(i = 0;i < nSlen;i++){
            byte_set_32_ne_impl(pu8Data + 4*i,pu32Data[i]);
        }   
    }

    return WOW_SUCCESS;
}

int int2char_le(uint32_t* pu32Data,int nSlen,uint8_t* pu8Data,int nDlen) 
{
	CHECK_RET_VAL_P(pu32Data && pu8Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
    CHECK_RET_VAL_P(nDlen >= 4*nSlen,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    int i = 0;
    for(i = 0;i < nSlen;i++){
        byte_set_32_le_impl(pu8Data + 4*i,pu32Data[i]);
    }    

    return WOW_SUCCESS;
}
int int2char_be(uint32_t* pu32Data,int nSlen,uint8_t* pu8Data,int nDlen) 
{
	CHECK_RET_VAL_P(pu32Data && pu8Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
    CHECK_RET_VAL_P(nDlen >= 4*nSlen,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    int i = 0;
    for(i = 0;i < nSlen;i++){
        byte_set_32_be_impl(pu8Data + 4*i,pu32Data[i]);
    }   

    return WOW_SUCCESS;
}

/*brief    数组转int类型
 *param ： pu8Data:数组指针
 *param ： nSlen:数组指针长度
 *param ： pu32Data:int数据指针
 *param ： nDlen:数组长度
 *return： 成功返回0 失败返回<0
 */
int char2int(uint8_t* pu8Data,int nSlen,uint32_t *pu32Data,int nDlen) 
{
	CHECK_RET_VAL_P(pu8Data && pu32Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(nDlen >= nSlen/4,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    int i = 0;
    if(WOW_BIG_ENDIAN == 1){
        for(i = 0; i < nSlen/4; i++){
           pu32Data[i] = byte_get_32_be_impl(pu8Data + 4*i);     
        }
    }else if(WOW_LITTLE_ENDIAN == 1){
	    for(i = 0; i < nSlen/4; i++){
           pu32Data[i] = byte_get_32_le_impl(pu8Data + 4*i);     
        }
    }else{
        for(i = 0; i < nSlen/4; i++){
           pu32Data[i] = byte_get_32_ne_impl(pu8Data + 4*i);     
        }
    }

    return WOW_SUCCESS;
}
int char2int_le(uint8_t* pu8Data,int nSlen,uint32_t *pu32Data,int nDlen) 
{
	CHECK_RET_VAL_P(pu8Data && pu32Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(nDlen >= nSlen/4,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    int i = 0;
    for(i = 0; i < nSlen/4; i++){
        pu32Data[i] = byte_get_32_le_impl(pu8Data + 4*i);     
    }

    return WOW_SUCCESS;
}
int char2int_be(uint8_t* pu8Data,int nSlen,uint32_t *pu32Data,int nDlen) 
{
	CHECK_RET_VAL_P(pu8Data && pu32Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(nDlen >= nSlen/4,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    int i = 0;
    for(i = 0; i < nSlen/4; i++){
        pu32Data[i] = byte_get_32_be_impl(pu8Data + 4*i);     
    }

    return WOW_SUCCESS;
}
/*brief    long类型转数组
 *param ： u64Data:long数据
 *param ： nSlen:long数据长度
 *param ： pu8Data:数组指针
 *param ： nDlen:数组数据长度
 *return： 成功返回0 失败返回<0
 */
int long2char(uint64_t* pu64Data,int nSlen,uint8_t* pu8Data,int nDlen) 
{
	CHECK_RET_VAL_P(pu8Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
    CHECK_RET_VAL_P(nDlen >= 8*nSlen,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    int i = 0;
    if(WOW_BIG_ENDIAN == 1){
        for(i = 0;i < nSlen;i++){
            byte_set_64_be_impl(pu8Data + 8*i,pu64Data[i]);    
        }
    }else if(WOW_LITTLE_ENDIAN == 1){
        for(i = 0;i < nSlen;i++){
            byte_set_64_le_impl(pu8Data + 8*i,pu64Data[i]);    
        }
    }else{
        for(i = 0;i < nSlen;i++){
            byte_set_64_ne_impl(pu8Data + 8*i,pu64Data[i]);    
        }
    }

    return WOW_SUCCESS;
}
int long2char_le(uint64_t* pu64Data,int nSlen,uint8_t* pu8Data,int nDlen) 
{
	CHECK_RET_VAL_P(pu8Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
    CHECK_RET_VAL_P(nDlen >= 8*nSlen,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    int i = 0;
    for(i = 0;i < nSlen;i++){
        byte_set_64_le_impl(pu8Data + 8*i,pu64Data[i]);    
    }

    return WOW_SUCCESS;
}
int long2char_be(uint64_t* pu64Data,int nSlen,uint8_t* pu8Data,int nDlen) 
{
	CHECK_RET_VAL_P(pu8Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
    CHECK_RET_VAL_P(nDlen >= 8*nSlen,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    int i = 0;
    for(i = 0;i < nSlen;i++){
        byte_set_64_be_impl(pu8Data + 8*i,pu64Data[i]);    
    }

    return WOW_SUCCESS;
}
/*brief    数组转long类型
 *param ： pu8Data:数组指针
 *param ： nSlen:数组指针长度
 *param ： pu64Data:long数据指针
 *param ： nDelen:long数据长度
 *return： 成功返回0 失败返回<0
 */
int char2long(uint8_t* pu8Data,int nSlen,uint64_t *pu64Data,int nDlen) 
{
	CHECK_RET_VAL_P(pu8Data && pu64Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(nDlen >= nSlen/8,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    int i = 0;
    if(WOW_BIG_ENDIAN == 1){
        for(i = 0; i < nSlen/8; i++){
           pu64Data[i] = byte_get_64_be_impl(pu8Data);     
        }    
    }else if(WOW_LITTLE_ENDIAN == 1){
	    for(i = 0; i < nSlen/8; i++){
           pu64Data[i] = byte_get_64_le_impl(pu8Data);     
        }
    }else{
	    for(i = 0; i < nSlen/8; i++){
           pu64Data[i] = byte_get_64_ne_impl(pu8Data);     
        }
    }

    return WOW_SUCCESS;
}

int char2long_le(uint8_t* pu8Data,int nSlen,uint64_t *pu64Data,int nDlen) 
{
	CHECK_RET_VAL_P(pu8Data && pu64Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(nDlen >= nSlen/8,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    int i = 0;
    for(i = 0; i < nSlen/8; i++){
        pu64Data[i] = byte_get_64_le_impl(pu8Data);     
    }

    return WOW_SUCCESS;
}
int char2long_be(uint8_t* pu8Data,int nSlen,uint64_t *pu64Data,int nDlen) 
{
	CHECK_RET_VAL_P(pu8Data && pu64Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(nDlen >= nSlen/8,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    int i = 0;
    for(i = 0; i < nSlen/8; i++){
        pu64Data[i] = byte_get_64_be_impl(pu8Data);     
    }    

    return WOW_SUCCESS;
}

/*brief    数组转float类型
 *param ： pu8Data:数组指针
 *param ： nSlen:数组指针长度
 *param ： pfData:float数据指针
 *param ： nDlen:float数据长度
 *return： 成功返回0 失败返回<0
 */
int char2float(uint8_t* pu8Data,int nSlen,float *pfData,int nDlen)
{
    CHECK_RET_VAL_P(pu8Data && pfData,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(nDlen >= nSlen/4,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");
    
    int i = 0;
	uint32_t tmp = 0;
    
    if(WOW_BIG_ENDIAN == 1){
        for(i = 0; i < nSlen/4; i++){
           tmp = byte_get_32_be_impl(pu8Data + 4*i);  
           memcpy(&pfData[i], &tmp, sizeof(float));   
        }
    }else if(WOW_LITTLE_ENDIAN == 1){
	    for(i = 0; i < nSlen/4; i++){
           tmp = byte_get_32_le_impl(pu8Data + 4*i);     
           memcpy(&pfData[i], &tmp, sizeof(float)); 
        }
    }else{
        for(i = 0; i < nSlen/4; i++){
           tmp = byte_get_32_ne_impl(pu8Data + 4*i);    
           memcpy(&pfData[i], &tmp, sizeof(float));  
        }
    }

    return WOW_SUCCESS;
}

int char2float_le(uint8_t* pu8Data,int nSlen,float *pfData,int nDlen)
{
    CHECK_RET_VAL_P(pu8Data && pfData,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(nDlen >= nSlen/4,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    int i = 0;
	uint32_t tmp = 0;

    for(i = 0; i < nSlen/4; i++){
        tmp = byte_get_32_le_impl(pu8Data + 4*i);     
        memcpy(&pfData[i], &tmp, sizeof(float)); 
    }

    return WOW_SUCCESS;
}
int char2float_be(uint8_t* pu8Data,int nSlen,float *pfData,int nDlen)
{
    CHECK_RET_VAL_P(pu8Data && pfData,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(nDlen >= nSlen/4,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    int i = 0;
	uint32_t tmp = 0;

    for(i = 0; i < nSlen/4; i++){
        tmp = byte_get_32_be_impl(pu8Data + 4*i);  
        memcpy(&pfData[i], &tmp, sizeof(float));   
    }

    return WOW_SUCCESS;
}

/*brief    float类型转数组
 *param ： pfData:float数据
 *param ： nSlen:float数据长度
 *param ： pu8Data:数组指针
 *param ： nDlen:数组指针长度
 *return： 成功返回0 失败返回<0
 */
int float2char(float* pfData, int nSlen,uint8_t *pu8Data,int nDlen)
{
    CHECK_RET_VAL_P(pu8Data ,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(nDlen >= nSlen*4,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    int i = 0;
    uint32_t tmp = 0;

    if(WOW_BIG_ENDIAN == 1){
        for(i = 0;i < nSlen;i++){
            memcpy(&tmp, &pfData[i], sizeof(uint32_t));
            byte_set_32_be_impl(pu8Data + 4*i,tmp);
        }    
    }else if(WOW_LITTLE_ENDIAN == 1){
	    for(i = 0;i < nSlen;i++){
            memcpy(&tmp, &pfData[i], sizeof(uint32_t));
            byte_set_32_le_impl(pu8Data + 4*i,tmp);
        }   
    }else{
	    for(i = 0;i < nSlen;i++){
            memcpy(&tmp, &pfData[i], sizeof(uint32_t));
            byte_set_32_ne_impl(pu8Data + 4*i,tmp);
        }   
    }

    return WOW_SUCCESS;
}
int float2char_le(float* pfData, int nSlen,uint8_t *pu8Data,int nDlen)
{
    CHECK_RET_VAL_P(pu8Data ,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(nDlen >= nSlen*4,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    int i = 0;
    uint32_t tmp = 0;

    for(i = 0;i < nSlen;i++){
        memcpy(&tmp, &pfData[i], sizeof(uint32_t));
        byte_set_32_le_impl(pu8Data + 4*i,tmp);
    }  

    return WOW_SUCCESS;
}
int float2char_be(float* pfData, int nSlen,uint8_t *pu8Data,int nDlen)
{
    CHECK_RET_VAL_P(pu8Data ,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(nDlen >= nSlen*4,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");
    
    int i = 0;
    uint32_t tmp = 0;

    for(i = 0;i < nSlen;i++){
        memcpy(&tmp, &pfData[i], sizeof(uint32_t));
        byte_set_32_be_impl(pu8Data + 4*i,tmp);
    }    

    return WOW_SUCCESS;
}


/*brief    将8421BCD码转换为十进制数字串
 *param ： pu8BcdData:BCD码数据指针
 *param ： nnBcdLen:BCD码数据长度
 *param ： pu8StrData:转码数据指针
 *param ： nStrLen:转码数据长度
 *return： 成功返回0 失败返回<0
 */
int BCD2String(uint8_t *pu8BcdData, int nBcdLen, uint8_t *pu8StrData, int nStrLen)
{
    int  iPosFlag = 0;
    char cBCD     = 0;
	
    CHECK_RET_VAL_P(pu8BcdData && pu8StrData ,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
    CHECK_RET_VAL_P(2 * nBcdLen <= nStrLen,-PARAM_INPUT_SIZE_IS_SMALL,"param input size invalid!\n");

    for (iPosFlag = 0; iPosFlag < nBcdLen; iPosFlag++)
    {
        cBCD = (pu8BcdData[iPosFlag] >> 4);
        if (0x0f == cBCD){
            break;
        }
        pu8StrData[2 * iPosFlag] = cBCD + '0';
		CHECK_RET_VAL_P(isdigit(pu8StrData[2 * iPosFlag]),-1,"pu8StrData[2 * %d] is not digit!\n", iPosFlag);


        cBCD = (pu8BcdData[iPosFlag] & 0x0f);
        if (0x0f == cBCD){
            break;
        }
		pu8StrData[2 * iPosFlag + 1] = cBCD + '0';
		CHECK_RET_VAL_P(isdigit(pu8StrData[2 * iPosFlag + 1]),-1,"pu8StrData[2 * %d] is not digit!\n", iPosFlag);
        pu8StrData[2 * iPosFlag + 1] = cBCD + '0';
    }
    
    return WOW_SUCCESS;
}

/*brief    将十进制数字串转换为8421BCD码
 *param ： pu8StrData:原数据指针
 *param ： nStrLen:原数据长度
 *param ： pu8BcdData:BCD码数据指针
 *param ： nnBcdLen:BCD码数据长度
 *return： 成功返回0 失败返回<0
 */
int String2BCD(const uint8_t *pu8StrData, int nStrLen, uint8_t *pu8BcdData, int nBcdLen)
{
    int  iPosFlag = 0;
	
	CHECK_RET_VAL_P(pu8BcdData && pu8StrData ,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
    CHECK_RET_VAL_P(2 * nBcdLen >= nStrLen,-PARAM_INPUT_SIZE_IS_SMALL,"param input size invalid!\n");

    for (iPosFlag = 0; iPosFlag < nStrLen; iPosFlag+=2)
    {
    	CHECK_RET_VAL_P(isdigit(pu8StrData[iPosFlag]),-1,"pu8StrData[%d] is not digit!\n", iPosFlag);

        pu8BcdData[iPosFlag / 2] = pu8StrData[iPosFlag] - '0';
        pu8BcdData[iPosFlag / 2] <<= 4;

        if ((iPosFlag + 1 >= nStrLen)){
            break;
        }

		CHECK_RET_VAL_P(isdigit(pu8StrData[iPosFlag+ 1]),-1,"pu8StrData[%d+ 1] is not digit!\n", iPosFlag);

        pu8BcdData[iPosFlag / 2] += pu8StrData[iPosFlag + 1] - '0';
    }
    
    return WOW_SUCCESS;
}




int uint32_to_uint16(uint32_t* pu32Value,int sDlen, uint16_t *pu16Data,int nDlen)
{
    CHECK_RET_VAL_P(pu32Value && pu16Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
    CHECK_RET_VAL_P(nDlen >= sDlen*2,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");
    if(WOW_BIG_ENDIAN == 1){
        uint32_to_uint16_be(pu32Value,sDlen,pu16Data,nDlen);
    }else if(WOW_LITTLE_ENDIAN == 1){
        uint32_to_uint16_le(pu32Value,sDlen,pu16Data,nDlen);
    }else{
        uint32_to_uint16_be(pu32Value,sDlen,pu16Data,nDlen);
    }
    return WOW_SUCCESS;

}
int uint32_to_uint16_le(uint32_t* pu32Value,int sDlen, uint16_t *pu16Data,int nDlen)
{
    CHECK_RET_VAL_P(pu32Value && pu16Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
    CHECK_RET_VAL_P(nDlen >= sDlen*2,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");
    
    for(int i = 0;i < sDlen;i++){
        pu16Data[2*i] = (uint16_t)(pu32Value[i] & 0x0000ffff);
        pu16Data[2*i+1] = (uint16_t)(pu32Value[i] >> 16);
    }
    
    return WOW_SUCCESS;
}
int uint32_to_uint16_be(uint32_t* pu32Value,int sDlen, uint16_t *pu16Data,int nDlen)
{
    CHECK_RET_VAL_P(pu16Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
    CHECK_RET_VAL_P(nDlen >= sDlen*2,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");
    
    for(int i = 0;i < sDlen;i++){
        pu16Data[2*i] = (uint16_t)(pu32Value[i] >> 16);
        pu16Data[2*i+1] = (uint16_t)(pu32Value[i] & 0x0000ffff);
    }
    
    return WOW_SUCCESS;
}

int uint16_to_uint32(uint16_t *pu16Data, int sDlen, uint32_t *pu32Value,int nDlen)
{
    CHECK_RET_VAL_P(pu16Data && pu32Value,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

    if(WOW_BIG_ENDIAN == 1){
        uint16_to_uint32_be(pu16Data,sDlen,pu32Value,nDlen);
    }else if(WOW_LITTLE_ENDIAN == 1){
        uint16_to_uint32_le(pu16Data,sDlen,pu32Value,nDlen);
    }else{
        uint16_to_uint32_be(pu16Data,sDlen,pu32Value,nDlen);
    }

    return WOW_SUCCESS;
}
int uint16_to_uint32_le(uint16_t *pu16Data, int sDlen, uint32_t *pu32Value,int nDlen)
{
    CHECK_RET_VAL_P(pu16Data && pu32Value,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
    CHECK_RET_VAL_P(nDlen >= sDlen*2,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    for(int i = 0;i < sDlen;i++){
        *pu32Value = (uint32_t)(pu16Data[2*i] & 0x0000ffff) | (uint32_t)(pu16Data[2*i+1] << 16);
    }
    
    return WOW_SUCCESS;
}
int uint16_to_uint32_be(uint16_t *pu16Data, int sDlen, uint32_t *pu32Value,int nDlen)
{
    CHECK_RET_VAL_P(pu16Data && pu32Value,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
    CHECK_RET_VAL_P(nDlen >= sDlen*2,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");
    
    for(int i = 0;i < sDlen;i++){
        *pu32Value = (uint32_t)(pu16Data[2*i] << 16) | (uint32_t)(pu16Data[2*i+1] & 0x0000ffff);
    }   
    
    return WOW_SUCCESS;
}


int uint16_to_float(uint16_t *pu16Data, int sDlen, float *pfValue,int nDlen)
{
    CHECK_RET_VAL_P(pu16Data && pfValue,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

    if(WOW_BIG_ENDIAN == 1){
        uint16_to_float_be(pu16Data,sDlen,pfValue,nDlen);
    }else if(WOW_LITTLE_ENDIAN == 1){
        uint16_to_float_le(pu16Data,sDlen,pfValue,nDlen);
    }else{
        uint16_to_float_be(pu16Data,sDlen,pfValue,nDlen);
    }

    return WOW_SUCCESS;
}
int uint16_to_float_le(uint16_t *pu16Data, int sDlen, float *pfValue,int nDlen)
{
    CHECK_RET_VAL_P(pu16Data && pfValue,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
    CHECK_RET_VAL_P(nDlen >= sDlen*2,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");
    
    uint32_t tmp = 0;
    for(int i = 0;i < sDlen;i++){
        tmp = (uint32_t)(pu16Data[2*i] & 0x0000ffff) | (uint32_t)(pu16Data[2*i+1] << 16);
        memcpy(pfValue+i*sizeof(float), &tmp, sizeof(float));   
    }

    
    return WOW_SUCCESS;
}

int uint16_to_float_be(uint16_t *pu16Data, int sDlen, float *pfValue,int nDlen)
{
    CHECK_RET_VAL_P(pu16Data && pfValue,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
    CHECK_RET_VAL_P(nDlen >= sDlen*2,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    uint32_t tmp = 0;
    for(int i = 0;i < sDlen;i++){
        tmp = (uint32_t)(pu16Data[2*i] << 16) | (uint32_t)(pu16Data[2*i+1] & 0x0000ffff);
        memcpy(pfValue+i*sizeof(float), &tmp, sizeof(float));   
    }
    
    return WOW_SUCCESS;
}


int float_to_uint16(float *pfValue, int sDlen, uint16_t *pu16Data,int nDlen)
{
    CHECK_RET_VAL_P(pfValue && pu16Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
    CHECK_RET_VAL_P(nDlen >= sDlen*2,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    if(WOW_BIG_ENDIAN == 1){
        float_to_uint16_be(pfValue,sDlen,pu16Data,nDlen);
    }else if(WOW_LITTLE_ENDIAN == 1){
        float_to_uint16_le(pfValue,sDlen,pu16Data,nDlen);
    }else{
        float_to_uint16_be(pfValue,sDlen,pu16Data,nDlen);
    }
    
    return WOW_SUCCESS;
}

int float_to_uint16_le(float *pfValue, int sDlen, uint16_t *pu16Data,int nDlen)
{
    CHECK_RET_VAL_P(pfValue && pu16Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
    CHECK_RET_VAL_P(nDlen >= sDlen*2,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    uint32_t tmp = 0;
    for(int i = 0;i < sDlen;i++){
        memcpy(&tmp, pfValue+i*sizeof(float), sizeof(uint32_t));
        pu16Data[2*i] = (uint16_t)(tmp & 0x0000ffff);
        pu16Data[2*i+1] = (uint16_t)(tmp >> 16);
    }
    
    return WOW_SUCCESS;
}
int float_to_uint16_be(float *pfValue, int sDlen, uint16_t *pu16Data,int nDlen)
{
    CHECK_RET_VAL_P(pfValue && pu16Data,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
    CHECK_RET_VAL_P(nDlen >= sDlen*2,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    uint32_t tmp = 0;
    for(int i = 0;i < sDlen;i++){
        memcpy(&tmp, pfValue+i*sizeof(float), sizeof(uint32_t));

        pu16Data[2*i] = (uint16_t)(tmp >> 16);
        pu16Data[2*i+1] = (uint16_t)(tmp & 0x0000ffff);
    }
    
    return WOW_SUCCESS;
}
