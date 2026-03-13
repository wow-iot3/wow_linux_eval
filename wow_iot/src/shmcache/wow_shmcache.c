#include "prefix/wow_check.h"
#include "prefix/wow_common.h"
#include "prefix/wow_errno.h"
#include "system/wow_time.h"

#include "shmcache/wow_shmcache.h"

#include "shmcache/shmcache.h"


__EX_API__ Shmcache_T* wow_shmcache_init_from_file(const char *filename,bool create)
{
    CHECK_RET_VAL_P(filename,NULL,"paran input data invalid!\n");   

    int ret = 0;
    Shmcache_T* shmcache = CALLOC(1,Shmcache_T);
    CHECK_RET_VAL_P(shmcache,NULL,"malloc Shmcache_T failed!\n");   

    ret = shmcache_init_from_file_ex(shmcache, filename,create,true);
    CHECK_RET_GOTO_P(ret == 0,out,"shmcache_init_from_file failed!\n");

    return shmcache;
out:
    FREE(shmcache);
    return NULL;
}

__EX_API__ void wow_shmcache_destroy(Shmcache_T* shmcache)
{
    CHECK_RET_VOID(shmcache);

    shmcache_destroy(shmcache);

    FREE(shmcache);
}


__EX_API__ int wow_shmcache_set(Shmcache_T* shmcache,
        const char *key_data, const int key_len,
        const char *val_data, const int val_len, const int ttl)
{
    CHECK_RET_VAL_P(shmcache,-PARAM_INPUT_STRUCT_IS_NULL,"paran input strcut invalid!\n");   
    CHECK_RET_VAL_P(key_data && val_data,-PARAM_INPUT_DATA_IS_NULL,"paran input data invalid!\n");   
    CHECK_RET_VAL_P(key_len>0 && val_len>0,-PARAM_INPUT_SIZE_IS_ZERO,"paran input size invalid!\n");   

    struct shmcache_key_info key;
    struct shmcache_value_info value;

    key.data   = (char *)key_data;
    key.length = key_len;
   
    value.options = SHMCACHE_SERIALIZER_STRING;
    value.data    = (char *)val_data;
    value.length  = val_len;
    value.expires = HT_CALC_EXPIRES(get_current_time(), ttl);

    return shmcache_set_ex(shmcache,&key,&value);
}

__EX_API__ int wow_shmcache_get(Shmcache_T* shmcache,
        const char *key_data, const int key_len,
        const char **val_data, int* val_len)
{
    CHECK_RET_VAL_P(shmcache,-PARAM_INPUT_STRUCT_IS_NULL,"paran input strcut invalid!\n");   
    CHECK_RET_VAL_P(key_data && val_data && val_len,-PARAM_INPUT_DATA_IS_NULL,"paran input data invalid!\n");   
    CHECK_RET_VAL_P(key_len>0,-PARAM_INPUT_SIZE_IS_ZERO,"paran input size invalid!\n");   

    int ret = 0;
    struct shmcache_key_info key;
    struct shmcache_value_info value;

    key.data   = (char *)key_data;
    key.length = key_len;

    ret = shmcache_get(shmcache,&key,&value);
    CHECK_RET_VAL_P(ret == 0,ret,"shmcache_get failed!\n");

    *val_len  = value.length;
    *val_data = value.data;

    return 0;
}

__EX_API__ int wow_shmcache_delete(Shmcache_T* shmcache,const char *key_data, const int key_len)
{
    CHECK_RET_VAL_P(shmcache,-PARAM_INPUT_STRUCT_IS_NULL,"paran input strcut invalid!\n");   
    CHECK_RET_VAL_P(key_data ,-PARAM_INPUT_DATA_IS_NULL,"paran input data invalid!\n");   
    CHECK_RET_VAL_P(key_len>0,-PARAM_INPUT_SIZE_IS_ZERO,"paran input size invalid!\n");   

    struct shmcache_key_info key;

    key.data   = (char *)key_data;
    key.length = key_len;

    return shmcache_delete(shmcache,&key);
}

__EX_API__ int wow_shmcache_remove_all(Shmcache_T* shmcache)
{
    CHECK_RET_VAL_P(shmcache,-PARAM_INPUT_STRUCT_IS_NULL,"paran input strcut invalid!\n"); 

    return shmcache_remove_all(shmcache);
}

static char *key_value_seperator = "=";
static char *row_seperator = "\n";
static char *search_key  = NULL;

static struct shmcache_match_key_info *parse_search_key(
        struct shmcache_match_key_info *key_info)
{
    if (search_key == NULL || *search_key == '\0') {
        return NULL;
    }

