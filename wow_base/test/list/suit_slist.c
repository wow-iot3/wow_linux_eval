#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "prefix/wow_check.h"
#include "utils/wow_type.h"

#include "greatest/greatest.h"
#include "wow_test_config.h"

#include "list/wow_slist.h"

#define MOD_TAG "[slist]"


#if SUIT_RUN_ERROR_TEST
TEST test_slist_error(void)
{
	int ret = 0;
	void* ptr = 0;
	printf(MOD_TAG"suit_slist ----- test_slist_error\n");
	
    Slist_T* test_slist_00 = wow_slist_create();
    GREATEST_ASSERT(test_slist_00);
	
	ret = wow_slist_insert_tail(test_slist_00,NULL);	
	GREATEST_ASSERT(ret != 0);
	ret =  wow_slist_insert_head(test_slist_00,NULL);	
	GREATEST_ASSERT(ret != 0);
	ret = wow_slist_insert_by_index(test_slist_00,0,NULL);
	GREATEST_ASSERT(ret != 0);
	ret =  wow_slist_remove(test_slist_00,NULL);
	GREATEST_ASSERT(ret != 0);
	
	wow_slist_foreach(test_slist_00,NULL,NULL);

	ret = wow_slist_insert_by_match(test_slist_00,NULL,NULL,NULL);
	GREATEST_ASSERT(ret != 0);
	ptr= wow_slist_peek_by_match(test_slist_00,NULL,NULL);
	GREATEST_ASSERT(ptr == NULL);
	ret= wow_slist_remove_by_match(test_slist_00,NULL,NULL);
	GREATEST_ASSERT(ret != 0);
	
	wow_slist_clear(test_slist_00,NULL);
	
    wow_slist_destroy(&test_slist_00,NULL);
	
	ret = wow_slist_insert_tail(test_slist_00,(void*)"0000");	
	GREATEST_ASSERT(ret != 0);

	wow_slist_destroy(&test_slist_00,NULL);
	
	PASS();
}
#endif

typedef struct{
	int id;
	char name[64];
}TestPtr_T;
static TestPtr_T gt_test[15]={
	{1,"test1"},
	{2,"test2"},
	{3,"test3"},
	{4,"test4"},
	{5,"test5"},
	{6,"test6"},
	{7,"test7"},
	{8,"test8"},
	{9,"test9"},
	{10,"test10"},
	{11,"test11"},
	{12,"test12"},
	{13,"test13"},
	{14,"test14"},
	{15,"test15"},
};
static TestPtr_T gt_new_test[9]={
	{11,"test11"},
	{22,"test22"},
	{33,"test33"},
	{44,"test44"},
	{55,"test55"},
	{66,"test66"},
	{77,"test77"},
	{88,"test88"},
	{99,"test99"},
};

static void print_ptr_slist(const void *data, const void *user_data)
{
	TestPtr_T* testptr = (TestPtr_T*)data;
	CHECK_RET_VOID(testptr);
	printf(MOD_TAG"print----------id:%d name:%s\n",testptr->id,testptr->name);
}

static bool peek_match_ptr_slist(const void *data, const void *match_data)
{
	TestPtr_T* testptr = (TestPtr_T*)data;
	CHECK_RET_VAL(testptr,false);
	//printf(MOD_TAG"peek----------id:%d name:%s\n",testptr->id,testptr->name);
	return (testptr->id == p2s32(match_data))?true:false;
}

static bool insert_match_ptr_slist(const void *data, const void *match_data)
{
	TestPtr_T* testptr = (TestPtr_T*)data;
	CHECK_RET_VAL(testptr,false);
	//printf(MOD_TAG"insert----------id:%d name:%s\n",testptr->id,testptr->name);
	return (testptr->id < p2s32(match_data))?true:false;
}


