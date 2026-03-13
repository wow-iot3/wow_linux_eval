#ifndef __WOW_BASE_UTILS_WOW_MATH_H_
#define __WOW_BASE_UTILS_WOW_MATH_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <unistd.h>

/*brief    获取随机数 */
size_t wow_random_value(void);

/*brief    获取随机数
 *param ： begin:  起始值
 *param ： end:    结束值
 *return： 随机数
 */
size_t  wow_random_range_uint(size_t snBegin, size_t snEnd);
ssize_t wow_random_range_int(ssize_t ssnBegin, ssize_t ssEnd);
float   wow_random_range_float(float fBegin, float fEnd);
double  wow_random_range_double(double dBegin, double dEnd);


/*brief    获取最大素数
 *param ： u32Number:  待计算值
 *return： 成功最大素数
 */
uint32_t wow_math_getMaxPrime(uint32_t u32Number);

#ifdef __cplusplus
}
#endif

#endif
