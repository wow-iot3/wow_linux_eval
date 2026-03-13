#include "prefix/wow_check.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"
#include "prefix/wow_errno.h"

#include "file/wow_file.h"
#include "file/wow_dir.h"

#include "database/wow_database.h"
#include "database/wow_database_val.h"
#include "database/wow_database_log.h"

static const char* CREATE_TABLE_ENTRYS = "create table if not exists tbl_entry_log_index (id integer primary key, timestamp integer)";
static const char* CREATE_TABLE_ENTRY_DATA = "create table if not exists tbl_entry_log_data (id integer, def text, value blob, reason integer)";
static const char* GET_OLD_ENTRY = "select * from tbl_entry_log_index where id = (select min(id) from tbl_entry_log_index where timestamp = (select min(timestamp) from tbl_entry_log_index))";
static const char* GET_NEW_ENTRY = "select * from tbl_entry_log_index where id = (select max(id) from tbl_entry_log_index where timestamp = (select max(timestamp) from tbl_entry_log_index))";
static const char* GET_ENTRIES_COUNT = "select Count(*) from tbl_entry_log_index";

static const char* INSERT_ENTRY = "insert into tbl_entry_log_index (timestamp) values (?)";
static const char* INSERT_ENTRY_DATA = "insert into tbl_entry_log_data (id, def, value, reason) values (?,?,?,?)";
static const char* GET_ENTRIES_WITH_RANGE = "select id, timestamp from tbl_entry_log_index where timestamp >= ? and timestamp <= ?";
static const char* GET_ENTRIES_AFTER = "select id, timestamp from tbl_entry_log_index where id > ?";
static const char* GET_ENTRY_DATA = "select def, value, reason from tbl_entry_log_data where id = ?";
static const char* DELETE_ENTRY_DATA = "delete from tbl_entry_log_data where id=?";
static const char* DELETE_ENTRY = "delete from tbl_entry_log_index where id=?";

struct sql_log_t{
    DbSql_PT sql;
	int64_t cur_entry_size;
	int64_t max_entry_size;
    DbSqlStmt_PT insertEntryStmt;
    DbSqlStmt_PT insertEntryDataStmt;
    DbSqlStmt_PT getEntriesWithRange;
    DbSqlStmt_PT getEntriesAfter;
    DbSqlStmt_PT getEntryData;
    DbSqlStmt_PT getOldEntry;
    DbSqlStmt_PT getNewEntry;
    DbSqlStmt_PT getEntriesCount;
    DbSqlStmt_PT deleteEntryData;
    DbSqlStmt_PT deleteEntry;
} ;	

/*brief    初始化日志数据库
 *param ： file:日志数据文件路径
 *return： 成功返回数据库操作符 失败返回NULL
 */
