#ifndef __WOW_IOT_DATABASE_WOW_DATABASE_VALUE_H_
#define __WOW_IOT_DATABASE_WOW_DATABASE_VALUE_H_

#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

#include "prefix/wow_check.h"
#include "prefix/wow_keyword.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum{
	DATABASE_SQL_VALUE_TYPE_NULL         = 0,
	DATABASE_SQL_VALUE_TYPE_INT8         = 1,
	DATABASE_SQL_VALUE_TYPE_INT16        = 2,
	DATABASE_SQL_VALUE_TYPE_INT32        = 3,
	DATABASE_SQL_VALUE_TYPE_INT64        = 4,
	DATABASE_SQL_VALUE_TYPE_UINT8        = 5,
	DATABASE_SQL_VALUE_TYPE_UINT16       = 6,
	DATABASE_SQL_VALUE_TYPE_UINT32       = 7,
	DATABASE_SQL_VALUE_TYPE_UINT64       = 8,
	DATABASE_SQL_VALUE_TYPE_FLOAT		 = 9,
	DATABASE_SQL_VALUE_TYPE_DOUBLE		 = 10,
	DATABASE_SQL_VALUE_TYPE_TEXT         = 11,
	DATABASE_SQL_VALUE_TYPE_BLOB8        = 12
}DatabaseSqlValueType_E;

typedef struct{
    size_t      type;	
    char const* name;
    union 
    {
        int8_t           i8;
        int16_t          i16;
        int32_t          i32;
        int64_t          i64;

        uint8_t            u8;
        uint16_t           u16;
        uint32_t           u32;
        uint64_t           u64;
        float              f;
        double             d;

        struct {
            char const*    data;
            size_t         hint;
        }text;

		struct{
            uint8_t const*    data;
            size_t           size;
        }blob;
    }u; 

}DbSqlValue_T;

/*设置元素value值的数据长度*/
size_t   database_value_size(DbSqlValue_T const* value);

/*获取元素value值*/
int8_t  database_value_int8(DbSqlValue_T const* value);
int16_t database_value_int16(DbSqlValue_T const* value);
int32_t database_value_int32(DbSqlValue_T const* value);
int64_t database_value_int64(DbSqlValue_T const* value);
uint8_t  database_value_uint8(DbSqlValue_T const* value);
uint16_t database_value_uint16(DbSqlValue_T const* value);
uint32_t database_value_uint32(DbSqlValue_T const* value);
uint64_t database_value_uint64(DbSqlValue_T const* value);
float    database_value_float(DbSqlValue_T const* value);
double   database_value_double(DbSqlValue_T const* value);


/*设置元素value值*/
void database_value_set_null(DbSqlValue_T* value);
void database_value_set_int8(DbSqlValue_T* value, int8_t number);
void database_value_set_int16(DbSqlValue_T* value, int16_t number);
void database_value_set_int32(DbSqlValue_T* value, int32_t number);
void database_value_set_int64(DbSqlValue_T* value, int64_t number);
void database_value_set_uint8(DbSqlValue_T* value, uint8_t number);
void database_value_set_uint16(DbSqlValue_T* value, uint16_t number);
void database_value_set_uint32(DbSqlValue_T* value, uint32_t number);
void database_value_set_uint64(DbSqlValue_T* value, uint64_t number);
void database_value_set_float(DbSqlValue_T* value, float number);
void database_value_set_double(DbSqlValue_T* value, double number);
void database_value_set_text(DbSqlValue_T* value, char const* text, size_t hint);
void database_value_set_blob8(DbSqlValue_T* value, uint8_t const* data, size_t size);

/* //////////////////////////////////////////////////////////////////////////////////////
 * inlines
 */
static __inline_force__ bool database_value_is_null(DbSqlValue_T const* value)
{
    return (value && value->type == DATABASE_SQL_VALUE_TYPE_NULL)? true : false;
}

static __inline_force__ bool database_value_is_text(DbSqlValue_T const* value)
{
    return (value && value->type == DATABASE_SQL_VALUE_TYPE_TEXT)? true : false;
}

static __inline_force__ bool database_value_is_integer(DbSqlValue_T const* value)
{
    return (value && value->type >= DATABASE_SQL_VALUE_TYPE_INT8 && value->type <= DATABASE_SQL_VALUE_TYPE_UINT64)? true : false;
}

static __inline_force__ bool database_value_is_float(DbSqlValue_T const* value)
{
    return (  value 
            &&  (   value->type == DATABASE_SQL_VALUE_TYPE_FLOAT
                ||  value->type == DATABASE_SQL_VALUE_TYPE_DOUBLE))? true : false;
}

static __inline_force__ bool database_value_is_number(DbSqlValue_T const* value)
{
    return (value && value->type >= DATABASE_SQL_VALUE_TYPE_INT8)? true : false;
}
static __inline_force__ bool database_value_is_blob(DbSqlValue_T const* value)
{
    return (value && (value->type == DATABASE_SQL_VALUE_TYPE_BLOB8))? true : false;
}


static __inline_force__ size_t database_value_type(DbSqlValue_T const* value)
{
    CHECK_RET_VAL(value, DATABASE_SQL_VALUE_TYPE_NULL);

    return value->type;
}

static __inline_force__ char const* database_value_name(DbSqlValue_T const* value)
{
    CHECK_RET_VAL(value, NULL);

    return value->name;
}


static __inline_force__ char const* database_value_text(DbSqlValue_T const* value)
{
    CHECK_RET_VAL(value, NULL);

    if (database_value_is_text(value)) {
		return value->u.text.data;
	}
    else if (database_value_is_null(value)){	
		return NULL;
	}
       
    return NULL;
}

static __inline_force__ uint8_t const* database_value_blob(DbSqlValue_T const* value)
{
    CHECK_RET_VAL(value, NULL);

    if (database_value_is_blob(value)) {
		return value->u.blob.data;
	}
    else if (database_value_is_null(value)){	
		return NULL;
	}
       
    return NULL;
}

static __inline_force__ size_t database_value_blob_size(DbSqlValue_T const* value)
{
    CHECK_RET_VAL(value, 0);

    if (database_value_is_blob(value)) {
		return value->u.blob.size;
	}
    else if (database_value_is_null(value)){	
		return 0;
	}
       
    return 0;
}

static __inline_force__ void database_value_name_set(DbSqlValue_T* value, char const* name)
{
    CHECK_RET_VOID(value);

    value->name = name;
}

#ifdef __cplusplus
}
#endif

#endif
