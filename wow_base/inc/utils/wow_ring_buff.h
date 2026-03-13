#ifndef __WOW_BASE_UTILS_WOW_RING_BUFF_H_
#define __WOW_BASE_UTILS_WOW_RING_BUFF_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

typedef struct ring_buffer_t RingBuffer_T;

/* brief   创建环形缓冲区
 * param ： snSize ： 环形缓冲区大小
 * return： 成功返回环形缓冲区结构体指针  失败返回NULL
 */
RingBuffer_T* wow_ringbuf_create(size_t snSize);

/* brief   销毁环形缓冲区
 * param ： pptRbuf ： 环形缓冲区结构体指针
 * return： 无
 */
void wow_ringbuf_destroy(RingBuffer_T** pptRbuf);

/* brief   获取环形缓冲区未使用空间
 * param ： ptRbuf ： 环形缓冲区结构体指针
 * return： 成功返回未使用大小 失败返回<0
 */
int wow_ringbuf_getFree(RingBuffer_T* ptRbuf);

/* brief   获取环形缓冲区已使用空间
 * param ： ptRbuf ： 环形缓冲区结构体指针
 * return： 成功返回已使用大小 失败返回<0
 */
int wow_ringbuf_getUsed(RingBuffer_T* ptRbuf);

/* brief   向环形缓冲区写入内容
 * param ： ptRbuf ： 环形缓冲区结构体指针
 * param ： pData  ： 待写入数据内容
 * param ： snSize ： 待写入数据长度
 * return： 成功返回写入长度，失败返回<0
 * 写内容如果剩余空间小于实际需要写入空间不进行写入操作 返回失败
 */
int wow_ringbuf_write(RingBuffer_T* ptRbuf, const void *pData, size_t snSize);

/* brief   从环形缓冲区读取内容
 * param ： ptRbuf ： 环形缓冲区结构体指针
 * param ： pData  ： 待读取数据缓存区
 * param ： snSize ： 待缓存区大小
 * return： 成功返回读取长度，失败返回<0
 */
int wow_ringbuf_read(RingBuffer_T* ptRbuf, void *pData, size_t snSize);

/* brief   清空环形缓冲区
 * param ： ptRbuf ： 环形缓冲区结构体指针
 * return： 无
 */
void wow_ringbuf_clear(RingBuffer_T* ptRbuf);


#ifdef __cplusplus
}
#endif

#endif
