#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "prefix/wow_common.h"
#include "greatest/greatest.h"

#include "wow_test_config.h"

#include "list/wow_kv_list.h"

#define MOD_TAG "[kvlist]"

 
#if SUIT_RUN_ERROR_TEST
TEST test_kvlist_error(void)
{
	int ret = 0;
	printf(MOD_TAG"suit_kvlist ----- test_kvlist_error\n");
	
    KvList_T* test_kvlist = wow_kvlist_create();
    GREATEST_ASSERT(test_kvlist);
	
	ret = wow_kvlist_set(test_kvlist,NULL,"AAAAAAA");	
	GREATEST_ASSERT(ret != 0);
	ret = wow_kvlist_set(test_kvlist,"NULL",NULL);	
	GREATEST_ASSERT(ret != 0);
	char* data =  wow_kvlist_get(test_kvlist,NULL);	
	GREATEST_ASSERT(data == NULL);
	ret = wow_kvlist_delete(test_kvlist,NULL);
	GREATEST_ASSERT(ret != 0);
	
	wow_kvlist_foreach(test_kvlist,NULL,NULL,NULL);

	wow_kvlist_destroy(&test_kvlist);
	
	PASS();
}
#endif

TEST test_kvlist_func(void)
{
	int i = 0;
	int ret = 0;
	int size = 0;
	char* ptr = NULL;
	
	printf(MOD_TAG"suit_kvlist ----- test_kvlist_func\n");
	
    KvList_T* test_kvlist = wow_kvlist_create();
    GREATEST_ASSERT(test_kvlist);

	ret = wow_kvlist_set(test_kvlist,"1","11111111111");
	GREATEST_ASSERT(ret == 0);
	ret = wow_kvlist_set(test_kvlist,"2","22222222222");
	GREATEST_ASSERT(ret == 0);
	ret = wow_kvlist_set(test_kvlist,"3","33333333333");
	GREATEST_ASSERT(ret == 0);
	ret = wow_kvlist_set(test_kvlist,"4","44444444444");
	GREATEST_ASSERT(ret == 0);

	ptr = wow_kvlist_get(test_kvlist,"1");
	GREATEST_ASSERT(ptr && strncmp(ptr,"11111111111",strlen("11111111111")) == 0);

	ret = wow_kvlist_update(test_kvlist,"1","11111111112");
	GREATEST_ASSERT(ret == 0);
	
	ptr = wow_kvlist_get(test_kvlist,"1");
	GREATEST_ASSERT(ptr && strncmp(ptr,"11111111112",strlen("11111111111")) == 0);

	wow_kvlist_delete(test_kvlist,"1");
	GREATEST_ASSERT(ret == 0);
	
	ptr = wow_kvlist_get(test_kvlist,"1");
	GREATEST_ASSERT(ptr == NULL);

	int count = wow_kvlist_size(test_kvlist);
	GREATEST_ASSERT(count == 3);

	char** keys = CALLOC(count,char*);
	char** vals = CALLOC(count,char*);
	wow_kvlist_foreach(test_kvlist,keys,vals,&size);
	GREATEST_ASSERT(count == size);
	for(i = 0; i < count; i++){
		printf(" vals:%s vals:%s\r\n",keys[i],vals[i]);
	}
	FREE(keys);
	FREE(vals);

	wow_kvlist_destroy(&test_kvlist);
	PASS();
}


SUITE(suit_kv_list)
{
#if SUIT_RUN_ERROR_TEST
	RUN_TEST(test_kvlist_error);
#endif
	RUN_TEST(test_kvlist_func);
}

