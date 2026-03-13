#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "greatest/greatest.h"

#include "file/wow_dir.h"

#include "wow_test_config.h"

#define MOD_TAG "[dir]"

#define TEST_DIR_PATH_01 "file/1/1/1/"
#define TEST_DIR_PATH_02 "file/1/1/2/"


TEST test_dir_create(void)
{
	int  ret = -1;
	
	printf(MOD_TAG"suit_dir----test_dir_create\n");

	ret = wow_dir_create(WOW_TEST_FILE_STRING(TEST_DIR_PATH_01));
	GREATEST_ASSERT(ret == 0);

	ret = wow_dir_create(WOW_TEST_FILE_STRING(TEST_DIR_PATH_02));
	GREATEST_ASSERT(ret == 0);
	
	PASS();
}

TEST test_dir_remove(void)
{
	int  ret = -1;

	printf(MOD_TAG"suit_dir----test_dir_remove\n");
	
	ret =  wow_dir_remove(WOW_TEST_FILE_STRING(TEST_DIR_PATH_02));
	GREATEST_ASSERT(ret == 0);


	PASS();
}

TEST test_dir_check(void)
{
	int  ret = -1;

	printf(MOD_TAG"suit_dir----test_dir_check\n");
	
	ret =  wow_path_is_dir(WOW_TEST_FILE_STRING(TEST_DIR_PATH_01));
	GREATEST_ASSERT(ret == 1);

	ret =  wow_path_is_dir(WOW_TEST_FILE_STRING(TEST_DIR_PATH_02));
	GREATEST_ASSERT(ret != 1);

	ret =  wow_dir_remove(WOW_TEST_FILE_STRING("file"));
	GREATEST_ASSERT(ret == 0);

	PASS();
}


TEST test_dir_list(void)
{
	printf(MOD_TAG"suit_dir----test_dir_list\n");
	
	int i = 0;
	
	StringList_T* list =  wow_dir_fileList(SUIT_TEST_FILE_PATH);
	GREATEST_ASSERT(list);

	int size = wow_stringlist_size(list);
	if(size > 0){
		for(i = 0; i < size; i++){
			printf("%d --- file name:%s\n",i,wow_stringlist_data(list,i));
		}
	}
	
	//最后资源必须释放
	wow_stringlist_free(&list);
	
	PASS();
}


SUITE(suit_dir)
{
	RUN_TEST(test_dir_create);
	RUN_TEST(test_dir_remove);
	RUN_TEST(test_dir_check);
	RUN_TEST(test_dir_list);
}


