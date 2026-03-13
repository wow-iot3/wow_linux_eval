#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "greatest/greatest.h"

#include "wow_test_config.h"

#include "list/wow_string_list.h"

#define MOD_TAG "[stringlist]"


#if SUIT_RUN_ERROR_TEST
TEST test_stringlist_error(void)
{
	printf(MOD_TAG"suit_stringlist----test_stringlist_error\n");

	int ret = 0;
	
	StringList_T* test_stringlist_00 = wow_stringlist_new();
	GREATEST_ASSERT(test_stringlist_00);
	
	ret = wow_stringlist_append(test_stringlist_00,NULL);
	GREATEST_ASSERT(ret != 0);

    wow_stringlist_free(&test_stringlist_00);

	/*释放后异常测试*/
	ret = wow_stringlist_append(test_stringlist_00,"0000");
	GREATEST_ASSERT(ret != 0);

	wow_stringlist_free(&test_stringlist_00);
	
	PASS();

}
#endif

TEST test_stringlist_fun1(void)
{
	printf(MOD_TAG"suit_stringlist----test_stringlist_fun1\n");

	int   count = 0;

    StringList_T* test_stringlist_00 = wow_stringlist_new();
    GREATEST_ASSERT(test_stringlist_00);

	wow_stringlist_append(test_stringlist_00,"AAA");	
	wow_stringlist_append(test_stringlist_00,"1111");
	wow_stringlist_append(test_stringlist_00,"2222");
	wow_stringlist_append(test_stringlist_00,"33.33");
	wow_stringlist_append(test_stringlist_00,"44.44");

	count = wow_stringlist_size(test_stringlist_00);
	GREATEST_ASSERT(count == 5); 
	GREATEST_ASSERT(!strcmp(wow_stringlist_data(test_stringlist_00,0),"AAA"));
	GREATEST_ASSERT(wow_stringlist_toInt(test_stringlist_00,1) == 1111);
	GREATEST_ASSERT(wow_stringlist_toInt(test_stringlist_00,2) == 2222);
	GREATEST_ASSERT(abs(wow_stringlist_toFloat(test_stringlist_00,3)-33.33) < 0.00000001);
	GREATEST_ASSERT(abs(wow_stringlist_toFloat(test_stringlist_00,4)-44.44) < 0.00000001);


	wow_stringlist_free(&test_stringlist_00);
	
	PASS();
}

TEST test_stringlist_fun2(void)
{
	printf(MOD_TAG"suit_stringlist----test_stringlist_fun2\n");

	char str[80] = "123@32@10@320.0@240.0@test1";
	
	StringList_T* test_stringlist_00 = wow_stringlist_split(str,"@");
	GREATEST_ASSERT(test_stringlist_00);

	int count = wow_stringlist_size(test_stringlist_00);
	GREATEST_ASSERT(count == 6);
	GREATEST_ASSERT(wow_stringlist_toInt(test_stringlist_00,0) == 123);
	GREATEST_ASSERT(wow_stringlist_toInt(test_stringlist_00,1) == 32);
	GREATEST_ASSERT(wow_stringlist_toInt(test_stringlist_00,2) == 10);
	GREATEST_ASSERT(abs(wow_stringlist_toFloat(test_stringlist_00,3)-320.0) < 0.00000001);
	GREATEST_ASSERT(abs(wow_stringlist_toFloat(test_stringlist_00,4)-240.0) < 0.00000001);
	GREATEST_ASSERT(!strcmp(wow_stringlist_data(test_stringlist_00,5),"test1"));
	
	wow_stringlist_free(&test_stringlist_00);
	PASS();
}



SUITE(suit_string_list)
{
#if SUIT_RUN_ERROR_TEST
    RUN_TEST(test_stringlist_error);
#endif
    RUN_TEST(test_stringlist_fun1);
	RUN_TEST(test_stringlist_fun2);
}



