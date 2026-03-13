#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <stdint.h>

#include "prefix/wow_check.h"
#include "utils/wow_type.h"

#include "hash/wow_hash.h"
#include "element/wow_element.h"


static size_t element_uint16_hash( int index,const void* data)
{
    return wow_hash_uint32(index,p2u32(data));
}
static ssize_t element_uint16_comp(const void* ldata, const void* rdata)
{
    return ((p2u16(ldata) < p2u16(rdata))? -1 : (p2u16(ldata) > p2u16(rdata)));
}
static void* element_uint16_data(const void* buff)
{
    CHECK_RET_VAL(buff, NULL);

    return u2p(*((uint16_t*)buff));
}
static char const* element_uint16_cstr(const void* data, char* cstr, size_t maxn)
{
    CHECK_RET_VAL(cstr, "");

    ssize_t n = snprintf(cstr, maxn, "%u", (uint16_t)(size_t)data);
    if (n >= 0 && n < (ssize_t)maxn) cstr[n] = '\0';

    return (char const*)cstr;
}
static void element_uint16_free(void* buff)
{
    CHECK_RET_VOID(buff);

    *((uint16_t*)buff) = 0;
}
static void element_uint16_copy(void* buff, const void* data)
{
    CHECK_RET_VOID(buff);

    *((uint16_t*)buff) = p2u16(data);
}


Element_T ele_uint16 = {
	ELEMENT_TYPE_UINT16,
	sizeof(uint16_t),
	element_uint16_hash,
	element_uint16_comp,
	element_uint16_data,
	element_uint16_cstr,
	element_uint16_free,
	element_uint16_copy,
	element_uint16_copy,
	element_uint16_copy
};

