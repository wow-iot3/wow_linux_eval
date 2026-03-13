#ifndef __WOW_BASE_PREFIX_WOW_KEYWORD_H_
#define __WOW_BASE_PREFIX_WOW_KEYWORD_H_


#include <stdint.h>
#include "prefix/wow_cpu.h"

///<定义常用变量字节大小
#define MAX_PATH_SIZE 512
#define MAX_URL_SIZE  256
#define MAX_DESC_SIZE 256
#define MAX_NAME_SIZE 64
#define MAX_CMD_SIZE  1024
#define MAX_IP_SIZE   16

#define MAXS8                ((int8_t)(0x7f))
#define MINS8                ((int8_t)(0x81))
#define MAXU8                ((uint8_t)(0xff))
#define MINU8                ((uint8_t)(0))
#define MAXSTR8              (3)

#define MAXS16               ((int16_t)(0x7fff))
#define MINS16               ((int16_t)(0x8001))
#define MAXU16               ((uint16_t)(0xffff))
#define MINU16               ((uint16_t)(0))
#define MAXSTR16             (5)

#define MAXS32               ((int32_t)(0x7fffffff))
#define MINS32               ((int32_t)(0x80000001))
#define MAXU32               ((uint32_t)(0xffffffff))
#define MINU32               ((uint32_t)(0))
#define MAXSTR32             (10)

#define MAXS64               ((int64_t)(0x7fffffffffffffffLL))
#define MINS64               ((int64_t)(0x8000000000000001LL))
#define MAXU64               ((uint64_t)(0xffffffffffffffffULL))
#define MINU64               ((uint64_t)(0))
#define MAXSTR64             (22)

#define MAXSTRFLOAT          (8)
#define MAXSTRDOUBLE         (17)

#ifndef PATH_SPLIT
	#define PATH_SPLIT    '/'
#endif

#ifndef LINE_SEPARATOR
	#define LINE_SEPARATOR '\n'
#endif

#ifndef ENVIRONMENT_SEP
	#define ENVIRONMENT_SEP  ':'
#endif

#ifndef FILE_SUFFIX
	#define FILE_SUFFIX      '.'
#endif

#define PATH_IS_SPLIT(c) ('/' == (c))
#define DUMMY_PTR       ((void*)-1)

///<标识函数声明接口类型
#define __EX_API__
#define __IN_API__


#ifndef OFF
#define OFF 0
#endif

#ifndef ON
#define ON 1
#endif

#define __data_aligned__      __attribute__((aligned(WOW_CPU_BYTE))) 
#define __inline_force__      __inline__ __attribute__((always_inline))

#endif