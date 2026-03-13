#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "prefix/wow_check.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"
#include "prefix/wow_errno.h"

#include "hash/wow_hash_map.h"
#include "element/wow_element.h"

#define hash_map_index_make(buck, item)           (((size_t)((item) & 0xffff) << 16) | ((buck) & 0xffff))
#define hash_map_index_buck(index)                ((index) & 0xffff)
#define hash_map_index_item(index)                (((index) >> 16) & 0xffff)

#define HASH_MAP_ITEM_SIZE_DEFAULT                (8)
#define HASH_MAP_ITEM_SIZE_MAX                    (256)
 
#define HASH_MAP_BUCKET_SIZE_DEFAULT              (16)
#define HASH_MAP_BUCKET_SIZE_MAX                  (32)


/*! the hash map ref type
 *
 * <pre>
 *                 0        1        3       ...     ...                n       n + 1
 * hash_list: |--------|--------|--------|--------|--------|--------|--------|--------|
 *                         |
 *                       -----    
 * item_list:           |     |       key:0                                      
 *                       -----   
 *                      |     |       key:1                                              
 *                       -----               <= insert by binary search algorithm
 *                      |     |       key:2                                               
 *                       -----  
 *                      |     |       key:3                                               
 *                       -----   
 *                      |     |       key:4                                               
 *                       -----  
 *                      |     |                                              
 *                       -----  
 *                      |     |                                              
 *                       -----  
 *                      |     |                                              
 *                       -----  
 *
 * </pre>
 *
 * @note the itor of the same item is mutable
 */
 typedef struct {	 
	 void*	  name;  
	 void*	  data;
 }HashMapItem_T;
 
typedef struct{
    size_t  size;	///<链表元素当前个数
    size_t  maxn;   ///<链表元素最大个数
}HashMapItemList_T;

 struct hash_map_T{
    HashMapItemList_T** hash_list;  //存储key-val的hash链表
    size_t				hash_size;  //存储key-val的hash大小

    HashMapItem_T       item;		
    size_t              item_step;  //元素占用的字节数
    size_t              item_size;	//当前元素个数
    size_t              item_maxn;  //最大元素个数
    size_t              item_grow;  //元素个数申请基数

    Element_T*          ele_key;	//key hash操作符
    Element_T*          ele_val;	//val hash操作符

};



static int hash_map_item_find(HashMap_T* hash_map, const void* key, size_t* pbuck, size_t* pitem)
{
	int i = 0;
    *pbuck = hash_map->ele_key->fHash(0,key)%(hash_map->hash_size-1);	

    HashMapItemList_T* list = hash_map->hash_list[*pbuck];
    CHECK_RET_VAL(list && list->size > 0, -1);

	for(i = 0; i < list->size; i++){
		uint8_t const* item = ((uint8_t*)&list[1]) + i * hash_map->item_step;
		CHECK_RET_CONTINUE(hash_map->ele_key->fComp(key,hash_map->ele_key->fData(item)) == 0);
		*pitem = i; 

		return 0;
	}

    return -1;
}

static int hash_map_item_at(HashMap_T* hash_map, size_t buck, size_t item, void** pkey, void** pdata)
{
    HashMapItemList_T* list = hash_map->hash_list[buck];
    CHECK_RET_VAL(list &&  item < list->size, -1);

    if(pkey){
        *pkey = hash_map->ele_key->fData(((uint8_t*)&list[1]) + item * hash_map->item_step);
    } 

    if(pdata) *pdata = hash_map->ele_val->fData(((uint8_t*)&list[1]) + item * hash_map->item_step + hash_map->ele_key->size);

    return 0;
}


static int hash_map_itor_remove(HashMap_T* hash_map, size_t itor)
{
    size_t buck = hash_map_index_buck(itor);
    size_t item = hash_map_index_item(itor);
    CHECK_RET_VAL(buck && item,-1); buck--; item--;
    CHECK_RET_VAL(buck < hash_map->hash_size,-1);

    HashMapItemList_T* list = hash_map->hash_list[buck];
    CHECK_RET_VAL(list && list->size && item < list->size,-1);

    if (hash_map->ele_key->fFree) hash_map->ele_key->fFree(((uint8_t*)&list[1]) + item * hash_map->item_step);
    if (hash_map->ele_val->fFree) hash_map->ele_val->fFree(((uint8_t*)&list[1]) + item * hash_map->item_step + hash_map->ele_key->size);

    if (list->size > 1){
        if (item < list->size - 1){
            memmove(((uint8_t*)&list[1]) + item * hash_map->item_step, ((uint8_t*)&list[1]) + (item + 1) * hash_map->item_step, (list->size - item - 1) * hash_map->item_step);    
        } 
        list->size--;
    }else {
        FREE(list);
        hash_map->hash_list[buck] = NULL;
    }

    hash_map->item_size--;
	return 0;
}

