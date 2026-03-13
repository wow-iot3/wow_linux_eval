#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <errno.h>

#include "file/wow_file.h"
#include "shmcache/wow_shmcache.h"

static void usage(const char *prog)
{
    fprintf(stderr, "shmcache delete key.\n"
         "Usage: %s [config_filename] <key>\n", prog);
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
    if (argc < 2) {
        usage(argv[0]);
        return EINVAL;
    }

    config_filename = "/etc/libshmcache.conf";
    if (wow_file_is_file(argv[1])) {
        if (argc < 3) {
            usage(argv[0]);
            return EINVAL;
        }
        config_filename = argv[1];
        index = 2;
    } else {
        index = 1;
    }

    context = wow_shmcache_init_from_file(config_filename,false);
    if(context == NULL) return -1;

    char *key_data = argv[index++];
    int key_len = strlen(key_data);

    result = wow_shmcache_delete(context, key_data,key_len);
    if (result == 0) {
        printf("delete key: %s successfully.\n", key_data);
    } else {
        fprintf(stderr, "delete key: %s fail, errno: %d\n", key_data, result);
    }

    wow_shmcache_destroy(context);
	return result;
}
