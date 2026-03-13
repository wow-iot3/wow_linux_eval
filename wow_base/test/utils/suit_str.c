#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "prefix/wow_check.h"
#include "prefix/wow_common.h"
#include "greatest/greatest.h"

#include "wow_test_config.h"

#include "utils/wow_str.h"

#define MOD_TAG "[str]"

#if SUIT_RUN_ERROR_TEST
TEST test_str_error()
{
	printf(MOD_TAG"suit_str----test_str_error\n");
	
	int ret  = 0;
	char *s1 = NULL;
	char **p = NULL;
	char test1[64] = "AAA BBB CCC AAA BBB CCC AAA BBB CCC";

	ret = wow_str_has_prefix(NULL,"AAA");
	GREATEST_ASSERT(ret != 0);
	ret = wow_str_has_prefix(test1,NULL);
	GREATEST_ASSERT(ret != 0);
	ret = wow_str_has_suffix(NULL,"AAA");
	GREATEST_ASSERT(ret != 0);
	ret = wow_str_has_suffix (test1,NULL);
	GREATEST_ASSERT(ret != 0);

	s1 = wow_str_replace(NULL," ","@");
	GREATEST_ASSERT(s1 == NULL);

	s1 = wow_str_replace(test1," ",NULL);
	GREATEST_ASSERT(s1 == NULL);
	s1 = wow_str_replace(test1,NULL,"@");
	GREATEST_ASSERT(s1 == NULL);
	
	p = wow_str_split(NULL," ",NULL);
	GREATEST_ASSERT(p == NULL);
	p = wow_str_split(test1," ",NULL);
	GREATEST_ASSERT(p == NULL);
	
	PASS();
}
#endif

TEST test_str_fun1()
{
	printf(MOD_TAG"suit_str----test_str_fun1\n");

	int num = 0;
	int ret = 0;
	char test1[64] = "AAA BBB CCC AAA BBB CCC AAA BBB CCC";
	
	char *s1 = wow_str_replace(test1," ","@");
	GREATEST_ASSERT(STRING_EQUAL(s1,"AAA@BBB@CCC@AAA@BBB@CCC@AAA@BBB@CCC"));
	wow_str_replace_free(&s1);
	
	char *s2 = wow_str_replace(test1," ","++");
	GREATEST_ASSERT(STRING_EQUAL(s2,"AAA++BBB++CCC++AAA++BBB++CCC++AAA++BBB++CCC"));
	wow_str_replace_free(&s2);	
	
	char **p = wow_str_split(test1," ",&num);
	GREATEST_ASSERT(num == 9);
	wow_str_split_free(p);
	
	ret = wow_str_has_prefix (test1,"AAA");
	GREATEST_ASSERT(ret == 1);
	ret = wow_str_has_prefix (test1,"CCC");
	GREATEST_ASSERT(ret == 0);

	ret = wow_str_has_suffix (test1,"AAA");
	GREATEST_ASSERT(ret == 0);
	ret = wow_str_has_suffix (test1,"CCC");
	GREATEST_ASSERT(ret == 1);


	char* data = wow_str_combine(5,"AAA"," ","BBB"," ","CCC","");
	GREATEST_ASSERT(STRING_EQUAL(data,"AAA BBB CCC"));
	wow_str_combine_free(&data);
	
	PASS();
}

SUITE(suit_str)
{
#if SUIT_RUN_ERROR_TEST
	RUN_TEST(test_str_error);
#endif
    RUN_TEST(test_str_fun1);
}


