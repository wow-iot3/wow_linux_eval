#include <stdio.h>
#include <string.h>
#include <stdlib.h> 

#include "prefix/wow_check.h"
#include "prefix/wow_common.h"
#include "utils/wow_type.h"

#include "hash/wow_hash.h"
#include "element/wow_element.h"


static size_t element_str_hash(int index,const void* data)
{
    return wow_hash_data(index,(const uint8_t*)data,strlen((const char *)data));
}
static ssize_t element_str_comp(const void* ldata, const void* rdata)
{
    CHECK_RET_VAL(ldata && rdata, -1);

    return strcmp((char const*)ldata, (char const*)rdata);
}
static void* element_str_data(const void* buff)
{
    CHECK_RET_VAL(buff, NULL);
	
    return *(void**)buff;
}
static char const* element_str_cstr(const void* data, char* cstr, size_t maxn)
{
    return (char const*)data;
}
static void element_str_free(void* buff)
{
    CHECK_RET_VOID(buff);

    void* cstr = *((void**)buff);
    if (cstr) {
        FREE(cstr);
        *((void**)buff) = NULL;
    }
}

static void element_str_copy(void* buff, const void* data)
{
    CHECK_RET_VOID(buff);

    buff = (void*)data;
}

static void element_str_repl(void* buff, const void* data)
{
    CHECK_RET_VOID(buff  && data);
    void* cstr = *(void**)buff;
	
    char*          p = (char*)cstr;
    char const*    q = (char const*)data;
    while (*p && *q) *p++ = *q++;

    // not enough space?
    if (!*p && *q)
    {
        size_t left = strlen(q);
        size_t copy = p - (char*)cstr;
        cstr =  REALLOC(cstr, copy + left + 1);
		memset(cstr+copy,0,left + 1);
        // copy the left data
        memcpy((char*)cstr + copy, q, left + 1); 
		
        *(char const**)buff = cstr;
    }
    else
	{
		*p = '\0';
	}

}

static void element_str_dupl(void* buff, const void* data)
{
    CHECK_RET_VOID(buff);
   
    if (data){
	    size_t n = strlen((const char*)data);
	    char*  p = MALLOC(n + 1);
		memset(p,0,n+1);
		memcpy(p, data, n);
		p[n] = '\0';
		*(char const**)buff = p;
	}
    else{
		//buff = NULL;
	}

}

Element_T ele_str = {
    ELEMENT_TYPE_STR,
	sizeof(char*),
    element_str_hash,
    element_str_comp,
    element_str_data,
    element_str_cstr,
    element_str_free,
    element_str_repl,
	element_str_copy,
	element_str_dupl
};
	
