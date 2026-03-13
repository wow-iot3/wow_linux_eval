#ifndef __WOW_IOT_ENCRYPT_WOW_RSA_H_
#define __WOW_IOT_ENCRYPT_WOW_RSA_H_

#ifdef __cplusplus
extern "C" {
#endif


/*brief    生成RSA密钥对
 *param ： pu8PubKey   ：公钥存储地址
 *param ： u16PubLen   ：公钥存储长度
 *param ： pu8PrivKey  ：私钥存储地址
 *param ： u16PrivLen  ：私钥存储长度
 *return： 成功返回0  失败返回错误值
 */
int wow_rsa_generate_by_str(uint8_t* pu8PubKey,uint16_t u16PubLen,uint8_t* pu8PrivKey,uint16_t u16PrivLen);

									
/*brief    RSA加密
 *param ： pu8PubKey ：公钥存储地址
 *param ： pu8Sdata  ：待加密数据存储地址
 *param ： u16Slen   ：待加密数据长度
 *param ： pu8Ddata  ：加密数据存储地址
 *param ： pu16Dlen  ：加密数据存储长度
 *return： 成功返回0  失败返回错误值
 */
int wow_rsa_encrypt_by_str(const char* pu8PubKey,uint8_t* pu8Sdata,uint16_t u16Slen,uint8_t* pu8Ddata,uint16_t* pu16Dlen);


/*brief    RSA解密
 *param ： pu8PrivKey：私钥存储地址
 *param ： pu8Sdata  ：待解密数据存储地址
 *param ： u16Slen   ：待解密数据长度
 *param ： pu8Ddata  ：解密数据存储地址
 *param ： pu16Dlen  ：解密数据存储长度
 *return： 成功返回0  失败返回错误值
 */
int wow_rsa_decrypt_by_str(const char* pu8PrivKey,uint8_t* pu8Sdata,uint16_t u16Slen,uint8_t* pu8Ddata,uint16_t* pu16Dlen);


/*brief    RSA签名
 *param ： pu8PubKey ：私钥存储地址
 *param ： pcSrcFiile  ：原文件
 *param ： pcSignFile  ：签名文件
 *return： 成功返回0  失败返回错误值
 */
int wow_rsa_sign_by_str(const char* pu8PrivKey,const char* pcSrcFiile,const char* pcSignFile);


/*brief    RSA验证
 *param ： pu8PrivKey：公钥存储地址
 *param ： pcSrcFiile  ：原文件
 *param ： pcSignFile  ：签名文件
 *return： 成功返回0  失败返回错误值
 */
int wow_rsa_verify_by_str(const char* pu8PubKey,const char* pcSrcFiile,const char* pcSignFile);


/*brief    生成RSA密钥对文件
 *param ： pcPubFile   ：公钥文件地址
 *param ： pcPrivFile  ：私钥文件地址
 *return： 成功返回0  失败返回错误值
 */
int wow_rsa_generate_by_file(const char* pcPubFile,const char* pcPrivFile);

/*brief    RSA加密
 *param ： pcPubFile ：公钥文件地址
 *param ： pu8Sdata  ：待加密数据存储地址
 *param ： u16Slen   ：待加密数据长度
 *param ： pu8Ddata  ：加密数据存储地址
 *param ： pu16Dlen  ：加密数据存储长度
 *return： 成功返回0  失败返回错误值
 */
int wow_rsa_encrypt_by_file(const char* pcPubFile,uint8_t* pu8Sdata,uint16_t u16Slen,uint8_t* pu8Ddata,uint16_t* pu16Dlen);

/*brief    RSA解密
 *param ： pcPrivFile：私钥文件地址
 *param ： pu8Sdata  ：待解密数据存储地址
 *param ： u16Slen   ：待解密数据长度
 *param ： pu8Ddata  ：解密数据存储地址
 *param ： pu16Dlen  ：解密数据存储长度
 *return： 成功返回0  失败返回错误值
 */
int wow_rsa_decrypt_by_file(const char* pcPrivFile,uint8_t* pu8Sdata,uint16_t u16Slen,uint8_t* pu8Ddata,uint16_t* pu16Dlen);

/*brief    RSA签名
 *param ： pcPrivFile：私钥文件地址
 *param ： pcSrcFiile  ：原文件
 *param ： pcSignFile  ：签名文件
 *return： 成功返回0  失败返回错误值
 */
int wow_rsa_sign_by_file(const char* pcPrivFile,const char* pcSrcFiile,const char* pcSignFile);

/*brief    RSA验证
 *param ： pcPubFile ：公钥文件地址
 *param ： pcSrcFiile  ：原文件
 *param ： pcSignFile  ：签名文件
 *return： 成功返回0  失败返回错误值
 */
int wow_rsa_verify_by_file(const char* pcPubFile,const char* pcSrcFiile,const char* pcSignFile);

#ifdef __cplusplus
}
#endif

#endif


