#include <string.h>
#include <stdlib.h>

#include "sqlite3.h"

#include "prefix/wow_check.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"
#include "prefix/wow_errno.h"

#include "utils/wow_singleton.h"
#include "element/wow_element.h"
#include "datastruct/wow_iterator.h"

#include "wow_iot_internal.h"
#include "wow_iot_errno.h"
#include "../wow_database_internal.h"


typedef struct{
    Iterator_T               itor;
    int                      row;
    int                      count;
    DbSqlValue_T             value;
}DbSqlite3ResultRow_T;


typedef struct{
    Iterator_T              itor;
    char**                  result;
    sqlite3_stmt*           statement;
    int                     count;
	int 					rid;
    DbSqlite3ResultRow_T    row;
}DbSqlite3Result_T;

typedef struct{
    DbSqlImpl_T  		base;
    sqlite3*    		database;
    DbSqlite3Result_T   result;
}DbSqlite3_T;



/***************************************内部接口*************************************************/
static int db_sqlite3_library_init(void)
{
    int ret = sqlite3_initialize();
	CHECK_RET_VAL_P(ret == SQLITE_OK, -SQLITE_INIT_ERROR,"sqlite3_initialize failed!\n");

    return 0;
}

static void db_sqlite3_library_exit(void)
{
    sqlite3_shutdown();
}

__IN_API__ int _wow_sqlite3_library_init(void)
{
    return wow_singleton_instance(SINGLETON_IOT_MOD_SQLITE, db_sqlite3_library_init, db_sqlite3_library_exit);
}



static int db_sqlite3_result_row_iterator_size(Iterator_PT iterator)
{
    DbSqlite3Result_T* result = (DbSqlite3Result_T*)iterator;
    CHECK_RET_VAL(result, -1);
    
    return result->count;
}

static int db_sqlite3_result_row_iterator_head(Iterator_PT iterator)
{
    return 0;
}

static int db_sqlite3_result_row_iterator_tail(Iterator_PT iterator)
{
    DbSqlite3Result_T* result = (DbSqlite3Result_T*)iterator;
    CHECK_RET_VAL(result, -1);

    return result->count;
}

