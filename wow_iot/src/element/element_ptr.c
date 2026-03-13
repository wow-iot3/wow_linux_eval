#include <stdio.h>
#include <string.h>
#include <stdlib.h> 

#include "prefix/wow_check.h"
#include "utils/wow_type.h"

#include "hash/wow_hash.h"
#include "element/wow_element.h"

static size_t element_ptr_hash(int index,const void* data)
{
	return wow_hash_uint32(index,p2u32(data));
}

static ssize_t element_ptr_comp(const void* ldata, const void* rdata)
{
    return (ldata < rdata)? -1 : (ldata > rdata);
}
static void* element_ptr_data(const void* buff)
{
    CHECK_RET_VAL(buff, NULL);

    return *((void**)buff);
}
static char const* element_ptr_cstr(const void* data, char* cstr, size_t maxn)
{
    CHECK_RET_VAL(cstr, "");

    // format string
    int n = snprintf(cstr, maxn - 1, "%p", data);
    if (n >= 0 && n < (int)maxn) cstr[n] = '\0';

    return (char const*)cstr;
}
static void element_ptr_free(void* buff)
{
    //buff = NULL;
}
static void element_ptr_repl(void* buff, const void* data)
{
    CHECK_RET_VOID(buff);
    *((const void**)buff) = data;
}
static void element_ptr_copy(void* buff, const void* data)
{
    CHECK_RET_VOID(buff);
	*((const void**)buff) = data;
}

Element_T ele_ptr ={
	ELEMENT_TYPE_PTR,
	sizeof(void*),
	element_ptr_hash,	
	element_ptr_comp,
	element_ptr_data,
	element_ptr_cstr,
	element_ptr_free,
	element_ptr_repl,
	element_ptr_copy,
	element_ptr_copy
};


