
#include "mbedtls/rsa.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/pk.h"
#include "mbedtls/md.h"

#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_common.h"
#include "prefix/wow_keyword.h"

#include "file/wow_file.h"
#include "encrypt/wow_rsa.h"

#include "wow_iot_config.h"
#include "wow_iot_errno.h"

#define KEY_SIZE 1024
#define EXPONENT 65537

typedef void* RSA_PT;


/*brief    生成RSA密钥对
 *param ： pu8PubKey   ：公钥存储地址
 *param ： u16PubLen   ：公钥存储长度
 *param ： pu8PrivKey  ：私钥存储地址
 *param ： u16PrivLen  ：私钥存储长度
 *return： 成功返回0  失败返回错误值
 */
__EX_API__ int wow_rsa_generate_by_str(uint8_t* pu8PubKey,uint16_t u16PubLen,uint8_t* pu8PrivKey,uint16_t u16PrivLen)
{
	int ret = 1;
	mbedtls_pk_context  ctx_pk;
	mbedtls_rsa_context rsa;
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_mpi N, P, Q, D, E, DP, DQ, QP;
	const char *pers = "rsa_genkey";
	
	CHECK_RET_VAL_P(pu8PubKey && pu8PrivKey,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(u16PubLen > 0 && u16PrivLen > 0,-PARAM_INPUT_SIZE_IS_ZERO,"param input size invalid!\n");

	mbedtls_ctr_drbg_init( &ctr_drbg );
	mbedtls_rsa_init( &rsa, MBEDTLS_RSA_PKCS_V15, 0 );
	mbedtls_mpi_init( &N ); mbedtls_mpi_init( &P ); mbedtls_mpi_init( &Q );
	mbedtls_mpi_init( &D ); mbedtls_mpi_init( &E ); mbedtls_mpi_init( &DP );
	mbedtls_mpi_init( &DQ ); mbedtls_mpi_init( &QP );
	

	mbedtls_entropy_init( &entropy );
	ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
								   (const unsigned char *) pers,strlen( pers ));
	CHECK_RET_GOTO_P(ret == 0,exit, "mbedtls_ctr_drbg_seed failed(-0x%0x)!\n", -ret);

	ret = mbedtls_rsa_gen_key( &rsa, mbedtls_ctr_drbg_random, &ctr_drbg, KEY_SIZE,EXPONENT);
	CHECK_RET_GOTO_P(ret == 0,exit, "mbedtls_rsa_gen_key failed(-0x%0x)!\n", -ret);
	
	ret = mbedtls_rsa_export( &rsa, &N, &P, &Q, &D, &E );
	CHECK_RET_GOTO_P(ret == 0,exit, "could not export RSA parameters failed(-0x%0x)!\n", -ret);
	ret = mbedtls_rsa_export_crt( &rsa, &DP, &DQ, &QP );
	CHECK_RET_GOTO_P(ret == 0,exit, "could not export RSA parameters failed(-0x%0x)!\n", -ret);

    ctx_pk.pk_info = mbedtls_pk_info_from_type( MBEDTLS_PK_RSA);
    ctx_pk.pk_ctx  = &rsa; 

	mbedtls_pk_write_pubkey_pem(&ctx_pk,pu8PubKey,u16PubLen-1);
    mbedtls_pk_write_key_pem(&ctx_pk,pu8PrivKey,u16PrivLen-1);
	
exit:
	mbedtls_mpi_free( &N ); mbedtls_mpi_free( &P ); mbedtls_mpi_free( &Q );
	mbedtls_mpi_free( &D ); mbedtls_mpi_free( &E ); mbedtls_mpi_free( &DP );
	mbedtls_mpi_free( &DQ ); mbedtls_mpi_free( &QP );
	mbedtls_rsa_free( &rsa );
	mbedtls_ctr_drbg_free( &ctr_drbg );
	mbedtls_entropy_free( &entropy );
	
	return ret;
}

									
/*brief    RSA加密
 *param ： pu8PubKey ：公钥存储地址
 *param ： pu8Sdata  ：待加密数据存储地址
 *param ： u16Slen   ：待加密数据长度
 *param ： pu8Ddata  ：加密数据存储地址
 *param ： pu16Dlen  ：加密数据存储长度
 *return： 成功返回0  失败返回错误值
 */
