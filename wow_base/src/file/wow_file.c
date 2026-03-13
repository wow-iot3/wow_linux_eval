#define _GNU_SOURCE
#include <dlfcn.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/vfs.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <libgen.h>
#include <limits.h>
#include <inttypes.h>



#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_keyword.h"

#include "file/wow_file.h"
#include "file/wow_dir.h"

/*brief    判断当前路径是否为文件
 *param ： pcPath:文件全路径
 *return： 是返回1 不是返回0 失败返回<0
 */
int wow_file_is_file(const char* pcPath)
{
    CHECK_RET_VAL_P(pcPath,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

	struct stat buf;
	if (stat(pcPath, &buf) != 0){
		return 0;
	}else{
        return S_ISREG(buf.st_mode);
    }
}

/*brief    检查文件/文件夹是否存在
 *param ： pcFile:文件全路径
 *return： 存在返回1 不存在返回0 执行错误返回< 0
 */
__EX_API__ int wow_file_check_exist(const char *pcFile)
{
	CHECK_RET_VAL_P(pcFile,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	
    return (access(pcFile, F_OK|W_OK|R_OK) == 0) ? 1 : 0;
}


/*brief    创建文件
 *param ： pcFile:文件全路径
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_file_create(const char *pcFile)
{
	CHECK_RET_VAL_P(pcFile,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

	char *tmp = strrchr((char *)pcFile, '/');
	if(tmp){
		size_t len = tmp - pcFile;
		char* dir = malloc(len+1);
		memset(dir,0,len+1);
		memcpy(dir,pcFile,len);
		wow_dir_create(dir);
		free(dir);
	}
    FILE *fp = fopen(pcFile, "w"); 
	CHECK_RET_VAL(fp,-SYSTEM_FOPEN_FAILED);

    fclose(fp);
	
    return WOW_SUCCESS;
}

/*brief    两个文件内容比较
 *param ： pcFile1:文件全路径1
 *param ： pcFil2:文件全路径2
 *return： 内容相同返回0 不相同返回1 错误参数返回< 0
 */
__EX_API__ int wow_file_compare(const char* pcFile1,const char* pcFil2)
{
    int ret = 0;
	
	CHECK_RET_VAL_P(pcFile1 && pcFil2,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	
	FILE* const fp1 = fopen(pcFile1, "rb");
    FILE* const fp2 = fopen(pcFil2, "rb");
	CHECK_RET_VAL_P(fp1 && fp2,-SYSTEM_FOPEN_FAILED,"file open failed!\n");
	
    while (ret == 0) {
        char b0[1024];
        char b1[1024];
        size_t const r0 = fread(b0, 1, sizeof(b0), fp1);
        size_t const r1 = fread(b1, 1, sizeof(b1), fp2);

        ret = (r0 != r1);
        if (!r0 || !r1) break;
        if (!ret) ret = memcmp(b0, b1, r0);

		fclose(fp1);
        fclose(fp2);
    }

    return ret;
}

/*brief    删除文件
 *param ： pcFile:文件全路径
 *return： 成功返回0 失败返回< 0
 */
__EX_API__ int wow_file_remove(const char *pcFile)
{
	CHECK_RET_VAL_P(pcFile,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

    return remove(pcFile);
}

/*brief    获取文件大小
 *param ： pcFile:文件路径
 *param ： pnSize:存储文件大小
 *return： 成功返回0 失败返回< 0
 */
__EX_API__ int wow_file_size(const char *pcFile,int* pnSize)
{
	CHECK_RET_VAL_P(pcFile,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

	struct stat st;
	int ret = stat(pcFile, &st);
	CHECK_RET_VAL_P_A(ret == 0, -SYSTEM_STAT_FAILED,"file stat failed!\n");
    *pnSize = st.st_size;


    return WOW_SUCCESS;
}

/*brief    获取文件行数
 *param ： pcFile:文件全路径
 *param ： pnLine:存储文件行数
 *return： 成功返回0 失败返回< 0
 */
__EX_API__ int wow_file_line_num(const char *pcFile,int* pnLine)
{
	CHECK_RET_VAL_P(pcFile,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

	int ch; 	
    int lineCnt = 0;
	
	FILE *fp = fopen(pcFile, "r"); 
	CHECK_RET_VAL_P(fp,-SYSTEM_FOPEN_FAILED,"file open failed!\n");
	
	while((ch = fgetc(fp)) != EOF) 
	{	
		 if(ch == '\n')  
		 {	
			 lineCnt++;  
		 } 
	 }	

    fclose(fp);
	
    *pnLine = lineCnt;
    return WOW_SUCCESS;
}

/*brief    文件重命名
 *param ： pcSrcFile:源文件名称
 *param ： pcDestFile:目标文件名称
 *return： 成功返回0 失败返回< 0
 */
__EX_API__ int wow_file_rename(const char *pcSrcFile, const char *pcDestFile)
{
	CHECK_RET_VAL_P(pcSrcFile && pcDestFile,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
		
    return rename(pcSrcFile, pcDestFile);
}

/*brief    获取文件信息
 *param ： pcFile:文件路径
 *param ： ptInfo:文件信息存储指针
 *return： 成功返回0 失败返回< 0
 */
__EX_API__ int wow_file_info(const char *pcFile, FileInfo_T *ptInfo)
{
	CHECK_RET_VAL_P(pcFile,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

    struct stat st;
    int ret = stat(pcFile, &st);
	CHECK_RET_VAL(ret == 0, -SYSTEM_STAT_FAILED);

    switch (st.st_mode & S_IFMT) {
    case S_IFSOCK:///<套接字
        ptInfo->type = FILE_SOCKET;
        break;
    case S_IFLNK:///<符号链接
        ptInfo->type = FILE_LINK;
        break;
    case S_IFBLK:///<块设备
		ptInfo->type = FILE_BLK;
        break;
    case S_IFCHR:///<字符设备
        ptInfo->type = FILE_CHR;
        break;
    case S_IFREG:///<常规文件
        ptInfo->type = FILE_REG;
        break;
    case S_IFDIR:///<目录
        ptInfo->type = FILE_DIR;
        break;
    default:
        break;
    }
    ptInfo->size  = st.st_size;
    ptInfo->atime = st.st_atim.tv_sec;
    ptInfo->mtime = st.st_ctim.tv_sec;
    
    return 0;
}

/*brief    获取文件后缀名
 *param ： pcFile:文件路径
 *return： 成功返回后缀名称 失败返回NULL
 */
__EX_API__ char *wow_file_suffix(const char *pcFile)
{
    CHECK_RET_VAL_ERRNO_P(pcFile,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

    char *tmp = strrchr((char *)pcFile, FILE_SUFFIX);
    if (tmp) {
        return tmp+1;
    }
	
    return NULL;
}

