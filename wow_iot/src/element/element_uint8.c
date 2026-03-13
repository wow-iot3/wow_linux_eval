#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <stdint.h>

#include "prefix/wow_check.h"
#include "utils/wow_type.h"

#include "hash/wow_hash.h"
#include "element/wow_element.h"



static size_t element_uint8_hash(int index,const void* data)
{
    return wow_hash_uint32(index,p2u32(data));
}
static ssize_t element_uint8_comp(const void* ldata, const void* rdata)
{
    return ((p2u8(ldata) < p2u8(rdata))? -1 : (p2u8(ldata) > p2u8(rdata)));
}

static void* element_uint8_data(const void* buff)
{
    CHECK_RET_VAL(buff, NULL);
	//printf("element_uint8_data data(%p):%d\n",u2p(*((uint8_t*)buff)),*((uint8_t*)buff));

    return u2p(*((uint8_t*)buff));
}

static char const* element_uint8_cstr(const void* data, char* cstr, size_t maxn)
{
    CHECK_RET_VAL(cstr, "");

    int n = snprintf(cstr, maxn, "%u", (uint8_t)(size_t)data);
    if (n >= 0 && n < (int )maxn) cstr[n] = '\0';

    return (char const*)cstr;
}

static void element_uint8_free(void* buff)
{
    CHECK_RET_VOID(buff);
    *((uint8_t*)buff) = 0;
}

static void element_uint8_copy(void* buff, const void* data)
{
    CHECK_RET_VOID(buff);
    *((uint8_t*)buff) = p2u8(data);
}


Element_T ele_uint8 = {
	ELEMENT_TYPE_UINT8,
	sizeof(uint8_t),
	element_uint8_hash,
	element_uint8_comp,
	element_uint8_data,
	element_uint8_cstr,
	element_uint8_free,
	element_uint8_copy,
	element_uint8_copy,
	element_uint8_copy
};

