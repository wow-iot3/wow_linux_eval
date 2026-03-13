#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"

#include "file.h"

#define MAX_RETRY_CNT   (3)



static FileDesc_T *io_open(const char *path, FileOpenMode_E mode)
{
    int flags  = 0;

	
    switch(mode) {
    case FILE_RDONLY:
        flags = O_RDONLY;//以只读的方式打开文件
        break;
    case FILE_WRONLY:
        flags = O_WRONLY;//以只写的方式打开文件
        break;
    case FILE_RDWR:
        flags = O_RDWR;//以读写的方式打开文件.
        break;
    case FILE_CREATE:
        flags = O_RDWR|O_CREAT;//创建一个文件
        break;
    case FILE_WRCLEAR:
        flags = O_WRONLY|O_TRUNC|O_CREAT; //如果文件已经存在,则删除文件的内容
        break;
    case FILE_APPEND:
    default:
        flags = O_RDWR|O_APPEND;//以追加的方式打开文件
        break;
    }

    int fd = open(path, flags, 0666);
	CHECK_RET_VAL_P_A(fd  >= 0,NULL,"open %s failed!\n", path);

	FileDesc_T *file = CALLOC(1,FileDesc_T);
	CHECK_RET_VAL_P_A(file,NULL,"malloc FileDesc_T failed!\n");
	
	file->fd   = fd;
    memcpy(file->name,path,MAX_PATH_SIZE-1);
	
    return file;
}

static ssize_t io_read(FileDesc_T *file, void *buf, size_t len)
{
    int n  = 0;
    char *p = (char *)buf;
    size_t left = len;
    size_t step = 1024*1024;
	int cnt = 0;
	
	CHECK_RET_VAL_P(file->fd > 0,-1,"file is not opened!\n");

	
    while (left > 0) {
        if (left < step)
            step = left;
        n = read(file->fd, (void *)p, step);
        if (n > 0) {
            p += n;
            left -= n;
            continue;
        } else if (n == 0) {
            break;
        }
        if (errno == EINTR || errno == EAGAIN) {
            if (++cnt > MAX_RETRY_CNT) {
                break;
            }
            continue;
        } else {
            break;
        }
    }
	
    return (len - left);
}

static ssize_t io_read_line(FileDesc_T *file, void *buf, size_t len)
{
	int   ret = 0;
    char  ch = EOF;
    int reads = 0;
	char *p = (char *)buf;

	CHECK_RET_VAL_P(file->fd > 0,-1,"file is not opened!\n");

	ret = read(file->fd,&ch,1);
	CHECK_RET_VAL(ret == 1,0);
    while(ch != '\n' && ch != EOF)
    {
    	*p++ = ch;
		reads++;
		CHECK_RET_BREAK(reads < len);
		ret = read(file->fd,&ch,1);
		CHECK_RET_BREAK(ret == 1);
    }
    *p = '\0';
    return reads;

}

static ssize_t io_write(FileDesc_T *file, const void *buf, size_t len)
{
    ssize_t n = 0;
    char *p = (char *)buf;
    size_t left = len;
    size_t step = 1024 * 1024;
    int cnt = 0;
	
	CHECK_RET_VAL_P(file->fd > 0,-1,"file is not opened!\n");

    while (left > 0) {
        if (left < step)
            step = left;
        n = write(file->fd, (void *)p, step);
        if (n > 0) {
            p += n;
            left -= n;
            continue;
        } else if (n == 0) {
            break;
        }
        if (errno == EINTR || errno == EAGAIN) {
            if (++cnt > MAX_RETRY_CNT) {
                break;
            }
            continue;
        } else {
            break;
        }
    }
    
    return (len - left);
}

static int io_seek(FileDesc_T *file, off_t offset, int whence)
{
	CHECK_RET_VAL_P(file->fd > 0,-1,"file is not opened!\n");

    return (lseek(file->fd, offset, whence) == offset)?0:-1;
}

static int io_sync(FileDesc_T *file)
{
	CHECK_RET_VAL_P(file->fd > 0,-1,"file is not opened!\n");

    return fsync(file->fd);
}

static int io_lines(FileDesc_T *file)
{
    return -1;
}

static size_t io_size(FileDesc_T *file)
{
    struct stat buf;
	if (!file) return 0;
    if (stat(file->name, &buf) < 0) {
        return 0;
    }
    return (size_t)buf.st_size;
}

static void io_close(FileDesc_T *file)
{
	CHECK_RET_VOID(file);
	
	close(file->fd);
	FREE(file);
}

FileOps_T io_ops = {
    io_open,
    io_write,
    io_read,
    io_read_line,
    io_seek,
    io_sync,
    io_lines,
    io_size,
    io_close,
};
