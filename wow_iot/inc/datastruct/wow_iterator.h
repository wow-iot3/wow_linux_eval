#ifndef __WOW_BASE_DATASTRUCT_ITERATOR_H_
#define __WOW_BASE_DATASTRUCT_ITERATOR_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

struct iterator;
typedef struct{
    int  (*size)(struct iterator* iterator);
    int  (*head)(struct iterator* iterator);
    int  (*last)(struct iterator* iterator);
    int  (*tail)(struct iterator* iterator);
    int  (*prev)(struct iterator* iterator, size_t itor);
    int  (*next)(struct iterator* iterator, size_t itor);
    void*(*item)(struct iterator* iterator, size_t itor);
    int  (*comp)(struct iterator* iterator, const void* litem, const void* ritem);
    int  (*copy)(struct iterator* iterator, size_t itor, const void* item);
    int  (*remove)(struct iterator* iterator, size_t itor);
}IteratorOps_T;


typedef struct iterator{
    size_t           step;
    void*            priv;
    IteratorOps_T*   ops;
}Iterator_T,*Iterator_PT;



int  wow_iterator_step(Iterator_T* ptIter);
int wow_iterator_size(Iterator_T* ptIter);
int wow_iterator_head(Iterator_T* ptIter);
int wow_iterator_last(Iterator_T* ptIter);
int wow_iterator_tail(Iterator_T* ptIter);
int   wow_iterator_prev(Iterator_T* ptIter, size_t itor);
int   wow_iterator_next(Iterator_T* ptIter, size_t itor);
void*  wow_iterator_item(Iterator_T* ptIter, size_t itor);
int  wow_iterator_remove(Iterator_T* ptIter, size_t itor);
int  wow_iterator_copy(Iterator_T* ptIter, size_t itor, const void* item);
int  wow_iterator_comp(Iterator_T* ptIter, const void* litem, const void* ritem);

#ifdef __cplusplus
}
#endif

#endif