__EX_API__ int wow_rsa_encrypt_by_str(const char* pu8PubKey,uint8_t* pu8Sdata,uint16_t u16Slen,uint8_t* pu8Ddata,uint16_t* pu16Dlen)
{
	CHECK_RET_VAL_P(pu8PubKey && pu8Sdata && pu8Ddata && pu16Dlen,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(u16Slen > 0,-PARAM_INPUT_SIZE_IS_ZERO,"param input size invalid!\n");

	int ret = 0;
    const char* pers = "rsa_encrypt";
	mbedtls_rsa_context rsa;
	mbedtls_pk_context ctx_pk;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;

	mbedtls_pk_init(&ctx_pk);
	ret = mbedtls_pk_parse_public_key(&ctx_pk, (uint8_t*)pu8PubKey, strlen(pu8PubKey)+1);
	CHECK_RET_VAL_P(ret == 0, -RSA_PK_PARSE_ERROR,"mbedtls_pk_parse_public_key failed(-0x%0x)!\n", -ret);

    mbedtls_rsa_init(&rsa, MBEDTLS_RSA_PKCS_V15, 0);
    mbedtls_ctr_drbg_init(&ctr_drbg);      //初始化ctr drbg结构体,用于随机数的生成
    mbedtls_entropy_init(&entropy);       //初始化熵源
    
    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char*)pers, strlen(pers));  
	CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_DRBG_SEED_ERROR,"mbedtls_ctr_drbg_seed failed(-0x%0x)!\n", -ret);

	rsa = *(mbedtls_rsa_context*)ctx_pk.pk_ctx;
	ret = mbedtls_rsa_pkcs1_encrypt(&rsa, mbedtls_ctr_drbg_random, &ctr_drbg, MBEDTLS_RSA_PUBLIC, u16Slen, pu8Sdata, pu8Ddata);
	CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_PKCS1_ENCRYPT_ERROR,"mbedtls_rsa_pkcs1_encrypt failed(-0x%0x)!\n", -ret);

	*pu16Dlen = rsa.len;
exit:
	/*释放资源*/
	mbedtls_ctr_drbg_free(&ctr_drbg);
	mbedtls_entropy_free(&entropy);
	mbedtls_rsa_free(&rsa);

	return ret;
}


/*brief    RSA解密
 *param ： pu8PrivKey：私钥存储地址
 *param ： pu8Sdata  ：待解密数据存储地址
 *param ： u16Slen   ：待解密数据长度
 *param ： pu8Ddata  ：解密数据存储地址
 *param ： pu16Dlen  ：解密数据存储长度
 *return： 成功返回0  失败返回错误值
 */
__EX_API__ int wow_rsa_decrypt_by_str(const char* pu8PrivKey,uint8_t* pu8Sdata,uint16_t u16Slen,uint8_t* pu8Ddata,uint16_t* pu16Dlen)
{
	CHECK_RET_VAL_P(pu8PrivKey && pu8Sdata && pu8Ddata && pu16Dlen,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(u16Slen > 0,-PARAM_INPUT_SIZE_IS_ZERO,"param input size invalid!\n");
	
	int ret = 0;
	size_t i;
	const char* pers = "rsa_decrypt";
	mbedtls_pk_context ctx_pk;
    mbedtls_rsa_context rsa;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
	
	mbedtls_pk_init(&ctx_pk);
	ret = mbedtls_pk_parse_key(&ctx_pk,  (uint8_t*)pu8PrivKey, strlen(pu8PrivKey)+1, NULL, 0);
	CHECK_RET_VAL_P(ret == 0, -RSA_PK_PARSE_ERROR,"mbedtls_pk_parse_key failed(-0x%0x)!\n", -ret);
	
	mbedtls_rsa_init(&rsa, MBEDTLS_RSA_PKCS_V15, 0);
	mbedtls_ctr_drbg_init(&ctr_drbg);
	mbedtls_entropy_init(&entropy);


    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char*)pers, strlen(pers));  
	CHECK_RET_GOTO_P(ret == 0, exit, " mbedtls_ctr_drbg_seed failed(-0x%0x)!\n", -ret);

	rsa = *(mbedtls_rsa_context*)ctx_pk.pk_ctx;	

	ret = mbedtls_rsa_pkcs1_decrypt(&rsa, mbedtls_ctr_drbg_random, &ctr_drbg, MBEDTLS_RSA_PRIVATE, &i, 
										pu8Sdata, pu8Ddata, 1024);
	CHECK_RET_GOTO_P(ret == 0, exit, "mbedtls_rsa_pkcs1_decrypt failed(-0x%0x)!\n", -ret);

	*pu16Dlen = rsa.len;

exit:
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
    mbedtls_rsa_free(&rsa);
	return 0;
}


/*brief    RSA签名
 *param ： pu8PubKey ：私钥存储地址
 *param ： pcSrcFiile  ：原文件
 *param ： pcSignFile  ：签名文件
 *return： 成功返回0  失败返回错误值
 */