static int db_sqlite3_result_row_iterator_prev(Iterator_PT iterator, size_t itor)
{
    DbSqlite3Result_T* result = (DbSqlite3Result_T*)iterator;
	
    CHECK_RET_VAL(result && result->rid >= 0 && result->rid <= result->count, -1);

    return result->rid--;
}
static int db_sqlite3_result_row_iterator_next(Iterator_PT iterator, size_t itor)
{
    DbSqlite3Result_T* result = (DbSqlite3Result_T*)iterator;
    CHECK_RET_VAL(result, -1);

   	if(result->result){
		 CHECK_RET_VAL(result->rid < result->count && result->rid >= -1, -1);
		 return result->rid++;	
	}
	
    if(result->statement){
	   if(result->rid == 0){
	   		result->rid++;
			return 0;
	   }
	   int ret = sqlite3_step(result->statement);
	   CHECK_RET_VAL(ret == SQLITE_DONE || ret == SQLITE_ROW,-1);
	   if (ret == SQLITE_ROW){
		   return 0;
	   }else {
		   sqlite3_reset(result->statement);
	   }
    }

    return -1;
}
static void* db_sqlite3_result_row_iterator_item(Iterator_PT iterator, size_t itor)
{
    DbSqlite3Result_T* result = (DbSqlite3Result_T*)iterator;
    CHECK_RET_VAL(result && (result->result || result->statement) && itor < result->count, NULL);

    // save the row
    result->row.row = itor;

    // the row iterator
    return (void*)&result->row;
}
static int db_sqlite3_result_col_iterator_size(Iterator_PT iterator)
{
    DbSqlite3ResultRow_T* row = (DbSqlite3ResultRow_T*)iterator;
    CHECK_RET_VAL(row,  -1);

    return row->count;
}
static int db_sqlite3_result_col_iterator_head(Iterator_PT iterator)
{
    DbSqlite3ResultRow_T* row = (DbSqlite3ResultRow_T*)iterator;
    CHECK_RET_VAL(row,  -1);

    return 0;
}
static int db_sqlite3_result_col_iterator_tail(Iterator_PT iterator)
{    DbSqlite3ResultRow_T* row = (DbSqlite3ResultRow_T*)iterator;
    CHECK_RET_VAL(row,  -1);

    return row->count;
}
static int db_sqlite3_result_col_iterator_prev(Iterator_PT iterator, size_t itor)
{
    DbSqlite3ResultRow_T* row = (DbSqlite3ResultRow_T*)iterator;
    CHECK_RET_VAL(row && itor && itor <= row->count, -1);

    return itor - 1;
}
static int db_sqlite3_result_col_iterator_next(Iterator_PT iterator, size_t itor)
{
    DbSqlite3ResultRow_T* row = (DbSqlite3ResultRow_T*)iterator;
    CHECK_RET_VAL(row && itor < row->count, row->count);

    return itor + 1;
}
static void* db_sqlite3_result_col_iterator_item(Iterator_PT iterator, size_t itor)
{
    DbSqlite3ResultRow_T* row = (DbSqlite3ResultRow_T*)iterator;
    CHECK_RET_VAL(row && itor < row->count, NULL);
	
    DbSqlite3_T* sqlite = (DbSqlite3_T*)iterator->priv;
    CHECK_RET_VAL(sqlite, NULL);

    if (sqlite->result.result)
    {
        database_value_name_set(&row->value, (char const*)sqlite->result.result[itor]);
        database_value_set_text(&row->value, (char const*)sqlite->result.result[((1 + sqlite->result.row.row) * row->count) + itor], 0);
        return (void*)&row->value;
    }


	if (sqlite->result.statement)
    {
        database_value_name_set(&row->value, sqlite3_column_name(sqlite->result.statement, (int)itor));

        size_t type = sqlite3_column_type(sqlite->result.statement, (int)itor);
        switch (type)
        {
        case SQLITE_INTEGER:
            database_value_set_int64(&row->value, sqlite3_column_int64(sqlite->result.statement, (int)itor));
            break;
        case SQLITE_TEXT:
            database_value_set_text(&row->value, (char const*)sqlite3_column_text(sqlite->result.statement, (int)itor), 
										sqlite3_column_bytes(sqlite->result.statement, (int)itor));
            break;
        case SQLITE_FLOAT:
            database_value_set_double(&row->value, sqlite3_column_double(sqlite->result.statement, (int)itor));
            break;
        case SQLITE_BLOB:
			database_value_set_blob8(&row->value, (uint8_t const*)sqlite3_column_blob(sqlite->result.statement, (int)itor), 
										sqlite3_column_bytes(sqlite->result.statement, (int)itor));
            break;
        case SQLITE_NULL:
            database_value_set_null(&row->value);
            break;
        default:
            return NULL;
        }
        return (void*)&row->value;
    }

    return NULL;
}


