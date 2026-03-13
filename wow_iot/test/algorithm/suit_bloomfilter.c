#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "greatest/greatest.h"

#include "prefix/wow_keyword.h"
#include "utils/wow_type.h"

#include "algorithm/wow_bloomfilter.h"
#include "hash/wow_hash.h"
#include "plugin/wow_debug.h"

#include "wow_test_config.h"


#define MOD_TAG "bloomfilter"

#if SUIT_RUN_ERROR_TEST
TEST test_bloomfilter_error(void)
{	
	ModDebug_I(MOD_TAG,"suit_bloomfiliter------test_bloomfilter_error\n");

	int ret = 0;
	BloomFilter_T* test2;
	BloomFilter_T* test3;
	BloomFilter_T* test4;

	test2 = wow_bloomfilter_init(TB_BLOOM_FILTER_PROBABILITY_0_01, 11025);
	test3 = wow_bloomfilter_init(TB_BLOOM_FILTER_PROBABILITY_0_001, 21025);
	test4 = wow_bloomfilter_init(TB_BLOOM_FILTER_PROBABILITY_0_0001, 31025);
	GREATEST_ASSERT( test2 &&test3 &&test4);

	ret = wow_bloomfilter_set_1(test2,NULL);
	GREATEST_ASSERT(ret != 0);
	ret = wow_bloomfilter_set_0(test2,NULL);
	GREATEST_ASSERT(ret != 0);
	ret = wow_bloomfilter_get(test2,NULL);
	GREATEST_ASSERT(ret != 0);
	
	wow_bloomfilter_exit(&test2);
	wow_bloomfilter_exit(&test3);
	wow_bloomfilter_exit(&test4);
	
	ret = wow_bloomfilter_set_1(test2,NULL);
	GREATEST_ASSERT(ret != 0);
	ret = wow_bloomfilter_set_0(test2,NULL);
	GREATEST_ASSERT(ret != 0);
	ret = wow_bloomfilter_get(test2,NULL);
	GREATEST_ASSERT(ret != 0);
	
	wow_bloomfilter_exit(&test2);
	wow_bloomfilter_exit(&test3);
	wow_bloomfilter_exit(&test4);
	wow_bloomfilter_exit(&test4);

    PASS();

}
#endif

TEST test_bloomfilter_ptr(void)
{
	int i   = 0;
	int ret = 0;
	BloomFilter_T* test;
	char str[32];
	int  value = 0;

	ModDebug_I(MOD_TAG,"suit_bloomfiliter------test_bloomfilter_ptr\n");
	
	test = wow_bloomfilter_init(TB_BLOOM_FILTER_PROBABILITY_0_001, 100000);
	GREATEST_ASSERT(test);
	wow_bloomfilter_set_1(test,(const void*)"12345");
	
	for(i = 1; i < MAXU8 ; i++){
		value = wow_hash_uint32(HASH_UINT_BJ,i);
		memset(str,0,32);
		S32toStr(value,str,32);	
		
		ret = wow_bloomfilter_set_1(test,str);
		GREATEST_ASSERT(ret == 0);

		ret =  wow_bloomfilter_get(test,str);
		GREATEST_ASSERT(ret == 1);

		ret = wow_bloomfilter_set_0(test,str);
		GREATEST_ASSERT(ret == 0);

		ret =  wow_bloomfilter_get(test,str);
		GREATEST_ASSERT(ret == 0);
	}

	ret = wow_bloomfilter_get(test,"12345");
	GREATEST_ASSERT(ret == 1);
	
	wow_bloomfilter_exit(&test);
	
	PASS();
}


SUITE(suit_bloomfilter)
{
	wow_debug_setStatus(MOD_TAG,SUIT_RUN_DEBUG_FLAG);

#if SUIT_RUN_ERROR_TEST
    RUN_TEST(test_bloomfilter_error);
#endif
	RUN_TEST(test_bloomfilter_ptr);
}