__EX_API__ int wow_rsa_sign_by_str(const char* pu8PrivKey,const char* pcSrcFiile,const char* pcSignFile)
{
	FILE *f;
    int ret = 0;
    unsigned char hash[32];
    unsigned char buf[MBEDTLS_MPI_MAX_SIZE];
	const char* pers = "rsa_sign";
	mbedtls_rsa_context rsa;
	mbedtls_pk_context ctx_pk;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;

	CHECK_RET_VAL_P(pu8PrivKey && pcSrcFiile && pcSignFile,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	
	mbedtls_pk_init(&ctx_pk);
	ret = mbedtls_pk_parse_key(&ctx_pk, (uint8_t*)pu8PrivKey, strlen(pu8PrivKey)+1, NULL, 0);
	CHECK_RET_VAL_P(ret == 0, -RSA_PK_PARSE_ERROR, "mbedtls_pk_parse_public_key failed(-0x%0x)!\n", -ret);
	
	mbedtls_rsa_init(&rsa, MBEDTLS_RSA_PKCS_V15, 0);
	mbedtls_ctr_drbg_init(&ctr_drbg);
	mbedtls_entropy_init(&entropy);

    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char*)pers, strlen(pers));  
	CHECK_RET_GOTO_EXE_P(ret == 0, exit,ret = -RSA_DRBG_SEED_ERROR, "mbedtls_ctr_drbg_seed failed(-0x%0x)!\n", ret);

	rsa = *(mbedtls_rsa_context*)ctx_pk.pk_ctx;	

	ret = mbedtls_rsa_check_privkey( &rsa );
    CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_CHECK_PRIVKEY_ERROR, "mbedtls_rsa_check_privkey failed(-0x%0x)!\n",-ret);

    /* Compute the SHA-256 hash of the input file,then calculate the RSA signature of the hash.*/
	ret = mbedtls_md_file(mbedtls_md_info_from_type( MBEDTLS_MD_SHA256 ),pcSrcFiile, hash);
    CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_MD_FILE_ERROR, "Could not open or read %s (-0x%0x)\n",pcSrcFiile,-ret);

    ret = mbedtls_rsa_pkcs1_sign(&rsa,NULL,NULL,MBEDTLS_RSA_PRIVATE, MBEDTLS_MD_SHA256,20, hash, buf);
	CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_PKCS1_SIGN_ERROR, "mbedtls_rsa_pkcs1_sign failed(-0x%0x)!\n\n",-ret );


	f = fopen(pcSignFile, "wb+");
    CHECK_RET_GOTO_EXE_P(f, exit,  ret = -RSA_DEST_FILE_INVALID, "create %s failed!\n", pcSignFile);

	fseek(f, 0L, SEEK_END);
	for(int i = 0; i < rsa.len; i++ )
        fprintf( f, "%02X%s", buf[i],( i + 1 ) % 16 == 0 ? "\r\n" : " " );

    fclose( f );


exit:
    mbedtls_rsa_free( &rsa );
    return ret;	
}


/*brief    RSA验证
 *param ： pu8PrivKey：公钥存储地址
 *param ： pcSrcFiile  ：原文件
 *param ： pcSignFile  ：签名文件
 *return： 成功返回0  失败返回错误值
 */
__EX_API__ int wow_rsa_verify_by_str(const char* pu8PubKey,const char* pcSrcFiile,const char* pcSignFile)
{
	int i = 0;
	int ret = 0;
	uint32_t c = 0;
	FILE *f = NULL;
    unsigned char hash[32];
    unsigned char buf[MBEDTLS_MPI_MAX_SIZE];
    const char* pers = "rsa_verify";
	mbedtls_rsa_context rsa;   
    mbedtls_pk_context ctx_pk;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;

	CHECK_RET_VAL_P(pu8PubKey && pcSrcFiile && pcSignFile,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

	mbedtls_pk_init(&ctx_pk);
	ret = mbedtls_pk_parse_public_key(&ctx_pk, (uint8_t*)pu8PubKey, strlen(pu8PubKey)+1);
	CHECK_RET_VAL_P(ret == 0, -RSA_PK_PARSE_ERROR, "mbedtls_pk_parse_public_key failed(-0x%0x)!\n", -ret);

    mbedtls_rsa_init(&rsa, MBEDTLS_RSA_PKCS_V15, 0);
    mbedtls_ctr_drbg_init(&ctr_drbg);      //初始化ctr drbg结构体,用于随机数的生成
    mbedtls_entropy_init(&entropy);       //初始化熵源
    
    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char*)pers, strlen(pers));  
	CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_DRBG_SEED_ERROR, "mbedtls_ctr_drbg_seed failed(-0x%0x)!\n", -ret);

	rsa = *(mbedtls_rsa_context*)ctx_pk.pk_ctx;

    /*  Extract the RSA signature from the text file*/
    f = fopen(pcSignFile, "rb" );
    CHECK_RET_GOTO_EXE_P( f, exit, ret = -RSA_DEST_FILE_INVALID, "open %s failed!\n", pcSignFile );

    while( fscanf( f, "%02X", &c ) > 0 && i < (int) sizeof( buf ) ){
            buf[i++] =  c;
    } 
    fclose( f );
    CHECK_RET_GOTO_EXE_P(i == rsa.len,exit, ret = -RSA_DEST_FILE_INVALID, "Invalid RSA signature format!\n");

    /* Compute the SHA-256 hash of the input file and verify the signature */
    ret = mbedtls_md_file(mbedtls_md_info_from_type( MBEDTLS_MD_SHA256 ),pcSrcFiile, hash );
    CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_MD_FILE_ERROR, "Could not open or read %s (-0x%0x)!\n", pcSrcFiile,-ret);

    ret = mbedtls_rsa_pkcs1_verify( &rsa, NULL, NULL, MBEDTLS_RSA_PUBLIC,MBEDTLS_MD_SHA256, 20, hash, buf );
    CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_PKCS1_VERIFY_ERROR, "mbedtls_rsa_pkcs1_verify failed(-0x%0x)!\n", -ret);

