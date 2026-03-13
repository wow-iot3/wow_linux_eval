#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "greatest/greatest.h"
#include "wow_test_config.h"

#include "utils/wow_math.h"
#include "utils/wow_type.h"
#include "hash/wow_hash_map.h"

#define MOD_TAG "[hash_map]"

#if SUIT_RUN_ERROR_TEST
TEST test_hash_map_error(void)
{
	printf(MOD_TAG"suit_hash_map-----test_hash_map_error\n");

	int ret = 0;
	void* val = NULL;
	HashMap_T* hash_map_1;
	HashMap_T* hash_map_2;
	HashMap_T* hash_map_3;
	HashMap_T* hash_map_4;
	HashMap_T* hash_map_5;
	HashMap_T* hash_map_6;

	hash_map_1 = wow_hash_map_create(ELEMENT_TYPE_STR,ELEMENT_TYPE_PTR);
	GREATEST_ASSERT(hash_map_1);

	hash_map_2 = wow_hash_map_create(ELEMENT_TYPE_UINT8,ELEMENT_TYPE_PTR);
	GREATEST_ASSERT(hash_map_2);

	hash_map_3 = wow_hash_map_create(ELEMENT_TYPE_UINT16,ELEMENT_TYPE_PTR);
	GREATEST_ASSERT(hash_map_3);
	
	hash_map_4 = wow_hash_map_create(ELEMENT_TYPE_UINT32,ELEMENT_TYPE_PTR);
	GREATEST_ASSERT(hash_map_4);
	
	hash_map_5 = wow_hash_map_create(ELEMENT_TYPE_STR,ELEMENT_TYPE_PTR);
	GREATEST_ASSERT(hash_map_5);
	
	hash_map_6 = wow_hash_map_create(ELEMENT_TYPE_PTR,ELEMENT_TYPE_PTR);
	GREATEST_ASSERT(hash_map_6);
	

	wow_hash_map_destroy(&hash_map_1);
	wow_hash_map_destroy(&hash_map_2);
	wow_hash_map_destroy(&hash_map_3);
	wow_hash_map_destroy(&hash_map_4);
	wow_hash_map_destroy(&hash_map_5);
	wow_hash_map_destroy(&hash_map_6);
	wow_hash_map_destroy(&hash_map_6);

	ret = wow_hash_map_insert(NULL,"AAA","BBB");
	GREATEST_ASSERT(ret != 0);
	
	val = wow_hash_map_get(NULL,"AAA");
	GREATEST_ASSERT(val == NULL);
	
	wow_hash_map_remove(NULL,"AAA");
	wow_hash_map_clear(NULL);
	wow_hash_map_destroy(NULL);
	
	hash_map_1 = wow_hash_map_create(ELEMENT_TYPE_STR,ELEMENT_TYPE_PTR);
	GREATEST_ASSERT(hash_map_1);

	ret = wow_hash_map_insert(hash_map_1,NULL,"BBB");
	GREATEST_ASSERT(ret != 0);
	ret = wow_hash_map_insert(hash_map_1,"AAA",NULL);
	GREATEST_ASSERT(ret != 0);
	val = wow_hash_map_get(hash_map_1,NULL);
	GREATEST_ASSERT(val == NULL);
	wow_hash_map_remove(hash_map_1,NULL);
	wow_hash_map_destroy(&hash_map_1);

	PASS();

}
#endif

TEST test_hash_map_test1(void)
{
	int i = 0;
	int ret = 0;
	HashMap_T* hash_map_1;
    int numStrings = 100; // 需要存储的字符串数量

	printf(MOD_TAG"suit_hash_map-----test_hash_map_test1\n");
	
    char **strings = malloc(numStrings * sizeof(char *)); // 分配指针数组
	GREATEST_ASSERT(strings);

	hash_map_1 = wow_hash_map_create(ELEMENT_TYPE_UINT8,ELEMENT_TYPE_PTR);
	GREATEST_ASSERT(hash_map_1);

    for (i = 1; i <= numStrings; i++) {
        int length = i + 1; // 字符串长度
        strings[i] = malloc((length + 1) * sizeof(char)); // 分配字符串空间
		GREATEST_ASSERT(strings[i]);	
        // 填充字符串
        for (int j = 0; j < length; j++) {
            strings[i][j] = '0' + i; // 填充 '1', '2', '3', ...
        }
        strings[i][length] = '\0'; // 添加字符串结尾的空字符
		
		ret = wow_hash_map_insert(hash_map_1,u2p(i),strings[i]);
		GREATEST_ASSERT(ret == 0);
    }

    for (i = 1; i <= numStrings; i++) {
		GREATEST_ASSERT(!strcmp(strings[i],(char*)wow_hash_map_get(hash_map_1,u2p(i))));
        free(strings[i]); // 释放字符串内存
    }
	
	free(strings); // 释放指针数组内存
	
	wow_hash_map_destroy(&hash_map_1);
	
	PASS();
}


