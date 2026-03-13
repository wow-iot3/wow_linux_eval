#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "file/wow_file.h"
#include "wow_iot_init.h"
#include "shmcache/wow_shmcache.h"

static void usage(const char *prog)
{
    fprintf(stderr, "shmcache set key value.\n"
         "Usage: %s [config_filename] <key> <value> <ttl>\n", prog);
}

int main(int argc, char *argv[])
{
	int result;
    int index;
    char *config_filename;
    Shmcache_T* context;


    if (argc >= 2 && (strcmp(argv[1], "-h") == 0 ||
                strcmp(argv[1], "help") == 0 ||
                strcmp(argv[1], "--help") == 0))
    {
        usage(argv[0]);
        return 0;
    }
    if (argc < 4) {
        usage(argv[0]);
        return -1;
    }

    config_filename = "/etc/libshmcache.conf";
    if (wow_file_is_file(argv[1])) {
        if (argc < 5) {
            usage(argv[0]);
            return -1;
        }
        config_filename = argv[1];
        index = 2;
    } else {
        index = 1;
    }
    wow_iot_init();

    context = wow_shmcache_init_from_file(config_filename,true);
    if(context == NULL) return -1;

    char *key_data = argv[index++];
    int key_len = strlen(key_data);
    char* val_data = argv[index++];
    int   val_len = strlen(val_data);
    int  ttl = atoi(argv[index++]);
    result = wow_shmcache_set(context,key_data ,key_len , val_data, val_len, ttl);
    if (result == 0) {
        printf("set key: %s success.\n", key_data);
    } else {
        fprintf(stderr, "set key: %s fail, errno: %d\n", key_data, result);
    }
    wow_shmcache_destroy(context);

    wow_iot_exit();
	return result;
}
