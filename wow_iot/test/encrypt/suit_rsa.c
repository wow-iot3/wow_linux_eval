
#define _GNU_SOURCE
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <libgen.h>
#include <stdlib.h>

#include "greatest/greatest.h"
#include "wow_test_config.h"

#include "prefix/wow_common.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_check.h"

#include "utils/wow_hex.h"
#include "utils/wow_str.h"
#include "encrypt/wow_rsa.h"
#include "plugin/wow_debug.h"

#define MOD_TAG "rsa"

static char gc_src_file[MAX_PATH_SIZE] = {0};
static char gc_sign_file[MAX_PATH_SIZE] = {0};
static char gc_pub_file[MAX_PATH_SIZE] = {0};
static char gc_priv_file[MAX_PATH_SIZE] = {0};

int get_test_file_name(void)
{
    char dpath[MAX_PATH_SIZE] = {0};

    int len = readlink("/proc/self/exe", dpath, MAX_PATH_SIZE - 1);
    CHECK_RET_VAL_P(len > 1,-1,"realink path failed!\n");
    dpath[len] = '\0';

    ///< 提取目录部分
    char *dir = dirname(dpath);
	char* libiot = wow_str_combine(2,dir,"/libwowiot.so");
	char* libiotsign = wow_str_combine(2,dir,"/libwowiot.so.sig");
	memcpy(gc_src_file,libiot,strlen(libiot));
	memcpy(gc_sign_file,libiotsign,strlen(libiotsign));

    wow_str_combine_free(&libiot);
	wow_str_combine_free(&libiotsign);

    return 0;
}

int get_key_file_name(void)
{
    char dpath[MAX_PATH_SIZE] = {0};

    int len = readlink("/proc/self/exe", dpath, MAX_PATH_SIZE - 1);
    CHECK_RET_VAL_P(len > 1,-1,"realink path failed!\n");
    dpath[len] = '\0';

    ///< 提取目录部分
    char *dir = dirname(dpath);
	char* pub = wow_str_combine(2,dir,"/public.txt");
	char* priv = wow_str_combine(2,dir,"/private.txt");
	memcpy(gc_pub_file,pub,strlen(pub));
	memcpy(gc_priv_file,priv,strlen(priv));

    wow_str_combine_free(&pub);
	wow_str_combine_free(&priv);

    return 0;
}

TEST test_rsa_str_test(void)
{
	ModDebug_I(MOD_TAG, "suit_rsa-------------------test_rsa_str_test\n");

	int ret = 0;
	uint8_t public_key[1024];
    uint8_t private_key[4096];
	uint8_t src[16] = "10123456789";
    uint8_t dest[512];
	uint8_t buffer[512];
	uint16_t dest_len = 0;
	uint16_t src_len = 0;
	ret = wow_rsa_generate_by_str(public_key,sizeof(public_key)-1,
							private_key,sizeof(private_key)-1);
	GREATEST_ASSERT(ret == 0);
	
    //printf("%s",public_key);
    //printf("%s",private_key);

	ret = wow_rsa_encrypt_by_str((const char*)public_key,src,strlen((const char*)src),dest,&dest_len);
	GREATEST_ASSERT(ret == 0);

	ret = wow_rsa_decrypt_by_str((const char*)private_key,dest,dest_len,buffer,&src_len);
	GREATEST_ASSERT(ret == 0);
	ret = STRING_EQUAL_N((char*)buffer,(char*)src,src_len);
	GREATEST_ASSERT(ret == 1);

	ret = wow_rsa_sign_by_str((const char*)private_key,gc_src_file,gc_sign_file);
	GREATEST_ASSERT(ret == 0);

	ret = wow_rsa_verify_by_str((const char*)public_key,gc_src_file,gc_sign_file);
	GREATEST_ASSERT(ret == 0);

	PASS();
}

TEST test_rsa_file_test(void)
{
	ModDebug_I(MOD_TAG, "suit_rsa-------------------test_rsa_file_test\n");

	int ret = 0;
	uint8_t src[16] = "10123456789";
    uint8_t dest[512];
	uint8_t buffer[512];
	uint16_t dest_len = 0;
	uint16_t src_len = 0;

	
	ret = wow_rsa_generate_by_file(gc_pub_file,gc_priv_file);
	GREATEST_ASSERT(ret == 0);

	ret = wow_rsa_encrypt_by_file(gc_pub_file,src,strlen((const char*)src),dest,&dest_len);
	GREATEST_ASSERT(ret == 0);

	ret = wow_rsa_decrypt_by_file(gc_priv_file,dest,dest_len,buffer,&src_len);
	GREATEST_ASSERT(ret == 0);
	ret = STRING_EQUAL_N((char*)buffer,(char*)src,src_len);
	GREATEST_ASSERT(ret == 1);	
	
	ret = wow_rsa_sign_by_file(gc_priv_file,gc_src_file,gc_sign_file);
	GREATEST_ASSERT(ret == 0);

	ret = wow_rsa_verify_by_file(gc_pub_file,gc_src_file,gc_sign_file);
	GREATEST_ASSERT(ret == 0);

	PASS();
}

SUITE(suit_rsa)
{
	wow_debug_setStatus(MOD_TAG, SUIT_RUN_DEBUG_FLAG);
	
	get_key_file_name();
	get_test_file_name();

	RUN_TEST(test_rsa_str_test);
	RUN_TEST(test_rsa_file_test);
}

