#include <stdio.h>

#include "prefix/wow_keyword.h"

#include "element/wow_element.h"
#include "hash/wow_hash.h"

extern Element_T ele_uint8;
extern Element_T ele_uint16;
extern Element_T ele_uint32;
extern Element_T ele_str;
extern Element_T ele_ptr;

__EX_API__ Element_T* wow_element_ops(ElementType_E eType)
{
	switch(eType){
		case ELEMENT_TYPE_UINT8:
			return (&ele_uint8);
		case ELEMENT_TYPE_UINT16:
			return (&ele_uint16);
		case ELEMENT_TYPE_UINT32:
			return (&ele_uint32);
		case ELEMENT_TYPE_STR:
			return (&ele_str);
		case ELEMENT_TYPE_PTR:
			return (&ele_ptr);
		case ELEMENT_TYPE_MAX:
		default:
			return NULL;
	}
}

__EX_API__ int wow_element_hashItem(ElementType_E eType)
{
	switch(eType){
		case ELEMENT_TYPE_UINT8:
		case ELEMENT_TYPE_UINT16:
		case ELEMENT_TYPE_UINT32:
			return HASH_UINT_MAX-1;
		case ELEMENT_TYPE_STR:
		case ELEMENT_TYPE_PTR:
			return HASH_ARRAY_MAX-1;
		case ELEMENT_TYPE_MAX:	
		default:
			return -1;
	}
}

__EX_API__ size_t  wow_element_maxItem(ElementType_E eType)
{
	switch(eType){
		case ELEMENT_TYPE_UINT8:
			return MAXU8;
		case ELEMENT_TYPE_UINT16:
			return MAXU16;
		case ELEMENT_TYPE_UINT32:
		case ELEMENT_TYPE_STR:
		case ELEMENT_TYPE_PTR:
			return MAXU32;
		case ELEMENT_TYPE_MAX:	
		default:
			return 0;
	}
}

