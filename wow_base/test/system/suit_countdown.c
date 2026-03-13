#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>

#include "system/wow_countdown.h"

#include "greatest/greatest.h"
#include "wow_test_config.h"

#define MOD_TAG "[countdown]"

#if SUIT_RUN_ERROR_TEST
TEST test_countdown_error(void)
{
	int ret = 0;
	
	printf(MOD_TAG"suit_countdown----test_countdown_error\n");

	wow_cdown_reset(NULL);
	wow_cdown_set(NULL,0xFFFF);
	wow_cdown_free(NULL);
	
	ret = wow_cdown_remain(NULL);
	GREATEST_ASSERT(ret == 0);
	
	ret = wow_cdown_is_expired(NULL);
	GREATEST_ASSERT(ret == 0);
	
	PASS();
}
#endif

TEST test_countdown_fun(void)
{
	printf(MOD_TAG"suit_countdown----test_countdown_fun\n");
	
	CountDown_T* cdown = wow_cdown_new();
	GREATEST_ASSERT(cdown);
	
	wow_cdown_reset(cdown);
	
	wow_cdown_set(cdown,3*1000);
	sleep(1);
	printf(MOD_TAG"2-1-countdown remain:%d expired:%d\n",
	wow_cdown_remain(cdown),wow_cdown_is_expired(cdown));

	sleep(1);
	printf(MOD_TAG"2-2-countdown remain:%d expired:%d\n",
	wow_cdown_remain(cdown),wow_cdown_is_expired(cdown));

	sleep(1);
	printf(MOD_TAG"2-3-countdown remain:%d expired:%d\n",
	wow_cdown_remain(cdown),wow_cdown_is_expired(cdown));

	sleep(1);
	printf(MOD_TAG"2-4-countdown remain:%d expired:%d\n",
	wow_cdown_remain(cdown),wow_cdown_is_expired(cdown));

	wow_cdown_reset(cdown);
	printf(MOD_TAG"2-6-countdown remain:%d expired:%d\n",
	wow_cdown_remain(cdown),wow_cdown_is_expired(cdown));

	wow_cdown_free(&cdown);
	
	PASS();
}


SUITE(suit_countdown)
{
#if SUIT_RUN_ERROR_TEST
    RUN_TEST(test_countdown_error);
	
#endif    
	RUN_TEST(test_countdown_fun);
		
}