exit:
    mbedtls_rsa_free( &rsa );
    return ret;	
}


/*brief    生成RSA密钥对文件
 *param ： pcPubFile   ：公钥文件地址
 *param ： pcPrivFile  ：私钥文件地址
 *return： 成功返回0  失败返回错误值
 */
__EX_API__ int wow_rsa_generate_by_file(const char* pcPubFile,const char* pcPrivFile)
{
	int ret = 0;
	FILE* fpub = NULL;
	FILE* fpriv = NULL;
	mbedtls_rsa_context rsa;
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_mpi N, P, Q, D, E, DP, DQ, QP;
	const char *pers = "rsa_genkey";
	
	CHECK_RET_VAL_P(pcPubFile && pcPubFile,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

	mbedtls_ctr_drbg_init( &ctr_drbg );
	mbedtls_rsa_init( &rsa, MBEDTLS_RSA_PKCS_V15, 0 );
	mbedtls_mpi_init( &N ); mbedtls_mpi_init( &P ); mbedtls_mpi_init( &Q );
	mbedtls_mpi_init( &D ); mbedtls_mpi_init( &E ); mbedtls_mpi_init( &DP );
	mbedtls_mpi_init( &DQ ); mbedtls_mpi_init( &QP );
	

	mbedtls_entropy_init( &entropy );
	ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
								   (const unsigned char *) pers,strlen( pers ));
	CHECK_RET_GOTO_P(ret == 0,exit, "mbedtls_ctr_drbg_seed failed! returned %d\n", ret);

	ret = mbedtls_rsa_gen_key( &rsa, mbedtls_ctr_drbg_random, &ctr_drbg, KEY_SIZE,EXPONENT);
	CHECK_RET_GOTO_P(ret == 0,exit, "mbedtls_rsa_gen_key failed! returned %d\n", ret);
	
	ret = mbedtls_rsa_export( &rsa, &N, &P, &Q, &D, &E );
	CHECK_RET_GOTO_P(ret == 0,exit, "could not export RSA parameters failed! \n\n" );
	ret = mbedtls_rsa_export_crt( &rsa, &DP, &DQ, &QP );
	CHECK_RET_GOTO_P(ret == 0,exit, "could not export RSA parameters failed!\n\n" );

	fpub = fopen(pcPubFile, "wb+" );
	CHECK_RET_GOTO_EXE_P(fpub,exit,ret = -RSA_KEY_FILE_INVALID,"could not open %s for writing\n",pcPubFile);

	ret = mbedtls_mpi_write_file( "N = ", &N, 16, fpub );
	CHECK_RET_GOTO_EXE_P(fpub,exit,ret = -RSA_MSI_WRITE_ERROR,"mbedtls_mpi_write_file failed(-0x%0x)!\n",-ret);
	ret = mbedtls_mpi_write_file( "E = ", &E, 16, fpub );
	CHECK_RET_GOTO_EXE_P(fpub,exit,ret = -RSA_MSI_WRITE_ERROR,"mbedtls_mpi_write_file failed(-0x%0x)!\n",-ret);


	fpriv = fopen(pcPrivFile, "wb+" );
	CHECK_RET_GOTO_EXE_P(fpub,exit,ret = -RSA_KEY_FILE_INVALID,"could not open %s for writing\n",pcPrivFile);

    ret = mbedtls_mpi_write_file( "N = " , &N , 16, fpriv );
	CHECK_RET_GOTO_EXE_P(fpub,exit,ret = -RSA_MSI_WRITE_ERROR,"mbedtls_mpi_write_file failed(-0x%0x)!\n",-ret);
    ret = mbedtls_mpi_write_file( "E = " , &E , 16, fpriv );
	CHECK_RET_GOTO_EXE_P(fpub,exit,ret = -RSA_MSI_WRITE_ERROR,"mbedtls_mpi_write_file failed(-0x%0x)!\n",-ret);
    ret = mbedtls_mpi_write_file( "D = " , &D , 16, fpriv );
	CHECK_RET_GOTO_EXE_P(fpub,exit,ret = -RSA_MSI_WRITE_ERROR,"mbedtls_mpi_write_file failed(-0x%0x)!\n",-ret);
    ret = mbedtls_mpi_write_file( "P = " , &P , 16, fpriv );
	CHECK_RET_GOTO_EXE_P(fpub,exit,ret = -RSA_MSI_WRITE_ERROR,"mbedtls_mpi_write_file failed(-0x%0x)!\n",-ret);
    ret = mbedtls_mpi_write_file( "Q = " , &Q , 16, fpriv );
	CHECK_RET_GOTO_EXE_P(fpub,exit,ret = -RSA_MSI_WRITE_ERROR,"mbedtls_mpi_write_file failed(-0x%0x)!\n",-ret);
    ret = mbedtls_mpi_write_file( "DP = ", &DP, 16, fpriv );
	CHECK_RET_GOTO_EXE_P(fpub,exit,ret = -RSA_MSI_WRITE_ERROR,"mbedtls_mpi_write_file failed(-0x%0x)!\n",-ret);
    ret = mbedtls_mpi_write_file( "DQ = ", &DQ, 16, fpriv );
	CHECK_RET_GOTO_EXE_P(fpub,exit,ret = -RSA_MSI_WRITE_ERROR,"mbedtls_mpi_write_file failed(-0x%0x)!\n",-ret);
    ret = mbedtls_mpi_write_file( "QP = ", &QP, 16, fpriv );
	CHECK_RET_GOTO_EXE_P(fpub,exit,ret = -RSA_MSI_WRITE_ERROR,"mbedtls_mpi_write_file failed(-0x%0x)!\n",-ret);


exit:

    if( fpub  != NULL ) fclose( fpub );
    if( fpriv != NULL ) fclose( fpriv );
	mbedtls_mpi_free( &N ); mbedtls_mpi_free( &P ); mbedtls_mpi_free( &Q );
	mbedtls_mpi_free( &D ); mbedtls_mpi_free( &E ); mbedtls_mpi_free( &DP );
	mbedtls_mpi_free( &DQ ); mbedtls_mpi_free( &QP );
	mbedtls_rsa_free( &rsa );
	mbedtls_ctr_drbg_free( &ctr_drbg );
	mbedtls_entropy_free( &entropy );

	return ret;
}


