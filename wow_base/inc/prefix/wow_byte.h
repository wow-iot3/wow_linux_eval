#ifndef __WOW_BASE_PREFIX_WOW_BYTE_H_
#define __WOW_BASE_PREFIX_WOW_BYTE_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif



///<将指针p数据转化为uint32_t小端数据
#define byte_get_1(p)           (((*(p)) >> 7) & 1)
#define byte_get_8(p)           (*(p))
#define byte_get_16_le_impl(p)  ((uint16_t)(((uint16_t)*((p) + 1)) << 8 | ((uint16_t)*(p))))
#define byte_get_16_be_impl(p)  ((uint16_t)(((uint16_t)*(p)) << 8 | ((uint16_t)*((p) + 1))))
#define byte_get_16_ne_impl(p)  (*((uint16_t*)(p)))
#define byte_get_32_le_impl(p)  ((uint32_t)(((uint32_t)*((p) + 3)) << 24 | ((uint32_t)*((p) + 2)) << 16 | ((uint32_t)*((p) + 1)) << 8 | ((uint32_t)*(p))))
#define byte_get_32_be_impl(p)  ((uint32_t)(((uint32_t)*(p)) << 24 | ((uint32_t)*((p) + 1)) << 16 | ((uint32_t)*((p) + 2)) << 8 | ((uint32_t)*((p) + 3))))
#define byte_get_32_ne_impl(p)  (*((uint32_t*)(p)))
#define byte_get_64_le_impl(p)  ((uint64_t)(((uint64_t)*((p) + 7)) << 56 | ((uint64_t)*((p) + 6)) << 48 | ((uint64_t)*((p) + 5)) << 40 | ((uint64_t)*((p) + 4)) << 32 | ((uint64_t)*((p) + 3)) << 24 |((uint64_t)*((p) + 2)) << 16 | ((uint64_t)*((p) + 1)) << 8 | ((uint64_t)*(p))))
#define byte_get_64_be_impl(p)  ((uint64_t)(((uint64_t)*(p)) << 56 | ((uint64_t)*((p) + 1)) << 48 | ((uint64_t)*((p) + 2)) << 40 | ((uint64_t)*((p) + 3)) << 32 | ((uint64_t)*((p) + 4)) << 24 | ((uint64_t)*((p) + 5)) << 16 | ((uint64_t)*((p) + 6)) << 8 | ((uint64_t)*((p) + 7))))
#define byte_get_64_ne_impl(p)  (*((uint64_t*)(p)))


static __inline__ void byte_set_u16_le(uint8_t* p, uint16_t x)
{
    p[0] = (uint8_t)x&0xFF;
    p[1] = (uint8_t)(x >> 8)&0xFF; 
}
static __inline__ void byte_set_u16_be(uint8_t* p, uint16_t x)
{
    p[0] = (uint8_t)(x >> 8)&0xFF; 
    p[1] = (uint8_t)x&0xFF;
}
static __inline__ void byte_set_u32_le(uint8_t* p, uint32_t x)
{ 
    p[0] = (uint8_t)x&0xFF;
    p[1] = (uint8_t)(x >> 8)&0xFF; 
    p[2] = (uint8_t)(x >> 16)&0xFF;
    p[3] = (uint8_t)(x >> 24)&0xFF;
}
static __inline__ void byte_set_u32_be(uint8_t* p, uint32_t x)
{
    p[0] = (uint8_t)(x >> 24)&0xFF; 
    p[1] = (uint8_t)(x >> 16)&0xFF; 
    p[2] = (uint8_t)(x >> 8)&0xFF; 
    p[3] = (uint8_t)x;
}
static __inline__ void byte_set_u64_le(uint8_t* p, uint64_t x)
{ 
    p[0] = (uint8_t)x&0xFF;
    p[1] = (uint8_t)(x >> 8)&0xFF; 
    p[2] = (uint8_t)(x >> 16)&0xFF;
    p[3] = (uint8_t)(x >> 24)&0xFF;
    p[4] = (uint8_t)(x >> 32)&0xFF;
    p[5] = (uint8_t)(x >> 40)&0xFF;
    p[6] = (uint8_t)(x >> 48)&0xFF;
    p[7] = (uint8_t)(x >> 56)&0xFF;
}
static __inline__ void byte_set_u64_be(uint8_t* p, uint64_t x)
{
    p[0] = (uint8_t)(x >> 56)&0xFF; 
    p[1] = (uint8_t)(x >> 48)&0xFF; 
    p[2] = (uint8_t)(x >> 40)&0xFF; 
    p[3] = (uint8_t)(x >> 32)&0xFF; 
    p[4] = (uint8_t)(x >> 24)&0xFF; 
    p[5] = (uint8_t)(x >> 16)&0xFF; 
    p[6] = (uint8_t)(x >> 8)&0xFF; 
    p[7] = (uint8_t)x&0xFF;
}

