#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "greatest/greatest.h"

#include "wow_test_config.h"

#include "file/wow_hfile.h"

#define MOD_TAG "[hfile]"
#define HFILE_FIO_FILE_DEMO "file/hfile_fio.text"
#define HFILE_IO_FILE_DEMO  "file/hfile_io.text"

TEST test_hfile_fun1(File_T* file)
{
	int  ret = -1;
	char data[1024] = {};
	ret =  wow_hfile_read(file,data,sizeof(data));
	printf("1-1-read(%d):%s\n",ret,data);

	ret =  wow_hfile_seek(file,ret - 10,SEEK_SET);

	ret =  wow_hfile_read(file,data,sizeof(data));
	printf("1-2-read off(%d):%s\n",ret,data);
	
	ret =  wow_hfile_seek(file,1,SEEK_SET);
	ret =  wow_hfile_write(file,"BBBBBBBBBBB",strlen("BBBBBBBBBBB"));
	GREATEST_ASSERT(ret > 0);

	ret =  wow_hfile_seek(file,0,SEEK_SET);
	ret =  wow_hfile_read(file,data,sizeof(data));
	printf("1-3-read off(%d):%s\n",ret,data);
	
	ret =  wow_hfile_seek(file,100,SEEK_SET);
	ret =  wow_hfile_write(file,"CCCCCCCCCCCCC",strlen("CCCCCCCCCCCCC"));
	GREATEST_ASSERT(ret > 0);
	
	ret =  wow_hfile_seek(file,0,SEEK_SET);
	ret =  wow_hfile_read(file,data,sizeof(data));
	printf("1-4-read off(%d):%s\n",ret,data);

	PASS();
}


TEST test_hfile_fio(void)
{
	int  ret = -1;
	File_T* file = NULL;
	
	printf(MOD_TAG"suit_hfile----test_hfile_fio---\n");
	
	file =  wow_hfile_open(FILE_FIO,WOW_TEST_FILE_STRING(HFILE_FIO_FILE_DEMO),FILE_CREATE|FILE_RDWR);
	GREATEST_ASSERT(file);

	ret =  wow_hfile_write(file,"AAAAAAAAAAAAAAA",strlen("AAAAAAAAAAAAAAA"));
	GREATEST_ASSERT(ret > 0);

	wow_hfile_close(file);
	
	file =  wow_hfile_open(FILE_FIO,WOW_TEST_FILE_STRING(HFILE_FIO_FILE_DEMO),FILE_RDWR);
	GREATEST_ASSERT(file);

	test_hfile_fun1(file);

	wow_hfile_close(file);

	PASS();
}

TEST test_hfile_io(void)
{
	int  ret = -1;
	File_T* file = NULL;
	
	printf(MOD_TAG"suit_hfile----test_hfile_io---\n");
	
	file =  wow_hfile_open(FILE_IO,WOW_TEST_FILE_STRING(HFILE_IO_FILE_DEMO),FILE_CREATE|FILE_RDWR);
	GREATEST_ASSERT(file);

	ret =  wow_hfile_write(file,"AAAAAAAAAAAAAAA",strlen("AAAAAAAAAAAAAAA"));
	GREATEST_ASSERT(ret > 0);

	wow_hfile_close(file);
	
	file =  wow_hfile_open(FILE_IO,WOW_TEST_FILE_STRING(HFILE_IO_FILE_DEMO),FILE_RDWR);
	GREATEST_ASSERT(file);

	test_hfile_fun1(file);

	wow_hfile_close(file);

	PASS();
}

SUITE(suit_hfile)
{
	RUN_TEST(test_hfile_fio);
	RUN_TEST(test_hfile_io);
}



