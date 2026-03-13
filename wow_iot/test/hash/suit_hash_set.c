#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "utils/wow_type.h"
#include "hash/wow_hash_set.h"

#include "greatest/greatest.h"
#include "wow_test_config.h"

#define MOD_TAG "[hash_set]"

#if SUIT_RUN_ERROR_TEST
TEST test_hash_set_error(void)
{
	printf(MOD_TAG"suit_hash_set-----test_hash_set_error\n");

	int ret = 0;
	HashSet_T* hash_set_1;
	HashSet_T* hash_set_2;
	HashSet_T* hash_set_3;
	HashSet_T* hash_set_4;
	HashSet_T* hash_set_5;

	hash_set_1 = wow_hash_set_create(ELEMENT_TYPE_UINT8);
	GREATEST_ASSERT(hash_set_1);

	hash_set_2 = wow_hash_set_create(ELEMENT_TYPE_UINT16);
	GREATEST_ASSERT(hash_set_2);

	hash_set_3 = wow_hash_set_create(ELEMENT_TYPE_UINT32);
	GREATEST_ASSERT(hash_set_3);
	
	hash_set_4 = wow_hash_set_create(ELEMENT_TYPE_STR);
	GREATEST_ASSERT(hash_set_4);
	
	hash_set_5 = wow_hash_set_create(ELEMENT_TYPE_PTR);
	GREATEST_ASSERT(hash_set_5);

	wow_hash_set_destroy(&hash_set_1);
	wow_hash_set_destroy(&hash_set_2);
	wow_hash_set_destroy(&hash_set_3);
	wow_hash_set_destroy(&hash_set_4);
	wow_hash_set_destroy(&hash_set_5);
	wow_hash_set_destroy(&hash_set_5);


	ret = wow_hash_set_insert(NULL,"AAA");
	GREATEST_ASSERT(ret != 0);
	
	ret = wow_hash_set_get(NULL,"AAA");
	GREATEST_ASSERT(ret != 0);	
	
	wow_hash_set_remove(NULL,"AAA");
	wow_hash_set_clear(NULL);
	wow_hash_set_destroy(NULL);
	
	hash_set_1 = wow_hash_set_create(ELEMENT_TYPE_STR);
	GREATEST_ASSERT(hash_set_1);

	ret = wow_hash_set_insert(hash_set_1,NULL);
	GREATEST_ASSERT(ret != 0);	

	ret = wow_hash_set_get(hash_set_1,NULL);
	GREATEST_ASSERT(ret != 0);	
	wow_hash_set_remove(hash_set_1,NULL);
	wow_hash_set_destroy(&hash_set_1);

	PASS();

}
#endif


TEST test_hash_set_test1(void)
{
	int ret = 0;
	HashSet_T* hash_set_1;

	printf(MOD_TAG"suit_hash_set-----test_hash_set_test1\n");
	
	hash_set_1 = wow_hash_set_create(ELEMENT_TYPE_UINT16);
	GREATEST_ASSERT(hash_set_1);

	ret = wow_hash_set_insert(hash_set_1,u2p(1));
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_set_insert(hash_set_1,u2p(2));
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_set_insert(hash_set_1,u2p(3));
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_set_insert(hash_set_1,u2p(4));
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_set_insert(hash_set_1,u2p(5));
	GREATEST_ASSERT(ret == 0);

	GREATEST_ASSERT(wow_hash_set_get(hash_set_1,u2p(1)) == 0 && wow_hash_set_get(hash_set_1,u2p(2)) == 0 &&
					wow_hash_set_get(hash_set_1,u2p(3)) == 0 && wow_hash_set_get(hash_set_1,u2p(4)) == 0 && 
					wow_hash_set_get(hash_set_1,u2p(5)) == 0 );

	ret = wow_hash_set_size(hash_set_1);
	GREATEST_ASSERT(ret == 5);

	wow_hash_set_remove(hash_set_1,u2p(3));
	ret = wow_hash_set_get(hash_set_1,u2p(3));
	GREATEST_ASSERT(ret == -1);

	ret = wow_hash_set_size(hash_set_1);
	GREATEST_ASSERT(ret == 4);

	wow_hash_set_clear(hash_set_1);
	ret = wow_hash_set_get(hash_set_1,u2p(1));
	GREATEST_ASSERT(ret == -1);

	ret = wow_hash_set_size(hash_set_1);
	GREATEST_ASSERT(ret == 0);

	
	wow_hash_set_destroy(&hash_set_1);
	
	PASS();
}

