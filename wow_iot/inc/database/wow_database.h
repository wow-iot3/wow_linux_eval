#ifndef __WOW_IOT_DATABASE_WOW_DATABASE_H_
#define __WOW_IOT_DATABASE_WOW_DATABASE_H_


#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

#include "datastruct/wow_iterator.h"
#include "database/wow_database_val.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum{
	DATABASE_SQL_TYPE_SQLITE3 = 0,
	DATABASE_SQL_TYPE_MAX,
}DBSqlType_E;
	
typedef struct {
	char  addr[128];	/* 数据库服务地址 */
	uint16_t port;		/* 端口号 */
	char  name[128];	/* 数据库名称 */
	char  user[128];	/* 数据库帐号 */
	char  pass[128];	/* 帐号密码 */
	char  file[128];	/* 文件路径 */
	DBSqlType_E type;   /* 数据库类型*/ 
} DBInfo_T,*DBInfo_PT;

typedef void* DbSql_PT;
typedef void* DbSqlStmt_PT;

/*brief    数据库初始化
 *param ： ptDbInfo:数据库连接参数
 *return： 成功返回数据库操作符 失败返回NULL
 */
DbSql_PT  wow_database_init(DBInfo_PT ptDbInfo);

/*brief    数据库退出
 *param ： ptDbSql:数据库操作符
 *return： 无
 */
void  wow_database_exit(DbSql_PT* ptDbSql);


/*brief    打开数据库
 *param ： ptDbSql:数据库操作符
 *return： 成功返回0 失败返回-1
 */
int wow_database_open(DbSql_PT ptDbSql);

/*brief    关闭数据库
 *param ： ptDbSql:数据库操作符
 *return： 无
 */
void wow_database_close(DbSql_PT ptDbSql);


/*brief    执行数据库语句
 *param ： ptDbSql:数据库操作符
 *param ： pcSql:执行语句
 *return： 成功返回0(有返回信息)/1(无返回信息) 失败返回-1
 *注： !!!!不推荐使用此方式获取表内容(不确定表大小) 且返回值不支持blob
 */
int  wow_database_exec(DbSql_PT ptDbSql, char const* pcSql);


/*brief    加载数据库语句执行结果
 *param ： ptDbSql:数据库操作符
 *return： 成功返回行操作描述符 失败返回NULL
 */
Iterator_PT wow_database_result_next(DbSql_PT ptDbSql);

/*brief    加载数据库语句执行值
 *param ： row:行操作操作符
 *param ： col:列数
 *return： 成功返回值信息 失败返回NULL
 */
DbSqlValue_T* wow_database_result_value(Iterator_PT row,int col);


/*brief    数据库描述语句初始化
 *param ： ptDbSql:数据库操作符
 *param ： pcSql:执行语句
 *return： 成功返回描述语句操作符 失败返回NULL
 */
DbSqlStmt_PT wow_database_statement_init(DbSql_PT ptDbSql, char const* pcSql);

/*brief    数据库描述语句绑定值
 *param ： ptDbSql:数据库操作符
 *param ： ptStat:描述语句操作符
 *param ： ptValList:绑定值链表信息	
 *param ： snSize:绑定值链表个数
 *return： 成功返回0 失败返回-1
 */
int   wow_database_statement_bind(DbSql_PT ptDbSql, DbSqlStmt_PT ptStat, 
												DbSqlValue_T const* ptValList, size_t snSize);

/*brief    数据库描述语句执行
 *param ： ptDbSql:数据库操作符
 *param ： ptStat:描述语句操作符
 *return： 成功返回0/1 失败返回-1
 */
int  wow_database_statement_exec(DbSql_PT ptDbSql, DbSqlStmt_PT ptStat);

/*brief    数据库描述语句退出
 *param ： ptDbSql:数据库操作符
 *param ： ptStat:描述语句操作符
 *return： 无
 */
void  wow_database_statement_exit(DbSql_PT ptDbSql, DbSqlStmt_PT ptStat);

/*brief    获取数据库最后一次插入rowid
 *param ： ptDbSql:数据库操作符
 *return： 成功返回唯一rowid整数键 失败返回-1
 */
int64_t wow_database_insert_last_rowid(DbSql_PT ptDbSql);

#ifdef __cplusplus
}
#endif

#endif