/*brief	:  hash map初始化
 *param ： key_type:key数据类型
 *param ： val_type:val数据类型
 *return： 成功返回hash_map操作辨识，失败返回NULL
 */
__EX_API__ HashMap_T* wow_hash_map_create(ElementType_E eKtype,ElementType_E eVtype)
{
    CHECK_RET_VAL_ERRNO_P(eKtype < ELEMENT_TYPE_MAX && eVtype< ELEMENT_TYPE_MAX ,
                            -PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    HashMap_T* hash_map = CALLOC(1,HashMap_T);
    CHECK_RET_VAL_ERRNO_P(hash_map,-SYSTEM_MALLOC_FAILED,"malloc HashMap_T failed!\n");
	
    hash_map->ele_key = wow_element_ops(eKtype);
    hash_map->ele_val = wow_element_ops(eVtype);


    hash_map->hash_size = Align_pow2(HASH_MAP_BUCKET_SIZE_DEFAULT);
    hash_map->hash_list = CALLOC(hash_map->hash_size,HashMapItemList_T*);
    CHECK_RET_GOTO_ERRNO_P(hash_map->hash_list,out,-SYSTEM_MALLOC_FAILED,"malloc HashMapItemList_T* failed!\n");

    hash_map->item_grow = HASH_MAP_ITEM_SIZE_DEFAULT;
	hash_map->item_step = hash_map->ele_key->size + hash_map->ele_val->size;

    return hash_map;
	
out:
	FREE(hash_map);
	return NULL;
}

/*brief :  释放hash map
 *param ： pptHashMap:hash_map操作符
 *return： 无
 */
__EX_API__ void wow_hash_map_destroy(HashMap_T** pptHashMap)
{
	CHECK_RET_VOID(pptHashMap && *pptHashMap);

	HashMap_T* hash_map = (HashMap_T*)*pptHashMap;
	wow_hash_map_clear(hash_map);
	
    if (hash_map->hash_list){
		FREE(hash_map->hash_list);
		hash_map->hash_list = NULL;
	}

    FREE(hash_map);
	*pptHashMap = NULL;
}

/*brief	:  hash map清空操作
 *param ： hmap:hash_map操作符
 *return： 无
 */
__EX_API__ void wow_hash_map_clear(HashMap_T* ptHashMap)
{
    CHECK_RET_VOID(ptHashMap && ptHashMap->hash_list);

    size_t i = 0;
    for (i = 0; i < ptHashMap->hash_size; i++){
        HashMapItemList_T* list = ptHashMap->hash_list[i];
        CHECK_RET_CONTINUE(list);

        if (ptHashMap->ele_key->fFree || ptHashMap->ele_val->fFree){
            size_t j = 0;
            size_t m = list->size;
            for (j = 0; j < m; j++){
                uint8_t* item = ((uint8_t*)&list[1]) + j * ptHashMap->item_step;
                if (ptHashMap->ele_key->fFree) {
                    ptHashMap->ele_key->fFree(item);
                }

                if (ptHashMap->ele_val->fFree){
                    ptHashMap->ele_val->fFree(item + ptHashMap->ele_key->size);
                }
            }
        }
        FREE(list);
        ptHashMap->hash_list[i] = NULL;
    }

    ptHashMap->item_size = 0;
    ptHashMap->item_maxn = 0;
    memset(&ptHashMap->item, 0, sizeof(HashMapItem_T));
}

/*brief	:  hash map获取key映射信息
 *param ： ptHashMap:hash_map操作符
 *param ： pKey:key内容
 *return： 成功返回key映射内容，失败返回NULL
 */
__EX_API__ void* wow_hash_map_get(HashMap_T* ptHashMap, const void* pKey)
{
    CHECK_RET_VAL_P(ptHashMap && pKey, NULL,"param input is NULL!\n");

    int ret = 0;
    size_t buck = 0;
    size_t item = 0;
    void* data = NULL;

    ret = hash_map_item_find(ptHashMap, pKey, &buck, &item);
    CHECK_RET_VAL(ret == 0,NULL);    

    ret = hash_map_item_at(ptHashMap, buck, item, NULL, &data);
    CHECK_RET_VAL(ret == 0,NULL);  

    return data;
}

/*brief	:  hash map插入key映射信息
 *param ： ptHashMap:hash_map操作符
 *param ： pKey:key内容
 *param ： pVal:val内容
 *return： 成功返回0，失败返回-1
 */
__EX_API__ int wow_hash_map_insert(HashMap_T* ptHashMap, const void* pKey, const void* pVal)
{
    CHECK_RET_VAL_P(ptHashMap,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(pKey && pVal,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

    int ret = 0;

    size_t buck = 0;
    size_t item = 0;

	ret = hash_map_item_find(ptHashMap, pKey, &buck, &item);
    HashMapItemList_T* list = ptHashMap->hash_list[buck];
	
    if (ret == 0) {///<存在直接覆盖
        ptHashMap->ele_val->fRepl(((uint8_t*)&list[1]) + item * ptHashMap->item_step + ptHashMap->ele_key->size, pVal);
    }else{
        if (list){
            if (list->size >= list->maxn){ ///<元素已存满list
                size_t maxn = list->maxn + ptHashMap->item_grow;
				char* tmp = REALLOC(list, sizeof(HashMapItemList_T) + maxn * ptHashMap->item_step);
				CHECK_RET_VAL_P(tmp, -SYSTEM_MALLOC_FAILED,"malloc size(%zu) failed!\n",sizeof(HashMapItemList_T) + maxn * ptHashMap->item_step);

                list = (HashMapItemList_T*)tmp;  
                list->maxn = maxn;
				memset(list + sizeof(HashMapItemList_T) + list->maxn * ptHashMap->item_step,0,ptHashMap->item_grow * ptHashMap->item_step);				
                
                ptHashMap->hash_list[buck] = list;
                ptHashMap->item_maxn += ptHashMap->item_grow;
            }

            item = list->size;
            ptHashMap->ele_key->fDupl(((uint8_t*)&list[1]) + item * ptHashMap->item_step, pKey);
            ptHashMap->ele_val->fDupl(((uint8_t*)&list[1]) + item * ptHashMap->item_step + ptHashMap->ele_key->size, pVal);
            list->size++;
        }else{
            //创建初始化列表
            list = (HashMapItemList_T*)MALLOC(sizeof(HashMapItemList_T) + ptHashMap->item_grow * ptHashMap->item_step);
            CHECK_RET_VAL_P_A(list, 0 ,"malloc HashMapItemList_T failed!\n");
			memset(list,0,sizeof(HashMapItemList_T) + ptHashMap->item_grow * ptHashMap->item_step);
			 
            list->size = 1;
            list->maxn = ptHashMap->item_grow;

            ptHashMap->ele_key->fDupl(((uint8_t*)&list[1]), pKey);
            ptHashMap->ele_val->fDupl(((uint8_t*)&list[1]) + ptHashMap->ele_key->size, pVal);
			
            ptHashMap->hash_list[buck] = list;
            ptHashMap->item_maxn += list->maxn;
        }
        ptHashMap->item_size++;
    }

	return 0;
}

/*brief	:  hash map移除key映射信息
 *param ： ptHashMap:hash_map操作符
 *param ： pKey:key内容
 *return： 无
 */
__EX_API__ void wow_hash_map_remove(HashMap_T* ptHashMap, const void* pKey)
{
    CHECK_RET_VOID(ptHashMap && pKey);

    int ret = 0;
    size_t buck = 0;
    size_t item = 0;

    ret = hash_map_item_find(ptHashMap, pKey, &buck, &item);
    CHECK_RET_VOID(ret == 0);

    hash_map_itor_remove(ptHashMap, hash_map_index_make(buck + 1, item + 1));
}

/*brief	:  hash map获取key映射个数
 *param ： ptHashMap:hash_map操作符
 *return： 返回已存在映射个数
 */
__EX_API__ size_t wow_hash_map_size(HashMap_T* ptHashMap)
{
    CHECK_RET_VAL(ptHashMap, 0);

    return ptHashMap->item_size;
}


/* brief    获取hash map的实例内容
 * param ： ptHashMap : hash_map操作符
 * parma ： fCallBack：匹配回调函数 不可为NULL 
 * parma ： pArg ：需要操作内容 可为NULL
 * return： 无
 */
__EX_API__ void wow_hash_map_foreach(HashMap_T* ptHashMap, hashmap_foreach_func_t fCallBack,void *pArg)
{
	size_t i = 0;
	size_t j = 0;
	int ret = 0;
    void* data = NULL;

    CHECK_RET_VOID(ptHashMap && fCallBack);

	for(i = 0; i < ptHashMap->hash_size; i++){
		HashMapItemList_T* list = ptHashMap->hash_list[i];
		CHECK_RET_CONTINUE(list);
  		for(j = 0; j < list->size; j++){
			ret = hash_map_item_at(ptHashMap, i, j, NULL, &data);
			CHECK_RET_CONTINUE(ret == 0);
		
			fCallBack(data,pArg);
		}
	}
}

