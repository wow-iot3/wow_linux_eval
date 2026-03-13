#ifndef __WOW_BASE_UTILS_WOW_BYTE_ARRAY_H_
#define __WOW_BASE_UTILS_WOW_BYTE_ARRAY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

///<动态数组内部默认分配字节大小为128；
///<当添加内容导致内部长度不够时，则分配当前容量*2个字节以供存储；
#define DEFALUT_BYTE_ARRAY_SZIE 128
typedef struct byte_array_t ByteArray_T;


/*brief    创建动态数组
*param ：  nSize:数组字节数
 *return： 成功返回创建动态数组指针，失败返回NULL
 */
ByteArray_T* wow_byte_array_new(int nSize);

/*brief    释放动态数组
 *param ： pptArray:动态数组指针
 *return： 无
 */
void wow_byte_array_free(ByteArray_T** pptArray);

/*brief    获取动态数组内容
 *param ： ptArray:动态数组指针
 *return： 成功返回内容信息,失败返回NULL
 */
uint8_t* wow_byte_array_data(ByteArray_T* ptArray);

/*brief    获取动态数组存储数据大小
 *param ： ptArray:动态数组指针
 *return： 成功返回数组大小 失败返回<0
 */
int wow_byte_array_size(ByteArray_T* ptArray) ;

/*brief    清空动态数组内容
 *param ： ptArray:动态数组指针
 *return： 成功返回0 失败返回<0
 */
int wow_byte_array_clear(ByteArray_T* ptArray);


/*******************************数据插入/移植************************************************/
/*brief    在动态数组追加内容
 *param ： ptArray:动态数组指针
 *param ： u8Value:待添加内容
 *return： 成功返回0,失败返回<0
 */
int wow_byte_array_append_data(ByteArray_T* ptArray, uint8_t u8Value) ;
int wow_byte_array_append_ndata(ByteArray_T* self, uint8_t* u8Value,uint8_t snLen);

/*brief    在动态数组追加内容
 *param ： ptArray:动态数组指针
 *param ： ptToadd:待添加内容
 *return： 成功返回0,失败返回<0
 */
int wow_byte_array_append_array(ByteArray_T* ptArray, ByteArray_T* ptToadd);

/*brief    在动态数组插入内容
 *param ： ptArray:动态数组指针
 *param ： snIdx:插入坐标
 *param ： u8Value:待添加内容
 *return： 成功返回0,失败返回<0
 */
int wow_byte_array_insert(ByteArray_T* ptArray,size_t snIdx,uint8_t u8Value);

/*brief    删除动态数组内容
 *param ： ptArray:动态数组指针
 *param ： snIdx:指定坐标
 *return： 成功返回0,失败返回<0
 */
int wow_byte_array_remove(ByteArray_T* self, size_t snIdx ,size_t snLen);

/*******************************数据截断************************************************/
/*brief    截取动态数组左边len个长度
 *param ： ptArray:动态数组指针
 *param ： snLen:截取内容长度
 *return： 成功返回0,失败返回<0
 */
int wow_byte_array_left(ByteArray_T* ptArray,size_t snLen);

/*brief    截取动态数组右边len个长度
 *param ： ptArray:动态数组指针
 *param ： snLen:截取内容长度
 *return： 成功返回0,失败返回<0
 */
int wow_byte_array_right(ByteArray_T* ptArray,size_t snLen);

/*brief    截取动态数组idx位置len个长度
 *param ： ptArray:动态数组指针
 *param ： snIdx:截取内容坐标
 *param ： snLen:截取内容长度
 *return： 成功返回0,失败返回<0
 */
int wow_byte_array_mid(ByteArray_T* ptArray,size_t snIdx, size_t snLen);

/*******************************修改/查找/替换************************************************/
/*brief    获取动态数组内容
 *param ： ptArray:动态数组指针
 *param ： snIdx:指定坐标
 *param ： pu8Value:内容存储指针
 *return： 成功返回0,失败返回<0
 */
int wow_byte_array_get( ByteArray_T* ptArray, size_t snIdx,uint8_t* pu8Value);

/*brief    设置动态数组内容
 *param ： ptArray:动态数组指针
 *param ： snIdx:指定坐标
 *param ： u8Value:待s设置内容
 *return： 成功返回0,失败返回<0
 */
int wow_byte_array_set(ByteArray_T* ptArray, size_t snIdx, uint8_t u8Value);

/*brief    查找动态数组内容
 *param ： ptArray:动态数组指针
 *param ： u8Value:新内容
 *return： 成功返回位置标识,失败返回-<0
 */
int wow_byte_array_find(ByteArray_T* ptArray,uint8_t u8Value);

/*brief    替换动态数组内容
 *param ： ptArray:动态数组指针
 *param ： u8Obj:待替换内容
 *param ： u8Value:新内容
 *return： 成功返回0,失败返回<0
 */
int wow_byte_array_replace(ByteArray_T* ptArray,uint8_t u8Obj,uint8_t u8Value);



#ifdef __cplusplus
}
#endif

#endif  
