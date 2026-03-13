


#include <stdio.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#include <limits.h>
#include <inttypes.h>

#include "prefix/wow_check.h"
#include "prefix/wow_cpu.h"
#include "encrypt/wow_crc.h"
#include "file/wow_hfile.h"

#define READ_BUFFER_SIZE (16*1024)
int main(int argc,char* argv[])
{
    int ret = -1;
    int left_size = 0;
    File_T *file = NULL;
    int step = READ_BUFFER_SIZE;
    uint8_t buff[READ_BUFFER_SIZE] = {0};
    uint16_t crc = 0;
    uint8_t data[2] = {0};

	if(argc != 2){
		printf("Usage: %s <file>\n", argv[0]);
		return -1;
	}

	file = wow_hfile_open(FILE_FIO,argv[1], FILE_RDWR);
	CHECK_RET_VAL_P(file,-1,"wow_hfile_open %s failed!\n", argv[1]);

    left_size = wow_hfile_size(file);
    CHECK_RET_GOTO_P(left_size > 0,out,"get %s size failed!\n",argv[1]);

    do {
        step = (left_size>(READ_BUFFER_SIZE))?READ_BUFFER_SIZE:left_size;
        ret = wow_hfile_read(file,(void *)buff, step);
		CHECK_RET_GOTO_P(ret > 0,out,"wow_hfile_read %s failed!\n",argv[1]);

        crc += wow_crc16_add(buff,ret);
        left_size -= ret;
    } while(left_size > 0);

       
    if(WOW_LITTLE_ENDIAN){
        memcpy(data,&crc,2);
    }else{
        uint8_t* c = (uint8_t*)&crc;
        data[0] = c[1];
        data[1] = c[0];
    }

    ret = wow_hfile_write(file,data, 2);
    CHECK_RET_GOTO_P(ret == 2,out,"wow_hfile_write %s failed!\n",argv[1]);

    wow_hfile_close(file);

    printf("wow_tools_crc success!\n");

    return 0;
out:
    printf("wow_tools_crc failed!\n");  
    wow_hfile_close(file);
    return -1;
}

