#ifndef __WOW_BASE_ALGORITHM_WOW_BITMAP_H_
#define __WOW_BASE_ALGORITHM_WOW_BITMAP_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct bit_map_T BitMap_T;

/*brief    申请bitmap缓存
 *param ： snOpacity   ：bit长度
 *return： 成功返回bitmap操作符 失败返回NULL
 */
BitMap_T* wow_bitmap_new(size_t snOpacity);

/*brief    释放bitmap缓存
 *param ： bitmap：bitmap操作符
 *return： 无
 */
void wow_bitmap_free(BitMap_T** pptBitmap);

/*brief    设置bitmap位置值
 *param ： ptBitmap：bitmap操作符
 *param ： snIdx   ：位置索引
 *return： 成功返回0 失败返回<0
 */
int wow_bitmap_set_1(BitMap_T* ptBitmap, size_t snIdx);
int wow_bitmap_set_0(BitMap_T* ptBitmap, size_t snIdx);


/*brief    获取bitmap位置值
 *param ： ptBitmap：bitmap操作符
 *param ： snIdx   ：位置索引
 *return： 成功返回0/1 失败返回<0
 */
int wow_bitmap_get(BitMap_T* ptBitmap, size_t snIdx);

/*brief    获取bitmap位置最大连续长度0及起始位置
 *param ： ptBitmap：bitmap操作符
 *param ： piMaxLen：最大连续长度
 *param ： spiStartPos：起始位置
 *return： 成功返回0 失败返回<0
 */
int wow_bitmap_longest_consecutive_0(BitMap_T* ptBitmap, int* piMaxLen, int *piStartPos); 
int wow_bitmap_longest_consecutive_1(BitMap_T* ptBitmap, int* piMaxLen, int *piStartPos);


#ifdef __cplusplus
}
#endif

#endif

