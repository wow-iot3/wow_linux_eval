#ifndef __WOW_HAL_WOW_HAL_INIT_H_
#define __WOW_HAL_WOW_HAL_INIT_H_

typedef enum{
	DEV_HAL_UBUNTU = 0,
	DEV_HAL_AARCH64,	
	DEV_HAL_ARM_A,
	DEV_HAL_ARM_B,
	DEV_HAL_ARM_C,
	DEV_HAL_MAX
}DeviceIndex_E;
	
/*brief    hal库初始化
 *return： 成功返回0 失败返回<0
 */
int wow_hal_init(void);

/*brief    hal库退出
 *return： 成功返回0 失败返回<0
 */
int wow_hal_exit(void);

/*brief    获取当前产品型号
 *return： 成功返回标识 失败返回<0
 */
int wow_hal_get_product(void);

#endif
