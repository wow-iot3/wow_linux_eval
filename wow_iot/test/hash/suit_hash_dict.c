#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "greatest/greatest.h"
#include "wow_test_config.h"

#include "prefix/prefix.h"
#include "hash/wow_hash_dict.h"

#define MOD_TAG "[hash_dict]"

#if SUIT_RUN_ERROR_TEST
TEST test_dict_error(void)
{
	int ret = 0;
	char* value = NULL;
	HashDict_T* dict_1 = NULL;
	HashDict_T* dict_2 = NULL;
	HashDict_T* dict_3 = NULL;
	
	printf(MOD_TAG"suit_hash_dict-----test_dict_error\n");
	dict_1 = wow_hash_dict_create();
	GREATEST_ASSERT(dict_1);
	dict_2 = wow_hash_dict_create();
	GREATEST_ASSERT(dict_2);
	dict_3 = wow_hash_dict_create();
	GREATEST_ASSERT(dict_3);
	
	ret =wow_hash_dict_add(dict_1,NULL,u2p(7));
	GREATEST_ASSERT(ret == -1);
	ret =wow_hash_dict_add(dict_1,"test",NULL);
	GREATEST_ASSERT(ret == 0);
	value = wow_hash_dict_get(dict_1,NULL);
	GREATEST_ASSERT(value == NULL);
	ret = wow_hash_dict_del(dict_1,NULL);
	GREATEST_ASSERT(ret == -1);
	ret = wow_hash_dict_add(dict_1,"test2","22222");
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_dict_add(dict_1,"test2","7777");
	GREATEST_ASSERT(ret == -1);

	wow_hash_dict_destroy(&dict_1);
	wow_hash_dict_destroy(&dict_2);
	wow_hash_dict_destroy(&dict_3);
	wow_hash_dict_destroy(&dict_3);

	ret =wow_hash_dict_add(dict_1,NULL,u2p(7));
	GREATEST_ASSERT(ret == -1);
	ret =wow_hash_dict_add(dict_1,"test",NULL);
	GREATEST_ASSERT(ret == -1);
	value = wow_hash_dict_get(dict_1,NULL);
	GREATEST_ASSERT(value == NULL);
	ret = wow_hash_dict_del(dict_1,NULL);
	GREATEST_ASSERT(ret == -1);
	PASS();
}
#endif

TEST test_dict_test1(void)
{
	HashDict_T* dict_1;
	int ret = 0;
	
	printf(MOD_TAG"suit_hash_dict-----test_dict_test1\n");
	dict_1 = wow_hash_dict_create();
	GREATEST_ASSERT(dict_1);

	ret =wow_hash_dict_add(dict_1,"test1",u2p(7));
	GREATEST_ASSERT(ret == 0);
	ret =wow_hash_dict_add(dict_1,"test2",u2p(0));
	GREATEST_ASSERT(ret == 0);
	ret =wow_hash_dict_add(dict_1,"test3",u2p(255));
	GREATEST_ASSERT(ret == 0);
	ret =wow_hash_dict_add(dict_1,"test4",u2p(89));
	GREATEST_ASSERT(ret == 0);
	ret =wow_hash_dict_add(dict_1,"test5",u2p(110));
	GREATEST_ASSERT(ret == 0);
	

	GREATEST_ASSERT(p2u8(wow_hash_dict_get(dict_1,"test1")) == 7);
	GREATEST_ASSERT(p2u8(wow_hash_dict_get(dict_1,"test2")) == 0);
	GREATEST_ASSERT(p2u8(wow_hash_dict_get(dict_1,"test3")) == 255);
	GREATEST_ASSERT(p2u8(wow_hash_dict_get(dict_1,"test4")) == 89);
	GREATEST_ASSERT(p2u8(wow_hash_dict_get(dict_1,"test5")) == 110);


	ret = wow_hash_dict_del(dict_1,"test5");
	GREATEST_ASSERT(ret == 0);
	
	char* data = wow_hash_dict_get(dict_1,"test5");
	GREATEST_ASSERT(data == NULL);

	wow_hash_dict_destroy(&dict_1);
	
	PASS();
}

void dict_func_foreach(DictKeyVal_T* data, const void *user_data)
{
	CHECK_RET_VOID(data);
	printf(MOD_TAG"%s:%s\n",data->key,data->val);
}



TEST test_dict_test2(void)
{
	HashDict_T* dict_1;
	int ret = 0;
	
	printf("suit_hash_dict-----test_dict_test2\n");
	
	dict_1 = wow_hash_dict_create();
	GREATEST_ASSERT(dict_1);

	ret = wow_hash_dict_add(dict_1,"test1","111");
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_dict_add(dict_1,"test2","22222");
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_dict_add(dict_1,"test3","333333");
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_dict_add(dict_1,"test4","4444444");
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_dict_add(dict_1,"test5","55555555");
	GREATEST_ASSERT(ret == 0);
	
	GREATEST_ASSERT(!strcmp("111",(char*)wow_hash_dict_get(dict_1,"test1")));
	GREATEST_ASSERT(!strcmp("22222",(char*)wow_hash_dict_get(dict_1,"test2")));
	GREATEST_ASSERT(!strcmp("333333",(char*)wow_hash_dict_get(dict_1,"test3")));
	GREATEST_ASSERT(!strcmp("4444444",(char*)wow_hash_dict_get(dict_1,"test4")));
	GREATEST_ASSERT(!strcmp("55555555",(char*)wow_hash_dict_get(dict_1,"test5")));
	

	ret = wow_hash_dict_del(dict_1,"test1");
	GREATEST_ASSERT(ret == 0);
	char* data = wow_hash_dict_get(dict_1,"test1");
	GREATEST_ASSERT(data == NULL);

	wow_hash_dict_foreach(dict_1,dict_func_foreach,NULL);
	
	wow_hash_dict_destroy(&dict_1);
	
	PASS();
}


SUITE(suit_hash_dict)
{
#if SUIT_RUN_ERROR_TEST
    RUN_TEST(test_dict_error);
#endif
    RUN_TEST(test_dict_test1);
	RUN_TEST(test_dict_test2);
}



