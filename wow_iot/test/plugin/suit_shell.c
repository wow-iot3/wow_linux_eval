#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>


#include "greatest/greatest.h"

#include "plugin/wow_shell.h"


static int shell_test1(int argc, char *argv[])
{
	printf("***********shell_test1***********\n");
	return 0;
}
static int shell_test2(int argc, char *argv[])
{
	printf("***********shell_test2***********\n");
	return 0;
}
WOW_DECLARE_SHELL_CMD("suit_shell_1",shell_test1,"suit_shell_1");
WOW_DECLARE_SHELL_CMD("suit_shell_2",shell_test2,"suit_shell_2");


TEST test_shell(void)
{
    PASS();
}


SUITE(suit_shell)
{
    RUN_TEST(test_shell);
}



