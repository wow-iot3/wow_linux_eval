#ifndef __WOW_BASE_SYSTEM_WOW_SHM_H_
#define __WOW_BASE_SYSTEM_WOW_SHM_H_

#include <stdbool.h>

/*brief    创建共享内存
 *param ： snKey  ：共享内存标识字符
 *param ： nSize  ：共享内存存储大小
 *return： 成功返回共享内存地址 失败返回NULL
 */
void* wow_shm_create(size_t snKey,size_t snSize);

/*brief    附着共享内存
 *param ： snKey  ：共享内存标识字符
 *param ： nSize  ：共享内存存储大小
 *return： 成功返回共享内存地址 失败返回NULL
 */
void* wow_shm_attch(size_t snKey,size_t snSize);

/*brief    检测共享内存是否有效
 *return： 有效返回true 失败返回false
 */
bool wow_shm_check(size_t snKey);


/*brief    移除共享内存
 *param ： snKey  ：共享内存标识字符
 *param ： pAddr  ：共享内存地址
 *return： 无
 */
void wow_shm_remove(size_t snKey,void* pAddr);

#endif




