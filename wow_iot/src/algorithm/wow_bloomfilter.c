#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_common.h"
#include "prefix/wow_keyword.h"

#include "hash/wow_hash.h"
#include "algorithm/wow_bloomfilter.h"

#include "wow_iot_errno.h"

/*! 
 * A Bloom filter is a space-efficient probabilistic data structure, 
 * conceived by Burton Howard Bloom in 1970, that is used to test whether an element is a member of a set. 
 * False positive matches are possible, but false negatives are not; 
 * i.e. a query returns either "possibly in set" or "definitely not in set". 
 * Elements can be added to the set, but not removed (though this can be addressed with a "counting" filter). 
 * The more elements that are added to the set, the larger the probability of false positives.
 *
 * Assume that a hash function selects each array position with equal probability. 
 * If m is the number of bits in the array, and k is the number of hash functions, 
 * then the probability that a certain bit is not set to 1 by a certain hash function 
 * during the insertion of an element is then
 * 1 - 1 / m
 *
 * The probability that it is not set to 1 by any of the hash functions is
 * (1 - 1/ m) ^ k
 *
 * If we have inserted n elements, the probability that a certain bit is still 0 is
 * (1 - 1/ m) ^ kn
 *
 * the probability that it is 1 is therefore
 * 1 - ((1 - 1/ m) ^ kn)
 *
 * Now test membership of an element that is not in the set.
 * Each of the k array positions computed by the hash functions is 1 with a probability as above.
 * The probability of all of them being 1, 
 * which would cause the algorithm to erroneously claim that the element is in the set, is often given as
 * p = (1 - ((1 - 1/ m) ^ kn))^k ~= (1 - e^(-kn/m))^k
 *
 * For a given m and n, the value of k (the number of hash functions) that minimizes the probability is
 * k = (m / n) * ln2 ~= (m / n) * (9 / 13)
 *
 * which gives
 * 2 ^ -k ~= 0.6185 ^ (m / n)
 *
 * The required number of bits m, given n (the number of inserted elements) 
 * and a desired false positive probability p (and assuming the optimal value of k is used) 
 * can be computed by substituting the optimal value of k in the probability expression above:
 * p = (1 - e ^-(m/nln2)n/m))^(m/nln2)
 *
 * which can be simplified to:
 * lnp = -m/n * (ln2)^2
 *
 * This optimal results in:
 * s = m/n = -lnp / (ln2 * ln2) = -log2(p) / ln2
 * k = s * ln2 = -log2(p) <= note: this k will be larger
 *
 * compute s(m/n) for given k and p:
 * p = (1 - e^(-kn/m))^k = (1 - e^(-k/s))^k
 * => lnp = k * ln(1 - e^(-k/s))
 * => (lnp) / k = ln(1 - e^(-k/s))
 * => e^((lnp) / k) = 1 - e^(-k/s)
 * => e^(-k/s) = 1 - e^((lnp) / k) = 1 - (e^lnp)^(1/k) = 1 - p^(1/k)
 * => -k/s = ln(1 - p^(1/k))
 * => s = -k / ln(1 - p^(1/k)) and define c = p^(1/k)
 * => s = -k / ln(1 - c)) and ln(1 + x) ~= x - 0.5x^2 while x < 1 
 * => s ~= -k / (-c-0.5c^2) = 2k / (2c + c * c)
 *
 * so 
 * c = p^(1/k)
 * s = m / n = 2k / (2c + c * c)
 */
 

#define BLOOM_FILTER_DATA_MAXN            (1 << 28)
#define BLOOM_FILTER_ITEM_MAXN            (1 << 16)

#define bloom_filter_set1(data, i)           do {(data)[(i) >> 3] |= (0x1 << ((i) & 7));} while (0)
#define bloom_filter_set0(data, i)           do {(data)[(i) >> 3] &= ~(0x1 << ((i) & 7));} while (0)
#define bloom_filter_bset(data, i)           ((data)[(i) >> 3] & (0x1 << ((i) & 7)))
#define wow_math_round(x)                    ((x) > 0? (size_t)round((x) + 0.5) : (size_t)round((x) - 0.5))

struct bloom_filter_t{
    size_t       probability;
    size_t       maxn;
    size_t       size;
    uint8_t*     data;
    size_t       mask;
};

/*brief    初始化布隆过滤器
 *param ： eAbility : 期望精度
 *param ： snMaxItem:元素最大个数
 *return： 成功返回操作描述内容 失败返回NULL
 */
BloomFilter_T* wow_bloomfilter_init(BloomFilterAbility_E eAbility,size_t snMaxItem)

