#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "greatest/greatest.h"

#include "utils/wow_string.h"

#include "wow_test_config.h"

#define MOD_TAG "[string]"

#if SUIT_RUN_ERROR_TEST
TEST test_string_error(void)
{

	int ret = 0;

	printf(MOD_TAG"suit_string----test_string_error\n");
	
	String_T* test_string_00 = wow_string_new();
	GREATEST_ASSERT(test_string_00);
	
	ret = wow_string_append_data(test_string_00,NULL); 
	GREATEST_ASSERT(ret != 0);

    wow_string_free(&test_string_00);

	/*释放后异常测试*/
	ret = wow_string_append_data(test_string_00,"0000");
	GREATEST_ASSERT(ret != 0);

	wow_string_free(&test_string_00);
	
	PASS();
}
#endif

TEST test_string_fun1(void)
{
	printf(MOD_TAG"suit_string----test_string_fun1\n");

    String_T* test_string_00 = wow_string_new();
    GREATEST_ASSERT(test_string_00);


	/*追加数据测试*/
	wow_string_append_data(test_string_00,"0000");	
	wow_string_append_data(test_string_00,"1111");
	wow_string_append_data(test_string_00,"2222");
	wow_string_append_data(test_string_00,"3333");
	wow_string_append_data(test_string_00,"4444");
	GREATEST_ASSERT(!strcmp("00001111222233334444",wow_string_data(test_string_00)));


	/*截取功能测试*/
	int ret = wow_string_left(test_string_00,7);
	GREATEST_ASSERT(ret == 0);
	GREATEST_ASSERT(!strcmp("0000111",wow_string_data(test_string_00)));
	

	ret = wow_string_right(test_string_00,4);
	GREATEST_ASSERT(ret == 0);
	GREATEST_ASSERT(!strcmp("0111",wow_string_data(test_string_00)));
	
	ret = wow_string_mid(test_string_00,1,2);
	GREATEST_ASSERT(ret == 0);
	GREATEST_ASSERT(!strcmp("11",wow_string_data(test_string_00)));

	wow_string_free(&test_string_00);

	PASS();
}


TEST test_string_fun2(void)
{
	printf(MOD_TAG"suit_string----test_string_fun2\n");

	int ret = -1;
	
	String_T* test_string_00 = wow_string_new();
	GREATEST_ASSERT(test_string_00);
	
	String_T* test_string_01 = wow_string_new();
	GREATEST_ASSERT(test_string_01);

	/*追加数据测试*/
	wow_string_append_data(test_string_00,"00"); 
	wow_string_append_data(test_string_00,"11");
	wow_string_append_data(test_string_01,"22");
	wow_string_append_data(test_string_01,"33");

	/*追加对象测试*/
	wow_string_append_string(test_string_00,test_string_01);

	/*strstr测试*/
	char* p = wow_string_str(test_string_00,"1122");
	GREATEST_ASSERT(p);
	GREATEST_ASSERT(!strcmp(p,"112233"));

	/*strcmp测试*/
	ret =  wow_string_cmp(test_string_00,"00112233");
	GREATEST_ASSERT(ret == 0);
	
	ret =  wow_string_cmp(test_string_00,"1122");
	GREATEST_ASSERT(ret < 0);
	
	wow_string_free(&test_string_00);
	wow_string_free(&test_string_01);

	PASS();
}


TEST test_string_fun3(void)
{
	printf(MOD_TAG"suit_string----test_string_fun3\n");

	int i = 0;
	
	String_T* test_string_00 = wow_string_new();
	GREATEST_ASSERT(test_string_00);

	/*大长度测试*/
	for(i = 0; i < 100; i++){
		wow_string_append_data(test_string_00,"0000");	
		wow_string_append_data(test_string_00,"1111");
		wow_string_append_data(test_string_00,"2222");
		wow_string_append_data(test_string_00,"3333");
		wow_string_append_data(test_string_00,"4444");

	}
	/*容量大小验证*/
	int size = wow_string_size(test_string_00);
	GREATEST_ASSERT(size == 2000);

	wow_string_free(&test_string_00);
	PASS();
}



TEST test_string_fun4(void)
{
	printf(MOD_TAG"suit_string----test_string_fun4\n");

	int ret = 0;
	
	String_T* test_string_00 = wow_string_new();
	GREATEST_ASSERT(test_string_00);
	String_T* test_string_01 = wow_string_new();
	GREATEST_ASSERT(test_string_01);


	wow_string_append_data(test_string_00,"12");	
	wow_string_append_data(test_string_00,"34");
	wow_string_append_data(test_string_01,"12.34");

	int val00 = 0;
	float val01 =0.0;
	
	ret = wow_string_toInt(test_string_00,&val00);
	GREATEST_ASSERT(ret == 0 && val00 == 1234);

	ret = wow_string_toFloat(test_string_01,&val01);
	GREATEST_ASSERT(ret == 0 && abs(val01 -12.34)<0.00000001);

	wow_string_free(&test_string_00);
	wow_string_free(&test_string_01);
	PASS();
}


SUITE(suit_string)
{
#if SUIT_RUN_ERROR_TEST
    RUN_TEST(test_string_error);
#endif
    RUN_TEST(test_string_fun1);
	RUN_TEST(test_string_fun2);

	RUN_TEST(test_string_fun3);
	RUN_TEST(test_string_fun4);
}