TEST test_slist_ptr(void)
{
	int ret   = 0;
	int count = 0;
	int index = 0;
	TestPtr_T* data = NULL;
	Slist_T* test_slist_00 = NULL;

	printf(MOD_TAG"suit_slist ----- test_slist_ptr\n");
	
    test_slist_00 = wow_slist_create();
    GREATEST_ASSERT(test_slist_00);

	wow_slist_insert_tail(test_slist_00,(void*)&gt_test[0]);	
	wow_slist_insert_tail(test_slist_00,(void*)&gt_test[1]);
	wow_slist_insert_tail(test_slist_00,(void*)&gt_test[2]);
	wow_slist_insert_tail(test_slist_00,(void*)&gt_test[3]);
	wow_slist_insert_tail(test_slist_00,(void*)&gt_test[4]);

	wow_slist_insert_head(test_slist_00,(void*)&gt_test[5]);
	wow_slist_insert_head(test_slist_00,(void*)&gt_test[6]);
	wow_slist_insert_head(test_slist_00,(void*)&gt_test[7]);
	wow_slist_insert_head(test_slist_00,(void*)&gt_test[8]);
	wow_slist_insert_head(test_slist_00,(void*)&gt_test[9]);
	
	wow_slist_insert_by_index(test_slist_00,0,(void*)&gt_test[10]);
	wow_slist_insert_by_index(test_slist_00,6,(void*)&gt_test[11]);
	wow_slist_insert_by_index(test_slist_00,14,(void*)&gt_test[12]);

	ret = wow_slist_insert_by_match(test_slist_00,insert_match_ptr_slist,(void*)7,(void*)&gt_test[13]);
	GREATEST_ASSERT(ret == 0);


	printf(MOD_TAG"-------------------1-------------------\n");
	wow_slist_foreach(test_slist_00,print_ptr_slist,NULL);

	wow_slist_replace_head(test_slist_00, &gt_new_test[0]);
	wow_slist_replace_tail(test_slist_00, &gt_new_test[1]);
	wow_slist_replace_by_index(test_slist_00,7, &gt_new_test[2]);
	wow_slist_replace_by_match(test_slist_00,peek_match_ptr_slist,(void*)7, &gt_new_test[6]);
	printf(MOD_TAG"-------------------2-------------------\n");
	wow_slist_foreach(test_slist_00,print_ptr_slist,NULL);
	
	data = wow_slist_peek_by_match(test_slist_00,peek_match_ptr_slist,(void*)6);
	GREATEST_ASSERT(data && data->id == 6);
	
	data = wow_slist_peek_by_match(test_slist_00,peek_match_ptr_slist,(void*)16);
	GREATEST_ASSERT(data==NULL);


	wow_slist_remove_head(test_slist_00);
	wow_slist_remove_tail(test_slist_00);
	wow_slist_remove_by_match(test_slist_00,peek_match_ptr_slist,(void*)4);

	data = wow_slist_peek_by_match(test_slist_00,peek_match_ptr_slist,(void*)6);
	GREATEST_ASSERT(data);
	wow_slist_remove(test_slist_00,data);
	
	data = wow_slist_peek_by_match(test_slist_00,peek_match_ptr_slist,(void*)5);
	GREATEST_ASSERT(data);
	wow_slist_remove(test_slist_00,data);
	printf(MOD_TAG"-----------------3---------------------\n");
	wow_slist_foreach(test_slist_00,print_ptr_slist,NULL);

	count = wow_slist_size(test_slist_00);
	for(index = count-1; index >= 0; index--){
		data = wow_slist_peek_by_index(test_slist_00,index);
		wow_slist_remove_by_index(test_slist_00,index);
	}

	printf(MOD_TAG"-----------------4---------------------\n");
	wow_slist_foreach(test_slist_00,print_ptr_slist,NULL);

	wow_slist_destroy(&test_slist_00,NULL);
	PASS();
}



SUITE(suit_slist)
{	
#if SUIT_RUN_ERROR_TEST
	RUN_TEST(test_slist_error);
#endif
	RUN_TEST(test_slist_ptr);
}


