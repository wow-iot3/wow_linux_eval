#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "greatest/greatest.h"
#include "wow_test_config.h"

#include "plugin/wow_debug.h"
#include "algorithm/wow_bitmap.h"

#define MOD_TAG "bitmap"

#if SUIT_RUN_ERROR_TEST
TEST test_bitmap_error(void)
{
	printf(MOD_TAG"suit_bitmap---------test_bitmap_error.\n");

    BitMap_T*  bitmap = NULL;
	int ret = 0;
	
	ret =  wow_bitmap_set_1(NULL, 31);
	GREATEST_ASSERT(ret == -1);
	ret =  wow_bitmap_set_0(NULL, 31);
	GREATEST_ASSERT(ret == -1);
	ret =  wow_bitmap_get(NULL, 31);
	GREATEST_ASSERT(ret == -1);

	bitmap = wow_bitmap_new(30);
	GREATEST_ASSERT(bitmap);
	
	//字节对齐
	ret = wow_bitmap_get(bitmap, 31);
	GREATEST_ASSERT(ret == -1);
	ret = wow_bitmap_get(bitmap, 32);
	GREATEST_ASSERT(ret == -1);
	
	wow_bitmap_free(&bitmap);
	wow_bitmap_free(&bitmap);

	PASS();
}
#endif

TEST test_bitmap_fun_01(void)
{
	printf(MOD_TAG"suit_bitmap---------test_bitmap_fun_01.\n");
	
    int a[8] = {3, 5, 9, 6, 10, 29, 16, 18};
    int i = 0;
	
    BitMap_T* bitmap = wow_bitmap_new(30);
	GREATEST_ASSERT(bitmap);
	
    for ( i = 0; i < 8; i++ ){
       wow_bitmap_set_1( bitmap, a[i]);
    }

	GREATEST_ASSERT(wow_bitmap_get(bitmap, 3) == 1);
    wow_bitmap_set_0(bitmap, 3);
	GREATEST_ASSERT(wow_bitmap_get(bitmap, 3) == 0);

    wow_bitmap_free(&bitmap);

	PASS();
}

TEST test_bitmap_fun_02(void)
{
	printf(MOD_TAG"suit_bitmap---------test_bitmap_fun_02.\n");
	
	int ret = 0;
	int len = 0;
	int pos = 0;

    BitMap_T* bitmap = wow_bitmap_new(210);
	GREATEST_ASSERT(bitmap);
	
    for (int i = 21; i < 57; i++ ){
       wow_bitmap_set_1(bitmap, i);
    }

    wow_bitmap_set_1( bitmap, 3);
	wow_bitmap_set_1( bitmap, 63);
	wow_bitmap_set_1( bitmap, 83);
	wow_bitmap_set_1( bitmap, 113);
	wow_bitmap_set_1( bitmap, 153);
	wow_bitmap_set_1( bitmap, 173);

	ret = wow_bitmap_longest_consecutive_0(bitmap, &len, &pos);
	GREATEST_ASSERT(ret == 0);
	printf("wow_bitmap_longest_consecutive_0 len = %d, pos = %d\n", len, pos);

	ret = wow_bitmap_longest_consecutive_1(bitmap, &len, &pos);
    GREATEST_ASSERT(ret == 0);
	printf("wow_bitmap_longest_consecutive_1 len = %d, pos = %d\n", len, pos);

	wow_bitmap_free(&bitmap);

	PASS();
}


SUITE(suit_bitmap)
{
#if SUIT_RUN_ERROR_TEST
	RUN_TEST(test_bitmap_error);
#endif
    RUN_TEST(test_bitmap_fun_01);
	RUN_TEST(test_bitmap_fun_02);
}

