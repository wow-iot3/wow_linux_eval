
#include "prefix/wow_check.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"
#include "prefix/wow_errno.h"

#include "wow_hal_internal.h"

#include "wow_hal_errno.h"
#include "wow_hal_init.h"
#include "hal_dev_conf.h"

extern HalDevConfig_T gt_devUbuntu;

static HalDevConfig_T* register_hal_dev(void)
{
	switch(WOW_DEV_INDEX){
		case DEV_HAL_UBUNTU:
		default:	
			return &gt_devUbuntu;
			break;
	}
}



__EX_API__ int wow_hal_init(void)
{
	int ret = -1;

	HalDevConfig_T* hal = register_hal_dev();
	CHECK_RET_VAL_P(hal, -1,"register hal dev failed!\n");

	if(hal->enable->enable_bit.usart){
		ret = hal_uart_env_init(hal->usart);
		CHECK_RET_VAL_P(ret == 0, -INIT_USART_ERROR,"hal uart env init failed!\n");
	}
	
	return WOW_SUCCESS;
}

__EX_API__ int wow_hal_exit(void)
{
	HalDevConfig_T* hal = register_hal_dev();
	CHECK_RET_VAL_P(hal, -1,"register hal dev failed!\n");

	
	if(hal->enable->enable_bit.usart){
		hal_uart_env_exit();
	}
	
	return WOW_SUCCESS;
}

__EX_API__ int wow_hal_get_product(void)
{
	return WOW_DEV_INDEX;
}
