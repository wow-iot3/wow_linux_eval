#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#include "config/wow_ini.h"
#include "file/wow_hfile.h"
#include "plugin/wow_debug.h"

#include "greatest/greatest.h"
#include "wow_test_config.h"


#define MOD_TAG "ini"
#define INI_FILE_TEST "config/test_ini.ini"
#define INI_FILE_SAVE "config/save_ini.ini"
#define INI_FILE_DEMO "config/demo.ini"
#define INI_FILE_SIMP "config/simple.ini"

#define TEST_STR "[Pizza]\n" \
    "\n" \
    "Ham       = yes ;\n" \
    "Mushrooms = TRUE ;\n" \
    "Capres    = 0 ;\n" \
    "Cheese    = Non ;\n"\
    "\n"\
    "\n"\
    "[Wine]\n"\
    "\n"\
    "Grape     = Cabernet Sauvignon ;\n"\
    "Year      = 1989 ;\n"\
    "Country   = Spain ;\n"\
    "Alcohol   = 12.5  ;\n"\
    "\n"


TEST test_ini_create(void)
{
	ModDebug_I(MOD_TAG,"suit_ini.........test_ini_file_create\n");
	File_T* hfile = wow_hfile_open(FILE_FIO,WOW_TEST_FILE_STRING(INI_FILE_TEST),FILE_RDONLY|FILE_CREATE);
	GREATEST_ASSERT(hfile);

	int ret = wow_hfile_write(hfile, TEST_STR, strlen(TEST_STR));
	GREATEST_ASSERT(ret  == strlen(TEST_STR));

	wow_hfile_close(hfile);

	PASS();
}

TEST test_ini_get(void)
{
	int ret = 0;
	bool b = false;
	int  i = 0;
	double d = 0.0;
    char* s = NULL;

	ModDebug_I(MOD_TAG,"suit_ini.........test_ini_file_get\n");
		
    Ini_PT ini = wow_ini_init(WOW_TEST_FILE_STRING(INI_FILE_TEST));
	GREATEST_ASSERT(ini);

    ret = wow_ini_get_bool(ini, "pizza:ham",&b);
	GREATEST_ASSERT(ret == 0 && b == true);
    ret = wow_ini_get_bool(ini, "pizza:mushrooms",&b);
    GREATEST_ASSERT(ret == 0 && b == true);
    ret = wow_ini_get_bool(ini, "pizza:capres",&b);
    GREATEST_ASSERT(ret == 0 && b == false);
    ret = wow_ini_get_bool(ini, "pizza:cheese",&b);
    GREATEST_ASSERT(ret == 0 && b == false);

    ret = wow_ini_get_string(ini, "wine:grape",&s);
   	GREATEST_ASSERT(ret == 0 && !strcmp("Cabernet Sauvignon",s));

    ret = wow_ini_get_int(ini, "wine:year", &i);
    GREATEST_ASSERT(ret == 0 && i == 1989);

    ret = wow_ini_get_string(ini, "wine:country",&s);
    GREATEST_ASSERT(ret == 0 && !strcmp("Spain",s));

    ret = wow_ini_get_double(ini, "wine:alcohol",&d);
    GREATEST_ASSERT(ret == 0 && d == 12.5);

    wow_ini_exit(&ini);
	
    PASS(); 
}

TEST test_ini_set(void)
{
	int ret = 0;

	ModDebug_I(MOD_TAG,"suit_ini.........test_ini_file_set\n");
		
    Ini_PT ini = wow_ini_init(WOW_TEST_FILE_STRING(INI_FILE_TEST));
	GREATEST_ASSERT(ini);

    ret = wow_ini_set_string(ini, "new",NULL);
	GREATEST_ASSERT(ret == 0);

    ret = wow_ini_set_string(ini, "new:test1","y");
	GREATEST_ASSERT(ret == 0);
	
    ret = wow_ini_set_string(ini, "new:test5","n");
	GREATEST_ASSERT(ret == 0);

    ret = wow_ini_set_string(ini, "new:test2","test2");
   	GREATEST_ASSERT(ret == 0);

	ret = wow_ini_set_string(ini, "new:test3","1999");
	GREATEST_ASSERT(ret == 0);
	ret = wow_ini_set_string(ini, "new:test6","2000");
	GREATEST_ASSERT(ret == 0);

	ret = wow_ini_set_string(ini, "new:test4","24.04");
	GREATEST_ASSERT(ret == 0);

	ret = wow_ini_del(ini, "new:test6");
	GREATEST_ASSERT(ret == 0);

	ret = wow_ini_save(ini,WOW_TEST_FILE_STRING(INI_FILE_SAVE));
	GREATEST_ASSERT(ret == 0);

    wow_ini_exit(&ini);
	
    PASS(); 
}


TEST test_ini_normal(void)
{
	int ret = 0;
	
	ModDebug_I(MOD_TAG,"suit_ini.........test_ini_normal\n");
		
	Ini_PT ini = wow_ini_init(WOW_TEST_FILE_STRING(INI_FILE_DEMO));
	GREATEST_ASSERT(ini);

	int size = 0;
	int addr = 0;
	int dl = 0;
	char* device;
	
	ret = wow_ini_get_int(ini, "FLASH:CacheExcludeSize", &size);
    GREATEST_ASSERT(ret == 0 && size == 0x12);
	ret = wow_ini_get_int(ini, "FLASH:CacheExcludeAddr", &addr);
    GREATEST_ASSERT(ret == 0 && addr == 0x01);
	ret = wow_ini_get_int(ini, "FLASH:EnableFlashDL", &dl);
    GREATEST_ASSERT(ret == 0 && dl == 2);
	
	ret = wow_ini_get_string(ini, "FLASH:Device", &device);
    GREATEST_ASSERT(ret == 0 && !strcmp("ARM7",device));

	PASS();
}

TEST test_ini_simple(void)
{
	int ret = 0;
	
	ModDebug_I(MOD_TAG,"suit_ini.........test_ini_simple\n");
		
	Ini_PT ini = wow_ini_init(WOW_TEST_FILE_STRING(INI_FILE_SIMP));
	GREATEST_ASSERT(ini);

	int size = 0;
	int addr = 0;
	int dl = 0;
	char* device;
	
	ret = wow_ini_get_int(ini, ":CacheExcludeSize", &size);
    GREATEST_ASSERT(ret == 0 && size == 0x12);
	ret = wow_ini_get_int(ini, ":CacheExcludeAddr", &addr);
    GREATEST_ASSERT(ret == 0 && addr == 0x01);
	ret = wow_ini_get_int(ini, ":EnableFlashDL", &dl);
    GREATEST_ASSERT(ret == 0 && dl == 2);
	
	ret = wow_ini_get_string(ini, ":Device", &device);
    GREATEST_ASSERT(ret == 0 && !strcmp("ARM7",device));

	PASS();
}

SUITE(suit_ini)
{
	wow_debug_setStatus(MOD_TAG,SUIT_RUN_DEBUG_FLAG);

	RUN_TEST(test_ini_create);
    RUN_TEST(test_ini_get);
	RUN_TEST(test_ini_set);
	RUN_TEST(test_ini_normal);
	RUN_TEST(test_ini_simple);
}