TEST test_hash_map_test2(void)
{
	int ret = 0;
	HashMap_T* hash_map_1;

	printf(MOD_TAG"suit_hash_map-----test_hash_map_test2\n");
	
	hash_map_1 = wow_hash_map_create(ELEMENT_TYPE_STR,ELEMENT_TYPE_STR);
	GREATEST_ASSERT(hash_map_1);

	ret = wow_hash_map_insert(hash_map_1,"key1","111");
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_map_insert(hash_map_1,"key2","22222");
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_map_insert(hash_map_1,"key3","333333");
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_map_insert(hash_map_1,"key4","4444444");
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_map_insert(hash_map_1,"key5","55555555");
	GREATEST_ASSERT(ret == 0);
	
	GREATEST_ASSERT(!strcmp("111",(char*)wow_hash_map_get(hash_map_1,"key1")));
	GREATEST_ASSERT(!strcmp("22222",(char*)wow_hash_map_get(hash_map_1,"key2")));
	GREATEST_ASSERT(!strcmp("333333",(char*)wow_hash_map_get(hash_map_1,"key3")));
	GREATEST_ASSERT(!strcmp("4444444",(char*)wow_hash_map_get(hash_map_1,"key4")));
	GREATEST_ASSERT(!strcmp("55555555",(char*)wow_hash_map_get(hash_map_1,"key5")));
	
	//对已存在key进行内容跟新
	ret = wow_hash_map_insert(hash_map_1,"key1","6666666666");
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_map_insert(hash_map_1,"key2","777");
	GREATEST_ASSERT(ret == 0);
	GREATEST_ASSERT(!strcmp("6666666666",(char*)wow_hash_map_get(hash_map_1,"key1")));
	GREATEST_ASSERT(!strcmp("777",(char*)wow_hash_map_get(hash_map_1,"key2")));

	ret = wow_hash_map_size(hash_map_1);
	GREATEST_ASSERT(ret == 5);

	wow_hash_map_remove(hash_map_1,"key3");
	char* val = (char*)wow_hash_map_get(hash_map_1,"key3");
	GREATEST_ASSERT(val == NULL);
	ret = wow_hash_map_size(hash_map_1);
	GREATEST_ASSERT(ret == 4);

	wow_hash_map_clear(hash_map_1);
	val = (char*)wow_hash_map_get(hash_map_1,"key1");
	GREATEST_ASSERT(val == NULL);
	ret = wow_hash_map_size(hash_map_1);
	GREATEST_ASSERT(ret == 0);

	
	wow_hash_map_destroy(&hash_map_1);
	
	PASS();

}