__EX_API__ SqlLog_T* wow_sql_log_init(const char* pcFile)
{
	int ret = 0;
	DBInfo_T dbInfo;
	SqlLog_T* sqllog  = NULL;
	Iterator_PT  row = NULL;

	CHECK_RET_VAL_P(pcFile,NULL,"param input invalid!\n");

	sqllog = CALLOC(1, SqlLog_T);
	CHECK_RET_VAL_P(sqllog,NULL,"malloc SqlLog_T failed!\n");
	memset(sqllog,0,sizeof(SqlLog_T));
	
	dbInfo.type = DATABASE_SQL_TYPE_SQLITE3;
	memset(dbInfo.file,0,128);
	memcpy(dbInfo.file,pcFile,strlen(pcFile));
	sqllog->sql = wow_database_init(&dbInfo);
	CHECK_RET_GOTO_P(sqllog->sql,out1,"wow_database_init %s failed!\n",dbInfo.file);
	
	ret = wow_database_open(sqllog->sql);
	CHECK_RET_GOTO_P(sqllog->sql,out2,"wow_database_open %s failed!\n",dbInfo.file);

	ret = wow_database_exec(sqllog->sql, CREATE_TABLE_ENTRYS);
	CHECK_RET_GOTO_P(ret == 0,out3,"wow_database_exec %s failed!\n",CREATE_TABLE_ENTRYS);
	
	ret = wow_database_exec(sqllog->sql, CREATE_TABLE_ENTRY_DATA);
	CHECK_RET_GOTO_P(ret == 0,out3,"wow_database_exec %s failed!\n",CREATE_TABLE_ENTRY_DATA);

	sqllog->insertEntryStmt = wow_database_statement_init(sqllog->sql,INSERT_ENTRY);
	CHECK_RET_GOTO_P(sqllog->insertEntryStmt,out3,"wow_database_statement_init %s failed!\n",INSERT_ENTRY);
	sqllog->insertEntryDataStmt = wow_database_statement_init(sqllog->sql,INSERT_ENTRY_DATA);
	CHECK_RET_GOTO_P(sqllog->insertEntryDataStmt,out3,"wow_database_statement_init %s failed!\n",INSERT_ENTRY_DATA);
	sqllog->getEntriesWithRange = wow_database_statement_init(sqllog->sql,GET_ENTRIES_WITH_RANGE);
	CHECK_RET_GOTO_P(sqllog->getEntriesWithRange,out3,"wow_database_statement_init %s failed!\n",GET_ENTRIES_WITH_RANGE);
	sqllog->getEntriesAfter = wow_database_statement_init(sqllog->sql,GET_ENTRIES_AFTER);
	CHECK_RET_GOTO_P(sqllog->getEntriesAfter,out3,"wow_database_statement_init %s failed!\n",GET_ENTRIES_AFTER);
	sqllog->getEntryData = wow_database_statement_init(sqllog->sql,GET_ENTRY_DATA);
	CHECK_RET_GOTO_P(sqllog->getEntryData,out3,"wow_database_statement_init %s failed!\n",GET_ENTRY_DATA);
	sqllog->getOldEntry = wow_database_statement_init(sqllog->sql,GET_OLD_ENTRY);
	CHECK_RET_GOTO_P(sqllog->getOldEntry,out3,"wow_database_statement_init %s failed!\n",GET_OLD_ENTRY);
	sqllog->getNewEntry = wow_database_statement_init(sqllog->sql,GET_NEW_ENTRY);
	CHECK_RET_GOTO_P(sqllog->getNewEntry,out3,"wow_database_statement_init %s failed!\n",GET_NEW_ENTRY);
	sqllog->getEntriesCount = wow_database_statement_init(sqllog->sql,GET_ENTRIES_COUNT);
	CHECK_RET_GOTO_P(sqllog->getEntriesCount,out3,"wow_database_statement_init %s failed!\n",GET_ENTRIES_COUNT);
	sqllog->deleteEntryData = wow_database_statement_init(sqllog->sql,DELETE_ENTRY_DATA);
	CHECK_RET_GOTO_P(sqllog->deleteEntryData,out3,"wow_database_statement_init %s failed!\n",DELETE_ENTRY_DATA);
	sqllog->deleteEntry = wow_database_statement_init(sqllog->sql,DELETE_ENTRY);
	CHECK_RET_GOTO_P(sqllog->deleteEntry,out3,"wow_database_statement_init %s failed!\n",DELETE_ENTRY);
	
		
	ret = wow_database_statement_exec(sqllog->sql, sqllog->getEntriesCount);
	CHECK_RET_GOTO_P(ret == 0,out3,"wow_database_statement_exec failed!\n");
	
	row = wow_database_result_next(sqllog->sql);
	CHECK_RET_GOTO_P(row,out3,"wow_database_result_next failed!\n");
	
	sqllog->cur_entry_size = database_value_int64(wow_database_result_value(row, 0));

	return sqllog;
out3:
	wow_database_close(sqllog->sql);
out2:	
	wow_database_exit(&sqllog->sql);
out1:
	FREE(sqllog);
	return NULL;
}


/*brief    退出日志数据库
 *param ： ptSqllog:数据库操作符
 *return： 无
 */
