#ifndef __WOW_BASE_FILE_WOW_DIR_H_
#define __WOW_BASE_FILE_WOW_DIR_H_

#include "list/wow_string_list.h"

#ifdef __cplusplus
extern "C" {
#endif

/*brief    判断当前路径是否为文件夹
 *param ： pcPath:文件全路径
 *return： 是返回1 不是返回0 失败返回<0
 */
int wow_dir_is_dir(const char* pcPath);

/*brief    创建文件夹-逐层创建
 *param ： pcPath:文件全路径
 *return： 成功返回0 失败返回<0
 */
int wow_dir_create(const char* pcPath);

/*brief    删除文件夹-删除子文件夹及子文件
 *param ： pcPath:文件全路径
 *return： 成功返回0 失败返回<0
 */
int wow_dir_remove( const char* pcPath);

/*brief    判断当前路径是否为文件夹
 *param ： str:源文件全路径
 *return： 文件夹返回1 不是返回0 执行失败返回<0
 */
int wow_path_is_dir(char const* pcPath);

/*brief    获取文件夹文件列表
 *param ： pcPath:源文件全路径
 *param ： dest:目标文件全路径
 *return： 成功返回链表 失败返回NULL
 *注： 返回值StringList_T链表需要调用wow_stringlist_free手动释放
 */
StringList_T* wow_dir_fileList(char const* pcPath);

#ifdef __cplusplus
}
#endif

#endif

