#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"

#include "file.h"


#define MAX_RETRY_CNT   (3)

static FileDesc_T *fio_open(const char *path, FileOpenMode_E mode)
{
    const char *flags = NULL;
	
    switch(mode) {
    case FILE_RDONLY:
        flags = "r";
        break;
    case FILE_WRONLY:
        flags = "w";
        break;
    case FILE_RDWR:
        flags = "r+";
        break;
    case FILE_CREATE:
        flags = "a+";
        break;
    case FILE_WRCLEAR:
        flags = "w+";
        break;
    case FILE_APPEND:
    default:
        flags = "a+";
        break;
    }

    FILE *fp = fopen(path, flags);
	CHECK_RET_VAL_P(fp,NULL,"fopen %s failed:%d %s\n", path, errno, strerror(errno));

	FileDesc_T *file = CALLOC(1,FileDesc_T);
	CHECK_RET_VAL_P(file,NULL,"malloc failed:%s\n",strerror(errno));

	file->fp   = fp;
	memcpy(file->name,path,MAX_PATH_SIZE-1);
	
    return file;
}

static ssize_t fio_read(FileDesc_T *file, void *buf, size_t len)
{
    int n = 0;
    FILE *fp = NULL;
    char *p = (char *)buf;
    size_t left = len;
    size_t step = 1024*1024;
    int cnt = 0;

	CHECK_RET_VAL_P(file->fp,-1,"file is not opened!\n");
	
    fp = file->fp;
    while (left > 0) {
        if (left < step)
            step = left;
        n = fread((void *)p, 1, step, fp);
        if (n > 0) {
            p += n;
            left -= n;
            continue;
        } else {
            if (0 != feof(fp)) {
                clearerr(fp);
                break;
            } else {
                if (++cnt > MAX_RETRY_CNT)
                    break;
                continue;
            }
        }
    }
    return (len - left);
}

static ssize_t fio_read_line(FileDesc_T *file, void *buf, size_t len)
{
	CHECK_RET_VAL_P(file->fp,-1,"file is not opened!\n");

    if(fgets((char *)buf, len, file->fp) != NULL){
        return strlen((char *)buf);
    }else{
        return 0;
    }
}

static ssize_t fio_write(FileDesc_T *file, const void *buf, size_t len)
{
    FILE *fp = NULL;
    ssize_t n = 0;
    size_t step = 0;
    size_t left = 0;
    char *p = (char *)buf;
    int retry = 0;
	
	CHECK_RET_VAL_P(file->fp,-1,"file is not opened!\n");

    fp = file->fp;
    step = 1024 * 1024;
    left = len;
    while (left > 0) {
        if (left < step)
            step = left;
        n = fwrite((void *)p, 1, step, fp);
        if (n > 0) {
            p += n;
            left -= n;
            continue;
        } else {
            if (errno == EINTR || errno == EAGAIN) {
                if (++retry > MAX_RETRY_CNT) {
                    break;
                }
                continue;
            } else {
                break;
            }
        }
    }
    return (len - left);
}

static int fio_seek(FileDesc_T *file, off_t offset, int whence)
{
 	CHECK_RET_VAL_P(file->fp,-1,"file is not opened!\n");
	
    return fseek(file->fp, offset, whence);
}

static int fio_sync(FileDesc_T *file)
{
	CHECK_RET_VAL_P(file->fp,-1,"file is not opened!\n");

    return fflush(file->fp);
}

static int fio_lines(FileDesc_T *file)
{
    int ch = EOF;
    int lines = 0;
    int pc = '\n';

	CHECK_RET_VAL_P(file->fp,-1,"file is not opened!\n");
		
    while((ch = getc(file->fp)) != EOF)
    {
        if(ch == '\n')
            lines++;
        pc = ch;
    }
    if(pc != '\n')
        lines++;
    rewind(file->fp);
    return lines;
}

static size_t fio_size(FileDesc_T *file)
{
    long size = 0;
    long tmp  = 0;
	
	CHECK_RET_VAL_P(file->fp,-1,"file is not opened!\n");
	
    tmp = ftell(file->fp);
    fseek(file->fp, 0L, SEEK_END);
    size = ftell(file->fp);
    fseek(file->fp, tmp, SEEK_SET);
	
    return (size_t)size;
}

static void fio_close(FileDesc_T *file)
{
	CHECK_RET_VOID(file);
	
	fclose(file->fp);
	FREE(file);
}

FileOps_T fio_ops = {
    fio_open,
    fio_write,
    fio_read,
    fio_read_line,
    fio_seek,
    fio_sync,
    fio_lines,
    fio_size,
    fio_close,
};
