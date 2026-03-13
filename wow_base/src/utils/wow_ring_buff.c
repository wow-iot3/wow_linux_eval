#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"

#include "utils/wow_ring_buff.h"

struct  ring_buffer_t{
    void *buffer;
    int length;
    size_t start;
    size_t end;
} __data_aligned__;

static void *ringbuf_end_ptr(RingBuffer_T* ringbuf)
{
    return (void *)((char *)ringbuf->buffer + ringbuf->end);
}

static void *ringbuf_start_ptr(RingBuffer_T* ringbuf)
{
    return (void *)((char *)ringbuf->buffer + ringbuf->start);
}


/* brief   创建环形缓冲区
* param ： snSize： 环形缓冲区大小
* return： 成功返回环形缓冲区结构体指针  失败返回NULL
*/
__EX_API__ RingBuffer_T* wow_ringbuf_create(size_t snSize)
{
    RingBuffer_T* rbuf = CALLOC(1, RingBuffer_T);
	CHECK_RET_VAL_ERRNO_P(rbuf,-SYSTEM_MALLOC_FAILED,"malloc RingBuffer_T faild!\n");
	
    rbuf->length = snSize;
    rbuf->start  = 0;
    rbuf->end    = 0;
    rbuf->buffer = MALLOC(snSize);
	memset(rbuf->buffer,0,snSize);
	CHECK_RET_GOTO_ERRNO_P(rbuf->buffer,out,-SYSTEM_MALLOC_FAILED,"malloc size(%zu) failed!\n",snSize);
	
    return (RingBuffer_T*)rbuf;
out:
	FREE(rbuf);
	return NULL;
}

/* brief   销毁环形缓冲区
* param ： pptRbuf： 环形缓冲区结构体指针
* return： 无
*/
__EX_API__ void wow_ringbuf_destroy(RingBuffer_T** pptRbuf)
{
	CHECK_RET_VOID(pptRbuf && *pptRbuf);
	RingBuffer_T* rbuf = (RingBuffer_T*)*pptRbuf;
	
	if(rbuf->buffer){
		FREE(rbuf->buffer);
	}
    rbuf->buffer = NULL;
	
    FREE(rbuf);
	*pptRbuf = NULL;
}

/* brief   获取环形缓冲区未使用空间
 * param ： ptRbuf ： 环形缓冲区结构体指针
 * return： 成功返回未使用大小 失败返回<0
 */
__EX_API__ int wow_ringbuf_getFree(RingBuffer_T* ptRbuf)
{
	CHECK_RET_VAL_P(ptRbuf, -PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");

    if (ptRbuf->end >= ptRbuf->start) {
        return ptRbuf->length - (ptRbuf->end - ptRbuf->start);
    } else {
        return ptRbuf->start - ptRbuf->end;
    }
}

/* brief   获取环形缓冲区已使用空间
 * param ： ptRbuf ： 环形缓冲区结构体指针
 * return： 成功返回已使用大小 失败返回<0
 */
__EX_API__ int wow_ringbuf_getUsed(RingBuffer_T* ptRbuf)
{
	CHECK_RET_VAL_P(ptRbuf, -PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");

    if (ptRbuf->end >= ptRbuf->start) {
        return ptRbuf->end - ptRbuf->start;
    } else {
        return ptRbuf->length - (ptRbuf->start - ptRbuf->end);
    }
}

/* brief   向环形缓冲区写入内容
 * param ： ptRbuf ： 环形缓冲区结构体指针
 * param ： pData  ： 待写入数据内容
 * param ： snSize ： 待写入数据长度
 * return： 成功返回写入长度，失败返回<0
 * 写内容如果剩余空间小于实际需要写入空间不进行写入操作 返回失败
 */
__EX_API__ int wow_ringbuf_write(RingBuffer_T* ptRbuf, const void *data, size_t snSize)
{
	CHECK_RET_VAL_P(ptRbuf, -PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(data, -PARAM_INPUT_DATA_IS_NULL, "param input data invalid!\n");
	CHECK_RET_VAL_P(snSize > 0, -PARAM_INPUT_ARG_INVALID, "param input arg invalid!\n");

    size_t left = wow_ringbuf_getFree(ptRbuf);
	CHECK_RET_VAL(left > snSize, -RING_BUFFER_NO_SPACE);

    if ((ptRbuf->length - ptRbuf->end) < snSize) {
        int half_tail = ptRbuf->length - ptRbuf->end;
        memcpy(ringbuf_end_ptr(ptRbuf), data, half_tail);
        ptRbuf->end = (ptRbuf->end + half_tail) % ptRbuf->length;

        int half_head = snSize - half_tail;
        memcpy(ringbuf_end_ptr(ptRbuf), data+half_tail, half_head);
        ptRbuf->end = (ptRbuf->end + half_head) % ptRbuf->length;
    } else {
        memcpy(ringbuf_end_ptr(ptRbuf), data, snSize);
        ptRbuf->end = (ptRbuf->end + snSize) % ptRbuf->length;
    }
	
    return snSize;
}

/* brief   从环形缓冲区读取内容
 * param ： ptRbuf ： 环形缓冲区结构体指针
 * param ： pData  ： 待读取数据缓存区
 * param ： snSize ： 待缓存区大小
 * return： 成功返回读取长度，失败返回<0
 */
__EX_API__ int wow_ringbuf_read(RingBuffer_T* ptRbuf, void *data, size_t snSize)
{
	CHECK_RET_VAL_P(ptRbuf, -PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(data, -PARAM_INPUT_DATA_IS_NULL, "param input data invalid!\n");
	CHECK_RET_VAL_P(snSize > 0, -PARAM_INPUT_ARG_INVALID, "param input arg invalid!\n");

    size_t rlen = MIN2(snSize, wow_ringbuf_getUsed(ptRbuf));

    if ((ptRbuf->length - ptRbuf->start) < rlen) {
        int half_tail = ptRbuf->length - ptRbuf->start;
        memcpy(data, ringbuf_start_ptr(ptRbuf), half_tail);
        ptRbuf->start = (ptRbuf->start + half_tail) % ptRbuf->length;

        int half_head = rlen - half_tail;
        memcpy(data+half_tail, ringbuf_start_ptr(ptRbuf), half_head);
        ptRbuf->start = (ptRbuf->start + half_head) % ptRbuf->length;
    } else {
        memcpy(data, ringbuf_start_ptr(ptRbuf), rlen);
        ptRbuf->start = (ptRbuf->start + rlen) % ptRbuf->length;
    }

    if ((ptRbuf->start == ptRbuf->end) || (wow_ringbuf_getUsed(ptRbuf) == 0)) {
        ptRbuf->start = ptRbuf->end = 0;
    }
	
    return rlen;
}

/* brief    清空环形缓冲区
 * param ： ptRbuf： 环形缓冲区结构体指针
 * return： 无
 */
__EX_API__ void wow_ringbuf_clear(RingBuffer_T* ptRbuf)
{
	CHECK_RET_VOID(ptRbuf);
	
    ptRbuf->start = ptRbuf->end = 0;
}
