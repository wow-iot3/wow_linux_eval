#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "greatest/greatest.h"

#include "wow_test_config.h"

#include "file/wow_dir.h"
#include "file/wow_file.h"

#define MOD_TAG "[file]"
#define TEST_DIR_PATH     "file/1/1/3/"
#define TEST_FILE_PATH_01 "file/1/1/3/a.txt"
#define TEST_FILE_PATH_02 "file/1/1/3/b.txt"
#define TEST_FILE_PATH_03 "config/demo.xml"
#define TEST_FILE_PATH_04 "file/1/1/3/bbb.txt"



TEST test_file_fun1(void)
{
	int  ret = -1;

	printf(MOD_TAG"suit_file----test_file_fun1----\n");

	ret =  wow_dir_create(WOW_TEST_FILE_STRING(TEST_DIR_PATH));
	GREATEST_ASSERT(ret == 0);
	
	ret =  wow_file_create(WOW_TEST_FILE_STRING(TEST_FILE_PATH_01));
	GREATEST_ASSERT(ret == 0);

	ret =  wow_file_create(WOW_TEST_FILE_STRING(TEST_FILE_PATH_02));
	GREATEST_ASSERT(ret == 0);

	ret = wow_file_check_exist(WOW_TEST_FILE_STRING(TEST_FILE_PATH_01));
	GREATEST_ASSERT(ret == 1);

	ret = wow_file_check_exist(WOW_TEST_FILE_STRING(TEST_FILE_PATH_02));
	GREATEST_ASSERT(ret == 1);

	ret = wow_file_remove(WOW_TEST_FILE_STRING(TEST_FILE_PATH_01));
	GREATEST_ASSERT(ret == 0);

	ret = wow_file_check_exist(WOW_TEST_FILE_STRING(TEST_FILE_PATH_01));
	GREATEST_ASSERT(ret == 0);

	PASS();
}

TEST test_file_fun2(void)
{
	int  ret = -1;
	int  size = -1;
	int  line = -1;
	FileInfo_T finfo;

	printf(MOD_TAG"suit_file----test_file_fun2----\n");
	
	ret =  wow_file_create(WOW_TEST_FILE_STRING(TEST_FILE_PATH_03));
	GREATEST_ASSERT(ret == 0);

	ret =  wow_file_size(WOW_TEST_FILE_STRING(TEST_FILE_PATH_03),&size);
	GREATEST_ASSERT(ret == 0 && size == 0);
	
	ret =  wow_file_line_num(WOW_TEST_FILE_STRING(TEST_FILE_PATH_03),&line);
	GREATEST_ASSERT(ret == 0 && line == 0);

	ret = wow_file_rename(WOW_TEST_FILE_STRING(TEST_FILE_PATH_02),WOW_TEST_FILE_STRING(TEST_FILE_PATH_04));
	GREATEST_ASSERT(ret == 0);

	ret = wow_file_info(WOW_TEST_FILE_STRING(TEST_FILE_PATH_04),&finfo);
	GREATEST_ASSERT(ret == 0);

	printf(MOD_TAG"2-2-bbb.txt mtime:%zu atime:%zu type:%d size:%zu\n",
							finfo.mtime,finfo.atime,finfo.type,finfo.size);
	
	PASS();
}


SUITE(suit_file)
{
	RUN_TEST(test_file_fun1);
	RUN_TEST(test_file_fun2);
}



