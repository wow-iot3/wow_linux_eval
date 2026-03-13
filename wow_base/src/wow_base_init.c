#include <stdio.h>

#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_common.h"
#include "prefix/wow_keyword.h"



#include "wow_base_config.h"
#include "wow_base_internal.h"
#include "wow_base_init.h"

__EX_API__ int wow_base_init(void)
{
	int ret = 0;

	ret = _wow_singleton_init();
	CHECK_RET_VAL_P(ret == 0,-INIT_SINGLETON_ERROR,"_wow_singleton_init failed!\n");

#if WOW_MLEAK_ENABLE	
	ret = _wow_mleak_init();
	CHECK_RET_VAL_P(ret == 0,-INIT_MLEAK_ERROR,"_wow_mleak_init failed!\n");
#endif	
	_wow_error_init();

	return 0;
}

__EX_API__ int wow_base_exit(void)
{	
	_wow_singleton_exit();

#if WOW_MLEAK_ENABLE
	_wow_mleak_exit();
#endif	

    _wow_error_exit();

	return 0;
}

