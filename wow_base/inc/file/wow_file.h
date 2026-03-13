#ifndef __WOW_BASE_FILE_WOW_FILE_H_
#define __WOW_BASE_FILE_WOW_FILE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <stdlib.h>

typedef enum {
	FILE_NOFILE = 0, 	/**< no file type determined */
	FILE_REG,			/**< a regular file */
	FILE_DIR,			/**< a directory */
	FILE_CHR,			/**< a character device */
	FILE_BLK,			/**< a block device */
	FILE_PIPE,			/**< a FIFO / pipe */
	FILE_LINK,			/**< a symbolic link */
	FILE_SOCKET,		/**< a [unix domain] socket */
	FILE_UNKFILE = 127	/**< a file of some other unknown type */
} FileType_E; 
	
typedef struct{
	size_t mtime;	  //最后一次改变文件时间
	size_t atime;	  //最后一次访问时间
	FileType_E type;  //文件对应的模式，文件，目录等
	size_t size;	  //文件大小
}FileInfo_T;


/*brief    判断当前路径是否为文件
 *param ： pcPath:文件全路径
 *return： 是返回1 不是返回0 失败返回<0
 */
int wow_file_is_file(const char* pcPath);

/*brief    检查文件/文件夹是否存在
 *param ： pcFile:文件全路径
 *return： 存在返回1 不存在返回0 执行错误返回<0
 */
int wow_file_check_exist(const char *pcFile);

/*brief    创建文件
 *param ： pcFile:文件全路径
 *return： 成功返回0 失败返回<0
 */
int wow_file_create(const char *pcFile);

/*brief    删除文件
 *param ： pcFile:文件全路径
 *return： 成功返回0 失败返回<0
 */
int wow_file_remove(const char *pcFile);

/*brief    获取文件大小
 *param ： pcFile:文件路径
 *param ： pnSize:存储文件大小
 *return： 成功返回0 失败返回< 0
 */
int wow_file_size(const char *pcFile,int* pnSize);

/*brief    获取文件行数
 *param ： pcFile:文件全路径
 *param ： pnLine:存储文件行数
 *return： 成功返回0 失败返回< 0
 */
int wow_file_line_num(const char *pcFile,int* pnLine);

/*brief    文件重命名
 *param ： pcSrcFile:源文件名称
 *param ： pcDestFile:目标文件名称
 *return： 成功返回0 失败返回< 0
 */
int wow_file_rename(const char *pcSrc, const char *pcDest);

/*brief    获取文件信息
 *param ： pcFile:文件路径
 *param ： ptInfo:文件信息存储指针
 *return： 成功返回0 失败返回<0
 */
int wow_file_info(const char *pcFile, FileInfo_T *ptInfo);

/*brief    获取文件后缀名
 *param ： pcFile:文件路径
 *return： 成功返回后缀名称 失败返回NULL
 */
char *wow_file_suffix(const char *pcFile);

#ifdef __cplusplus
}
#endif

#endif