#define byte_set_1(p, x)                do { *(p) &= 0x7f; *(p) |= (((x) & 0x1) << 7); } while (0)
#define byte_set_8(p, x)                do { *(p) = (uint8_t)(x); } while (0)
#define byte_set_16_le_impl(p, x)       byte_set_u16_le(p, x)
#define byte_set_16_be_impl(p, x)       byte_set_u16_be(p, x)
#define byte_set_16_ne_impl(p, x)       do { *((uint16_t*)(p)) = (uint16_t)(x); } while (0)
#define byte_set_32_le_impl(p, x)       byte_set_u32_le(p, x)
#define byte_set_32_be_impl(p, x)       byte_set_u32_be(p, x)
#define byte_set_32_ne_impl(p, x)      do { *((uint32_t*)(p)) = (uint32_t)(x); } while (0)
#define byte_set_64_le_impl(p, x)       byte_set_u64_le(p, x)
#define byte_set_64_be_impl(p, x)       byte_set_u64_be(p, x)
#define byte_set_64_ne_impl(p, x)       do { *((uint64_t*)(p)) = (uint64_t)(x); } while (0)



static __inline__ uint16_t byte_swap_u16(uint16_t x)
{
    x = (x >> 8) | (x << 8);
    return x;
}
static __inline__ uint32_t byte_swap_u24(uint32_t x)
{
    return (x >> 16) | (x & 0x0000ff00) | (x << 16);
}
static __inline__ uint32_t byte_swap_u32(uint32_t x)
{
    x = ((x << 8) & 0xff00ff00) | ((x >> 8) & 0x00ff00ff);
    x = (x >> 16) | (x << 16);
    return x;
}
static __inline__ uint64_t byte_swap_u64(uint64_t x)
{
    union 
    {
        uint64_t u64;
        uint32_t u32[2];

    } w, r;

    w.u64 = x;

    r.u32[0] = byte_swap_u32(w.u32[1]);
    r.u32[1] = byte_swap_u32(w.u32[0]);

    return r.u64;
}


#define U8_2_U32(b1,b2,b3,b4)\
		   ((((uint32_t)(b1) & 0x000000ff) << 24) | \
		   (((uint32_t)(b2) & 0x000000ff) <<  16) | \
		   (((uint32_t)(b3) & 0x000000ff) <<  8) | \
		   (((uint32_t)(b4) & 0x000000ff) << 0))

static __inline__ float byte_get_float_abcd(uint8_t* p)
{
    float f;
    uint32_t i = U8_2_U32(p[0],p[1],p[2],p[3]);
	
    memcpy(&f, &i, sizeof(float));
    return f;
}

static __inline__ float byte_get_float_cdab(uint8_t* p)
{
    float f;
    uint32_t i = U8_2_U32(p[2],p[3],p[0],p[1]);

    memcpy(&f, &i, sizeof(float));
    return f;
}

static __inline__ void byte_set_float_abcd(uint8_t *p,float f)
{
    uint32_t i = 0;

    memcpy(&i, &f, sizeof(uint32_t));
	
    p[0] = (uint8_t)(i >> 24);
	p[1] = (uint8_t)(i >> 16);
	p[2] = (uint8_t)(i >> 8);
    p[3] = (uint8_t)i;
}

static __inline__ void byte_set_float_cdab(uint8_t *p,float f)
{
    uint32_t i = 0;

    memcpy(&i, &f, sizeof(uint32_t));
	
    p[0] = (uint8_t)(i >> 24);
	p[1] = (uint8_t)(i >> 16);
	p[2] = (uint8_t)(i >> 8);
    p[3] = (uint8_t)i;
}


static __inline__ void bytes_to_bits(uint8_t *pu8ByteData,uint32_t nByteLen,uint8_t *pu8BitData,uint32_t nBitLen)
{	
	int i = 0;
	int j = 0;
	int pos = 0;

	for (i = 0; i < nByteLen; i++){
		for ( j = 0; (j < 8) && (pos < nBitLen); j++) {
			pu8BitData[pos++] = (pu8ByteData[i] & (1 << j)) ? 1 : 0;
		}
	}
}
											 
static __inline__ void bits_to_bytes(uint8_t *pu8BitData,uint32_t nBitLen, uint8_t *pu8ByteData,uint32_t nByteLen)
{
	int i = 0;
	int j = 0;
	int pos = 0;

	for (i = 0; i < nByteLen; i++) {
		*(pu8ByteData+i)	= 0;
		for (j=0; (j < 8) && (pos < nBitLen); j++) {
			if ((uint8_t)pu8BitData[pos++]){
				*(pu8ByteData+i) |=	(1 << j);
			}else{
				*(pu8ByteData+i) &=~ (1 << j);
			}		
		}
	}
}


#ifdef __cplusplus
}
#endif

#endif