TEST test_hash_map_test3(void)
{
	int ret = 0;

	char data1[16] = "111";
	char data2[16] = "22222";
	char data3[16] = "333333";
	char data4[16] = "4444444";
	char data5[16] = "55555555";
	char data6[16] = "6666666666";
	char data7[16] = "777";
	HashMap_T* hash_map_1;

	printf(MOD_TAG"suit_hash_map-----test_hash_map_test3\n");
	
	hash_map_1 = wow_hash_map_create(ELEMENT_TYPE_UINT8,ELEMENT_TYPE_PTR);
	GREATEST_ASSERT(hash_map_1);

	ret = wow_hash_map_insert(hash_map_1,u2p(1),data1);
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_map_insert(hash_map_1,u2p(2),data2);
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_map_insert(hash_map_1,u2p(3),data3);
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_map_insert(hash_map_1,u2p(4),data4);
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_map_insert(hash_map_1,u2p(5),data5);
	GREATEST_ASSERT(ret == 0);
	
	GREATEST_ASSERT(!strcmp("111",(char*)wow_hash_map_get(hash_map_1,u2p(1))));
	GREATEST_ASSERT(!strcmp("22222",(char*)wow_hash_map_get(hash_map_1,u2p(2))));
	GREATEST_ASSERT(!strcmp("333333",(char*)wow_hash_map_get(hash_map_1,u2p(3))));
	GREATEST_ASSERT(!strcmp("4444444",(char*)wow_hash_map_get(hash_map_1,u2p(4))));
	GREATEST_ASSERT(!strcmp("55555555",(char*)wow_hash_map_get(hash_map_1,u2p(5))));


	//对已存在key进行内容跟新
	ret = wow_hash_map_insert(hash_map_1,u2p(1),data6);
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_map_insert(hash_map_1,u2p(2),data7);
	GREATEST_ASSERT(ret == 0);

	GREATEST_ASSERT(!strcmp("6666666666",(char*)wow_hash_map_get(hash_map_1,u2p(1))));
	GREATEST_ASSERT(!strcmp("777",(char*)wow_hash_map_get(hash_map_1,u2p(2))));


	ret = wow_hash_map_size(hash_map_1);
	GREATEST_ASSERT(ret == 5);

	wow_hash_map_remove(hash_map_1,u2p(3));
	char* val = (char*)wow_hash_map_get(hash_map_1,u2p(3));
	GREATEST_ASSERT(val == NULL);
	ret = wow_hash_map_size(hash_map_1);
	GREATEST_ASSERT(ret == 4);

	wow_hash_map_clear(hash_map_1);
	val = (char*)wow_hash_map_get(hash_map_1,u2p(1));
	GREATEST_ASSERT(val == NULL);
	ret = wow_hash_map_size(hash_map_1);
	GREATEST_ASSERT(ret == 0);

	
	wow_hash_map_destroy(&hash_map_1);
	
	PASS();
}


TEST test_hash_map_test4(void)
{
	int ret = 0;
	char data1[16] = "111";
	char data2[16] = "22222";
	char data3[16] = "333333";
	char data4[16] = "4444444";
	char data5[16] = "55555555";
	char data6[16] = "6666666666";
	char data7[16] = "777";
	HashMap_T* hash_map_1;

	printf(MOD_TAG"suit_hash_map-----test_hash_map_test4\n");
	
	hash_map_1 = wow_hash_map_create(ELEMENT_TYPE_STR,ELEMENT_TYPE_PTR);
	GREATEST_ASSERT(hash_map_1);

	ret = wow_hash_map_insert(hash_map_1,"key1",data1);
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_map_insert(hash_map_1,"key2",data2);
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_map_insert(hash_map_1,"key3",data3);
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_map_insert(hash_map_1,"key4",data4);
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_map_insert(hash_map_1,"key5",data5);
	GREATEST_ASSERT(ret == 0);
	
	GREATEST_ASSERT(!strcmp("111",(char*)wow_hash_map_get(hash_map_1,"key1")));
	GREATEST_ASSERT(!strcmp("22222",(char*)wow_hash_map_get(hash_map_1,"key2")));
	GREATEST_ASSERT(!strcmp("333333",(char*)wow_hash_map_get(hash_map_1,"key3")));
	GREATEST_ASSERT(!strcmp("4444444",(char*)wow_hash_map_get(hash_map_1,"key4")));
	GREATEST_ASSERT(!strcmp("55555555",(char*)wow_hash_map_get(hash_map_1,"key5")));


	//对已存在key进行内容跟新
	ret = wow_hash_map_insert(hash_map_1,"key1",data6);
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_map_insert(hash_map_1,"key2",data7);
	GREATEST_ASSERT(ret == 0);

	GREATEST_ASSERT(!strcmp("6666666666",(char*)wow_hash_map_get(hash_map_1,"key1")));
	GREATEST_ASSERT(!strcmp("777",(char*)wow_hash_map_get(hash_map_1,"key2")));

	ret = wow_hash_map_size(hash_map_1);
	GREATEST_ASSERT(ret == 5);

	wow_hash_map_remove(hash_map_1,"key3");
	char* val = (char*)wow_hash_map_get(hash_map_1,"key3");
	GREATEST_ASSERT(val == NULL);
	ret = wow_hash_map_size(hash_map_1);
	GREATEST_ASSERT(ret == 4);

	wow_hash_map_clear(hash_map_1);
	val = (char*)wow_hash_map_get(hash_map_1,"key1");
	GREATEST_ASSERT(val == NULL);
	ret = wow_hash_map_size(hash_map_1);
	GREATEST_ASSERT(ret == 0);

	
	wow_hash_map_destroy(&hash_map_1);
	
	PASS();

}