    key_info->op_type = 0;
    key_info->key = search_key;
    key_info->length = strlen(search_key);
    if (*search_key == '%') {
        key_info->op_type |= SHMCACHE_MATCH_KEY_OP_RIGHT;
        key_info->key++;
        key_info->length--;
        if (key_info->length == 0) {
            return NULL;
        }
    }

    if (key_info->key[key_info->length - 1] == '%') {
        key_info->op_type |= SHMCACHE_MATCH_KEY_OP_LEFT;
        key_info->length--;
        if (key_info->length == 0) {
            return NULL;
        }
    }

    return key_info;
}


__EX_API__ int wow_shmcache_keyval_dump(Shmcache_T* shmcache)
{
    CHECK_RET_VAL_P(shmcache,-PARAM_INPUT_STRUCT_IS_NULL,"paran input strcut invalid!\n"); 

    int ret = 0;
    struct shmcache_hentry_array array;
    struct shmcache_hash_entry *entry;
    struct shmcache_hash_entry *end;
    struct shmcache_match_key_info key_info;
    struct shmcache_match_key_info *pkey;
    
    pkey = parse_search_key(&key_info);

    ret = shm_ht_to_array_ex(shmcache, &array, pkey,0, 0);
    CHECK_RET_VAL_P(ret == 0,-1,"shm_ht_to_array_ex failed!\n");

    end = array.entries + array.count;
    for (entry=array.entries; entry<end; entry++) {
        if ((entry->value.options & SHMCACHE_SERIALIZER_STRING) != 0) {
            printf("%.*s%s%.*s%s",
                    entry->key.length, entry->key.data,
                    key_value_seperator,
                    entry->value.length, entry->value.data,
                    row_seperator);
        } else {
            printf("%.*s%s<%s serializer, data length: %d>%s",
                    entry->key.length, entry->key.data,
                    key_value_seperator,
                    shmcache_get_serializer_label(entry->value.options),
                    entry->value.length, row_seperator);
        }
    }

    shm_ht_free_array(&array);
    return 0;
}


