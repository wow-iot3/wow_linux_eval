#ifndef __WOW_IOT_HASH_WOW_HASH_H_
#define __WOW_IOT_HASH_WOW_HASH_H_

#include "stdio.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HASH_UINT_BJ   = 0,
    HASH_UINT_BIT,
    HASH_UINT_MAX
}HashUnitIndex_E;

typedef enum {
    HASH_ARRAY_BKDR   = 0,
    HASH_ARRAY_RS,
    HASH_ARRAY_FNV1A,
    HASH_ARRAY_DJB2,
    HASH_ARRAY_MAX
}HashArraryIndex_E;

typedef size_t (*hash_func_t) (HashArraryIndex_E eIndex,uint8_t const* pu8Data,size_t snSize);

/*brief    计算uint32_t类型数据hash值
 *param ： eIndex: 计算函数选择
 *param ： u32Value: 待计算原值
 *return： 返回hash值
 */
size_t wow_hash_uint32(HashUnitIndex_E eIndex,uint32_t u32Value);


/*brief    计算字符串类型数据hash值
 *param ： eIndex:   计算函数选择
 *param ： pu8Data:  待计算原值
 *param ： snSize:   待计算原值长度
 *return： 返回hash值  无效参数返回0
 */
size_t wow_hash_data(HashArraryIndex_E eIndex,uint8_t const* pu8Data,size_t snSize);

#ifdef __cplusplus
}
#endif

#endif
