#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"

#include "list/wow_kv_list.h"

typedef struct AVLNode {
    char *key;
    char *value;
    uint32_t height;
    struct AVLNode *left;
    struct AVLNode *right;
}__data_aligned__ AVLNode;



static AVLNode* avl_new_node(char *key, char *value) 
{
    AVLNode *node = CALLOC(1,AVLNode);
	CHECK_RET_VAL_P(node,NULL,"malloc AVLNode failed!\n");
	
    node->key = MALLOC(strlen(key)+1);
	memset(node->key,0,strlen(key)+1);
	memcpy(node->key,key,strlen(key));
	
    node->value = MALLOC(strlen(value)+1);
	memset(node->value,0,strlen(value)+1);
	memcpy(node->value,value,strlen(value));
	
    node->height = 1;
    node->left = NULL;
    node->right = NULL;
	
    return node;
}
static void avl_free_node(AVLNode* node)
{
	FREE(node->key);
	FREE(node->value);
	FREE(node);
}

static int avl_height(AVLNode *node) 
{
	CHECK_RET_VAL(node,0);
	
    return node->height;
}

// 右旋转
static AVLNode* avl_rotate_right(AVLNode *y) 
{
    AVLNode *x = y->left;
    AVLNode *T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = MAX2(avl_height(y->left), avl_height(y->right)) + 1;
    x->height = MAX2(avl_height(x->left), avl_height(x->right)) + 1;
    return x;
}

// 左旋转
static AVLNode* avl_rotate_left(AVLNode *x) 
{
    AVLNode *y = x->right;
    AVLNode *T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = MAX2(avl_height(x->left), avl_height(x->right)) + 1;
    y->height = MAX2(avl_height(y->left), avl_height(y->right)) + 1;
    return y;
}

// 获取节点平衡因子
static int avl_get_balance(AVLNode *node) 
{
    CHECK_RET_VAL(node,0);
	
    return avl_height(node->left) - avl_height(node->right);
}

// 插入节点
static AVLNode* avl_insert(AVLNode *node, char *key, char *value) 
{
    if (node == NULL) {
        return avl_new_node(key, value);
    }
	
    if (strcmp(key, node->key) < 0) {
        node->left = avl_insert(node->left, key, value);
    } else if (strcmp(key, node->key) > 0) {
        node->right = avl_insert(node->right, key, value);
    } else {
        //如果键已经存在，则更新值
        node->value = value;
        return node;
    }
	
    node->height = 1 + MAX2(avl_height(node->left), avl_height(node->right));
    int balance = avl_get_balance(node);
    if (balance > 1 && strcmp(key, node->left->key) < 0) {
        return avl_rotate_right(node);
    }
    if (balance < -1 && strcmp(key, node->right->key) > 0) {
        return avl_rotate_left(node);
    }
    if (balance > 1 && strcmp(key, node->left->key) > 0) {
        node->left = avl_rotate_left(node->left);
        return avl_rotate_right(node);
    }
    if (balance < -1 && strcmp(key, node->right->key) < 0) {
        node->right = avl_rotate_right(node->right);
        return avl_rotate_left(node);
    }
	
    return node;
}

// 查找节点
static void* avl_search(AVLNode *node, char *key) 
{
	CHECK_RET_VAL(node,NULL);
	
    if (strcmp(key, node->key) < 0) {
        return avl_search(node->left, key);
    } else if (strcmp(key, node->key) > 0) {
        return avl_search(node->right, key);
    } else {
        return node->value;
    }
}

// 获取最小节点
static AVLNode* avl_get_min(AVLNode *node) 
{
    AVLNode *current = node;
    while (current->left != NULL) {
        current = current->left;
    }
    return current;
}

// 删除节点
static AVLNode* avl_delete(AVLNode *node, char *key)
{
	CHECK_RET_VAL(node,node);
	
    if (strcmp(key, node->key) < 0) {
        node->left = avl_delete(node->left, key);
    } else if (strcmp(key, node->key) > 0) {
        node->right = avl_delete(node->right, key);
    } else {
        if (node->left == NULL && node->right == NULL) {
            avl_free_node(node);
            return NULL;
        } else if (node->left == NULL) {
            AVLNode *temp = node->right;
            avl_free_node(node);
            return temp;
        } else if (node->right == NULL) {
            AVLNode *temp = node->left;
            avl_free_node(node);
            return temp;
        } else {
            AVLNode *temp = avl_get_min(node->right);
            node->key = temp->key;
            node->value = temp->value;
            node->right = avl_delete(node->right, temp->key);
        }
    }
    node->height = MAX2(avl_height(node->left), avl_height(node->right)) + 1;
    int balance = avl_get_balance(node);
    if (balance > 1 && avl_get_balance(node->left) >= 0) {
        return avl_rotate_right(node);
    }
    if (balance > 1 && avl_get_balance(node->left) < 0) {
        node->left = avl_rotate_left(node->left);
        return avl_rotate_right(node);
    }
    if (balance < -1 && avl_get_balance(node->right) <= 0) {
        return avl_rotate_left(node);
    }
    if (balance < -1 && avl_get_balance(node->right) > 0) {
        node->right = avl_rotate_right(node->right);
        return avl_rotate_left(node);
    }
    return node;
}