/*brief    RSA加密
 *param ： pcPubFile ：公钥文件地址
 *param ： pu8Sdata  ：待加密数据存储地址
 *param ： u16Slen   ：待加密数据长度
 *param ： pu8Ddata  ：加密数据存储地址
 *param ： pu16Dlen  ：加密数据存储长度
 *return： 成功返回0  失败返回错误值
 */
__EX_API__ int wow_rsa_encrypt_by_file(const char* pcPubFile,uint8_t* pu8Sdata,uint16_t u16Slen,uint8_t* pu8Ddata,uint16_t* pu16Dlen)
{
    FILE *f;
    int ret = 0;
    mbedtls_rsa_context rsa;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    const char *pers = "rsa_encrypt";
    mbedtls_mpi N, E;

    mbedtls_mpi_init( &N ); mbedtls_mpi_init( &E );
    mbedtls_rsa_init( &rsa, MBEDTLS_RSA_PKCS_V15, 0 );
    mbedtls_ctr_drbg_init( &ctr_drbg );
    mbedtls_entropy_init( &entropy );

    ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func,&entropy, (const unsigned char *) pers,strlen( pers ) );
	CHECK_RET_GOTO_P(ret == 0,exit, "mbedtls_ctr_drbg_seed failed(-0x%0x)!\n", -ret);

    f = fopen(pcPubFile , "rb");
    CHECK_RET_VAL_P( f, -RSA_KEY_FILE_INVALID, "open %s failed!\n", pcPubFile);

    mbedtls_rsa_init( &rsa, MBEDTLS_RSA_PKCS_V15, 0 );
    ret = mbedtls_mpi_read_file( &rsa.N, 16, f );
    CHECK_RET_GOTO_EXE_P( ret == 0, exit, ret = -RSA_MSI_READ_ERROR, "mbedtls_mpi_read_file N failed(-0x%0x)!\n",-ret );
    ret = mbedtls_mpi_read_file( &rsa.E, 16, f );
    CHECK_RET_GOTO_EXE_P( ret == 0, exit, ret = -RSA_MSI_READ_ERROR, "mbedtls_mpi_read_file E failed(-0x%0x)!\n",-ret );

    fclose( f );

    ret = mbedtls_rsa_import( &rsa, &N, NULL, NULL, NULL, &E );
    CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_IMPORT_ERROR, "mbedtls_rsa_import failed(-0x%0x)!\n",-ret);

    ret = mbedtls_rsa_pkcs1_encrypt( &rsa, mbedtls_ctr_drbg_random,&ctr_drbg, MBEDTLS_RSA_PUBLIC,u16Slen,pu8Sdata, pu8Ddata );
    CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_PKCS1_ENCRYPT_ERROR,"mbedtls_rsa_pkcs1_encrypt failed(-0x%0x)!\n", -ret);


