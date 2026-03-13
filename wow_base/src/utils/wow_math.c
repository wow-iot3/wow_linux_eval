#include <stdio.h>
#include <stdlib.h>

#include "prefix/wow_check.h"
#include "prefix/wow_keyword.h"

#include "utils/wow_math.h"

void wow_random_seed(size_t seed)
{
    srandom(seed);
}

static unsigned int g_last_seed = 1;
size_t wow_random_value()
{
	size_t seed =  random();
	g_last_seed += (seed >> ((seed ^ g_last_seed) % 3));

	srandom(g_last_seed ^ seed);///<srand
    return (size_t)random();    ///<rand
}

size_t wow_random_range_uint(size_t begin, size_t end)
{
    CHECK_RET_VAL(begin < end, begin);

    return (begin + (wow_random_value() % (end - begin)));
}

ssize_t wow_random_range_int(ssize_t begin, ssize_t end)
{
    CHECK_RET_VAL(begin < end, begin);

    return (begin + (wow_random_value() % (end - begin)));
}

float wow_random_range_float(float begin, float end)
{
    CHECK_RET_VAL(begin < end, begin);

    double factor = (double)wow_random_range_int(0, MAXS32) / (double)MAXS32;
	double value = (end - begin) * factor;
		
    return ((float)value + begin);
}

double wow_random_range_double(double begin, double end)
{
    CHECK_RET_VAL(begin < end, begin);

    double factor = (double)wow_random_range_int(0, MAXS32) / (double)MAXS32;
	double value = (end - begin) * factor;
		
    return (value + begin);
}


/*brief    获取最大素数
 *param ： number:  待计算值
 *return： 成功最大素数
 */
uint32_t wow_math_getMaxPrime(uint32_t num)
{
	uint32_t i = 0;
	uint32_t j = 0;

	for(i = num ; i > 2; i--){
		for(j = 2;j < (i-1);j++){
			if(i%j==0)  {
				break;
			}
		}
		if(j == i -1){
			return i;
		}
	} 
	return 1;
}
