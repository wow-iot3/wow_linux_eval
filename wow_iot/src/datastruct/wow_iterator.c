#include "prefix/wow_check.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"
#include "prefix/wow_errno.h"

#include "datastruct/wow_iterator.h"

__IN_API__ int wow_iterator_step(Iterator_T* ptIter)
{
	CHECK_RET_VAL(ptIter,-1);

    return ptIter->step;
}

__IN_API__ int wow_iterator_size(Iterator_T* ptIter)
{
	CHECK_RET_VAL(ptIter,-1);
	CHECK_RET_VAL(ptIter->ops,-1);
	CHECK_RET_VAL(ptIter->ops->size,-1);
 
    return ptIter->ops->size(ptIter);
}

__IN_API__ int wow_iterator_head(Iterator_T* ptIter)
{
    CHECK_RET_VAL(ptIter,-1);
	CHECK_RET_VAL(ptIter->ops,-1);
	CHECK_RET_VAL(ptIter->ops->head,-1);
    
    return ptIter->ops->head(ptIter);
}

__IN_API__ int wow_iterator_last(Iterator_T* ptIter)
{
    CHECK_RET_VAL(ptIter,-1);
	CHECK_RET_VAL(ptIter->ops,-1);
	CHECK_RET_VAL(ptIter->ops->last,-1);

    return ptIter->ops->last(ptIter);
}

__IN_API__ int wow_iterator_tail(Iterator_T* ptIter)
{
    CHECK_RET_VAL(ptIter,-1);
	CHECK_RET_VAL(ptIter->ops,-1);
	CHECK_RET_VAL(ptIter->ops->tail,-1);

    return ptIter->ops->tail(ptIter);
}

__IN_API__ int wow_iterator_prev(Iterator_T* ptIter, size_t itor)
{
    CHECK_RET_VAL(ptIter,-1);
	CHECK_RET_VAL(ptIter->ops,-1);
	CHECK_RET_VAL(ptIter->ops->prev,-1);

    return ptIter->ops->prev(ptIter, itor);
}

__IN_API__ int wow_iterator_next(Iterator_T* ptIter, size_t itor)
{
    CHECK_RET_VAL(ptIter,-1);
	CHECK_RET_VAL(ptIter->ops,-1);
	CHECK_RET_VAL(ptIter->ops->next,-1);

    return ptIter->ops->next(ptIter, itor);
}

__IN_API__ void* wow_iterator_item(Iterator_T* ptIter, size_t itor)
{
    CHECK_RET_VAL(ptIter,NULL);
	CHECK_RET_VAL(ptIter->ops,NULL);
	CHECK_RET_VAL(ptIter->ops->item,NULL);

    return ptIter->ops->item(ptIter, itor);
}

__IN_API__ int wow_iterator_remove(Iterator_T* ptIter, size_t itor)
{
    CHECK_RET_VAL(ptIter,-1);
	CHECK_RET_VAL(ptIter->ops,-1);
	CHECK_RET_VAL(ptIter->ops->remove,-1);

    return ptIter->ops->remove(ptIter, itor);
}

__IN_API__ int wow_iterator_copy(Iterator_T* ptIter, size_t itor, const void* item)
{
    CHECK_RET_VAL(ptIter,-1);
	CHECK_RET_VAL(ptIter->ops,-1);
	CHECK_RET_VAL(ptIter->ops->copy,-1);

    return ptIter->ops->copy(ptIter, itor, item);
}

__IN_API__ int wow_iterator_comp(Iterator_T* ptIter, const void* litem, const void* ritem)
{
    CHECK_RET_VAL(ptIter,-1);
	CHECK_RET_VAL(ptIter->ops,-1);
	CHECK_RET_VAL(ptIter->ops->comp,-1);

    return ptIter->ops->comp(ptIter, litem, ritem);
}

