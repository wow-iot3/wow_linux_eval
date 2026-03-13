#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>

#include "wow_iot_init.h"
#include "system/wow_time.h"
#include "shmcache/wow_shmcache.h"


static void usage(const char *prog)
{
    fprintf(stderr, "show shmcache stats.\n"
         "Usage: %s [config_filename] [clean|clear]\n"
         "\tclean or clear to reset the shm stats\n\n", prog);
}

int main(int argc, char *argv[])
{
    int last_index;
    bool clear_stats;
    char *config_filename;
    Shmcache_T* context;

    if (argc >= 2 && (strcmp(argv[1], "-h") == 0 ||
                strcmp(argv[1], "help") == 0 ||
                strcmp(argv[1], "--help") == 0))
    {
        usage(argv[0]);
        return 0;
    }

    clear_stats = false;
    if (argc >= 2) {
        last_index = argc - 1;
        if (strcmp(argv[last_index], "clean") == 0 || 
                strcmp(argv[last_index], "clear") == 0)
        {
            clear_stats = true;
        }
    }

    config_filename = "/etc/libshmcache.conf";
    if (argc >= 2) {
        if (argc == 2) {
            if (!clear_stats) {
                config_filename = argv[1];
            }
        } else {
            config_filename = argv[1];
        }
    }

    context = wow_shmcache_init_from_file(config_filename,false);
    if(context == NULL) return -1;

    if (clear_stats) {
        printf("shm stats cleared.\n\n");
    } else {
        wow_shmcache_stats_dump(context);
    }

    wow_shmcache_destroy(context);

	return 0;
}
