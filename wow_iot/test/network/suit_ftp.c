#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "greatest/greatest.h"
#include "wow_test_config.h"

#include "file/wow_file.h"
#include "network/wow_ftp.h"
#include "plugin/wow_debug.h"


#define MOD_TAG "[ftp]"


static FtpInfo_T g_ftp ={
	.host = "172.16.49.150",
	.port = 21,
	.user = "wow",
	.pwd  = "888888",
};

TEST test_ftp_func(void)
{
	int ret = 0;
	int size1 = 0;
	int size2 = 0;
	Ftp_T* ftp = NULL;

	ModDebug_D(MOD_TAG, "suit_ftp-------------------test_ftp_func\n");
	
	ftp = wow_ftp_login(&g_ftp);
	GREATEST_ASSERT(ftp);

	ret = wow_ftp_mkdir(ftp,"/TEST");
	GREATEST_ASSERT(ret == 0);

	ret = wow_ftp_file_upload(ftp,"/home/wow/open/wow-iot7/TEST/libwow.so.0.0.1","/TEST/libwow.so.0.0.1");
	GREATEST_ASSERT(ret == 0);

	ret = wow_ftp_file_download(ftp,"/TEST/libwow.so.0.0.1","/home/wow/open/wow-iot7/TEST/libwow.so.0.0.1_bak");
	GREATEST_ASSERT(ret == 0);

	ret = wow_file_size("/home/wow/open/wow-iot7/TEST/libwow.so.0.0.1",&size1);
	GREATEST_ASSERT(ret == 0);

	ret = wow_file_size("/home/wow/open/wow-iot7/TEST/libwow.so.0.0.1_bak",&size2);
	GREATEST_ASSERT(ret == 0);
	GREATEST_ASSERT(size1 == size2);

	wow_ftp_exit(&ftp);

	PASS();
}

SUITE(suit_ftp)
{
	wow_debug_setStatus(MOD_TAG, SUIT_RUN_DEBUG_FLAG);
	
	RUN_TEST(test_ftp_func);
}

