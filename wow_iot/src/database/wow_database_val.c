#include <stdlib.h>
#include <string.h>

#include "prefix/wow_check.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"
#include "prefix/wow_errno.h"
#include "utils/wow_type.h"

#include "database/wow_database_val.h"

size_t database_value_size(DbSqlValue_T const* value)
{
    CHECK_RET_VAL(value, 0);

    switch (value->type)
    {
    case DATABASE_SQL_VALUE_TYPE_TEXT:
        if (!value->u.text.hint && value->u.text.data)
        {
            ((DbSqlValue_T*)value)->u.text.hint = strlen(value->u.text.data);
        }
        return value->u.text.hint;
	case DATABASE_SQL_VALUE_TYPE_BLOB8:
		return value->u.blob.size;
    case DATABASE_SQL_VALUE_TYPE_INT32:
    case DATABASE_SQL_VALUE_TYPE_UINT32:
    case DATABASE_SQL_VALUE_TYPE_FLOAT:
        return 4;
    case DATABASE_SQL_VALUE_TYPE_INT64:
    case DATABASE_SQL_VALUE_TYPE_UINT64:
    case DATABASE_SQL_VALUE_TYPE_DOUBLE:
        return 8;
    case DATABASE_SQL_VALUE_TYPE_INT16:
    case DATABASE_SQL_VALUE_TYPE_UINT16:
        return 2;
    case DATABASE_SQL_VALUE_TYPE_INT8:
    case DATABASE_SQL_VALUE_TYPE_UINT8:
        return 1;
    default:
        break;
    }

    return 0;
}
int8_t database_value_int8(DbSqlValue_T const* value)
{
    return (int8_t)database_value_int32(value);
}
int16_t database_value_int16(DbSqlValue_T const* value)
{
    return (int16_t)database_value_int32(value);
}
int32_t database_value_int32(DbSqlValue_T const* value)
{
    CHECK_RET_VAL(value, 0);

    switch (value->type)
    {
    case DATABASE_SQL_VALUE_TYPE_INT32:
        return (int32_t)value->u.i32;
    case DATABASE_SQL_VALUE_TYPE_INT64:
        return (int32_t)value->u.i64;
    case DATABASE_SQL_VALUE_TYPE_INT16:
        return (int32_t)value->u.i16;
    case DATABASE_SQL_VALUE_TYPE_INT8:
        return (int32_t)value->u.i8;
    case DATABASE_SQL_VALUE_TYPE_UINT32:
        return (int32_t)value->u.u32;
    case DATABASE_SQL_VALUE_TYPE_UINT64:
        return (int32_t)value->u.u64;
    case DATABASE_SQL_VALUE_TYPE_UINT16:
        return (int32_t)value->u.u16;
    case DATABASE_SQL_VALUE_TYPE_UINT8:
        return (int32_t)value->u.u8;
    case DATABASE_SQL_VALUE_TYPE_FLOAT:
        return (int32_t)value->u.f;
    case DATABASE_SQL_VALUE_TYPE_DOUBLE:
        return (int32_t)value->u.d;
    case DATABASE_SQL_VALUE_TYPE_TEXT:
        if(value->u.text.data){
            uint64_t val = 0;
            str2u64(value->u.text.data,&val);
            return (int32_t)val;
        }else{
            return 0;
        }
    default:
        break;
    }

    return 0;
}
int64_t database_value_int64(DbSqlValue_T const* value)
{
    CHECK_RET_VAL(value, 0);

    switch (value->type)
    {
    case DATABASE_SQL_VALUE_TYPE_INT64:
        return (int64_t)value->u.i64;
    case DATABASE_SQL_VALUE_TYPE_INT32:
        return (int64_t)value->u.i32;
    case DATABASE_SQL_VALUE_TYPE_INT16:
        return (int64_t)value->u.i16;
    case DATABASE_SQL_VALUE_TYPE_INT8:
        return (int64_t)value->u.i8;
    case DATABASE_SQL_VALUE_TYPE_UINT64:
        return (int64_t)value->u.u64;
    case DATABASE_SQL_VALUE_TYPE_UINT32:
        return (int64_t)value->u.u32;
    case DATABASE_SQL_VALUE_TYPE_UINT16:
        return (int64_t)value->u.u16;
    case DATABASE_SQL_VALUE_TYPE_UINT8:
        return (int64_t)value->u.u8;
    case DATABASE_SQL_VALUE_TYPE_FLOAT:
        return (int64_t)value->u.f;
    case DATABASE_SQL_VALUE_TYPE_DOUBLE:
        return (int64_t)value->u.d;
    case DATABASE_SQL_VALUE_TYPE_TEXT:
        if(value->u.text.data){
            int64_t val = 0;
            str2u64(value->u.text.data,(uint64_t*)&val);
            return val;
        }else{
            return 0;
        }
    default:
        break;
    }

    return 0;
}
uint8_t database_value_uint8(DbSqlValue_T const* value)
{
    return (uint8_t)database_value_uint32(value);
}
uint16_t database_value_uint16(DbSqlValue_T const* value)
{
    return (uint16_t)database_value_uint32(value);
}
uint32_t database_value_uint32(DbSqlValue_T const* value)
{
    CHECK_RET_VAL(value, 0);

    switch (value->type)
    {
    case DATABASE_SQL_VALUE_TYPE_UINT32:
        return (uint32_t)value->u.u32;
    case DATABASE_SQL_VALUE_TYPE_UINT64:
        return (uint32_t)value->u.u64;
    case DATABASE_SQL_VALUE_TYPE_UINT16:
        return (uint32_t)value->u.u16;
    case DATABASE_SQL_VALUE_TYPE_UINT8:
        return (uint32_t)value->u.u8;
    case DATABASE_SQL_VALUE_TYPE_INT32:
        return (uint32_t)value->u.i32;
    case DATABASE_SQL_VALUE_TYPE_INT64:
        return (uint32_t)value->u.i64;
    case DATABASE_SQL_VALUE_TYPE_INT16:
        return (uint32_t)value->u.i16;
    case DATABASE_SQL_VALUE_TYPE_INT8:
        return (uint32_t)value->u.i8;
    case DATABASE_SQL_VALUE_TYPE_FLOAT:
        return (uint32_t)value->u.f;
    case DATABASE_SQL_VALUE_TYPE_DOUBLE:
        return (uint32_t)value->u.d;
    case DATABASE_SQL_VALUE_TYPE_TEXT:
        if(value->u.text.data){
            uint64_t val = 0;
            str2u64(value->u.text.data,&val);
            return (uint32_t)val;
        }else{
            return 0;
        }
    default:
        break;
    }

    return 0;
}
uint64_t database_value_uint64(DbSqlValue_T const* value)
{
    CHECK_RET_VAL(value, 0);

    switch (value->type)
    {
    case DATABASE_SQL_VALUE_TYPE_UINT64:
        return (uint64_t)value->u.u64;
    case DATABASE_SQL_VALUE_TYPE_UINT32:
        return (uint64_t)value->u.u32;
    case DATABASE_SQL_VALUE_TYPE_UINT16:
        return (uint64_t)value->u.u16;
    case DATABASE_SQL_VALUE_TYPE_UINT8:
        return (uint64_t)value->u.u8;
    case DATABASE_SQL_VALUE_TYPE_INT64:
        return (uint64_t)value->u.i64;
    case DATABASE_SQL_VALUE_TYPE_INT32:
        return (uint64_t)value->u.i32;
    case DATABASE_SQL_VALUE_TYPE_INT16:
        return (uint64_t)value->u.i16;
    case DATABASE_SQL_VALUE_TYPE_INT8:
        return (uint64_t)value->u.i8;
    case DATABASE_SQL_VALUE_TYPE_FLOAT:
        return (uint64_t)value->u.f;
    case DATABASE_SQL_VALUE_TYPE_DOUBLE:
        return (uint64_t)value->u.d;
    case DATABASE_SQL_VALUE_TYPE_TEXT:
        if(value->u.text.data){
            uint64_t val = 0;
            str2u64(value->u.text.data,&val);
            return val;
        }else{
            return 0;
        }
    default:
        break;
    }

    return 0;
}