__EX_API__ int wow_shmcache_stats_dump(Shmcache_T* context)
{
     CHECK_RET_VAL_P(context,-PARAM_INPUT_STRUCT_IS_NULL,"paran input strcut invalid!\n"); 

    struct shmcache_stats stats;
    int avg_key_len;
    int avg_value_len;
    char total_ratio[32];
    char ratio[32];
    char rw_ratio[32];
    char time_buff[32];

    int64_t g_current_time = wow_time_get_sec();


    shmcache_stats(context, &stats);
    if (stats.hashtable.count > 0) {
        avg_key_len = stats.memory.usage.used.key / stats.hashtable.count;
        avg_value_len = stats.memory.usage.used.value / stats.hashtable.count;
    } else {
        avg_key_len = 0;
        avg_value_len = 0;
    }
    if (stats.shm.hashtable.get.total > 0) {
        sprintf(total_ratio, "%.2f%%", 100.00 * stats.shm.hashtable.get.success
                / (double)stats.shm.hashtable.get.total);
    } else {
        total_ratio[0] = '-';
        total_ratio[1] = '\0';
    }
    if (stats.hit.ratio >= 0.00) {
        sprintf(ratio, "%.2f%%", stats.hit.ratio * 100.00);
    } else {
        ratio[0] = '-';
        ratio[1] = '\0';
    }
    if (stats.shm.hashtable.set.total > 0) {
        sprintf(rw_ratio, "%.2f / 1.00", stats.shm.hashtable.get.total
                / (double)stats.shm.hashtable.set.total);
    } else {
        rw_ratio[0] = '-';
        rw_ratio[1] = '\0';
    }

    printf("\ntimestamp info:\n");

    memset(time_buff,0,sizeof(time_buff));
    wow_time_sec_to_stamp(context->memory->init_time,time_buff);
    printf("shm init time: %s\n", time_buff);

    if (context->memory->stats.hashtable.last_clear_time > 0) {
        memset(time_buff,0,sizeof(time_buff));
        wow_time_sec_to_stamp(context->memory->stats.hashtable.last_clear_time,time_buff);
        printf("last clear time: %s\n", time_buff);
    }

    memset(time_buff,0,sizeof(time_buff));
    wow_time_sec_to_stamp(context->memory->stats.init_time,time_buff);
    printf("stats begin time: %s\n", time_buff);

    if (context->memory->stats.memory.recycle.key.last_recycle_time > 0) {
        memset(time_buff,0,sizeof(time_buff));
        wow_time_sec_to_stamp(context->memory->stats.memory.recycle.key.last_recycle_time,time_buff);
        printf("last recycle by key time: %s\n",time_buff);
    }
    if (context->memory->stats.memory.recycle.value_striping.last_recycle_time > 0) {
        memset(time_buff,0,sizeof(time_buff));
        wow_time_sec_to_stamp(context->memory->stats.memory.recycle.value_striping.last_recycle_time,time_buff);
        printf("last recycle by value time: %s\n",time_buff);
    }
    if (context->memory->stats.lock.last_detect_deadlock_time > 0) {
        memset(time_buff,0,sizeof(time_buff));
        wow_time_sec_to_stamp(context->memory->stats.lock.last_detect_deadlock_time,time_buff);
        printf("last detect deadlock time: %s\n",time_buff);
    }
    if (context->memory->stats.lock.last_unlock_deadlock_time > 0) {
        memset(time_buff,0,sizeof(time_buff));
        wow_time_sec_to_stamp(context->memory->stats.lock.last_unlock_deadlock_time,time_buff);
        printf("last unlock deadlock time: %s\n", time_buff);
    }
    printf("\n");

    printf("\nhash table stats:\n");
    printf("max_key_count: %d\n"
            "current_key_count: %d\n"
            "segment_size: %.03f MB\n\n"
            "set.total_count: %"PRId64"\n"
            "set.success_count: %"PRId64"\n"
            "incr.total_count: %"PRId64"\n"
            "incr.success_count: %"PRId64"\n"
            "get.total_count: %"PRId64"\n"
            "get.success_count: %"PRId64"\n"
            "del.total_count: %"PRId64"\n"
            "del.success_count: %"PRId64"\n"
            "get.qps: %.2f\n"
            "hit ratio (last %d seconds): %s\n"
            "total hit ratio (last %d seconds): %s\n"
            "total RW ratio: %s\n\n",
            stats.max_key_count,
            stats.hashtable.count,
            (double)stats.hashtable.segment_size / (1024 * 1024),
            stats.shm.hashtable.set.total,
            stats.shm.hashtable.set.success,
            stats.shm.hashtable.incr.total,
            stats.shm.hashtable.incr.success,
            stats.shm.hashtable.get.total,
            stats.shm.hashtable.get.success,
            stats.shm.hashtable.del.total,
            stats.shm.hashtable.del.success,
            stats.hit.get_qps, stats.hit.seconds, ratio,
            (int)(g_current_time - context->memory->stats.init_time),
            total_ratio, rw_ratio);

    printf("\nmemory stats:\n");
    printf("total: %.03f MB\n"
            "alloced: %.03f MB\n"
            "used: %.03f MB\n"
            "free: %.03f MB\n"
            "avg_key_len: %d\n"
            "avg_value_len: %d\n\n",
            (double)stats.memory.max / (1024 * 1024),
            (double)stats.memory.usage.alloced / (1024 * 1024),
            (double)stats.memory.used / (1024 * 1024),
            (double)(stats.memory.max - stats.memory.used) /
            (1024 * 1024), avg_key_len, avg_value_len);

    printf("\nmemory recycle stats:\n");
    printf("clear_ht_entry.total_count: %"PRId64"\n"
            "clear_ht_entry.valid_count: %"PRId64"\n\n"
            "recycle.key.total_count: %"PRId64"\n"
            "recycle.key.success_count: %"PRId64"\n"
            "recycle.key.force_count: %"PRId64"\n\n"
            "recycle.value_striping.total_count: %"PRId64"\n"
            "recycle.value_striping.success_count: %"PRId64"\n"
            "recycle.value_striping.force_count: %"PRId64"\n\n",
            stats.shm.memory.clear_ht_entry.total,
            stats.shm.memory.clear_ht_entry.valid,
            stats.shm.memory.recycle.key.total,
            stats.shm.memory.recycle.key.success,
            stats.shm.memory.recycle.key.force,
            stats.shm.memory.recycle.value_striping.total,
            stats.shm.memory.recycle.value_striping.success,
            stats.shm.memory.recycle.value_striping.force);

    printf("\nlock stats:\n");
    printf("total_count: %"PRId64"\n"
            "retry_count: %"PRId64"\n"
            "detect_deadlock: %"PRId64"\n"
            "unlock_deadlock: %"PRId64"\n\n",
            stats.shm.lock.total,
            stats.shm.lock.retry,
            stats.shm.lock.detect_deadlock,
            stats.shm.lock.unlock_deadlock);

    return 0;
}