exit:
    mbedtls_mpi_free( &N ); mbedtls_mpi_free( &E );
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );
    mbedtls_rsa_free( &rsa );
    return ret;
}

/*brief    RSA解密
 *param ： pcPrivFile：私钥文件地址
 *param ： pu8Sdata  ：待解密数据存储地址
 *param ： u16Slen   ：待解密数据长度
 *param ： pu8Ddata  ：解密数据存储地址
 *param ： pu16Dlen  ：解密数据存储长度
 *return： 成功返回0  失败返回错误值
 */
__EX_API__ int wow_rsa_decrypt_by_file(const char* pcPrivFile,uint8_t* pu8Sdata,uint16_t u16Slen,uint8_t* pu8Ddata,uint16_t* pu16Dlen)
{
	FILE *f;
    int ret = 1;
    mbedtls_rsa_context rsa;
    mbedtls_mpi N, P, Q, D, E, DP, DQ, QP;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    const char *pers = "rsa_decrypt";


    mbedtls_rsa_init( &rsa, MBEDTLS_RSA_PKCS_V15, 0 );
    mbedtls_ctr_drbg_init( &ctr_drbg );
    mbedtls_entropy_init( &entropy );
    mbedtls_mpi_init( &N ); mbedtls_mpi_init( &P ); mbedtls_mpi_init( &Q );
    mbedtls_mpi_init( &D ); mbedtls_mpi_init( &E ); mbedtls_mpi_init( &DP );
    mbedtls_mpi_init( &DQ ); mbedtls_mpi_init( &QP );

    ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *) pers,strlen( pers ) );
 	CHECK_RET_GOTO_P(ret == 0,exit, "mbedtls_ctr_drbg_seed failed(-0x%0x)!\n", -ret);

 	f = fopen(pcPrivFile, "rb" );
	CHECK_RET_VAL_P(f,-RSA_KEY_FILE_INVALID,"open %s failed!\n",pcPrivFile);

    mbedtls_rsa_init( &rsa, MBEDTLS_RSA_PKCS_V15, 0 );
    mbedtls_mpi_init( &N ); 
	mbedtls_mpi_init( &P );
	mbedtls_mpi_init( &Q );
    mbedtls_mpi_init( &D ); 
	mbedtls_mpi_init( &E ); 
	mbedtls_mpi_init( &DP );
    mbedtls_mpi_init( &DQ ); 
	mbedtls_mpi_init( &QP );

	ret = mbedtls_mpi_read_file( &N , 16, f );
    CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_MSI_READ_ERROR, "mbedtls_mpi_read_file N failed(-0x%0x)!\n",-ret);
	ret = mbedtls_mpi_read_file( &E , 16, f );
    CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_MSI_READ_ERROR, "mbedtls_mpi_read_file E failed(-0x%0x)!\n",-ret);
	ret = mbedtls_mpi_read_file( &D , 16, f );
    CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_MSI_READ_ERROR, "mbedtls_mpi_read_file D failed(-0x%0x)!\n",-ret);
	ret = mbedtls_mpi_read_file( &P , 16, f );
    CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_MSI_READ_ERROR, "mbedtls_mpi_read_file P failed(-0x%0x)!\n",-ret);
	ret = mbedtls_mpi_read_file( &Q , 16, f );
    CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_MSI_READ_ERROR, "mbedtls_mpi_read_file Q failed(-0x%0x)!\n",-ret);
	ret = mbedtls_mpi_read_file( &DP , 16, f );
    CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_MSI_READ_ERROR, "mbedtls_mpi_read_file DP failed(-0x%0x)!\n",-ret);
	ret = mbedtls_mpi_read_file( &DQ , 16, f );
    CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_MSI_READ_ERROR, "mbedtls_mpi_read_file DQ failed(-0x%0x)!\n",-ret);
	ret = mbedtls_mpi_read_file( &QP , 16, f );
    CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_MSI_READ_ERROR, "mbedtls_mpi_read_file QP failed(-0x%0x)!\n",-ret);

    fclose(f);

	ret =  mbedtls_rsa_import( &rsa, &N, &P, &Q, &D, &E );
    CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_IMPORT_ERROR, "mbedtls_rsa_import failed(-0x%0x)!\n",-ret);

	ret = mbedtls_rsa_complete( &rsa );
	CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_COMPLETE_ERROR, "mbedtls_rsa_complete failed(-0x%0x)!\n",-ret);

	ret = mbedtls_rsa_check_privkey( &rsa );
    CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_CHECK_PRIVKEY_ERROR, "mbedtls_rsa_check_privkey failed(-0x%0x)!\n",-ret);
    

    ret = mbedtls_rsa_pkcs1_decrypt( &rsa, mbedtls_ctr_drbg_random,&ctr_drbg, MBEDTLS_RSA_PRIVATE, (size_t*)pu16Dlen,
                                            pu8Sdata, pu8Ddata, (size_t)u16Slen );
    CHECK_RET_GOTO_P(ret == 0, exit, "mbedtls_rsa_pkcs1_decrypt failed(-0x%0x)!\n", -ret);

