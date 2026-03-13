#include <stdio.h>


#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_common.h"
#include "prefix/wow_keyword.h"

#include "wow_iot_internal.h"
#include "wow_iot_errno.h"
#include "wow_iot_init.h"

__EX_API__ int wow_iot_init(void)
{
	int ret = -1;
#if 1
	ret = _wow_shell_env_init();
	CHECK_RET_VAL_P(ret == 0,-IOT_INIT_SHELL_ERROR,"_wow_shell_env_init failed!\n");
#endif	
	ret = _wow_debug_env_init();
	CHECK_RET_VAL_P(ret == 0,-IOT_INIT_DEBUG_ERROR,"_wow_shell_env_init failed!\n");

	ret = _wow_log_env_init();
	CHECK_RET_VAL_P(ret == 0,-IOT_INIT_LOG_ERROR,"_wow_log_env_init failed!\n");

	ret = _wow_sqlite3_library_init();
	CHECK_RET_VAL_P(ret == 0,-IOT_INIT_SQLITE3_ERROR,"_wow_sqlite3_library_init failed!\n");
	
	return 0;
}



__EX_API__ int wow_iot_exit(void)
{
	return 0;
}

