#ifndef __WOW_BASE_PREFIX_WOW_COMMON_H_
#define __WOW_BASE_PREFIX_WOW_COMMON_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "wow_base_config.h"

#define __typeref__(object)   typedef struct{}* ##object##_PT

//__builtin_expect实际上是为了满足在大多数情况不执行跳转指令，仅仅是告诉编译器优化，并没有改变其对真值的判断
//期望 x == 1, if执行func()的可能性小
#define LIKELY(x)       (__builtin_expect(!!(x), 1))
#define UNLIKELY(x)     (__builtin_expect(!!(x), 0))


#define ABS(x)   ((x) > 0? (x) : -(x))

#define MAX2(x, y)                   (((x) > (y))? (x) : (y))
#define MIN2(x, y)                   (((x) < (y))? (x) : (y))
#define MAX3(x, y, z)                (((x) > (y))? (((x) > (z))? (x) : (z)) : (((y) > (z))? (y) : (z)))
#define MIN3(x, y, z)                (((x) < (y))? (((x) < (z))? (x) : (z)) : (((y) < (z))? (y) : (z)))


#define offset_of(type, member)            (size_t)&(((type const*)0)->member)
#define container_of(ptr,type, member)     ((type*)(((uint8_t*)(ptr)) - offset_of(type, member)))



#define STRING_EQUAL(a,b)   ((strlen(a) == strlen(b)) && (0 == strcasecmp(a,b))) 
#define STRING_EQUAL_N(a,b,n) (!strncmp(a,b,n)) 

#define ARRAY_SIZE(x)   ((sizeof((x)) / sizeof((x)[0])))
#define SWAP(a, b)          \
    do { typeof(a) __tmp = (a); (a) = (b); (b) = __tmp; } while (0)


#if WOW_MLEAK_ENABLE
	#include "utils/wow_mem_leak.h"
	#define MALLOC(size)	    wow_mleak_malloc(size, __LINE__, __FILE__, __FUNCTION__)
	#define CALLOC(size, type)  (type *) wow_mleak_calloc(size, sizeof(type), __LINE__, __FILE__, __FUNCTION__)
	#define REALLOC(ptr, size)  wow_mleak_realloc(ptr,size, __LINE__, __FILE__, __FUNCTION__)   
	#define FREE(ptr)			wow_mleak_free(ptr)
#else
	#define MALLOC(size)	    malloc(size)
	#define CALLOC(size, type)  (type *) calloc(size, sizeof(type))
	#define REALLOC(ptr, size)  realloc(ptr,size)   
	#define FREE(ptr)			free(ptr)
#endif

#define ALIGN_MASK   ~(sizeof(void *) - 1)
#define ALIGN(x) ((x + ~ALIGN_MASK) & ALIGN_MASK)

#define Align4(x)                    (((x) + 3) >> 2 << 2)
#define Align8(x)                    (((x) + 7) >> 3 << 3)
#define Align(x, b)                  (((x) + ((b) - 1))/b*b)

#define bits_cl0_u32(x)    ((x)? (size_t)__builtin_clz((uint32_t)(x)) : 32)			 //求x的二进制数前导0的个数
#define bits_cl0_u64(x)    ((x)? (size_t)__builtin_clzll((uint64_t)(x)) : 64)	
#define bits_cb1_u32(x)    ((x)? (size_t)__builtin_popcount((uint32_t)(x)) : 0)	     //求x的二进制数中的1的个数
#define bits_cb1_u64(x)    ((x)? (size_t)__builtin_popcountll((uint64_t)(x)) : 0)
#define bits_fb1_u32(x)    ((x)? (size_t)__builtin_ffs((uint32_t)(x)) - 1 : 32)		 //求x的二进制数中最低位1的位置
#define bits_fb1_u64(x)    ((x)? (size_t)__builtin_ffsll((uint64_t)(x)) - 1 : 64)

/// ispow2: 1, 2, 4, 8, 16, 32, ...
#define ispow2(x)                    (!((x) & ((x) - 1)) && (x))
#define Align_pow2(x)                (((x) > 1)? (ispow2(x)? (x) : ((size_t)1 << (32 - bits_cl0_u32((uint32_t)(x))))) : 1)

#define PRINT_ADDR(var)   printf(" "#var": %p\n", var)

#if DEBUG
    #define dprintf(format,...)printf("FILE: "__FILE__", LINE: %d: "format"\n", __LINE__, ##__VA_ARGS__)
#else
    #define dprintf(format,...)
#endif

#if 0
#define isspace(x)               (((x) == 0x20) || ((x) > 0x8 && (x) < 0xe))
#define isgraph(x)               ((x) > 0x1f && (x) < 0x7f)
#define isalpha(x)               (((x) > 0x40 && (x) < 0x5b) || ((x) > 0x60 && (x) < 0x7b))
#define isupper(x)               ((x) > 0x40 && (x) < 0x5b)
#define islower(x)               ((x) > 0x60 && (x) < 0x7b)
#define isascii(x)               ((x) >= 0x0 && (x) < 0x80)
#define isdigit(x)               ((x) > 0x2f && (x) < 0x3a)

#define tolower(x)               (isupper(x)? (x) + 0x20 : (x))
#define toupper(x)               (islower(x)? (x) - 0x20 : (x))
#endif

#define isdigit2(x)              ((x) == '0' || (x) == '1')
#define isdigit8(x)              (((x) > 0x2f && (x) < 0x38))
#define isdigit10(x)             (isdigit(x))
#define isdigit16(x)             (((x) > 0x2f && (x) < 0x3a) || ((x) > 0x40 && (x) < 0x47) || ((x) > 0x60 && (x) < 0x67))



#ifdef __cplusplus
}
#endif

#endif
