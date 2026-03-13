#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <stdint.h>

#include "prefix/wow_check.h"
#include "utils/wow_type.h"

#include "hash/wow_hash.h"
#include "element/wow_element.h"


static size_t element_uint32_hash( int index,const void* data)
{
    return wow_hash_uint32(index,p2u32(data));
}
static ssize_t element_uint32_comp(const void* ldata, const void* rdata)
{
    return ((p2u32(ldata) < p2u32(rdata))? -1 : (p2u32(ldata) > p2u32(rdata)));
}
static void* element_uint32_data(const void* buff)
{
    CHECK_RET_VAL(buff, NULL);

    return u2p(*((uint32_t*)buff));
}
static char const* element_uint32_cstr(const void* data, char* cstr, size_t maxn)
{
    CHECK_RET_VAL(cstr, "");

    ssize_t n = snprintf(cstr, maxn, "%u", (uint32_t)(size_t)data);
    if (n >= 0 && n < (ssize_t)maxn) cstr[n] = '\0';

    return (char const*)cstr;
}
static void element_uint32_free(void* buff)
{
    CHECK_RET_VOID(buff);

    *((uint32_t*)buff) = 0;
}
static void element_uint32_copy(void* buff, const void* data)
{
    CHECK_RET_VOID(buff);

    *((uint32_t*)buff) = p2u32(data);
}

Element_T ele_uint32 ={
	ELEMENT_TYPE_UINT32,
	sizeof(uint32_t),
	element_uint32_hash,
	element_uint32_comp,
	element_uint32_data,
	element_uint32_cstr,
	element_uint32_free,
	element_uint32_copy,
	element_uint32_copy,
	element_uint32_copy
};