{
	CHECK_RET_VAL_ERRNO_P(eAbility  && eAbility <= TB_BLOOM_FILTER_PROBABILITY_0_000001,
								-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    BloomFilter_T* filter = CALLOC(1,BloomFilter_T);
    CHECK_RET_VAL_ERRNO_P(filter,-SYSTEM_MALLOC_FAILED,"malloc BloomFilter_T failed!\n");
	
    // init filter
    filter->probability = eAbility;
	filter->maxn        = snMaxItem;

    /* compute the storage space
     *
     * c = p^(1/k)
     * s = m / n = 2k / (2c + c * c)
     */
    double k = (double)HASH_ARRAY_MAX;
    double p = 1.0 / (double)(1 << eAbility);
    double c = pow(p, 1 / k);
    double s = (k + k) / (c + c + c * c);
    size_t n = snMaxItem;
    size_t m = wow_math_round(s *n);
	
    filter->size = Align8(m) >> 3;
	CHECK_RET_GOTO_ERRNO_P(filter->size <= BLOOM_FILTER_DATA_MAXN,out,
                                -ALGORITHM_SIZE_ERROR,"param input arg invalid!\n");

    filter->data = MALLOC(filter->size);
	CHECK_RET_GOTO_ERRNO_P(filter->data,out,-SYSTEM_MALLOC_FAILED,"malloc filter fata failed!\n");
	memset(filter->data,0,filter->size);
	
    filter->mask = Align_pow2((filter->size << 3)) - 1;

    return (BloomFilter_T*)filter;
out:
	FREE(filter);
	return NULL;
}


/*brief    退出布隆过滤器
 *param ： pptFilter: 操作描述内容地址
 *return： 无
 */
void wow_bloomfilter_exit(BloomFilter_T** pptFilter)
{
	CHECK_RET_VOID(pptFilter && *pptFilter);
	
	BloomFilter_T* filter = (BloomFilter_T*)*pptFilter;
    if(filter->data) {
		FREE(filter->data);
	}
    filter->data = NULL;
	
    FREE(filter);
	*pptFilter = NULL;
}

/*brief    清空布隆过滤器
 *param ： ptFilter: 操作描述内容
 *return： 无
 */
void wow_bloomfilter_clear(BloomFilter_T* ptFilter)
{
    CHECK_RET_VOID(ptFilter);

    if (ptFilter->data && ptFilter->size) {
		memset(ptFilter->data, 0, ptFilter->size);
	}
}

/*brief    设置布隆过滤器内容
 *param ： ptFilter: 操作描述内容
 *param ： pData   : 设置内容
 *return： 设置成功返回0 已存在返回1  失败返回<0
 */
int wow_bloomfilter_set_1(BloomFilter_T* ptFilter, const void* pData)
{
    CHECK_RET_VAL_P(ptFilter, -PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
    CHECK_RET_VAL_P(pData, -PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

    size_t i = 0;
	size_t index = 0;
	int repeact = 0;
    for (i = 0; i < HASH_ARRAY_MAX; i++)
    {
        index = wow_hash_data(i,pData,strlen((const char*)pData))% ptFilter->mask;
		
        if (index >= (ptFilter->size << 3)){
			index %= (ptFilter->size << 3);
		}
		
		if(bloom_filter_bset(ptFilter->data, index)){
			repeact++;
			continue;
		}
        bloom_filter_set1(ptFilter->data, index);
    }
	
	return (repeact == HASH_ARRAY_MAX);
}

/*brief    设置布隆过滤器内容
 *param ： ptFilter: 操作描述内容
 *param ： pData   : 设置内容
 *return： 设置成功返回0  失败返回<0
 */
int wow_bloomfilter_set_0(BloomFilter_T* ptFilter, const void* pData)
{
    CHECK_RET_VAL_P(ptFilter, -PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
    CHECK_RET_VAL_P(pData, -PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	
	 size_t i = 0;
	 size_t index = 0;
	 
	 for (i = 0; i < HASH_ARRAY_MAX; i++)
	 {
		 index = wow_hash_data(i,pData,strlen((const char*)pData))% ptFilter->mask;
		 
		 if (index >= (ptFilter->size << 3)){
			 index %= (ptFilter->size << 3);
		 }

		 bloom_filter_set0(ptFilter->data, index);
	 }
	 
	 return 0;

}

/*brief    获取布隆过滤器内容
 *param ： ptFilter: 操作描述内容
 *param ： pData   : 设置内容
 *return： 获取成功返回1 不存在返回0 失败返回<0
 */
int wow_bloomfilter_get(BloomFilter_T* ptFilter, const void* pData)
{
    CHECK_RET_VAL_P(ptFilter, -PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
    CHECK_RET_VAL_P(pData, -PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

    size_t i = 0;
	size_t index = 0;

    for (i = 0; i < HASH_ARRAY_MAX; i++)
    {
        index = wow_hash_data(i,pData,strlen((const char*)pData))% ptFilter->mask;
        if (index >= (ptFilter->size << 3)){
			index %= (ptFilter->size << 3);
		} 
        if (!bloom_filter_bset(ptFilter->data, index)){
			 break;
		}
    }
    return (i ==  HASH_ARRAY_MAX);
}