__EX_API__ void wow_sql_log_exit(SqlLog_T* ptSqllog)
{
    CHECK_RET_VOID(ptSqllog);
	
    wow_database_statement_exit(ptSqllog->sql,ptSqllog->insertEntryStmt);
    wow_database_statement_exit(ptSqllog->sql,ptSqllog->insertEntryDataStmt);
    wow_database_statement_exit(ptSqllog->sql,ptSqllog->getEntriesWithRange);
    wow_database_statement_exit(ptSqllog->sql,ptSqllog->getEntriesAfter);
    wow_database_statement_exit(ptSqllog->sql,ptSqllog->getEntryData);
    wow_database_statement_exit(ptSqllog->sql,ptSqllog->getOldEntry);
    wow_database_statement_exit(ptSqllog->sql,ptSqllog->getNewEntry);
    wow_database_statement_exit(ptSqllog->sql,ptSqllog->getEntriesCount);
    wow_database_statement_exit(ptSqllog->sql,ptSqllog->deleteEntryData);
    wow_database_statement_exit(ptSqllog->sql,ptSqllog->deleteEntry);

	wow_database_close(ptSqllog->sql);
	wow_database_exit(&ptSqllog->sql);
	
	FREE(ptSqllog);
}


/*brief    设置日志数据库存储最大条目数
 *param ： ptSqllog:数据库操作符
 *param ： s64Size:数据库最大条目数
 *return： 成功返回0 失败返回-1
 */
__EX_API__ int  wow_sql_log_SetMaxEntrySize(SqlLog_T* ptSqllog,int64_t s64Size)
{
	CHECK_RET_VAL_P(ptSqllog,-1,"param input invalid!\n");

	ptSqllog->max_entry_size = s64Size;
	return 0;
}

static void sql_log_deleteOldestEntry(SqlLog_T* ptSqllog)
{
	int ret = 0;
	DbSqlValue_T list;
	int64_t entry_id = 0;

	ret = wow_sql_log_getOldestEntries(ptSqllog,&entry_id,NULL);
	CHECK_RET_VOID(ret == 0);
	
	database_value_set_int64(&list,entry_id);
	ret = wow_database_statement_bind(ptSqllog->sql, ptSqllog->deleteEntryData, &list, 1);
	CHECK_RET_VOID_P(ret == 0,"wow_database_statement_bind failed!\n");
	ret = wow_database_statement_exec(ptSqllog->sql, ptSqllog->deleteEntryData);
	CHECK_RET_VOID_P(ret == 0,"wow_database_statement_exec failed!\n");

	ret = wow_database_statement_bind(ptSqllog->sql, ptSqllog->deleteEntry, &list, 1);
	CHECK_RET_VOID_P(ret == 0,"wow_database_statement_bind failed!\n");
	ret = wow_database_statement_exec(ptSqllog->sql, ptSqllog->deleteEntry);
	CHECK_RET_VOID_P(ret == 0,"wow_database_statement_exec failed!\n");

}

/*brief    获取添加条目index标识
 *param ： ptSqllog:数据库操作符
 *param ： s64Timestamp:待添加条目时间戳
 *return： 成功返回条目index标识 失败返回-1
 */
__EX_API__ int64_t wow_sql_log_addEntry(SqlLog_T* ptSqllog, int64_t s64Timestamp)
{
	CHECK_RET_VAL_P(ptSqllog && ptSqllog->sql && ptSqllog->insertEntryStmt,-1,"param input invalid!\n");

	int ret = 0;
	DbSqlValue_T list;
	int64_t rowid = 0;
	
	database_value_set_int64(&list,s64Timestamp);
	ret = wow_database_statement_bind(ptSqllog->sql, ptSqllog->insertEntryStmt, &list, 1);
	CHECK_RET_VAL_P(ret == 0,-1,"wow_database_statement_bind failed!\n");
	ret = wow_database_statement_exec(ptSqllog->sql, ptSqllog->insertEntryStmt);
	CHECK_RET_VAL_P(ret == 0,-1,"wow_database_statement_exec failed!\n");

	rowid = wow_database_insert_last_rowid(ptSqllog->sql);

	if(++ptSqllog->cur_entry_size > ptSqllog->max_entry_size){
		sql_log_deleteOldestEntry(ptSqllog);
	}

	return ptSqllog->cur_entry_size;	
}

