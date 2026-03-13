#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "greatest/greatest.h"

#include "plugin/wow_plugin.h"

#define MOD_TAG "[plugin]"

static int in_plugin_test_init_01(void)
{
	printf(MOD_TAG"--------in_plugin_test_init_01--------\n");

	return 0;
}

static void in_plugin_test_exit_01(void)
{
	printf(MOD_TAG"--------in_plugin_test_exit_01--------\n");
}

static int in_plugin_test_init_02(void)
{
	printf(MOD_TAG"--------in_plugin_test_init_02--------\n");
	return 0;
}

static void in_plugin_test_exit_02(void)
{
	printf(MOD_TAG"--------in_plugin_test_exit_02--------\n");
}
static int in_plugin_test_init_03(void)
{
	printf(MOD_TAG"--------in_plugin_test_init_03--------\n");
	return 0;
}
static void in_plugin_test_exit_03(void)
{
	printf(MOD_TAG"--------in_plugin_test_exit_03--------\n");
}
static int in_plugin_test_init_04(void)
{
	printf(MOD_TAG"--------in_plugin_test_init_04--------\n");
	return 0;
}

static void in_plugin_test_exit_04(void)
{
	printf(MOD_TAG"--------in_plugin_test_exit_04--------\n");
}

TEST test_plugin_fun(void)
{
	printf(MOD_TAG"suit_plugin----test_plugin_fun\n");

	int ret = 0;

	ret = wow_plugin_init("ex_plugin_test_01@ex_plugin_test_02@in_plugin_test01@in_plugin_test04@in_plugin_test02");
	GREATEST_ASSERT(ret == 0);


	wow_plugin_exit();
	
	PASS();
}

SUITE(suit_plugin)
{
	test_plugin_fun();
}

DECLARE_PLUGIN_BUILDIN(in_plugin_test01,WOW_PLUGIN_PRIORITY_DEFAULT,in_plugin_test_init_01, in_plugin_test_exit_01)
DECLARE_PLUGIN_BUILDIN(in_plugin_test02,WOW_PLUGIN_PRIORITY_LOW,in_plugin_test_init_02, in_plugin_test_exit_02)	
DECLARE_PLUGIN_BUILDIN(in_plugin_test03,WOW_PLUGIN_PRIORITY_HIGH,in_plugin_test_init_03, in_plugin_test_exit_03) 
DECLARE_PLUGIN_BUILDIN(in_plugin_test04,WOW_PLUGIN_PRIORITY_HIGH,in_plugin_test_init_04, in_plugin_test_exit_04) 

