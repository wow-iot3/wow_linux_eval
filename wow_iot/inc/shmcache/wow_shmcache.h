#ifndef __WOW_IOT_SHMCACHE_WOW_SHMCACHE_H__
#define __WOW_IOT_SHMCACHE_WOW_SHMCACHE_H__

#include <stdbool.h>

typedef struct shmcache_context Shmcache_T;


Shmcache_T* wow_shmcache_init_from_file(const char *filename,bool create);
void wow_shmcache_destroy(Shmcache_T* shmcache);


int wow_shmcache_set(Shmcache_T* shmcache,
        const char *key_data, const int key_len,
        const char *val_data, const int val_len, const int ttl);

int wow_shmcache_get(Shmcache_T* shmcache,
        const char *key_data, const int key_len,
        const char **val_data, int* val_len);

int wow_shmcache_delete(Shmcache_T* shmcache,const char *key_data, const int key_len);
int wow_shmcache_remove_all(Shmcache_T* shmcache);

int wow_shmcache_keyval_dump(Shmcache_T* shmcache);
int wow_shmcache_stats_dump(Shmcache_T* shmcache);

#endif