#ifndef __WOW_IOT_ALGORITHM_WOW_BLOOMFILTER_H_
#define __WOW_IOT_ALGORITHM_WOW_BLOOMFILTER_H_

#ifdef __cplusplus
extern "C" {
#endif


typedef struct bloom_filter_t  BloomFilter_T;

/// the probability of false positives
typedef enum{
	TB_BLOOM_FILTER_PROBABILITY_0_1         = 3,  ///!< 1 / 2^3 = 0.125 ~= 0.1
	TB_BLOOM_FILTER_PROBABILITY_0_01        = 6,  ///!< 1 / 2^6 = 0.015625 ~= 0.01
	TB_BLOOM_FILTER_PROBABILITY_0_001       = 10, ///!< 1 / 2^10 = 0.0009765625 ~= 0.001
	TB_BLOOM_FILTER_PROBABILITY_0_0001      = 13, ///!< 1 / 2^13 = 0.0001220703125 ~= 0.0001
	TB_BLOOM_FILTER_PROBABILITY_0_00001     = 16, ///!< 1 / 2^16 = 0.0000152587890625 ~= 0.00001
	TB_BLOOM_FILTER_PROBABILITY_0_000001    = 20  ///!< 1 / 2^20 = 0.00000095367431640625 ~= 0.000001
}BloomFilterAbility_E;



/*brief    初始化布隆过滤器
 *param ： eAbility : 期望精度
 *param ： snMaxItem:元素最大个数
 *return： 成功返回操作描述内容 失败返回NULL
 */
BloomFilter_T* wow_bloomfilter_init(BloomFilterAbility_E eAbility, size_t snMaxItem);


/*brief    退出布隆过滤器
 *param ： pptFilter: 操作描述内容地址
 *return： 无
 */
void wow_bloomfilter_exit(BloomFilter_T** pptFilter);


/*brief    清空布隆过滤器
 *param ： ptFilter: 操作描述内容
 *return： 无
 */
void wow_bloomfilter_clear(BloomFilter_T* ptFilter);

/*brief    设置布隆过滤器内容
 *param ： ptFilter: 操作描述内容
 *param ： pData   : 设置内容
 *return： 设置成功返回0 已存在返回1  失败返回<0
 */
int wow_bloomfilter_set_1(BloomFilter_T* ptFilter, const void* pData);

/*brief    设置布隆过滤器内容
 *param ： ptFilter: 操作描述内容
 *param ： pData   : 设置内容
 *return： 设置成功返回0  失败返回<0
 */
int wow_bloomfilter_set_0(BloomFilter_T* ptFilter, const void* pData);

/*brief    获取布隆过滤器内容
 *param ： ptFilter: 操作描述内容
 *param ： pData   : 设置内容
 *return： 获取成功返回1 不存在返回0 失败返回<0
 */
 int wow_bloomfilter_get(BloomFilter_T* ptFilter, const void* pData);
 
#ifdef __cplusplus
 }
#endif


#endif

