#include <string.h>
#include <stdint.h>

#include "prefix/wow_check.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"

#include "hash/wow_hash.h"

static size_t hash_data_func_int_bj(uint32_t value)
{
    // Bob Jenkins' 32 bit integer hash function
    value = (value + 0x7ed55d16) + (value << 12); 
    value = (value ^ 0xc761c23c) ^ (value >> 19); 
    value = (value + 0x165667b1) + (value << 5); 
    value = (value + 0xd3a2646c) ^ (value << 9); 
    value = (value + 0xfd7046c5) + (value << 3);
    value = (value ^ 0xb55a4f09) ^ (value >> 16);
	
    return value;
}

static size_t hash_data_func_int_bit(uint32_t value)
{
    value = ~value + (value << 15);
    value = value ^ (value >> 12); 
    value = value + (value << 2); 
    value = value ^ (value >> 4); 
    value = value * 2057;
    value = value ^ (value >> 16); 
    return value;
}

static size_t (*gf_int_hash_func[])(uint32_t) = {
	hash_data_func_int_bj,
	hash_data_func_int_bit,
};


static size_t hash_data_func_bkdr(uint8_t const* data, size_t size)
{
	size_t value = 0;
	CHECK_RET_VAL(data && size, 0);
	  
	while (size--){
		value = (value * 131313) + (*data++); 
	}  
	return value;
}

static size_t hash_data_func_rs (const uint8_t *data, size_t size) {
    /* hash function from Robert Sedgwicks 'Algorithms in C' book */
    size_t b    = 378551;
    size_t a    = 63689;
    size_t retval = 0;
	CHECK_RET_VAL(data && size, 0);

	while (size--) {
        retval = retval * a + (*data++);
        a *= b;
    }

    return retval;
}

static size_t hash_data_func_fnv1a(const uint8_t *array, size_t length) {
    size_t hash = 2166136261UL;
    for (int i = 0; i < length; ++i) {
        hash ^= (size_t)array[i];
        hash *= 16777619;
    }
    return hash;
}

static size_t  hash_data_func_djb2(const uint8_t * array, size_t length) {
    size_t hash = 5381;
    for (int i = 0; i < length; ++i) {
        hash = ((hash << 5) + hash) + (size_t)array[i]; /* hash * 33 + c */
    }
    return hash;
}


static size_t (*gf_data_hash_func[])(uint8_t const*,size_t) = {
	hash_data_func_bkdr,
	hash_data_func_rs,
    hash_data_func_fnv1a,
    hash_data_func_djb2
};



/*brief    计算uint32_t类型数据hash值
 *param ： eIndex: 计算函数选择
 *param ： u32Value: 待计算原值
 *return： 返回hash值
 */
__EX_API__ size_t wow_hash_uint32(HashUnitIndex_E eIndex,uint32_t u32Value)
{   
	CHECK_RET_VAL_P(eIndex < HASH_UINT_MAX, 0 ,"index type(%d) invalid!\n",eIndex);

    return gf_int_hash_func[eIndex](u32Value);
}


/*brief    计算字符串类型数据hash值
 *param ： pu8Data: 待计算原值
 *param ： snSize:  待计算原值长度
 *param ： eIndex:  计算函数选择
 *return： 返回hash值 无效参数返回0
 */
__EX_API__ size_t wow_hash_data(HashArraryIndex_E eIndex,uint8_t const* pu8Data, size_t snSize)
{
    CHECK_RET_VAL_P(pu8Data && snSize > 0, 0 ,"param input invalid!\n");
    CHECK_RET_VAL_P(eIndex < HASH_ARRAY_MAX, 0 ,"index type(%d) invalid!\n",eIndex);
	
    return gf_data_hash_func[eIndex](pu8Data, snSize);
}