static int db_sqlite3_open(struct DbSqlImpl* database)
{
	int ret = SQLITE_OK;

    DbSqlite3_T* sqlite = (DbSqlite3_T*)database;
    CHECK_RET_VAL_P(sqlite, -PARAM_INPUT_STRUCT_IS_NULL ,"param input struct invalid!\n");
	CHECK_RET_VAL_P(sqlite->database == NULL, 0,"sqlite database is inited!\n");
	
	ret = sqlite3_open_v2(sqlite->base.dbinfo.file, &sqlite->database, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	CHECK_RET_VAL_P(ret == SQLITE_OK&& sqlite->database, -SQLITE_OPEN_ERROR, 
					"sqlite3_open_v2 file:%s failed, error[%d]: %s\n", sqlite->base.dbinfo.file, 
						sqlite3_errcode(sqlite->database), sqlite3_errmsg(sqlite->database));

    // 设置PRAGMA指令以提高性能----关闭同步
    ret = sqlite3_exec(sqlite->database, "PRAGMA synchronous = OFF;", NULL, NULL, NULL);
    CHECK_RET_VAL_P(ret == SQLITE_OK, -SQLITE_PRAGMA_ERROR, "sqlite3_exec PRAGMA synchronousfailed, error[%d]: %s\n", 
                                            sqlite3_errcode(sqlite->database),sqlite3_errmsg(sqlite->database));

    // 设置PRAGMA指令以提高性能---关闭日志 内存不足而失败时 使用journal_mode = WAL 或 journal_mode = DELETE
    ret = sqlite3_exec(sqlite->database, "PRAGMA journal_mode = MEMORY;", NULL, NULL,NULL);
    CHECK_RET_VAL_P(ret == SQLITE_OK, -SQLITE_PRAGMA_ERROR, "sqlite3_exec PRAGMA journal_mode failed, error[%d]: %s\n", 
                                            sqlite3_errcode(sqlite->database),sqlite3_errmsg(sqlite->database));

    return 0;
}

static void db_sqlite3_close(struct DbSqlImpl* database)
{
    DbSqlite3_T* sqlite = (DbSqlite3_T*)database;
    CHECK_RET_VOID(sqlite);
    
    // exit result first if exists
    if (sqlite->result.result) {
		sqlite3_free_table(sqlite->result.result);
	}
    sqlite->result.result = NULL;

    // close database
    if (sqlite->database){
		sqlite3_close(sqlite->database);
	} 
    sqlite->database = NULL;
}

static void db_sqlite3_exit(struct DbSqlImpl* database)
{
    DbSqlite3_T* sqlite = (DbSqlite3_T*)database;
    CHECK_RET_VOID(sqlite);

    db_sqlite3_close(database);

    FREE(sqlite);
}

static void db_sqlite3_result_exit(DbSqlite3Result_T* sqlite3_result)
{
    if (sqlite3_result->result) {
		sqlite3_free_table(sqlite3_result->result);
	}
    sqlite3_result->result = NULL;

    // clear the statement
    sqlite3_result->statement = NULL;

    // clear result
    sqlite3_result->rid = 0;
    sqlite3_result->count = 0;
    sqlite3_result->row.count = 0;
}

static Iterator_PT db_sqlite3_result_load(struct DbSqlImpl* database)
{
    DbSqlite3_T* sqlite = (DbSqlite3_T*)database;
    CHECK_RET_VAL(sqlite && sqlite->database, NULL);
	
    return (sqlite->result.result || sqlite->result.statement)? (Iterator_PT)&sqlite->result : NULL;
}

static int db_sqlite3_exec(struct DbSqlImpl* database, char const* sql)
{
    int ret = SQLITE_OK;
    int row_count = 0;
    int col_count = 0;
    char*  error = NULL;
    DbSqlite3_T* sqlite = (DbSqlite3_T*)database;

    CHECK_RET_VAL_P(sqlite, -PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
    CHECK_RET_VAL_P(sqlite->database, -DATABASE_NOT_OPENED,"database sqlite not opened!\n");
    CHECK_RET_VAL_P(sql, -PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");


    db_sqlite3_result_exit(&sqlite->result);
    ret = sqlite3_get_table(sqlite->database, sql, &sqlite->result.result, &row_count, &col_count, &error);
	CHECK_RET_GOTO_P(ret == SQLITE_OK, out, "exec: sql: %s failed, error[%d]: %s\n",sql,
						sqlite3_errcode(sqlite->database), sqlite3_errmsg(sqlite->database));

    if (!row_count){// no result?
        if (sqlite->result.result) {
			sqlite3_free_table(sqlite->result.result);
		}
        sqlite->result.result = NULL;
        return 0;
    }
	
	sqlite->result.rid       = 0;
    sqlite->result.count     = row_count;
    sqlite->result.row.count = col_count;
	return 0;

out:
	if(error) sqlite3_free(error);
	return -1;
}


static DbSqlStmt_PT db_sqlite3_statement_init(struct DbSqlImpl* database, char const* sql)
{
    int ret = SQLITE_OK;
    sqlite3_stmt* statement = NULL;
    DbSqlite3_T* sqlite = (DbSqlite3_T*)database;
    CHECK_RET_VAL_ERRNO_P(sqlite, -PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
    CHECK_RET_VAL_ERRNO_P(sqlite->database, -DATABASE_NOT_OPENED,"database sqlite not opened!\n");
    CHECK_RET_VAL_ERRNO_P(sql, -PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");

	db_sqlite3_result_exit(&sqlite->result);
	  
	ret = sqlite3_prepare_v2(sqlite->database, sql, -1, &statement, 0);
	CHECK_RET_VAL_ERRNO_P(ret == SQLITE_OK, -SQLITE_PREPARE_ERROR, "statement: sql: %s failed, error[%d]: %s\n",sql,
						sqlite3_errcode(sqlite->database), sqlite3_errmsg(sqlite->database));

    return (DbSqlStmt_PT)statement;
}

static void db_sqlite3_statement_exit(struct DbSqlImpl* database, DbSqlStmt_PT statement)
{
    DbSqlite3_T* sqlite = (DbSqlite3_T*)database;
	if(sqlite){
		db_sqlite3_result_exit(&sqlite->result);
	}

    if (statement){
		sqlite3_finalize((sqlite3_stmt*)statement);
	} 
}

static int db_sqlite3_statement_exec(struct DbSqlImpl* database, DbSqlStmt_PT statement)
{
    DbSqlite3_T* sqlite = (DbSqlite3_T*)database;
    CHECK_RET_VAL_P(sqlite && statement, -PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
    CHECK_RET_VAL_P(sqlite->database, -DATABASE_NOT_OPENED,"database sqlite not opened!\n");
	
    int ret = sqlite3_step((sqlite3_stmt*)statement);
    CHECK_RET_VAL_P(ret == SQLITE_DONE || ret == SQLITE_ROW,-SQLITE_STEP_ERROR,
                        "sqlite3_step failed, error[%d]: %s\n",
						    sqlite3_errcode(sqlite->database), sqlite3_errmsg(sqlite->database));

    if (ret == SQLITE_ROW){
        // save statement for iterating it
        sqlite->result.statement = (sqlite3_stmt*)statement;
        // save result row count
        sqlite->result.rid   =  0;
        sqlite->result.count = -1;
        // save result col count
        sqlite->result.row.count = sqlite3_column_count((sqlite3_stmt*)statement);
    }else{
		CHECK_RET_VAL_P(sqlite3_reset((sqlite3_stmt*)statement) == SQLITE_OK, -SQLITE_RESET_ERROR, 
                        "sqlite3_reset failed, error[%d]: %s\n",
						sqlite3_errcode(sqlite->database), sqlite3_errmsg(sqlite->database));
    }

    return 0;
}

static int db_sqlite3_statement_bind(struct DbSqlImpl* database, DbSqlStmt_PT statement, DbSqlValue_T const* list, size_t size)
{
    DbSqlite3_T* sqlite = (DbSqlite3_T*)database;
    CHECK_RET_VAL_P(sqlite && statement && list, -PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
    CHECK_RET_VAL_P(sqlite->database, -DATABASE_NOT_OPENED,"database sqlite not opened!\n");
    CHECK_RET_VAL_P(size > 0, -PARAM_INPUT_ARG_INVALID,"param input size invalid!\n");

    size_t param_count = (size_t)sqlite3_bind_parameter_count((sqlite3_stmt*)statement);
    CHECK_RET_VAL_P(size == param_count, -SQLITE_BIND_COUNT_ERROR,"sqlite3_bind_parameter_count failed!\n");
   
    for (int i = 0; i < size; i++)
    {
        int    ok = SQLITE_ERROR;
        DbSqlValue_T const* value = &list[i];
        
        switch (value->type)
        {
        case DATABASE_SQL_VALUE_TYPE_TEXT:
            ok = sqlite3_bind_text((sqlite3_stmt*)statement, (int)(i + 1), value->u.text.data, (int)database_value_size(value), NULL);
            break;
        case DATABASE_SQL_VALUE_TYPE_INT64:
        case DATABASE_SQL_VALUE_TYPE_UINT64:
            ok = sqlite3_bind_int64((sqlite3_stmt*)statement, (int)(i + 1), database_value_int64(value));
            break;
        case DATABASE_SQL_VALUE_TYPE_INT32:
        case DATABASE_SQL_VALUE_TYPE_INT16:
        case DATABASE_SQL_VALUE_TYPE_INT8:
        case DATABASE_SQL_VALUE_TYPE_UINT32:
        case DATABASE_SQL_VALUE_TYPE_UINT16:
        case DATABASE_SQL_VALUE_TYPE_UINT8:
            ok = sqlite3_bind_int((sqlite3_stmt*)statement, (int)(i + 1), (int)database_value_int32(value));
            break;
        case DATABASE_SQL_VALUE_TYPE_FLOAT:
			ok = sqlite3_bind_double((sqlite3_stmt*)statement, (int)(i + 1), (float)database_value_double(value));
            break;
        case DATABASE_SQL_VALUE_TYPE_DOUBLE:
            ok = sqlite3_bind_double((sqlite3_stmt*)statement, (int)(i + 1), (double)database_value_double(value));
            break;
        case DATABASE_SQL_VALUE_TYPE_NULL:
            ok = sqlite3_bind_null((sqlite3_stmt*)statement, (int)(i + 1));
            break;
		case DATABASE_SQL_VALUE_TYPE_BLOB8:
			ok = sqlite3_bind_blob((sqlite3_stmt*)statement, (int)(i + 1), value->u.blob.data, (int)database_value_size(value), NULL);
			break;
        default:
            break;
        }
		CHECK_RET_VAL_P(ok == SQLITE_OK, -SQLITE_BIND_BODY_ERROR,"sqlite3_bind_body type:%d failed!\n",value->type);
    }	

   	return 0;
}

static int64_t db_sqlite3_last_insert_rowid(struct DbSqlImpl* database)
{
    DbSqlite3_T* sqlite = (DbSqlite3_T*)database;
    CHECK_RET_VAL_P(sqlite , -PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
    CHECK_RET_VAL_P(sqlite->database, -DATABASE_NOT_OPENED,"database sqlite not opened!\n");

	return sqlite3_last_insert_rowid(sqlite->database);
}

__IN_API__ DbSql_PT db_sqlite3_init(DBInfo_T* dbinfo)
{
    CHECK_RET_VAL_P(dbinfo,NULL,"param input invalid!\n");

    DbSqlite3_T* sqlite = CALLOC(1,DbSqlite3_T);
    CHECK_RET_VAL_P(sqlite,NULL,"calloc DbSqlite3_T dailed!\n");
	memset(sqlite, 0, sizeof(DbSqlite3_T));

	memcpy(&sqlite->base.dbinfo,dbinfo, sizeof(DBInfo_T));
	

    sqlite->base.open           = db_sqlite3_open;
    sqlite->base.close          = db_sqlite3_close;
    sqlite->base.exit           = db_sqlite3_exit;
    sqlite->base.exec           = db_sqlite3_exec;
    sqlite->base.result_load    = db_sqlite3_result_load;
    sqlite->base.stmt_init 		= db_sqlite3_statement_init;
    sqlite->base.stmt_exit 		= db_sqlite3_statement_exit;
    sqlite->base.stmt_exec 		= db_sqlite3_statement_exec;
    sqlite->base.stmt_bind 		= db_sqlite3_statement_bind;
	sqlite->base.last_rowid     = db_sqlite3_last_insert_rowid;
	sqlite->base.result  = NULL;
	sqlite->base.item    = 0;

    // init row operation
    static IteratorOps_T row_op = {
        db_sqlite3_result_row_iterator_size,
    	db_sqlite3_result_row_iterator_head,
    	NULL,
    	db_sqlite3_result_row_iterator_tail,
    	db_sqlite3_result_row_iterator_prev,
    	db_sqlite3_result_row_iterator_next,
    	db_sqlite3_result_row_iterator_item,
    	NULL,
    	NULL,
    	NULL,
    };

    // init col operation
    static IteratorOps_T col_op = {
        db_sqlite3_result_col_iterator_size,
    	db_sqlite3_result_col_iterator_head,
    	NULL,
    	db_sqlite3_result_col_iterator_tail,
    	db_sqlite3_result_col_iterator_prev,
    	db_sqlite3_result_col_iterator_next,
    	db_sqlite3_result_col_iterator_item,
    	NULL,
    	NULL,
    	NULL,
    };

    // init result row iterator
    sqlite->result.itor.priv     = (void* )sqlite;
    sqlite->result.itor.step     = 0;
    sqlite->result.itor.ops      = &row_op;

    // init result col iterator
    sqlite->result.row.itor.priv = (void* )sqlite;
    sqlite->result.row.itor.step = 0;
    sqlite->result.row.itor.ops  = &col_op;

	
	
    //sqlite->base.state = SQL_EXEC_STATUS_OK;
		
    return (DbSql_PT)sqlite;
}
