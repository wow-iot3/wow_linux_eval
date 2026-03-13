#ifndef __SQL_INTERNAL_H_
#define __SQL_INTERNAL_H_

#include "prefix/wow_keyword.h"

#include "database/wow_database.h"
#include "database/wow_database_val.h"

typedef struct DbSqlImpl
{
    DBInfo_T       dbinfo;  
    size_t         state; 
	Iterator_PT    result;
	int            item;
    int            (*open)(struct DbSqlImpl* database);
    void           (*close)(struct DbSqlImpl* database);
    void           (*exit)(struct DbSqlImpl* database);
    int            (*exec)(struct DbSqlImpl* database, char const* sql);
    int            (*begin)(struct DbSqlImpl* database);
    int            (*commit)(struct DbSqlImpl* database);
    int            (*rollback)(struct DbSqlImpl* database);
    Iterator_PT    (*result_load)(struct DbSqlImpl* database);
    DbSqlStmt_PT   (*stmt_init)(struct DbSqlImpl* database, char const* sql);
    void           (*stmt_exit)(struct DbSqlImpl* database, DbSqlStmt_PT stmt);
    int            (*stmt_exec)(struct DbSqlImpl* database, DbSqlStmt_PT stmt);
    int            (*stmt_bind)(struct DbSqlImpl* database, DbSqlStmt_PT stmt, DbSqlValue_T const* list, size_t size);
	int64_t        (*last_rowid)(struct DbSqlImpl* database);
}__data_aligned__ DbSqlImpl_T;

DbSql_PT db_mysql_init(DBInfo_T* dbinfo);
DbSql_PT db_sqlite3_init(DBInfo_T* dbinfo);

#endif