exit:
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );
    mbedtls_rsa_free( &rsa );
    mbedtls_mpi_free( &N ); mbedtls_mpi_free( &P ); mbedtls_mpi_free( &Q );
    mbedtls_mpi_free( &D ); mbedtls_mpi_free( &E ); mbedtls_mpi_free( &DP );
    mbedtls_mpi_free( &DQ ); mbedtls_mpi_free( &QP );

    return ret;

}

/*brief    RSA签名
 *param ： pcPrivFile：私钥文件地址
 *param ： pcSrcFiile  ：原文件
 *param ： pcSignFile  ：签名文件
 *return： 成功返回0  失败返回错误值
 */
__EX_API__ int wow_rsa_sign_by_file(const char* pcPrivFile,const char* pcSrcFiile,const char* pcSignFile)
{
	FILE *f;
    int ret = 0;
    mbedtls_rsa_context rsa;
    unsigned char hash[32];
    unsigned char buf[MBEDTLS_MPI_MAX_SIZE];
    mbedtls_mpi N, P, Q, D, E, DP, DQ, QP;

	CHECK_RET_VAL_P(pcPrivFile && pcSrcFiile && pcSignFile,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(wow_file_check_exist(pcPrivFile),-RSA_KEY_FILE_INVALID,"not found %s file!\n",pcPrivFile);

	f = fopen(pcPrivFile, "rb" );
	CHECK_RET_VAL_P(f,-RSA_KEY_FILE_INVALID,"open %s failed!\n",pcPrivFile);

    mbedtls_rsa_init( &rsa, MBEDTLS_RSA_PKCS_V15, 0 );
    mbedtls_mpi_init( &N ); 
	mbedtls_mpi_init( &P );
	mbedtls_mpi_init( &Q );
    mbedtls_mpi_init( &D ); 
	mbedtls_mpi_init( &E ); 
	mbedtls_mpi_init( &DP );
    mbedtls_mpi_init( &DQ ); 
	mbedtls_mpi_init( &QP );

	ret = mbedtls_mpi_read_file( &N , 16, f );
    CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_MSI_READ_ERROR, "mbedtls_mpi_read_file N failed(-0x%0x)!\n",-ret);
	ret = mbedtls_mpi_read_file( &E , 16, f );
    CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_MSI_READ_ERROR, "mbedtls_mpi_read_file E failed(-0x%0x)!\n",-ret);
	ret = mbedtls_mpi_read_file( &D , 16, f );
    CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_MSI_READ_ERROR, "mbedtls_mpi_read_file D failed(-0x%0x)!\n",-ret);
	ret = mbedtls_mpi_read_file( &P , 16, f );
    CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_MSI_READ_ERROR, "mbedtls_mpi_read_file P failed(-0x%0x)!\n",-ret);
	ret = mbedtls_mpi_read_file( &Q , 16, f );
    CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_MSI_READ_ERROR, "mbedtls_mpi_read_file Q failed(-0x%0x)!\n",-ret);
	ret = mbedtls_mpi_read_file( &DP , 16, f );
    CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_MSI_READ_ERROR, "mbedtls_mpi_read_file DP failed(-0x%0x)!\n",-ret);
	ret = mbedtls_mpi_read_file( &DQ , 16, f );
    CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_MSI_READ_ERROR, "mbedtls_mpi_read_file DQ failed(-0x%0x)!\n",-ret);
	ret = mbedtls_mpi_read_file( &QP , 16, f );
    CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_MSI_READ_ERROR, "mbedtls_mpi_read_file QP failed(-0x%0x)!\n",-ret);

    fclose(f);

	ret =  mbedtls_rsa_import( &rsa, &N, &P, &Q, &D, &E );
    CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_IMPORT_ERROR, "mbedtls_rsa_import failed(-0x%0x)!\n",-ret);

	ret = mbedtls_rsa_complete( &rsa );
	CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_COMPLETE_ERROR, "mbedtls_rsa_complete failed(-0x%0x)!\n",-ret);

	ret = mbedtls_rsa_check_privkey( &rsa );
    CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_CHECK_PRIVKEY_ERROR, "mbedtls_rsa_check_privkey failed(-0x%0x)!\n",-ret);

    /* Compute the SHA-256 hash of the input file,then calculate the RSA signature of the hash.*/
	ret = mbedtls_md_file(mbedtls_md_info_from_type( MBEDTLS_MD_SHA256 ),pcSrcFiile, hash);
    CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_MD_FILE_ERROR, "Could not open or read failed(-0x%0x)!\n",-ret);

    ret = mbedtls_rsa_pkcs1_sign(&rsa,NULL,NULL,MBEDTLS_RSA_PRIVATE, MBEDTLS_MD_SHA256,20, hash, buf);
	CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_PKCS1_SIGN_ERROR, "mbedtls_rsa_pkcs1_sign failed(-0x%0x)!\n\n", -ret );


	f = fopen(pcSignFile, "wb+");
    CHECK_RET_GOTO_EXE_P(f, exit,  ret = -RSA_DEST_FILE_INVALID, "create %s failed!\n", pcSignFile);

	fseek(f, 0L, SEEK_END);
	for(int i = 0; i < rsa.len; i++ )
        fprintf( f, "%02X%s", buf[i],( i + 1 ) % 16 == 0 ? "\r\n" : " " );

    fclose( f );


