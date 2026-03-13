#ifndef __WOW_HAL_HAL_DEV_CONF_H_
#define __WOW_HAL_HAL_DEV_CONF_H_

/*设计原则  
	---差异化较大的内容通过函数指针指向其实现内容
	---差异化较小的内容已结构体内容体现差异
	---通用模块复用，通用函数复用
*/
#include <stdbool.h>
#include <stdint.h>

#include "wow_hal_init.h"


///< 设备硬件型号 此参数后续通过makefile传递设置
#ifdef PRODUCT_UBUNTU
	#define WOW_DEV_INDEX    DEV_HAL_UBUNTU
#elif defined PRODUCT_AARCH64
	#define WOW_DEV_INDEX     DEV_HAL_AARCH64	
#elif defined PRODUCT_ARM_A
	#define WOW_DEV_INDEX     DEV_HAL_ARM_A
#elif defined PRODUCT_ARM_B
	#define	WOW_DEV_INDEX 	  DEV_HAL_ARM_B
#elif defined PRODUCT_ARM_C
	#define	WOW_DEV_INDEX 	  DEV_HAL_ARM_C
#else
	#define WOW_DEV_INDEX    DEV_HAL_UBUNTU
#endif


#define MAX_DEV_NAME_SIZE 32

/******************************整体功能 配置声明****************************************/
typedef union {
	uint16_t data;
	struct{
		uint16_t usart:1;
		uint16_t ble:1;
		uint16_t lte:1;
		uint16_t wifi:1;
		uint16_t eeprom:1;
		uint16_t flash:1;
		uint16_t din:1;
		uint16_t dout:1;
		uint16_t gpio:1;
		uint16_t led:1;
		uint16_t sdcard:1;
		uint16_t upan:1;
		uint16_t video:1;
	} enable_bit;
}HalDevEnable_U;


/******************************UATT 配置声明****************************************/
///<UART按照通用模块编写，填充差异配置
#define  UART_MAX_DEV_NUM  10
typedef struct{
	uint8_t	num;
    char    name[UART_MAX_DEV_NUM][MAX_DEV_NAME_SIZE];  
}HalUsartConfig_T;

typedef struct{
	DeviceIndex_E     index;
	HalDevEnable_U*   enable;
	HalUsartConfig_T* usart;
}HalDevConfig_T;


#endif
