#ifndef __WOW_BASE_FILE_WOW_HFILE_H_
#define __WOW_BASE_FILE_WOW_HFILE_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum{
	FILE_FIO = 0,
	FILE_IO,
}FileCtx_E;

typedef enum{
	FILE_RDONLY = 0,
	FILE_WRONLY,
	FILE_RDWR,
	FILE_CREATE,
	FILE_WRCLEAR,
	FILE_APPEND,
}FileOpenMode_E;

typedef struct file_t File_T;

/*brief    打开文件
 *param ： eCtx:文件实现操作方式
 *param ： pcPath:文件全路径
 *param ： eMode:文件打开方式
 *return： 成功返回文件操作结构体指针 失败返回NULL
 */
File_T* wow_hfile_open(FileCtx_E eCtx,const char *pcPath, FileOpenMode_E eMode);

/*brief    关闭文件
 *param ： file:文件处理结构体指针
 *return： 无
 */
void wow_hfile_close(File_T*       ptFile);

/*brief    文件读操作
 *param ： file:文件处理结构体指针
 *param ： pData:读取内容缓存地址
 *param ： snSize:读取长度
 *return： 成功返回读取长度 失败返回<0
 */
int wow_hfile_read(File_T* ptFile, void *pData, size_t snSize);

/*brief    读文件一行操作
 *param ： file:文件处理结构体指针
 *param ： pData:读取内容缓存地址
 *param ： snSize:读取长度
 *return： 成功返回读取长度 失败返回<0
 *注:  FILE_IO不支持此接口
 */
int wow_hfile_readLine(File_T* ptFile, void *pData, size_t snSize);

/*brief    获取文件大小
 *param ： file:文件处理结构体指针
 *return： 成功返回文件大小 失败返回<0
 */
int wow_hfile_size(File_T* ptFile);

/*brief    文件写操作
 *param ： file:文件处理结构体指针
 *param ： pData:写内容缓存地址
 *param ： snSize:写入长度
 *return： 成功返回读取长度 失败返回<0
 */
int wow_hfile_write(File_T* ptFile, const void *pData, size_t snSize);

/*brief    文件同步
 *param ： file:文件处理结构体指针
 *return： 成功返回0 失败返回<0
 */
int wow_hfile_sync(File_T* ptFile);

/*brief    文件跳转
 *param ： file:文件处理结构体指针
 *param ： offset:偏移值
 *param ： whence:文件制定位置--SEEK_SET
 *return： 成功返回0 失败返回<0
 */
int wow_hfile_seek(File_T* ptFile, size_t snOffset, int nWhence);

#ifdef __cplusplus
}
#endif

#endif

