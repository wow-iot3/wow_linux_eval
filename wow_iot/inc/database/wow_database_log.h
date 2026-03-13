#ifndef __WOW_DATABASE_WOW_DATABASE_LOG_H_
#define __WOW_DATABASE_WOW_DATABASE_LOG_H_


#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

typedef struct sql_log_t  SqlLog_T;


typedef bool (*sql_log_data_func_t) (void* param, const char* def, uint8_t* data, int dataSize, uint8_t reason);

/*brief    初始化日志数据库
 *param ： pcFile:日志数据文件路径
 *return： 成功返回数据库操作符 失败返回NULL
 */
SqlLog_T* wow_sql_log_init(const char* pcFile);

/*brief    退出日志数据库
 *param ： ptSqllog:数据库操作符
 *return： 无
 */
void wow_sql_log_exit(SqlLog_T* ptSqllog);

/*brief    设置日志数据库存储最大条目数
 *param ： ptSqllog:数据库操作符
 *param ： s64Size:数据库最大条目数
 *return： 成功返回0 失败返回-1
 */
int  wow_sql_log_SetMaxEntrySize(SqlLog_T* ptSqllog,int64_t s64Size);

/*brief    获取添加条目index标识
 *param ： ptSqllog:数据库操作符
 *param ： s64Timestamp:待添加条目时间戳
 *return： 成功返回条目index标识 失败返回-1
 */
int64_t wow_sql_log_addEntry(SqlLog_T* ptSqllog, int64_t s64Timestamp);

/*brief    获取添加条目内容
 *param ： ptSqllog:数据库操作符
 *param ： s64Idx:待添加条目标识
 *param ： pcDef:自定义描述符
 *param ： pu8Data:数据内容
 *param ： snSize:数据内容大小
 *param ： u8Reason:记录原因标识码
 *return： 成功返回0 失败返回-1
 */
int wow_sql_log_addEntryData(SqlLog_T* ptSqllog, int64_t s64Idx, const char* pcDef, 
											uint8_t* pu8Data, size_t snSize, uint8_t u8Reason);

/*brief    获取添加条目数据内容
 *param ： ptSqllog:数据库操作符
 *param ： s64Idx:条目标识
 *param ： fCallback:回调函数
 *param ： pArg:私有变量传递
 *return： 成功返回0 失败返回-1
 */
int wow_sql_log_getEntryData(SqlLog_T* ptSqllog, int64_t s64Idx, sql_log_data_func_t fCallback, void* pArg);

/*brief    获取添加条目数据内容
 *param ： ptSqllog:数据库操作符
 *param ： s64Stime:起始时间
 *param ： s64Etime:停止时间
 *param ： fCallback:回调函数
 *param ： pArg:私有变量传递
 *return： 成功返回0 失败返回-1
 */
int wow_sql_log_getEntry(SqlLog_T* ptSqllog, int64_t s64Stime, int64_t s64Etime,sql_log_data_func_t fCallback, void* pArg);

/*brief    获取第一次插入条目信息
 *param ： ptSqllog:数据库操作符
 *param ： ps64EntryIdx:条目标识
 *param ： ps64EntryTime:记录时间
 *return： 成功返回0 失败返回-1
 */
int wow_sql_log_getOldestEntries(SqlLog_T* ptSqllog,int64_t* ps64EntryIdx, int64_t* ps64EntryTime);

/*brief    获取最后一次插入条目信息
 *param ： ptSqllog:数据库操作符
 *param ： ps64EntryIdx:条目标识
 *param ： ps64EntryTime:记录时间
 *return： 成功返回0 失败返回-1
 */
int wow_sql_log_getNewestEntries(SqlLog_T* ptSqllog,int64_t* ps64EntryIdx, int64_t* ps64EntryTime);

#endif