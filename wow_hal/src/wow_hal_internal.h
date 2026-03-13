#ifndef __WOW_HAL_HAL_ENV_H_
#define __WOW_HAL_HAL_ENV_H_

#include "hal_dev_conf.h"

int hal_uart_env_init(HalUsartConfig_T* uart);
int hal_uart_env_exit(void);

#endif