TEST test_hash_set_test2(void)
{
	int ret = 0;
	HashSet_T* hash_set_1;

	printf(MOD_TAG"suit_hash_set-----test_hash_set_test2\n");
	
	hash_set_1 = wow_hash_set_create(ELEMENT_TYPE_STR);
	GREATEST_ASSERT(hash_set_1);

	ret = wow_hash_set_insert(hash_set_1,"111");
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_set_insert(hash_set_1,"22222");
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_set_insert(hash_set_1,"333333");
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_set_insert(hash_set_1,"4444444");
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_set_insert(hash_set_1,"55555555");
	GREATEST_ASSERT(ret == 0);
	
	GREATEST_ASSERT(wow_hash_set_get(hash_set_1,"111")== 0 && wow_hash_set_get(hash_set_1,"22222")== 0 &&
				    wow_hash_set_get(hash_set_1,"333333")== 0 && wow_hash_set_get(hash_set_1,"4444444")== 0 &&
				    wow_hash_set_get(hash_set_1,"55555555")== 0);


	ret = wow_hash_set_size(hash_set_1);
	GREATEST_ASSERT(ret == 5);

	wow_hash_set_remove(hash_set_1,"333333");
	ret = wow_hash_set_get(hash_set_1,"333333");
	GREATEST_ASSERT(ret == -1);

	ret = wow_hash_set_size(hash_set_1);
	GREATEST_ASSERT(ret == 4);

	wow_hash_set_clear(hash_set_1);
	ret = wow_hash_set_get(hash_set_1,"111");
	GREATEST_ASSERT(ret == -1);
	ret = wow_hash_set_size(hash_set_1);
	GREATEST_ASSERT(ret == 0);

	
	wow_hash_set_destroy(&hash_set_1);
	
	PASS();

}


TEST test_hash_set_test3(void)
{
	int ret = 0;

	char data1[16] = "111";
	char data2[16] = "22222";
	char data3[16] = "333333";
	char data4[16] = "4444444";
	char data5[16] = "55555555";
	HashSet_T* hash_set_1;

	printf(MOD_TAG"suit_hash_set-----test_hash_set_test3\n");
	
	hash_set_1 = wow_hash_set_create(ELEMENT_TYPE_PTR);
	GREATEST_ASSERT(hash_set_1);

	ret = wow_hash_set_insert(hash_set_1,data1);
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_set_insert(hash_set_1,data2);
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_set_insert(hash_set_1,data3);
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_set_insert(hash_set_1,data4);
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_set_insert(hash_set_1,data5);
	GREATEST_ASSERT(ret == 0);
	
	GREATEST_ASSERT(wow_hash_set_get(hash_set_1,data1)== 0 && wow_hash_set_get(hash_set_1,data2)== 0 &&
					wow_hash_set_get(hash_set_1,data3)== 0 && wow_hash_set_get(hash_set_1,data4)== 0 &&
					wow_hash_set_get(hash_set_1,data5)== 0);


	ret = wow_hash_set_size(hash_set_1);
	GREATEST_ASSERT(ret == 5);

	wow_hash_set_remove(hash_set_1,data3);
	ret = wow_hash_set_get(hash_set_1,data3);
	GREATEST_ASSERT(ret == -1);
	ret = wow_hash_set_size(hash_set_1);
	GREATEST_ASSERT(ret == 4);

	wow_hash_set_clear(hash_set_1);
	ret = wow_hash_set_get(hash_set_1,data1);
	GREATEST_ASSERT(ret == -1);
	ret = wow_hash_set_size(hash_set_1);
	GREATEST_ASSERT(ret == 0);

	
	wow_hash_set_destroy(&hash_set_1);
	
	PASS();
}


SUITE(suit_hash_set)
{
#if SUIT_RUN_ERROR_TEST
	RUN_TEST(test_hash_set_error);	
#endif
    RUN_TEST(test_hash_set_test1);
	RUN_TEST(test_hash_set_test2);
	RUN_TEST(test_hash_set_test3);
}



