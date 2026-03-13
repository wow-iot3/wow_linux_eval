#ifndef __WOW_IOT_ELEMENT_H_
#define __WOW_IOT_ELEMENT_H_


typedef enum{
	ELEMENT_TYPE_UINT8     = 1,     //!< uint8
	ELEMENT_TYPE_UINT16    = 2,     //!< uint16
	ELEMENT_TYPE_UINT32    = 3,     //!< uint32
	ELEMENT_TYPE_STR       = 4,     //!< string
	ELEMENT_TYPE_PTR       = 5,     //!< pointer
    ELEMENT_TYPE_MAX
}ElementType_E;

typedef size_t   (*ElementHashFunc_F)(int index,const void* data);
typedef ssize_t  (*ElementCompFunc_F)(const void* ldata, const void* rdata);
typedef void*    (*ElementDataFunc_F)(const void* buff);
typedef char const* (*ElementCstrFunc_F)(const void* data, char* cstr, size_t maxn);
typedef void (*ElementFreeFunc_F)(void* buff);
typedef void  (*ElementReplFunc_F)(void* buff, const void* data);
typedef void  (*ElementCopyFunc_F)(void* buff, const void* data);
typedef void  (*ElementDumpFunc_F)(void* buff, const void* data);

typedef struct element{
    size_t  type;   /// the element type
    size_t  size;	/// the element size

    ElementHashFunc_F      fHash;   /// the hash function
    ElementCompFunc_F      fComp;   /// the compare function
    ElementDataFunc_F      fData;   /// the data function
    ElementCstrFunc_F      fCstr;   /// the string function 
    ElementFreeFunc_F      fFree;   /// the free element
    ElementReplFunc_F      fRepl;   /// the replace function
    ElementCopyFunc_F      fCopy;   /// the copy function 
    ElementDumpFunc_F      fDupl;   /// the duplicate function
}Element_T;

Element_T* wow_element_ops(ElementType_E eType);
int        wow_element_hashItem(ElementType_E eType);
size_t     wow_element_maxItem(ElementType_E eType);

#endif
