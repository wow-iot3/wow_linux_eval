#include "prefix/wow_check.h"
#include "prefix/wow_common.h"
#include "prefix/wow_errno.h"

#include "algorithm/wow_bitmap.h"

struct bit_map_T{
    uint32_t   length;
    uint8_t*   bit;
};

#define CHAR_BIT_SIZE 8

/*brief    申请bitmap缓存
 *param ： snOpacity：bit长度
 *return： 成功返回bitmap操作符 失败返回NULL
 */
BitMap_T* wow_bitmap_new(size_t snOpacity)
{
	CHECK_RET_VAL_P(snOpacity > 0,NULL,"param input size invalid!\n");
	
    BitMap_T *bitmap = CALLOC(1,BitMap_T);
	CHECK_RET_VAL_P(bitmap,NULL,"malloc BitMap_T failed!\n");
	
    uint32_t bytes = (snOpacity + CHAR_BIT_SIZE - 1) / CHAR_BIT_SIZE;
    bitmap->bit = (uint8_t *)MALLOC(sizeof(uint8_t)* bytes);
	CHECK_RET_GOTO_P(bitmap->bit,out,"malloc size:%d failed!\n",bytes);
	memset(bitmap->bit,0,bytes);

    bitmap->length = snOpacity;
	
    return (BitMap_T*)bitmap;
out:
	FREE(bitmap);
	return NULL;
}


/*brief    释放bitmap缓存
 *param ： pptBitmap：bitmap操作符
 *return： 无
 */
void wow_bitmap_free(BitMap_T** pptBitmap)
{
	CHECK_RET_VOID(pptBitmap && *pptBitmap);
	
	if((*pptBitmap)->bit){
		FREE((*pptBitmap)->bit);
	} 
	
	FREE(*pptBitmap);
	*pptBitmap = NULL;
}

/*brief    设置bitmap位置值
 *param ： ptBitmap：bitmap操作符
 *param ： snIdx   ：位置索引
 *return： 成功返回0 失败返回<0
 */
int wow_bitmap_set_1(BitMap_T* ptBitmap, size_t snIdx)
{
	CHECK_RET_VAL_P(ptBitmap,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(snIdx < ptBitmap->length,-PARAM_INPUT_SIZE_IS_BIG,"param input size invalid!\n");

    ptBitmap->bit[snIdx/CHAR_BIT_SIZE] |= (0x01<<(snIdx%CHAR_BIT_SIZE));
	
	return 0;
}

int wow_bitmap_set_0(BitMap_T* ptBitmap, size_t snIdx)
{
	CHECK_RET_VAL_P(ptBitmap,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(snIdx < ptBitmap->length,-PARAM_INPUT_SIZE_IS_BIG,"param input size invalid!\n");

	ptBitmap->bit[snIdx/CHAR_BIT_SIZE] &= (~(0x01<<(snIdx%CHAR_BIT_SIZE)));
	
	return 0;
}

/*brief    获取bitmap位置值
 *param ： ptBitmap：bitmap操作符
 *param ： snIdx   ：位置索引
 *return： 成功返回0/1  失败返回<0
 */
int wow_bitmap_get(BitMap_T* ptBitmap, size_t snIdx)
{
	CHECK_RET_VAL_P(ptBitmap,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(snIdx < ptBitmap->length,-PARAM_INPUT_SIZE_IS_BIG,"param input size invalid!\n");

	int bit = ptBitmap->bit[snIdx/CHAR_BIT_SIZE] & (0x01<<(snIdx%CHAR_BIT_SIZE));

	return bit?1:0;
}

int bitmap_longest_consecutive(BitMap_T* ptBitmap, int data,int* piMaxLen, int *piStartPos) 
{
    int current_len = 0;
    int current_start = -1;

	*piMaxLen = 0;
	*piStartPos = 0;

    for (int i = 0; i < ptBitmap->length; i++) {
        if (wow_bitmap_get(ptBitmap, i) == data) {
            if (current_len == 0) {
                current_start = i;
            }
            current_len++;
            if (current_len > *piMaxLen) {
                *piMaxLen = current_len;
                *piStartPos = current_start;
            }
        } else {
            current_len = 0;
        }
    }

	return 0;
}

/*brief    获取bitmap位置最大连续长度0及起始位置
 *param ： ptBitmap：bitmap操作符
 *param ： piMaxLen：最大连续长度
 *param ： spiStartPos：起始位置
 *return： 成功返回0/1 失败返回<0
 */
int wow_bitmap_longest_consecutive_0(BitMap_T* ptBitmap,  int* piMaxLen, int *piStartPos) 
{
	CHECK_RET_VAL_P(ptBitmap,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(piMaxLen && piStartPos,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

	return bitmap_longest_consecutive(ptBitmap, 0, piMaxLen, piStartPos) ;
}

int wow_bitmap_longest_consecutive_1(BitMap_T* ptBitmap, int* piMaxLen, int *piStartPos) 
{
	CHECK_RET_VAL_P(ptBitmap,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(piMaxLen && piStartPos,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

	return bitmap_longest_consecutive(ptBitmap, 1, piMaxLen, piStartPos) ;
}