/*brief    获取添加条目内容
 *param ： ptSqllog:数据库操作符
 *param ： s64Idx:待添加条目标识
 *param ： pcDef:自定义描述符
 *param ： pu8Data:数据内容
 *param ： snSize:数据内容大小
 *param ： u8Reason:记录原因标识码
 *return： 成功返回0 失败返回-1
 */
__EX_API__ int wow_sql_log_addEntryData(SqlLog_T* ptSqllog, int64_t s64Idx, const char* pcDef, 
										uint8_t* pu8Data, size_t snSize, uint8_t u8Reason)
{
	CHECK_RET_VAL_P(ptSqllog && ptSqllog->sql && ptSqllog->insertEntryDataStmt,-1,"param input invalid!\n");
	
	int ret = 0;
	DbSqlValue_T list[4];
	
	database_value_set_int64(&list[0],s64Idx);
	database_value_set_text(&list[1], pcDef,strlen(pcDef));
	database_value_set_blob8(&list[2],pu8Data,snSize);
	database_value_set_int32(&list[3],u8Reason);
		
	ret = wow_database_statement_bind(ptSqllog->sql, ptSqllog->insertEntryDataStmt, list, 4);
	CHECK_RET_VAL_P(ret == 0,-1,"wow_database_statement_bind failed!\n");
	ret = wow_database_statement_exec(ptSqllog->sql, ptSqllog->insertEntryDataStmt);
	CHECK_RET_VAL_P(ret == 0,-1,"wow_database_statement_exec failed!\n");

	return 0;
}



/*brief    获取添加条目数据内容
 *param ： ptSqllog:数据库操作符
 *param ： s64Idx:条目标识
 *param ： fCallback:回调函数
 *param ： pArg:私有变量传递
 *return： 成功返回0 失败返回-1
 */
__EX_API__ int wow_sql_log_getEntryData(SqlLog_T* ptSqllog, int64_t s64Idx, sql_log_data_func_t fCallback, void* pArg)
{
	CHECK_RET_VAL_P(ptSqllog && ptSqllog->sql && ptSqllog->getEntryData && fCallback,-1,"param input invalid!\n");

	int ret = 0;
	bool sendFinalEvent = true;
	DbSqlValue_T list;
	Iterator_PT  row = NULL;
	DbSqlValue_T* def = NULL;
	DbSqlValue_T* dataEntry = NULL;
	uint8_t reasonCode = 0;
	uint8_t data[2048];
	int size = 0;
	
	database_value_set_int64(&list,s64Idx);
	ret = wow_database_statement_bind(ptSqllog->sql, ptSqllog->getEntryData, &list, 1);
	CHECK_RET_VAL_P(ret == 0,-1,"wow_database_statement_bind failed!\n");

	while(1){
		ret = wow_database_statement_exec(ptSqllog->sql, ptSqllog->getEntryData);
		CHECK_RET_BREAK(ret == 0);
	
		row = wow_database_result_next(ptSqllog->sql);
		CHECK_RET_BREAK(row);

		dataEntry = wow_database_result_value(row, 1);
		size = database_value_blob_size(dataEntry);
		memcpy(data,database_value_blob(dataEntry),size);
		reasonCode = database_value_uint8(wow_database_result_value(row, 2));
		def    = wow_database_result_value(row, 0);
		
        if (fCallback != NULL) {
           sendFinalEvent = fCallback(pArg, database_value_text(def),data,size,reasonCode);
		   if(sendFinalEvent == false) break;
        }
	}
	
	return 0;
}


