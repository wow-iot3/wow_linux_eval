#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"

#include "file/wow_file.h"

#include "hfile/file.h"

struct  file_t{
    FileDesc_T *fd;
    const FileOps_T *ops;
    size_t size;
}__data_aligned__;

extern const FileOps_T io_ops;
extern const FileOps_T fio_ops;

/*******************************文件读写操作*********************************************/
/*brief    打开文件
 *param ： eCtx:文件实现操作方式
 *param ： pcPath:文件全路径
 *param ： eMode:文件打开方式
 *return： 成功返回文件操作结构体指针 失败返回NULL
 */
File_T* wow_hfile_open(FileCtx_E eCtx,const char *pcPath, FileOpenMode_E eMode)
{
	CHECK_RET_VAL_ERRNO_P(pcPath,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

	File_T *hfile = CALLOC(1,File_T);
	CHECK_RET_VAL_ERRNO_P(hfile,-SYSTEM_MALLOC_FAILED,"calloc File_T failed!\n");

	switch (eCtx){
		case FILE_IO:
			hfile->ops = &io_ops;
			break;
		case FILE_FIO:
		default:
			hfile->ops = &fio_ops;
			break;
	}
    
    hfile->fd = hfile->ops->open(pcPath, eMode);
	CHECK_RET_GOTO_ERRNO_P(hfile->fd,out,-FILE_OPEN_FAILED,"ops open file failed!\n");

    return hfile;

out:
	FREE(hfile);
	return NULL;
}

/*brief    关闭文件
 *param ： file:文件处理结构体指针
 *return： 无
 */
void wow_hfile_close(File_T* ptFile)
{
	CHECK_RET_VOID(ptFile);
	
    ptFile->ops->close(ptFile->fd);
    FREE(ptFile);
}

/*brief    文件读操作
 *param ： ptFile:文件处理结构体指针
 *param ： data:读取内容缓存地址
 *param ： size:读取长度
 *return： 成功返回读取长度 失败返回<0
 */
int wow_hfile_read(File_T* ptFile, void *pData, size_t snSize)
{
	CHECK_RET_VAL_P(ptFile,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(pData,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(snSize > 0,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");
	
    return ptFile->ops->read(ptFile->fd, pData, snSize);
}

/*brief    读文件一行操作
 *param ： ptFile:文件处理结构体指针
 *param ： data:读取内容缓存地址
 *param ： size:读取长度
 *return： 成功返回读取长度 失败返回<0
 */
int wow_hfile_readLine(File_T* ptFile, void *pData, size_t snSize)
{
	CHECK_RET_VAL_P(ptFile,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(pData,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(snSize > 0,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    return ptFile->ops->readline(ptFile->fd, pData, snSize);
}


/*brief    文件写操作
 *param ： ptFile:文件处理结构体指针
 *param ： data:写内容缓存地址
 *param ： size:写入长度
 *return： 成功返回读取长度 失败返回<0
 */
int wow_hfile_write(File_T* ptFile, const void *pData, size_t snSize)
{
	CHECK_RET_VAL_P(ptFile,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(pData,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(snSize > 0,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

    return ptFile->ops->write(ptFile->fd, pData, snSize);
}


/*brief    获取文件大小
 *param ： ptFile:文件处理结构体指针
 *return： 成功返回文件大小 失败返回<0
 */
int wow_hfile_size(File_T* ptFile)
{
	CHECK_RET_VAL_P(ptFile,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");

    return ptFile->ops->size(ptFile->fd);
}


/*brief    文件同步
 *param ： ptFile:文件处理结构体指针
 *return： 成功返回0 失败返回<0
 */
int wow_hfile_sync(File_T* ptFile)
{
	CHECK_RET_VAL_P(ptFile,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");

    return ptFile->ops->sync(ptFile->fd);
}

/*brief    文件跳转
 *param ： ptFile:文件处理结构体指针
 *param ： snOffset:偏移值
 *param ： nWhence:文件制定位置--SEEK_SET
 *return： 成功返回0 失败返回<0
 */
int wow_hfile_seek(File_T* ptFile, size_t snOffset, int nWhence)
{
	CHECK_RET_VAL_P(ptFile,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");

    return ptFile->ops->seek(ptFile->fd, snOffset, nWhence);
}

