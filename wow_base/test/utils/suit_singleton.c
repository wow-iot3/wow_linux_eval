#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>


#include "greatest/greatest.h"

#include "utils/wow_singleton.h"

#define MOD_TAG "[singleton]"

int  test1_init(void)
{
	printf(MOD_TAG"----test1_init----\n");
	return 0;
}
int  test2_init(void)
{
	printf(MOD_TAG"----test2_init----\n");
	return 0;
}
int  test3_init(void)
{
	printf(MOD_TAG"----test3_init----\n");
	return 0;
}

void  test1_exit(void)
{
	printf(MOD_TAG"----test1_exit----\n");
}

void  test3_exit(void)
{
	printf(MOD_TAG"----test3_exit----\n");
}


TEST test_singleton(void)
{
	int ret = 0;
	
	printf(MOD_TAG"suit_singleton----test_singleton\n");

	ret = wow_singleton_instance(SINGLETON_TYPE_APP+1,test1_init,test1_exit);
	GREATEST_ASSERT(ret == 0);
	ret = wow_singleton_instance(SINGLETON_TYPE_APP+2,test2_init,NULL);
	GREATEST_ASSERT(ret == 0);
	ret = wow_singleton_instance(SINGLETON_TYPE_APP+3,test3_init,test3_exit);
	GREATEST_ASSERT(ret == 0);
	ret = wow_singleton_instance(SINGLETON_TYPE_APP+1,test1_init,test1_exit);
	GREATEST_ASSERT(ret == 0);

	PASS();
}

SUITE(suit_singleton)
{
    RUN_TEST(test_singleton);
}

