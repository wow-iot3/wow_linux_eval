#ifndef __WOW_BASE_SYSTEM_WOW_ATOMIC_H_
#define __WOW_BASE_SYSTEM_WOW_ATOMIC_H_

#ifdef __cplusplus
extern "C" {
#endif


#ifndef atomic_t
typedef __volatile__  int    atomic_t; 
#endif


//不支持 则使用线程锁替换
#define atomic_get(a)		atomic_get_gcc(a) 
#define atomic_set(a, v)	atomic_set_gcc(a, v) 

//这两个函数提供原子的比较和交换，如果*a == p,就将v写入*a,返回true 否则返回false
#define atomic_compare_and_swap_bool(a, p, v)   __sync_bool_compare_and_swap (a, p, v) 
//如果*a == p,就将v写入*a 并返回操作之前*ptr的值
#define atomic_compare_and_swap_val(a, p, v)    __sync_val_compare_and_swap(a, p, v) 


#define atomic_fetch_and_add(a, v)     __sync_fetch_and_add(a, v) 
#define atomic_fetch_and_sub(a, v)     __sync_fetch_and_sub(a, v) 
#define atomic_fetch_and_or(a, v)      __sync_fetch_and_or(a, v) 
#define atomic_fetch_and_and(a, v)     __sync_fetch_and_and(a, v) 

#define atomic_fetch_and_inc(a)     __sync_fetch_and_add(a, 1) 
#define atomic_fetch_and_dec(a)     __sync_fetch_and_sub(a, 1) 




static __inline__ int atomic_get_gcc(atomic_t* ptAtomic)
{
	__sync_synchronize();
	return (*ptAtomic);
}

static __inline__ void atomic_set_gcc(atomic_t* ptAtomic , int nVal)
{
    *ptAtomic = nVal;
    __sync_synchronize();
}

#ifdef __cplusplus
}
#endif


#endif

