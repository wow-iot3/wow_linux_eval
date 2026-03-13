#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "greatest/greatest.h"
#include "wow_test_config.h"

#include "network/wow_ntpdate.h"
#include "plugin/wow_debug.h"


#define MOD_TAG "[ntpdate]"


TEST test_ntpdate_func(void)
{
	int ret = 0;

	ret = wow_ntpdate_exec();
	GREATEST_ASSERT(ret == 0);


	PASS();
}

SUITE(suit_ntpdate)
{
	wow_debug_setStatus(MOD_TAG, SUIT_RUN_DEBUG_FLAG);
	
	RUN_TEST(test_ntpdate_func);
}

