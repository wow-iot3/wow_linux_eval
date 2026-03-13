#include "prefix/wow_check.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"
#include "prefix/wow_errno.h"

#include "plugin/wow_debug.h"
#include "database/wow_database.h"
#include "database/wow_database_val.h"

#include "wow_database_internal.h"
#include "wow_iot_config.h"


/*brief    数据库初始化
 *param ： ptDbInfo:数据库连接参数
 *return： 成功返回数据库操作符 失败返回NULL
 */
DbSql_PT wow_database_init(DBInfo_PT ptDbInfo)
{
    CHECK_RET_VAL_P(ptDbInfo, NULL,"param input invalid!\n");

	switch(ptDbInfo->type){
		case DATABASE_SQL_TYPE_SQLITE3:
			return (DbSql_PT)db_sqlite3_init(ptDbInfo);	
		case DATABASE_SQL_TYPE_MAX:
		default:
			break;
	}
    return NULL;
}

/*brief    数据库退出
 *param ： ptDbSql:数据库操作符
 *return： 无
 */
void wow_database_exit(DbSql_PT* ptDbSql)
{	
	CHECK_RET_VOID(ptDbSql && *ptDbSql);
    DbSqlImpl_T* impl = (DbSqlImpl_T*)*ptDbSql;
    
	impl->exit(impl);

	*ptDbSql = NULL;
}

/*brief    打开数据库
 *param ： ptDbSql:数据库操作符
 *return： 成功返回0 失败返回-1
 */
int wow_database_open(DbSql_PT ptDbSql)
{
    DbSqlImpl_T* impl = (DbSqlImpl_T*)ptDbSql;
	CHECK_RET_VAL_P(impl && impl->open, -1,"param input invalid!\n");
	
	return  impl->open(impl);
}

/*brief    关闭数据库
 *param ： ptDbSql:数据库操作符
 *return： 无
 */
void wow_database_close(DbSql_PT ptDbSql)
{
    DbSqlImpl_T* impl = (DbSqlImpl_T*)ptDbSql;
	CHECK_RET_VOID_P(impl  && impl->close ,"param input invalid!\n");

	impl->close(impl); 
}

/*brief    执行数据库语句
 *param ： ptDbSql:数据库操作符
 *param ： pcSql:执行语句
 *return： 成功返回0 失败返回-1
 */
int wow_database_exec(DbSql_PT ptDbSql, char const* pcSql)
{
    DbSqlImpl_T* impl = (DbSqlImpl_T*)ptDbSql;
    CHECK_RET_VAL_P(impl && pcSql &&impl->exec , -1,"param input invalid!\n");
	
    return impl->exec(impl, pcSql);
}

/*brief    加载数据库语句执行结果
 *param ： ptDbSql:数据库操作符
 *return： 成功返回行操作描述符 失败返回NULL
 */
Iterator_PT wow_database_result_next(DbSql_PT ptDbSql)
{
    DbSqlImpl_T* impl = (DbSqlImpl_T*)ptDbSql;
    CHECK_RET_VAL_P(impl && impl->result_load , NULL,"param input invalid!\n");

	Iterator_PT result = NULL;
	if(impl->result == NULL){
		impl->result = impl->result_load(impl);
	}

	impl->item = wow_iterator_next(impl->result,impl->item);
	if(impl->item >= 0 && (impl->item < wow_iterator_size(impl->result) || -1 == wow_iterator_size(impl->result))){
		result = (Iterator_PT)wow_iterator_item(impl->result,impl->item);
	}else{
		impl->result = NULL;
		impl->item   = 0;
	}
	
	return result;
}

/*brief    加载数据库语句执行值
 *param ： row:行操作操作符
 *param ： nCol:列数
 *return： 成功返回值信息 失败返回NULL
 */
DbSqlValue_T* wow_database_result_value(Iterator_PT result,int nCol)
{
	CHECK_RET_VAL_P(result, NULL,"param input invalid!\n");
	CHECK_RET_VAL_P(nCol < wow_iterator_size(result), NULL,"param input invalid!\n");
	
	return (DbSqlValue_T*)wow_iterator_item(result,nCol);
}


/*brief    数据库描述语句初始化
 *param ： ptDbSql:数据库操作符
 *param ： pcSql:执行语句
 *return： 成功返回描述语句操作符 失败返回NULL
 */
DbSqlStmt_PT wow_database_statement_init(DbSql_PT ptDbSql, char const* pcSql)
{
    DbSqlImpl_T* impl = (DbSqlImpl_T*)ptDbSql;
    CHECK_RET_VAL_P(impl && impl->stmt_init && pcSql, NULL,"param input invalid!\n");
		
    return impl->stmt_init(impl, pcSql);
    
}

/*brief    数据库描述语句退出
 *param ： ptDbSql:数据库操作符
 *param ： ptStat:描述语句操作符
 *return： 无
 */
void wow_database_statement_exit(DbSql_PT ptDbSql, DbSqlStmt_PT ptStat)
{
    DbSqlImpl_T* impl = (DbSqlImpl_T*)ptDbSql;
    CHECK_RET_VOID_P(impl && impl->stmt_exec  && ptStat,"param input invalid!\n");

    impl->stmt_exit(impl, ptStat);
}

/*brief    数据库描述语句执行
 *param ： ptDbSql:数据库操作符
 *param ： ptStat:描述语句操作符
 *return： 成功返回0 失败返回-1
 */
int wow_database_statement_exec(DbSql_PT ptDbSql, DbSqlStmt_PT ptStat)
{
    DbSqlImpl_T* impl = (DbSqlImpl_T*)ptDbSql;
    CHECK_RET_VAL_P(impl && impl->stmt_exec  && ptStat, -1,"param input invalid!\n");
        
	impl->result = NULL;
	
    return impl->stmt_exec(impl, ptStat);
}

/*brief    数据库描述语句绑定值
 *param ： ptDbSql:数据库操作符
 *param ： ptStat:描述语句操作符
 *param ： ptValList:绑定值链表信息	
 *param ： snSize:绑定值链表个数
 *return： 成功返回0 失败返回-1
 */
int wow_database_statement_bind(DbSql_PT ptDbSql, DbSqlStmt_PT ptStat, 
												DbSqlValue_T const* ptValList, size_t snSize)
{
    DbSqlImpl_T* impl = (DbSqlImpl_T*)ptDbSql;
    CHECK_RET_VAL_P(impl && impl->stmt_bind  && ptStat && ptValList && snSize,-1,"param input invalid!\n");
        
    return impl->stmt_bind(impl, ptStat, ptValList, snSize);
}

/*brief    获取数据库最后一次插入rowid
 *param ： ptDbSql:数据库操作符
 *return： 成功返回唯一rowid整数键 失败返回-1
 */
int64_t wow_database_insert_last_rowid(DbSql_PT ptDbSql)
{
	DbSqlImpl_T* impl = (DbSqlImpl_T*)ptDbSql;
	CHECK_RET_VAL_P(impl && impl->last_rowid  ,-1,"param input invalid!\n");
		
	return impl->last_rowid(impl);
}

