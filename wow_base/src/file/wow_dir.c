#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "prefix/wow_common.h"
#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_keyword.h"

#include "file/wow_dir.h"

/*brief    判断当前路径是否为文件夹
 *param ： pcPath:文件全路径
 *return： 是返回1 不是返回0 失败返回<0
 */
int wow_dir_is_dir(const char* pcPath)
{
    CHECK_RET_VAL_P(pcPath,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

	struct stat buf;
	if (stat(pcPath, &buf) != 0){
		return 0;
	}else{
        return S_ISDIR(buf.st_mode);
    }
}

/*brief    创建文件夹-逐层创建
 *param ： pcPath:文件全路径
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_dir_create(const char* pcPath)
{
	int ret = 0;
    char *temp, *pos;

	CHECK_RET_VAL_P(pcPath,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

    temp = strdup(pcPath);
    pos  = temp;

    if (strncmp(temp, "/", 1) == 0) {
        pos += 1;
    } else if (strncmp(temp, "./", 2) == 0) {
        pos += 2;
    }
	
    for ( ; *pos != '\0' && pos; ++ pos) {
        if (*pos == '/') {
            *pos = '\0';
			ret = mkdir(temp, 0775);
			if(errno == EEXIST) ret = 0;
			CHECK_RET_BREAK_EXE_P(ret == 0,ret = -SYSTEM_MKDIR_FAILED,"failed mkdir :%s\n",temp);
            *pos = '/';
        }
    }

    if (*(pos - 1) != '/') {
        ret = mkdir(temp, 0775);
		if(errno == EEXIST) ret = 0;
    }
	
    free(temp);
    return ret;
}

/*brief    删除文件夹-递归删除
 *param ： pcPath:文件夹路径
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_dir_remove(const char *pcPath)
{
    struct dirent *ent = NULL;
    char full_path[MAX_PATH_SIZE] = {0};
    int ret = 0;

	CHECK_RET_VAL_P(pcPath,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");


    DIR *pdir = opendir(pcPath);
	CHECK_RET_VAL_P_A(pdir,-SYSTEM_OPENDIR_FAILED,"can not open pcPath: %s\n", pcPath);

    while (NULL != (ent = readdir(pdir))) {
        if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) {
            continue;
        }
        memset(full_path, 0, MAX_PATH_SIZE);
        snprintf(full_path,MAX_PATH_SIZE-1, "%s/%s", pcPath, ent->d_name);
        if (ent->d_type == DT_DIR) {	///< 如果是文件夹，则递归删除
            ret = wow_dir_remove(full_path);
            CHECK_RET_VAL(ret == 0,-SYSTEM_RMDIR_FAILED);
        }else{
            ret = remove(full_path);
            CHECK_RET_VAL(ret == 0,-SYSTEM_REMOVE_FAILED);
        }
    }

    closedir(pdir);
    
    ///< 删除空文件夹
    return rmdir(pcPath);
}

/*brief    判断当前路径是否为文件夹
 *param ： str:源文件全路径
 *return： 文件夹返回1 不是返回0 执行失败返回<0
 */
__EX_API__ int wow_path_is_dir(char const* pcPath)
{
   CHECK_RET_VAL_P(pcPath,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

    struct stat buf = {0};
	CHECK_RET_VAL(stat (pcPath, &buf)  == 0,-SYSTEM_STAT_FAILED);
    
    return S_IFDIR == (S_IFDIR & buf.st_mode);
}

/*brief    获取文件夹文件列表
 *param ： pcPath:源文件全路径
 *param ： dest:目标文件全路径
 *return： 成功返回链表 失败返回NULL
 */
__EX_API__ StringList_T* wow_dir_fileList(char const* pcPath)
{
	struct dirent *ent = NULL;
	char full_path[MAX_PATH_SIZE];

	CHECK_RET_VAL_ERRNO_P(pcPath,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	
	DIR *pdir = opendir(pcPath);
	CHECK_RET_VAL_ERRNO_P(pdir,-SYSTEM_OPENDIR_FAILED,"can not open pcPath: %s\n", pcPath);

	StringList_T* fileList = wow_stringlist_new();
	while (NULL != (ent = readdir(pdir))) {
		if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) {
			continue;
		}
		memset(full_path, 0, ARRAY_SIZE(full_path));
		snprintf(full_path,MAX_PATH_SIZE-1, "%s%s", pcPath, ent->d_name);
		if (ent->d_type == DT_DIR) {
			continue;
		}else{
			wow_stringlist_append(fileList, full_path);
		}
	}
	closedir(pdir);
	
	return fileList;
}