// 修改节点
static AVLNode* avl_update(AVLNode *node, char *key, char *value) 
{
    CHECK_RET_VAL(node,node);
	
    if (strcmp(key, node->key) < 0) {
        node->left = avl_update(node->left, key, value);
    } else if (strcmp(key, node->key) > 0) {
        node->right = avl_update(node->right, key, value);
    } else {
		FREE(node->value);
		node->value = MALLOC(strlen(value)+1);
		memcpy(node->value,value,strlen(value));
    }
	
    node->height = MAX2(avl_height(node->left), avl_height(node->right)) + 1;
    int balance = avl_get_balance(node);
    if (balance > 1 && avl_get_balance(node->left) >= 0) {
        return avl_rotate_right(node);
    }
    if (balance > 1 && avl_get_balance(node->left) < 0) {
        node->left = avl_rotate_left(node->left);
        return avl_rotate_right(node);
    }
    if (balance < -1 && avl_get_balance(node->right) <= 0) {
        return avl_rotate_left(node);
    }
    if (balance < -1 && avl_get_balance(node->right) > 0) {
        node->right = avl_rotate_right(node->right);
        return avl_rotate_left(node);
    }
	
    return node;
}


static void avl_get_all(AVLNode *node, char **keys, char **values, int *count) 
{
    CHECK_RET_VOID(node);
	
    avl_get_all(node->left, keys, values, count);

    keys[*count] = (char *)node->key;
    values[*count] = (char *)node->value;
    (*count)++;
	
    avl_get_all(node->right, keys, values, count);
}

static int avl_get_size(AVLNode *node) 
{
    CHECK_RET_VAL(node,0);
	
    return avl_get_size(node->left) + avl_get_size(node->right) + 1;
}


static void avl_del_all(AVLNode *node) 
{
    CHECK_RET_VOID(node);
	
    avl_del_all(node->left);
    avl_del_all(node->right);
	
    avl_free_node(node);
}

struct kv_list_t {
    AVLNode *root;
};

/*brief    初始化键值链表
 *return： 成功返回键值链表操作符
 */
__EX_API__ KvList_T* wow_kvlist_create(void)
{
	KvList_T* root =  CALLOC(1,KvList_T);
	CHECK_RET_VAL_ERRNO_P(root, SYSTEM_MALLOC_FAILED,"malloc KvList_T failed!\n");
	
	return root;
}

/*brief    退出键值链表
 *param ： ptKvlist:键值链表操作符
 *return： 无
 */
__EX_API__ void wow_kvlist_destroy(KvList_T** pptKvlist)
{
	CHECK_RET_VOID(pptKvlist && *pptKvlist);

	KvList_T* kvlist = (KvList_T*)*pptKvlist;
	avl_del_all(kvlist->root);
	
	FREE(kvlist);
	kvlist = NULL;
}


/*brief    获取键值链表
 *param ： ptKvlist:键值链表操作符
 *param ： pcKey:键值
 *return： 成功返回值 失败返回NULL
 */
__EX_API__ char* wow_kvlist_get(KvList_T *ptKvlist, char *pcKey)
{
	CHECK_RET_VAL_ERRNO_P(ptKvlist,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_ERRNO_P(pcKey,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	
    return avl_search(ptKvlist->root, pcKey);
}

/*brief    设置键值链表
 *param ： ptKvlist:键值链表操作符
 *param ： pcKey:名称
 *param ： value:值
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_kvlist_set(KvList_T *ptKvlist,char *pcKey,char *pcVal)
{
	CHECK_RET_VAL_P(ptKvlist,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(pcKey && pcVal,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	
	ptKvlist->root = avl_insert(ptKvlist->root, pcKey, pcVal);

	return WOW_SUCCESS;
}

/*brief    遍历链表
 *param ： ptKvlist:键值链表操作符
 *param ： ppcKeys:名称存储地址
 *param ： ppcValues:值存储地址
 *param ： pnCount:键值对个数存储地址  
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_kvlist_foreach(KvList_T *ptKvlist,char **ppcKeys, char **ppcValues, int *pnCount)
{
	CHECK_RET_VAL_P(ptKvlist,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(ppcKeys && ppcValues && pnCount,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");


	*pnCount = 0;
	
	avl_get_all(ptKvlist->root, ppcKeys, ppcValues,pnCount);

	return WOW_SUCCESS;
}

/*brief    更新键值链表
 *param ： ptKvlist:键值链表操作符
 *param ： pcKey:名称
 *param ： pcVal:值
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_kvlist_update(KvList_T *ptKvlist,char *pcKey,char *pcVal)
{
	CHECK_RET_VAL_P(ptKvlist,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(pcKey && pcVal,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

	ptKvlist->root = avl_update(ptKvlist->root, pcKey, pcVal);

	return WOW_SUCCESS;
}

/*brief    删除键值链表
 *param ： ptKvlist:键值链表操作符
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_kvlist_delete(KvList_T *ptKvlist, char *pcKey) 
{
	CHECK_RET_VAL_P(ptKvlist,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(pcKey,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

    ptKvlist->root = avl_delete(ptKvlist->root, pcKey);

	return WOW_SUCCESS;
}

/*brief    获取链表键值个数
 *param ： ptKvlist:键值链表操作符
 *return： 成功返回个数 失败返回<0
 */
__EX_API__ int wow_kvlist_size(KvList_T *ptKvlist)
{
	CHECK_RET_VAL_P(ptKvlist,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");

	return avl_get_size(ptKvlist->root);
}