/*brief    获取添加条目数据内容
 *param ： ptSqllog:数据库操作符
 *param ： s64Stime:起始时间
 *param ： s64Etime:停止时间
 *param ： fCallback:回调函数
 *param ： pArg:私有变量传递
 *return： 成功返回0 失败返回-1
 */
__EX_API__ int wow_sql_log_getEntry(SqlLog_T* ptSqllog, int64_t s64Stime, int64_t s64Etime,
											sql_log_data_func_t fCallback, void* pArg)
{
	CHECK_RET_VAL_P(ptSqllog && ptSqllog->sql && ptSqllog->getEntriesWithRange,-1,"param input invalid!\n");


	int ret = 0;
	int64_t id = 0;
	Iterator_PT   row = NULL;
	DbSqlValue_T list[2];

	
	database_value_set_int64(&list[0],s64Stime);
	database_value_set_int64(&list[1],s64Etime);
	ret = wow_database_statement_bind(ptSqllog->sql, ptSqllog->getEntriesWithRange, list, 2);
	CHECK_RET_VAL_P(ret == 0,-1,"wow_database_statement_bind failed!\n");


	while(1){
		ret = wow_database_statement_exec(ptSqllog->sql, ptSqllog->getEntriesWithRange);
		CHECK_RET_BREAK(ret == 0);
	
		row = wow_database_result_next(ptSqllog->sql);
		CHECK_RET_BREAK(row);

	 	id   = database_value_int64(wow_database_result_value(row, 0));
		//database_value_int64(wow_database_result_value(row, 1));

		wow_sql_log_getEntryData(ptSqllog,id,fCallback,pArg);
	}

	return 0;
}
		
/*brief    获取第一次插入条目信息
 *param ： ptSqllog:数据库操作符
 *param ： ps64EntryIdx:条目标识
 *param ： ps64EntryTime:记录时间
 *return： 成功返回0 失败返回-1
 */
__EX_API__ int wow_sql_log_getOldestEntries(SqlLog_T* ptSqllog,int64_t* ps64EntryIdx, int64_t* ps64EntryTime)
{
	CHECK_RET_VAL_P(ptSqllog && ptSqllog->sql && ptSqllog->getOldEntry,-1,"param input invalid!\n");
	
	int ret = 0;
	Iterator_PT   row = NULL;

	while(1){	
		ret = wow_database_statement_exec(ptSqllog->sql, ptSqllog->getOldEntry);
		CHECK_RET_BREAK(ret == 0);
	
		row = wow_database_result_next(ptSqllog->sql);
		CHECK_RET_BREAK(ret == 0);
		
		if(ps64EntryIdx){
			*ps64EntryIdx =  database_value_int64(wow_database_result_value(row, 0));
		}

		if(ps64EntryTime){
			*ps64EntryTime =  database_value_int64(wow_database_result_value(row, 1));
		}
	}
	return 0;
}


/*brief    获取最后一次插入条目信息
 *param ： ptSqllog:数据库操作符
 *param ： ps64EntryIdx:条目标识
 *param ： ps64EntryTime:记录时间
 *return： 成功返回0 失败返回-1
 */
__EX_API__ int wow_sql_log_getNewestEntries(SqlLog_T* ptSqllog,int64_t* ps64EntryIdx, int64_t* ps64EntryTime)
{
	CHECK_RET_VAL_P(ptSqllog && ptSqllog->sql && ptSqllog->getNewEntry,-1,"param input invalid!\n");
		
	int ret = 0;
	Iterator_PT   row = NULL;
	
	while(1){	
		ret = wow_database_statement_exec(ptSqllog->sql, ptSqllog->getNewEntry);
		CHECK_RET_BREAK(ret == 0);
	
		row = wow_database_result_next(ptSqllog->sql);
		CHECK_RET_BREAK(ret == 0);
		if(ps64EntryIdx){
			*ps64EntryIdx =  database_value_int64(wow_database_result_value(row, 0));
		}

		if(ps64EntryTime){
			*ps64EntryTime=  database_value_int64(wow_database_result_value(row, 1));
		}
	}
	return 0;
}

