#ifndef __WOW_BASE_PREFIX_WOW_CPU_H_
#define __WOW_BASE_PREFIX_WOW_CPU_H_

#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif


static __inline__ size_t wow_cpu_count()
{
    static size_t ncpu = -1;
    if (ncpu == -1) {
        size_t count = sysconf(_SC_NPROCESSORS_ONLN);
        if (!count) count = sysconf(_SC_NPROCESSORS_CONF);
        if (!count) count = 1;
        ncpu = count;
    }
    return ncpu;
}

#ifndef WOW_BYTE_ORDER
    #if defined(i386)      || defined(__i386__)      || defined(_M_IX86) ||    \
        defined(__x86_64)  || defined(__x86_64__)    || defined(_M_X64)  ||    \
        defined(_M_AMD64)  || defined(_M_ARM)        || defined(__x86)   ||    \
        defined(__ARMEL__) || defined(__AARCH64EL__) || defined(_M_ARM64)
        #define WOW_BYTE_ORDER    1234
    #elif defined(sparc)     || defined(__ppc__) || \
        defined(__ARMEB__) || defined(__AARCH64EB__)
    #define WOW_BYTE_ORDER    4321
    #else
        #define WOW_BYTE_ORDER 0
    #endif
#endif

#if WOW_BYTE_ORDER==4321
    #define WOW_BIG_ENDIAN    1
    #define WOW_LITTLE_ENDIAN 0
#elif WOW_BYTE_ORDER==1234
    #define WOW_BIG_ENDIAN    0
    #define WOW_LITTLE_ENDIAN 1
#else
    #define WOW_BIG_ENDIAN    ((*(char *)&(int){1}) == 0)
    #define WOW_LITTLE_ENDIAN ((*(char *)&(int){1}) == 1)
#endif

#if defined(__x86_64) || defined(__amd64__) || defined(__aarch64__) || \
    defined(__ppc64__) || defined(__powerpc64__) ||  defined(__mips64)  
        #define WOW_CPU_BYTE (8)
#elif  defined(__arm__) || defined(__ppc__) || defined(__powerpc__)  || defined(__mips__)
    #define WOW_CPU_BYTE (4)
#else
    #define WOW_CPU_BYTE (4)
#endif


#ifdef __cplusplus
}
#endif

#endif


