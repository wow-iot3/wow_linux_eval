#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "greatest/greatest.h"
#include "wow_test_config.h"

#include "utils/wow_byte_array.h"

#define MOD_TAG "[bytearray]"

#if SUIT_RUN_ERROR_TEST
TEST test_bytearray_error(void)
{
	printf(MOD_TAG"suit_byte_array----test_bytearray_error\n");

	int ret = 0;
	uint8_t value = 0x00;
	uint8_t data[8] = {0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38};
	ByteArray_T* test_bytearray_00 = NULL;

	ret = wow_byte_array_size(test_bytearray_00);
	GREATEST_ASSERT(ret != 0);
	ret = wow_byte_array_append_data(test_bytearray_00,0x31);
	GREATEST_ASSERT(ret != 0);
	ret = wow_byte_array_append_ndata(test_bytearray_00,data,8);
	GREATEST_ASSERT(ret != 0);
	ret = wow_byte_array_insert(test_bytearray_00,2,0x39);
	GREATEST_ASSERT(ret != 0);
	ret = wow_byte_array_left(test_bytearray_00,3);
	GREATEST_ASSERT(ret != 0);
	ret = wow_byte_array_right(test_bytearray_00,3);
	GREATEST_ASSERT(ret != 0);
	ret = wow_byte_array_mid(test_bytearray_00,3,1);
	GREATEST_ASSERT(ret != 0);

	ret = wow_byte_array_get(test_bytearray_00,1,&value);
	GREATEST_ASSERT(ret != 0);
	ret = wow_byte_array_set(test_bytearray_00,1,value);
	GREATEST_ASSERT(ret != 0);
	ret = wow_byte_array_find(test_bytearray_00,value);
	GREATEST_ASSERT(ret != 0);
	ret = wow_byte_array_replace(test_bytearray_00,value,value);
	GREATEST_ASSERT(ret != 0);
	ret = wow_byte_array_remove(test_bytearray_00,0,2);
	GREATEST_ASSERT(ret != 0);
	
    test_bytearray_00 = wow_byte_array_new(0);
    GREATEST_ASSERT(test_bytearray_00);

	ret = wow_byte_array_append_ndata(test_bytearray_00,NULL,255);
	GREATEST_ASSERT(ret != 0);
	ret = wow_byte_array_append_array(test_bytearray_00,NULL);
	GREATEST_ASSERT(ret != 0);
	
    wow_byte_array_free(&test_bytearray_00);
	test_bytearray_00 = NULL;

	/*释放后异常测试*/
	ret = wow_byte_array_append_data(test_bytearray_00,0x03);
	GREATEST_ASSERT(ret != 0);

	wow_byte_array_free(&test_bytearray_00);
	wow_byte_array_free(&test_bytearray_00);
	
	PASS();

}
#endif

TEST test_byte_array_fun1(void)
{
	printf(MOD_TAG"suit_byte_array----test_byte_array_fun1\n");
	
	int ret = 0;
	uint8_t* val = NULL;
	
    ByteArray_T* test_bytearray_00 = wow_byte_array_new(0);
    GREATEST_ASSERT(test_bytearray_00);

	/*追加数据测试*/
	wow_byte_array_append_data(test_bytearray_00,0x00);	
	wow_byte_array_append_data(test_bytearray_00,0x01);
	wow_byte_array_append_data(test_bytearray_00,0x02);
	wow_byte_array_append_data(test_bytearray_00,0x03);
	wow_byte_array_append_data(test_bytearray_00,0x04);
	wow_byte_array_append_data(test_bytearray_00,0x04);
	wow_byte_array_append_data(test_bytearray_00,0x05);
	wow_byte_array_append_data(test_bytearray_00,0x06);
	wow_byte_array_append_data(test_bytearray_00,0x0F);
	

	/*截取功能测试*/
	ret = wow_byte_array_left(test_bytearray_00,6);
	GREATEST_ASSERT(ret == 0);
	
	ret = wow_byte_array_size(test_bytearray_00);
	GREATEST_ASSERT(ret == 6);
	val = wow_byte_array_data(test_bytearray_00);
	GREATEST_ASSERT(val[0] == 0x00 && val[1] == 0x01 && val[2] == 0x02 &&
						val[3] == 0x03 && val[4] == 0x04 && val[5] == 0x04);

	ret = wow_byte_array_right(test_bytearray_00,4);
	GREATEST_ASSERT(ret == 0);
	ret = wow_byte_array_size(test_bytearray_00);
	GREATEST_ASSERT(ret == 4);
	val = wow_byte_array_data(test_bytearray_00);
	GREATEST_ASSERT(val[0] == 0x02 &&val[1] == 0x03 && val[2] == 0x04 && val[3] == 0x04);


	ret = wow_byte_array_mid(test_bytearray_00,1,2);
	GREATEST_ASSERT(ret == 0);
	ret = wow_byte_array_size(test_bytearray_00);
	GREATEST_ASSERT(ret == 2);
	val = wow_byte_array_data(test_bytearray_00);
	GREATEST_ASSERT(val[0] == 0x03 &&val[1] == 0x04);

	wow_byte_array_free(&test_bytearray_00);
		
	PASS();
}


