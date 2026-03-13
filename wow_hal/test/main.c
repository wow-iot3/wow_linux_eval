#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "greatest.h"
#include "wow_hal_init.h"

GREATEST_MAIN_DEFS(); 


int main(int argc,char* argv[])
{
    GREATEST_MAIN_BEGIN();	

	wow_hal_init();

	wow_hal_exit();
	
    GREATEST_MAIN_END();

}

