#include <stdio.h>

#include "hal_dev_conf.h"

/*X86 功能项使能配置*/
static HalDevEnable_U gt_devEnableUbuntu ={
	.enable_bit.usart  = 1,
	.enable_bit.ble    = 0,
	.enable_bit.lte    = 0,
	.enable_bit.wifi   = 0,
	.enable_bit.eeprom = 0,
	.enable_bit.flash  = 0,
	.enable_bit.din    = 0,
	.enable_bit.dout   = 0,
	.enable_bit.gpio   = 0,
	.enable_bit.led    = 0,
	.enable_bit.sdcard = 0,
	.enable_bit.upan   = 0,
	.enable_bit.video  = 0,
};
	
/*X86  串口功能配置*/
static HalUsartConfig_T gt_devUsartUbuntu = {
	.num  = 2,
	.name = {"/dev/ttyUSB0","/dev/ttyUSB1"},
};

HalDevConfig_T gt_devUbuntu={ 
	.index   = DEV_HAL_UBUNTU,
	.enable  = &gt_devEnableUbuntu,
	.usart   = &gt_devUsartUbuntu,
};
	
