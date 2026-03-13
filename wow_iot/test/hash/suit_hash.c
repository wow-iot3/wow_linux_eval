#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "greatest/greatest.h"
#include "wow_test_config.h"

#include "prefix/wow_keyword.h"
#include "plugin/wow_debug.h"
#include "hash/wow_hash.h"

#define MOD_TAG "hash"


#define TEST_UINT_SIZE 16

#if SUIT_RUN_ERROR_TEST
TEST test_hash_error(void)
{
	ModDebug_I(MOD_TAG,"suit_hash--------test_hash_error\n");
		
	int ret = 0;
	uint8_t data;

	ret = wow_hash_data(0,NULL,100);
	GREATEST_ASSERT(ret == 0);
	
	ret = wow_hash_data(0,&data,0);
	GREATEST_ASSERT(ret == 0);	
	PASS();
}
#endif



TEST test_hash_uint32(void)
{
	ModDebug_I(MOD_TAG,"suit_hash--------test_hash_uint32\n");

	int i = 0;
	size_t data32[TEST_UINT_SIZE]  = {0};
	
	memset(data32,0,TEST_UINT_SIZE*sizeof(size_t));
	ModDebug_I(MOD_TAG,"1-1-wow_hash_uint32 HASH_UINT_BJ:");
	for(i = 0; i < TEST_UINT_SIZE; i++){
		data32[i] = wow_hash_uint32(HASH_UINT_BJ, i);
		printf("%zu ",data32[i]);
	}
	printf("\n");

	memset(data32,0,TEST_UINT_SIZE*sizeof(size_t));
	ModDebug_I(MOD_TAG,"1-1-wow_hash_uint32 HASH_UINT_BJ:");
	for(i = 0; i < TEST_UINT_SIZE; i++){
		data32[i] = wow_hash_uint32(HASH_UINT_BJ,i);
		printf("%zu ",data32[i]);
	}
	printf("\n");

	PASS();
}
TEST test_hash_data(void)
{
	ModDebug_I(MOD_TAG,"suit_hash--------test_hash_data\n");

	int i = 0;
	char key[4];
	size_t datastr[TEST_UINT_SIZE]  = {0};

	
	memset(datastr,0,TEST_UINT_SIZE*sizeof(size_t));
	ModDebug_I(MOD_TAG,"2-1-wow_hash_data HASH_ARRAY_BKDR:");
	for(i = 0; i < TEST_UINT_SIZE; i++){
		snprintf(key,4,"%d", i);
		datastr[i] = wow_hash_data(HASH_ARRAY_BKDR,(const uint8_t*)key,4);
		printf("%zu ",datastr[i]);
	}
	printf("\n");


	memset(datastr,0,TEST_UINT_SIZE*sizeof(size_t));
	ModDebug_I(MOD_TAG,"2-4-wow_hash_data HASH_ARRAY_RS:");
	for(i = 0; i < TEST_UINT_SIZE; i++){
		snprintf(key,4,"%d", i);
		datastr[i] = wow_hash_data(HASH_ARRAY_RS,(const uint8_t*)key,4);
		printf("%zu ",datastr[i]);
	}
	printf("\n");

	memset(datastr,0,TEST_UINT_SIZE*sizeof(size_t));
	ModDebug_I(MOD_TAG,"2-1-wow_hash_data HASH_ARRAY_FNV1A:");
	for(i = 0; i < TEST_UINT_SIZE; i++){
		snprintf(key,4,"%d", i);
		datastr[i] = wow_hash_data(HASH_ARRAY_FNV1A,(const uint8_t*)key,4);
		printf("%zu ",datastr[i]);
	}
	printf("\n");


	memset(datastr,0,TEST_UINT_SIZE*sizeof(size_t));
	ModDebug_I(MOD_TAG,"2-4-wow_hash_data HASH_ARRAY_DJB2:");
	for(i = 0; i < TEST_UINT_SIZE; i++){
		snprintf(key,4,"%d", i);
		datastr[i] = wow_hash_data(HASH_ARRAY_DJB2,(const uint8_t*)key,4);
		printf("%zu ",datastr[i]);
	}
	printf("\n");

	PASS();
}

SUITE(suit_hash)
{
	wow_debug_setStatus(MOD_TAG,SUIT_RUN_DEBUG_FLAG);

#if SUIT_RUN_ERROR_TEST
	RUN_TEST(test_hash_error);
#endif
    RUN_TEST(test_hash_uint32);

	RUN_TEST(test_hash_data);
}


