#ifndef __WOW_BASE_WOW_BASE_INTERNAL_H_
#define __WOW_BASE_WOW_BASE_INTERNAL_H_

#include "utils/wow_singleton.h"


/*内存泄漏接口初始化*/
int _wow_mleak_init(void);
int _wow_mleak_exit(void);

/*errno接口初始化*/
void _wow_error_init(void);
void _wow_error_exit(void);

/*singleton接口初始化*/
int _wow_singleton_init(void);
int _wow_singleton_exit(void);

#endif

