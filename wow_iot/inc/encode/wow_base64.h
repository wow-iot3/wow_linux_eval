#ifndef __WOW_IOT_ENCODE_WOW_BASE64_H_
#define __WOW_IOT_ENCODE_WOW_BASE64_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/*brief    base64算法编码
 *param ： pu8Idata   ：输入需加密数据
 *param ： u32Ilen    ：加密内容长度			       	
 *param ： pu8Odata   ：保存加密结果缓存
 *param ： u32Olen    ：保存加密结果缓存长度	---向上取整(原始字节数 / 3) * 4
 *return： 成功返回加密内容长度 失败返回-1
 */
int wow_base64_encode(const uint8_t *pu8Idata, uint32_t u32Ilen,uint8_t *pu8Odata, uint32_t u32Olen);


/*brief    base64算法解码
 *param ： pu8Idata   ：输入需解密数据
 *param ： u32Ilen    ：解密内容长度
 *param ： pu8Odata   ：保存解密结果缓存
 *param ： u32Olen    ：保存解密结果缓存长度
 *return： 成功返回解密内容长度 失败返回-1
 */
int wow_base64_decode(const uint8_t *pu8Idata, uint32_t u32Ilen,uint8_t *pu8Odata, uint32_t u32Olen);

#ifdef __cplusplus
}
#endif

#endif