TEST test_byte_array_fun2(void)
{
	printf(MOD_TAG"suit_byte_array----test_byte_array_fun2\n");

	int   ret = 0;
	uint8_t data = 0;
	uint8_t* val = NULL;

	
    ByteArray_T* test_bytearray_00 = wow_byte_array_new(0);
    GREATEST_ASSERT(test_bytearray_00);

	/*追加数据测试*/
	wow_byte_array_append_data(test_bytearray_00,0x00);	
	wow_byte_array_append_data(test_bytearray_00,0x01);
	wow_byte_array_append_data(test_bytearray_00,0x02);
	wow_byte_array_append_data(test_bytearray_00,0x03);
	wow_byte_array_append_data(test_bytearray_00,0x04);
	wow_byte_array_append_data(test_bytearray_00,0x04);
	wow_byte_array_append_data(test_bytearray_00,0x05);
	wow_byte_array_append_data(test_bytearray_00,0x06);

	/*增删改查测试*/
	ret = wow_byte_array_insert(test_bytearray_00,2,0x07);
	GREATEST_ASSERT(ret == 0);
	ret = wow_byte_array_insert(test_bytearray_00,2,0x08);
	GREATEST_ASSERT(ret == 0);
	ret = wow_byte_array_remove(test_bytearray_00,0,2);
	GREATEST_ASSERT(ret == 0);
	ret = wow_byte_array_replace(test_bytearray_00,0x04,0x09);
	GREATEST_ASSERT(ret == 0);
	ret = wow_byte_array_set(test_bytearray_00,3,0xFF);
	GREATEST_ASSERT(ret == 0);
	ret = wow_byte_array_get(test_bytearray_00,3,&data);
	GREATEST_ASSERT(ret == 0 && data == 0xFF);

	ret = wow_byte_array_find(test_bytearray_00,0x01);
	GREATEST_ASSERT(ret != 0);
	
	ret = wow_byte_array_find(test_bytearray_00,0x07);
	GREATEST_ASSERT(ret != 0);

	/*获取数据测试*/
	ret = wow_byte_array_size(test_bytearray_00);
	GREATEST_ASSERT(ret == 8);
	val = wow_byte_array_data(test_bytearray_00);
	GREATEST_ASSERT(val[0] == 0x08 && val[1] == 0x07 && val[2] == 0x02 &&
					val[3] == 0xFF && val[4] == 0x09 && val[5] == 0x09 &&
					val[6] == 0x05 && val[7] == 0x06);
		
	wow_byte_array_free(&test_bytearray_00);
		
	PASS();
}

TEST test_byte_array_fun3(void)
{
	printf(MOD_TAG"suit_byte_array----test_byte_array_fun3\n");

	int i = 0;
	int ret = 0;
	
    ByteArray_T* test_bytearray_00 = wow_byte_array_new(0);
    GREATEST_ASSERT(test_bytearray_00);

	/*大长度测试*/
	for(i = 0; i < 1024; i++){
		ret = wow_byte_array_append_data(test_bytearray_00,i%256);	
		GREATEST_ASSERT(ret == 0);
		
	}
	/*容量大小验证*/
	ret = wow_byte_array_size(test_bytearray_00);
	GREATEST_ASSERT(ret == 1024);

	wow_byte_array_free(&test_bytearray_00);

	PASS();
}

SUITE(suit_byte_array)
{	
#if SUIT_RUN_ERROR_TEST
	RUN_TEST(test_bytearray_error);
#endif
	RUN_TEST(test_byte_array_fun1);
	RUN_TEST(test_byte_array_fun2);
	RUN_TEST(test_byte_array_fun3);
}




