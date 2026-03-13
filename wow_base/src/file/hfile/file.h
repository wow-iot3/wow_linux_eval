#ifndef __FILE_H_
#define __FILE_H_

#include "file/wow_hfile.h"

typedef struct{
    union {
        int fd;
        FILE *fp;
    };
    char name[MAX_PATH_SIZE];
} FileDesc_T;

typedef struct{
    FileDesc_T * (*open)(const char *path, FileOpenMode_E mode);
    ssize_t (*write)(FileDesc_T *fd, const void *buf, size_t count);
    ssize_t (*read)(FileDesc_T *fd, void *buf, size_t count);
	ssize_t (*readline)(FileDesc_T *fd, void *buf, size_t count);
    int (*seek)(FileDesc_T *fd, off_t offset, int whence);
    int (*sync)(FileDesc_T *fd);
	int (*lines)(FileDesc_T *fd);
    size_t (*size)(FileDesc_T *fd);
    void (*close)(FileDesc_T *fd);
} FileOps_T;


#endif