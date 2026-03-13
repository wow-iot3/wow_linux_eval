#ifndef __WOW_HAL_WOW_HAL_ERRNO_H_
#define __WOW_HAL_WOW_HAL_ERRNO_H_

#include "prefix/wow_errno.h"

typedef enum{
	HAL_INIT_ERRNO = MODULE_HAL_ERRNO,
	HAL_USART_ERRNO,
}WowHalErrno_E;

typedef enum{
	INIT_DEV_ERROR = (HAL_INIT_ERRNO*100),
	INIT_USART_ERROR,
}WowHalInitErrno_E;

typedef enum{
	USART_NOT_INITED = (HAL_USART_ERRNO*100),
	USART_OPEN_FAILED,
	USART_NOT_OPENED,
	USART_SET_PARAM_FAILED,
}WowHalUsartErrno_E;

#endif