exit:
    mbedtls_rsa_free( &rsa );
    mbedtls_mpi_free( &N ); 
	mbedtls_mpi_free( &P ); 
	mbedtls_mpi_free( &Q );
    mbedtls_mpi_free( &D ); 
	mbedtls_mpi_free( &E ); 
	mbedtls_mpi_free( &DP );
    mbedtls_mpi_free( &DQ ); 
	mbedtls_mpi_free( &QP );

    return ret;
}

/*brief    RSA验证
 *param ： pcPubFile ：公钥文件地址
 *param ： pcSrcFiile  ：原文件
 *param ： pcSignFile  ：签名文件
 *return： 成功返回0  失败返回错误值
 */
__EX_API__ int wow_rsa_verify_by_file(const char* pcPubFile,const char* pcSrcFiile,const char* pcSignFile)
{
	int i = 0;
	int ret = 0;
	uint32_t c = 0;
	FILE *f = NULL;
    mbedtls_rsa_context rsa;
    unsigned char hash[32];
    unsigned char buf[MBEDTLS_MPI_MAX_SIZE];

	CHECK_RET_VAL_P(pcPubFile && pcSrcFiile && pcSignFile,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(wow_file_check_exist(pcPubFile),-RSA_KEY_FILE_INVALID,"not found %s file!\n",pcPubFile);

    f = fopen(pcPubFile , "rb");
    CHECK_RET_VAL_P( f, -RSA_KEY_FILE_INVALID, "open %s failed!\n", pcPubFile);

    mbedtls_rsa_init( &rsa, MBEDTLS_RSA_PKCS_V15, 0 );
    ret = mbedtls_mpi_read_file( &rsa.N, 16, f );
    CHECK_RET_GOTO_EXE_P( ret == 0, exit, ret = -RSA_MSI_READ_ERROR, "mbedtls_mpi_read_file N failed(-0x%0x)!\n",-ret );
    ret = mbedtls_mpi_read_file( &rsa.E, 16, f );
    CHECK_RET_GOTO_EXE_P( ret == 0, exit, ret = -RSA_MSI_READ_ERROR, "mbedtls_mpi_read_file E failed(-0x%0x)!\n",-ret );

    rsa.len = ( mbedtls_mpi_bitlen( &rsa.N ) + 7 ) >> 3;

    fclose( f );

    /*  Extract the RSA signature from the text file*/
    f = fopen(pcSignFile, "rb" );
    CHECK_RET_GOTO_EXE_P( f, exit, ret = -RSA_DEST_FILE_INVALID, "open %s failed!\n", pcSignFile );

    while( fscanf( f, "%02X", &c ) > 0 && i < (int) sizeof( buf ) ){
            buf[i++] = (unsigned char) c;
    } 
    fclose( f );
    CHECK_RET_GOTO_EXE_P(i == rsa.len,exit, ret = -RSA_DEST_FILE_INVALID, "Invalid RSA signature format!\n");


    /* Compute the SHA-256 hash of the input file and verify the signature */
    ret = mbedtls_md_file(mbedtls_md_info_from_type( MBEDTLS_MD_SHA256 ),pcSrcFiile, hash );
    CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_MD_FILE_ERROR, "Could not open or read failed(-0x%0x)!\n", -ret);

    ret = mbedtls_rsa_pkcs1_verify( &rsa, NULL, NULL, MBEDTLS_RSA_PUBLIC,MBEDTLS_MD_SHA256, 20, hash, buf );
    CHECK_RET_GOTO_EXE_P(ret == 0, exit, ret = -RSA_PKCS1_VERIFY_ERROR, "mbedtls_rsa_pkcs1_verify failed(-0x%0x)!\n", -ret);

exit:
    mbedtls_rsa_free( &rsa );
    return ret;
}