float database_value_float(DbSqlValue_T const* value)
{
    CHECK_RET_VAL(value, 0);

    switch (value->type)
    {
    case DATABASE_SQL_VALUE_TYPE_FLOAT:
        return value->u.f;
    case DATABASE_SQL_VALUE_TYPE_DOUBLE:
        return (float)value->u.d;
    case DATABASE_SQL_VALUE_TYPE_INT64:
        return (float)value->u.i64;
    case DATABASE_SQL_VALUE_TYPE_INT32:
        return (float)value->u.i32;
    case DATABASE_SQL_VALUE_TYPE_INT16:
        return (float)value->u.i16;
    case DATABASE_SQL_VALUE_TYPE_INT8:
        return (float)value->u.i8;
    case DATABASE_SQL_VALUE_TYPE_UINT64:
        return (float)value->u.u64;
    case DATABASE_SQL_VALUE_TYPE_UINT32:
        return (float)value->u.u32;
    case DATABASE_SQL_VALUE_TYPE_UINT16:
        return (float)value->u.u16;
    case DATABASE_SQL_VALUE_TYPE_UINT8:
        return (float)value->u.u8;
    case DATABASE_SQL_VALUE_TYPE_TEXT:
        if(value->u.text.data){
            double val = 0;
            str2double(value->u.text.data,&val);
            return (float)val;
        }else{
            return 0;
        }
    default:
        break;
    }

    return 0;
}
double database_value_double(DbSqlValue_T const* value)
{
    CHECK_RET_VAL(value, 0);

    switch (value->type)
    {
    case DATABASE_SQL_VALUE_TYPE_FLOAT:
        return (double)value->u.f;
    case DATABASE_SQL_VALUE_TYPE_DOUBLE:
        return value->u.d;
    case DATABASE_SQL_VALUE_TYPE_INT64:
        return (double)value->u.i64;
    case DATABASE_SQL_VALUE_TYPE_INT32:
        return (double)value->u.i32;
    case DATABASE_SQL_VALUE_TYPE_INT16:
        return (double)value->u.i16;
    case DATABASE_SQL_VALUE_TYPE_INT8:
        return (double)value->u.i8;
    case DATABASE_SQL_VALUE_TYPE_UINT64:
        return (double)value->u.u64;
    case DATABASE_SQL_VALUE_TYPE_UINT32:
        return (double)value->u.u32;
    case DATABASE_SQL_VALUE_TYPE_UINT16:
        return (double)value->u.u16;
    case DATABASE_SQL_VALUE_TYPE_UINT8:
        return (double)value->u.u8;
    case DATABASE_SQL_VALUE_TYPE_TEXT:
        if(value->u.text.data){
            double val = 0;
            str2double(value->u.text.data,&val);
            return val;
        }else{
            return 0;
        }
    default:
        break;
    }

    return 0;
}