TEST test_hash_map_test5(void)
{
	int ret = 0;
	char key1[8]  = "key1";
	char key2[16] = "key2";
	char key3[16] = "key3";
	char key4[16] = "key4";
	char key5[16] = "key5";

	char data1[16] = "111";
	char data2[16] = "22222";
	char data3[16] = "333333";
	char data4[16] = "4444444";
	char data5[16] = "55555555";
	char data6[16] = "6666666666";
	char data7[16] = "777";
	HashMap_T* hash_map_1;

	printf(MOD_TAG"suit_hash_map-----test_hash_map_test5\n");
	
	hash_map_1 = wow_hash_map_create(ELEMENT_TYPE_STR,ELEMENT_TYPE_PTR);
	GREATEST_ASSERT(hash_map_1);

	ret = wow_hash_map_insert(hash_map_1,key1,data1);
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_map_insert(hash_map_1,key2,data2);
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_map_insert(hash_map_1,key3,data3);
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_map_insert(hash_map_1,key4,data4);
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_map_insert(hash_map_1,key5,data5);
	GREATEST_ASSERT(ret == 0);
	
	GREATEST_ASSERT(!strcmp("111",(char*)wow_hash_map_get(hash_map_1,key1)));
	GREATEST_ASSERT(!strcmp("22222",(char*)wow_hash_map_get(hash_map_1,key2)));
	GREATEST_ASSERT(!strcmp("333333",(char*)wow_hash_map_get(hash_map_1,key3)));
	GREATEST_ASSERT(!strcmp("4444444",(char*)wow_hash_map_get(hash_map_1,key4)));
	GREATEST_ASSERT(!strcmp("55555555",(char*)wow_hash_map_get(hash_map_1,key5)));


	//对已存在key进行内容跟新
	ret = wow_hash_map_insert(hash_map_1,key1,data6);
	GREATEST_ASSERT(ret == 0);
	ret = wow_hash_map_insert(hash_map_1,key2,data7);
	GREATEST_ASSERT(ret == 0);

	GREATEST_ASSERT(!strcmp("6666666666",(char*)wow_hash_map_get(hash_map_1,key1)));
	GREATEST_ASSERT(!strcmp("777",(char*)wow_hash_map_get(hash_map_1,key2)));


	ret = wow_hash_map_size(hash_map_1);
	GREATEST_ASSERT(ret == 5);

	wow_hash_map_remove(hash_map_1,key3);
	char* val = (char*)wow_hash_map_get(hash_map_1,key3);
	GREATEST_ASSERT(val == NULL);
	ret = wow_hash_map_size(hash_map_1);
	GREATEST_ASSERT(ret == 4);

	wow_hash_map_clear(hash_map_1);
	val = (char*)wow_hash_map_get(hash_map_1,key1);
	GREATEST_ASSERT(val == NULL);
	ret = wow_hash_map_size(hash_map_1);
	GREATEST_ASSERT(ret == 0);

	
	wow_hash_map_destroy(&hash_map_1);
	
	PASS();

}

SUITE(suit_hash_map)
{
#if SUIT_RUN_ERROR_TEST
	RUN_TEST(test_hash_map_error);
#endif
    RUN_TEST(test_hash_map_test1);
	RUN_TEST(test_hash_map_test2);
	RUN_TEST(test_hash_map_test3);
	RUN_TEST(test_hash_map_test4);
	RUN_TEST(test_hash_map_test5);
}


