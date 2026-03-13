#ifndef __WOW_BASE_PREFIX_WOW_CHECK_H_
#define __WOW_BASE_PREFIX_WOW_CHECK_H_

#include <stdio.h>
#include <errno.h>
#include <error.h>

#ifdef __cplusplus
extern "C" {
#endif

/*********************无程序打印信息********************************/
#define CHECK_RET_ABORT(x)		\
do {							    \
	if (!(x)) {						\
		abort();				\
	}								\
} while(0);

#define CHECK_RET_VOID(x)		\
do {						\
	if (!(x)) {			\
		return;				\
	}						\
} while(0);

#define CHECK_RET_VAL(x,v)		\
do {						\
	if (!(x)) {			\
		return v;				\
	}						\
} while(0);

#define CHECK_RET_GOTO(x,v)		\
do {						\
	if (!(x)) {			\
		goto v;				\
	}						\
} while(0);

#define CHECK_RET_BREAK(x)		\
	if (!(x)) {			\
		break;				\
	}						

#define CHECK_RET_CONTINUE(x)		\
	if (!(x)) {			\
		continue;				\
	}	

/*********************程序打印信息********************************/
#define CHECK_RET_GOTO_P(x,out,args...)		\
do {							    \
	if (!(x)) {						\
		printf("assertion failed %s: %d:",	__FILE__, __LINE__);\
		printf(args);			\
		goto out;				\
	}							\
} while(0);
#define CHECK_RET_GOTO_EXE_P(x,out,exe,args...)		\
do {								\
	if (!(x)) { 					\
		printf("assertion %s: %d:", __FILE__, __LINE__);\
		printf(args);			\
		exe;					\
		goto out;				\
	}							\
} while(0);


#define CHECK_RET_VAL_P(x,v,args...)		\
do {							    \
	if (!(x)) {						\
		printf("assertion %s: %d:",	__FILE__, __LINE__);		\
		printf(args);			\
		return v;				\
	}							\
} while(0);
#define CHECK_RET_VAL_EXE_P(x,v,exec,args...)		\
do {								\
	if (!(x)) { 					\
		printf("assertion %s: %d:", __FILE__, __LINE__);		\
		printf(args);			\
		exec;					\
		return v;				\
	}							\
} while(0);
	

#define CHECK_RET_VOID_P(x,args...)		\
do {							    \
	if (!(x)) {						\
		printf("assertion %s: %d:",	__FILE__, __LINE__);		\
		printf(args);		\
		return;				\
	}						\
} while(0);

#define CHECK_RET_CONTINUE_P(x,args...)		\
	if (!(x)) {						\
		printf("assertion %s: %d:",	__FILE__, __LINE__);		\
		printf(args);			\
		continue;				\
	}	
#define CHECK_RET_CONTINUE_EXE_P(x,exe,args...)		\
	if (!(x)) { 					\
		printf("assertion %s: %d:", __FILE__, __LINE__);		\
		printf(args);			\
		exe;					\
		continue;				\
	}

#define CHECK_RET_BREAK_P(x,args...)		\
	if (!(x)) {						\
		printf("assertion %s: %d:",	__FILE__, __LINE__);		\
		printf(args);		\
		break;				\
	}								
#define CHECK_RET_BREAK_EXE_P(x,exe,args...)		\
			if (!(x)) { 					\
				printf("assertion %s: %d:", __FILE__, __LINE__);		\
				printf(args);		\
				exe;				\
				break;				\
			}			
#define CHECK_RET_STREQ_P(STR1, STR2,v,args...) \
do { \
	if (strcmp(STR1, STR2) != 0) { \
	printf("assertion %s: %d:",	__FILE__, __LINE__);		\
	printf(args);		\
	return v;				\
	} \
} while (0);

/*********************追加系统打印信息********************************/
#define CHECK_RET_GOTO_P_A(x,out,args...)		\
do {							    \
	if (!(x)) {						\
		printf("assertion %s: %d: err(%d):%s-",	__FILE__, __LINE__,errno,strerror(errno));\
		printf(args);			\
		goto out;				\
	}								\
} while(0);

#define CHECK_RET_VAL_P_A(x,v,args...)		\
do {							    \
	if (!(x)) {						\
		printf("assertion %s: %d: err(%d):%s-",	__FILE__, __LINE__,errno,strerror(errno));		\
		printf(args);			\
		return v;				\
	}								\
} while(0);

#define CHECK_RET_VAL_EXE_P_A(x,v,exec,args...)		\
do {								\
	if (!(x)) { 					\
		printf("assertion %s: %d: err(%d):%s-",	__FILE__, __LINE__,errno,strerror(errno));		\
		printf(args);			\
		exec;					\
		return v;				\
	}							\
} while(0);

#define CHECK_RET_VOID_P_A(x,args...)		\
do {							    \
	if (!(x)) {						\
		printf("assertion %s: %d: err(%d):%s-",	__FILE__, __LINE__,errno,strerror(errno));		\
		printf(args);		\
		return;				\
	}								\
} while(0);

#define CHECK_RET_EXE_P_A(x,exe,args...)		\
do {							    \
	if (!(x)) {						\
		printf("assertion %s: %d: err(%d):%s-",	__FILE__, __LINE__,errno,strerror(errno));		\
		printf(args);		\
		exe;					\
	}								\
} while(0);
	
#define CHECK_RET_CONTINUE_P_A(x,args...)		\
	if (!(x)) {						\
		printf("assertion %s: %d: err(%d):%s-",	__FILE__, __LINE__,errno,strerror(errno));		\
		printf(args);			\
		continue;				\
	}								

#define CHECK_RET_BREAK_P_A(x,args...)		\
	if (!(x)) {						\
		printf("assertion %s: %d: err(%d):%s-",	__FILE__, __LINE__,errno,strerror(errno));		\
		printf(args);		\
		break;				\
	}								
	
#define CHECK_RET_STREQ_P_A(STR1, STR2,v,args...) \
do { \
	if (strcmp(STR1, STR2) != 0) { \
	printf("assertion %s: %d: err(%d):%s-",	__FILE__, __LINE__,errno,strerror(errno));		\
	printf(args);		\
	return v;				\
	} \
} while (0);
		
/*********************设置errno标识********************************/
#include "utils/wow_error.h"

#define CHECK_RET_VOID_ERRNO_P(x,error,args...)		\
do {								\
	if (!(x)) { 					\
		printf("assertion %s: %d:", __FILE__, __LINE__);		\
		printf(args);			\
		wow_set_errno(error);	\
	}							\
} while(0);

#define CHECK_RET_VAL_ERRNO_P(x,error,args...)		\
do {								\
	if (!(x)) { 					\
		printf("assertion %s: %d:", __FILE__, __LINE__);		\
		printf(args);			\
		wow_set_errno(error);	\
		return NULL;				\
	}							\
} while(0);
#define CHECK_RET_VAL_ERRNO_P_A(x,error,args...)		\
do {								\
	if (!(x)) { 					\
		printf("assertion %s: %d: err(%d):%s-", __FILE__, __LINE__,errno,strerror(errno));		\
		printf(args);			\
		wow_set_errno(error);	\
		return NULL;				\
	}							\
} while(0);	
#define CHECK_RET_GOTO_ERRNO_P(x,out,error,args...)		\
do {								\
	if (!(x)) { 					\
		printf("assertion %s: %d:", __FILE__, __LINE__);\
		printf(args);			\
		wow_set_errno(error);	\
		goto out;				\
	}							\
} while(0); 	
	
#define CHECK_RET_GOTO_EXE_ERRNO_P(x,out,exe,error,args...)		\
do {								\
	if (!(x)) { 					\
		printf("assertion %s: %d:", __FILE__, __LINE__);\
		printf(args);			\
		exe;					\
		wow_set_errno(error);	\
		goto out;				\
	}							\
} while(0);
		

#define CHECK_RET_BREAK_ERRNO_P(x,error,args...)		\
if (!(x)) {						\
	printf("assertion %s: %d:",	__FILE__, __LINE__);		\
	printf(args);		\
	wow_set_errno(error);	\
	break;				\
}	

#ifdef __cplusplus
}
#endif

#endif