void database_value_set_null(DbSqlValue_T* value)
{
    CHECK_RET_VOID(value);

    value->type         = DATABASE_SQL_VALUE_TYPE_NULL;
}
void database_value_set_int8(DbSqlValue_T* value, int8_t number)
{
    CHECK_RET_VOID(value);

    value->type         = DATABASE_SQL_VALUE_TYPE_INT8;
    value->u.i8         = number;
}
void database_value_set_int16(DbSqlValue_T* value, int16_t number)
{
    CHECK_RET_VOID(value);

    value->type         = DATABASE_SQL_VALUE_TYPE_INT16;
    value->u.i16        = number;
}
void database_value_set_int32(DbSqlValue_T* value, int32_t number)
{
    CHECK_RET_VOID(value);

    value->type         = DATABASE_SQL_VALUE_TYPE_INT32;
    value->u.i32        = number;
}
void database_value_set_int64(DbSqlValue_T* value, int64_t number)
{   
    CHECK_RET_VOID(value);

    value->type         = DATABASE_SQL_VALUE_TYPE_INT64;
    value->u.i64            = number;
}
void database_value_set_uint8(DbSqlValue_T* value, uint8_t number)
{
    CHECK_RET_VOID(value);

    value->type         = DATABASE_SQL_VALUE_TYPE_INT8;
    value->u.u8         = number;
}
void database_value_set_uint16(DbSqlValue_T* value, uint16_t number)
{
    CHECK_RET_VOID(value);

    value->type         = DATABASE_SQL_VALUE_TYPE_INT16;
    value->u.u16        = number;
}
void database_value_set_uint32(DbSqlValue_T* value, uint32_t number)
{
    CHECK_RET_VOID(value);

    value->type         = DATABASE_SQL_VALUE_TYPE_INT32;
    value->u.u32        = number;
}
void database_value_set_uint64(DbSqlValue_T* value, uint64_t number)
{   
    CHECK_RET_VOID(value);

    value->type         = DATABASE_SQL_VALUE_TYPE_INT64;
    value->u.u64        = number;
}

void database_value_set_float(DbSqlValue_T* value, float number)
{
    CHECK_RET_VOID(value);

    value->type         = DATABASE_SQL_VALUE_TYPE_FLOAT;
    value->u.f          = number;
}
void database_value_set_double(DbSqlValue_T* value, double number)
{   
    CHECK_RET_VOID(value);

    value->type         = DATABASE_SQL_VALUE_TYPE_DOUBLE;
    value->u.d          = number;
}

void database_value_set_text(DbSqlValue_T* value, char const* text, size_t hint)
{
    CHECK_RET_VOID(value);

    value->type         = DATABASE_SQL_VALUE_TYPE_TEXT;
    value->u.text.data  = text;
    value->u.text.hint  = hint;
}

void database_value_set_blob8(DbSqlValue_T* value, uint8_t const* data, size_t size)
{
    CHECK_RET_VOID(value);

    value->type             = DATABASE_SQL_VALUE_TYPE_BLOB8;
    value->u.blob.data      = data;
    value->u.blob.size      = size;
}

