
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "greatest/greatest.h"
#include "wow_test_config.h"

#include "database/wow_database.h"
#include "database/wow_database_val.h"
#include "plugin/wow_debug.h"



#define DATABSE_FILE_TEST "database/test.db3"
#define DATABSE_FILE_DEMO "database/demo.db3"

#define MOD_TAG "sqlite"
static DBInfo_T gt_dbInfo = {
	.type = DATABASE_SQL_TYPE_SQLITE3,
	.file = WOW_TEST_FILE_STRING(DATABSE_FILE_TEST),
};



#if SUIT_RUN_ERROR_TEST
TEST test_database_error(void)
{
	ModDebug_I(MOD_TAG,"suit_sql_sqlite-----------------test_database_error\n");

	int ret = 0;
	DBInfo_T dbInfo;
	DbSql_PT database  = NULL;
	Iterator_PT result = NULL;
	DbSqlStmt_PT stmt  = NULL;
	DbSqlValue_T* value = NULL;
	
	dbInfo.type = DATABASE_SQL_TYPE_SQLITE3;
	memset(dbInfo.file,0,128);
	memcpy(dbInfo.file,WOW_TEST_FILE_STRING(DATABSE_FILE_DEMO),
				strlen(WOW_TEST_FILE_STRING(DATABSE_FILE_DEMO)));
	
	database = wow_database_init(NULL);
	GREATEST_ASSERT(database == NULL);


	ret = wow_database_open(NULL);
	GREATEST_ASSERT(ret == -1);
	wow_database_close(NULL);
	ret = wow_database_exec(NULL,"select * from table_project_info");
	GREATEST_ASSERT(ret == -1);
	result = wow_database_result_next(database);
	GREATEST_ASSERT(result == NULL);
	value = wow_database_result_value(result,0);
	GREATEST_ASSERT(value == NULL);
	stmt = wow_database_statement_init(database,NULL);
	GREATEST_ASSERT(stmt == NULL);
	stmt = wow_database_statement_init(database,"select * from table_channel_channel_1 where id =?");
	GREATEST_ASSERT(stmt == NULL);
	ret = wow_database_statement_bind(database,stmt, NULL,1);
	GREATEST_ASSERT(ret == -1);
	ret = wow_database_statement_exec(database,stmt);
	GREATEST_ASSERT(ret == -1);
	wow_database_statement_exit(database,stmt);
		
	database = wow_database_init(&dbInfo);
	GREATEST_ASSERT(database);
	ret = wow_database_open(database);
	GREATEST_ASSERT(ret == 0);
	ret = wow_database_exec(database,NULL);
	GREATEST_ASSERT(ret == -1);
	ret = wow_database_exec(database,"select * from table_project_info_");
	GREATEST_ASSERT(ret == -1);
	ret = wow_database_exec(database,"select * from table_project_info");
	GREATEST_ASSERT(ret == 0);
	result = wow_database_result_next(database);
	GREATEST_ASSERT(result);
	value = wow_database_result_value(result,0);
	GREATEST_ASSERT(value);
	value = wow_database_result_value(result,1);
	GREATEST_ASSERT(value);
	value = wow_database_result_value(result,2);
	GREATEST_ASSERT(value);
	value = wow_database_result_value(result,3);
	GREATEST_ASSERT(value == NULL);

	stmt = wow_database_statement_init(database,NULL);
	GREATEST_ASSERT(stmt == NULL);
	stmt = wow_database_statement_init(database,"select * from table_channel_channel_1 where id =?");
	GREATEST_ASSERT(stmt);

	DbSqlValue_T list[1];
	database_value_set_int32(&list[0], 3);
	ret = wow_database_statement_bind(database,stmt, list,1);
	GREATEST_ASSERT(ret == 0);
	ret = wow_database_statement_exec(database,stmt);
	GREATEST_ASSERT(ret == 0);

	result = wow_database_result_next(database);
	GREATEST_ASSERT(result);
	value = wow_database_result_value(result,0);
	GREATEST_ASSERT(value);
	value = wow_database_result_value(result,11);
	GREATEST_ASSERT(value == NULL);

	wow_database_statement_exit(database,stmt);

	result = wow_database_result_next(database);
	
	GREATEST_ASSERT(result == NULL);	

    wow_database_close(database);
	
	wow_database_exit(&database);

	ret = wow_database_open(database);
	GREATEST_ASSERT(ret == -1);


    PASS();
}
#endif


static void sql_load_result(DbSql_PT database)
{

	while(1){
		Iterator_PT row = wow_database_result_next(database);
		CHECK_RET_BREAK(row);
		int i = 0;
		for(i = 0; i < 3; i++){
			DbSqlValue_T* value = wow_database_result_value(row,i);
			printf("[%s:%s] ",database_value_name(value), database_value_text(value));
		}
		printf("\n");
	}
}

static void sql_stmt_load_result(DbSql_PT database)
{
	int i = 0;
	while(1){
		Iterator_PT row = wow_database_result_next(database);
		CHECK_RET_BREAK(row);

	 	DbSqlValue_T* id = wow_database_result_value(row, 0);
        CHECK_RET_BREAK_P(id,"param id is null!\n");
        printf("[%s:%d] ", database_value_name(id), database_value_int32(id));

		DbSqlValue_T* name = wow_database_result_value(row, 1);
        CHECK_RET_BREAK_P(name,"param name is null!\n");
        printf("[%s:%s] ", database_value_name(name), database_value_text(name));

		DbSqlValue_T* number = wow_database_result_value(row, 3);
        CHECK_RET_BREAK_P(number,"param numner is null!\n");
        printf("[%s:%f] ", database_value_name(number), database_value_float(number));

		DbSqlValue_T* data = wow_database_result_value(row, 2);
        CHECK_RET_BREAK_P(data,"param data is null!\n");
		const uint8_t* dval = database_value_blob(data);
		CHECK_RET_BREAK_P(dval,"data dval is null!\n");
		printf("[%s]: ", database_value_name(data));
		for(i = 0; i < data->u.blob.size; i++){
			printf("0x%02x ",*(dval+i));
		}
        printf("\n");

	}
}

static void sql_stmt_load_result_priv(DbSql_PT database)
{
	while(1){
		Iterator_PT row = wow_database_result_next(database);
		CHECK_RET_BREAK(row);

	 	DbSqlValue_T* id = wow_database_result_value(row, 0);
        CHECK_RET_BREAK_P(id,"param id is null!\n");
        printf("[%s:%d] ", database_value_name(id), database_value_int32(id));

		DbSqlValue_T* name = wow_database_result_value(row, 1);
        CHECK_RET_BREAK_P(name,"param name is null!\n");
        printf("[%s:%s] ", database_value_name(name), database_value_text(name));
		printf("\n");
	}
}

static void sql_stmt_load_result_ext(DbSql_PT database)
{
	while(1){
		Iterator_PT row = wow_database_result_next(database);
		CHECK_RET_BREAK(row);

	 	DbSqlValue_T* name = wow_database_result_value(row, 0);
        CHECK_RET_BREAK_P(name,"name id is null!\n");
        printf("[%s:%s] ", database_value_name(name), database_value_text(name));

		DbSqlValue_T* info = wow_database_result_value(row, 1);
        CHECK_RET_BREAK_P(info,"param name is null!\n");
        printf("[%s:%s] ", database_value_name(info), database_value_text(info));
		printf("\n");
	}
}

TEST test_database_sql_create_drop(void)
{
	int ret = 0;
	
	ModDebug_I(MOD_TAG,"suit_database----test_database_sql_create_drop\n");

	DbSql_PT database = wow_database_init(&gt_dbInfo);
	GREATEST_ASSERT(database);
	
	/*打开数据库*/
	ret = wow_database_open(database);
	GREATEST_ASSERT(ret == 0);
	
	/*删除table*/
	ret = wow_database_exec(database, "drop table if exists table1");
	GREATEST_ASSERT(ret == 1);
	ret = wow_database_exec(database, "drop table if exists table2");
	GREATEST_ASSERT(ret == 1);
	ret = wow_database_exec(database, "drop table if exists table3");
	GREATEST_ASSERT(ret == 1);	
	
	/*创建table*/
	ret = wow_database_exec(database, "create table table1(id int, name text,number float)");
	GREATEST_ASSERT(ret == 1);
	ret = wow_database_exec(database, "create table table2(id int, name text, data blob,number float)");
	GREATEST_ASSERT(ret == 1);
	ret = wow_database_exec(database, "create table table3(id int, info text)");
	GREATEST_ASSERT(ret == 1);


	/*关闭数据库*/
	wow_database_close(database);
	
	wow_database_exit(&database);

    PASS();
}

typedef struct{
	int  id;
	char name[8];
	uint8_t data[8];
	float  number;
}TestInfo_T;
static TestInfo_T gt_testInfo[7] = {
	{1, "name1",{0x00,0x01,0x03}, 52.642},
	{2, "name2",{0x10,0x11,0x13}, 57.127},
	{3, "name3",{0x20,0x21,0x23}, 90.00},
	{4, "name4",{0x30,0x31,0x33}, 2.9000},
	{4, "name4",{0x40,0x41,0x43}, 3.50000},
	{16, "name16",{0x50,0x51,0x53}, 2.1000},
	{17, "name17",{0x60,0x61,0x63}, 21.600},	
};
	
typedef struct{
	int  id;
	char info[8];
}ExtInfo_T;
static ExtInfo_T gt_extInfo[3] = {
	{1, "11111"},
	{2, "22222"},
	{3, "33333"},
};


TEST test_database_sql_insert(void)
{
	int i = 0;
	int ret = 0;
	char tbl_exec[2048] = {0};

	ModDebug_I(MOD_TAG,"suit_database----test_database_sql_insert---1\n");


	DbSql_PT database = wow_database_init(&gt_dbInfo);
	GREATEST_ASSERT(database);
	
	/*打开数据库*/
	ret = wow_database_open(database);
	GREATEST_ASSERT(ret == 0);

	/*操作方法1*/
	/* wow_database_exec 语句插入操作不适用于blob格式*/
	for(i = 0; i < 7; i++){
		memset(tbl_exec,0,2048);
		snprintf(tbl_exec,2048,"insert into table1 values(%d, '%s', %f);",
				gt_testInfo[i].id,gt_testInfo[i].name,gt_testInfo[i].number);

		ret = wow_database_exec(database, tbl_exec);
		GREATEST_ASSERT(ret == 1);
	}
	for(i = 0; i < 3; i++){
		memset(tbl_exec,0,2048);
		snprintf(tbl_exec,2048,"insert into table3 values(%d, '%s');",gt_extInfo[i].id,gt_extInfo[i].info);

		ret = wow_database_exec(database, tbl_exec);
		GREATEST_ASSERT(ret == 1);
	}

	ModDebug_I(MOD_TAG,"suit_database----test_database_sql_insert---2\n");
	/*操作方法2*/
	DbSqlStmt_PT statement;
	DbSqlValue_T list[4];
	statement = wow_database_statement_init(database, "insert into table2(id, name, data, number) values(?,?,?,?);");
	GREATEST_ASSERT(statement);
	
	for(i = 0; i < 7; i++){
		database_value_set_int32(&list[0], gt_testInfo[i].id);
		database_value_set_text(&list[1], gt_testInfo[i].name,8);
		database_value_set_blob8(&list[2], gt_testInfo[i].data,8);
		database_value_set_float(&list[3], gt_testInfo[i].number);

		ret = wow_database_statement_bind(database, statement, list, 4);
		GREATEST_ASSERT(ret == 0);
		ret = wow_database_statement_exec(database, statement);	
		GREATEST_ASSERT(ret == 1);
	}
	
	wow_database_statement_exit(database, statement);


	/*关闭数据库*/
	wow_database_close(database);
	
	wow_database_exit(&database);

	PASS();
}



TEST test_database_sql_count(void)
{
	int ret = 0;
	Iterator_PT row;
	DbSqlValue_T* value;

	ModDebug_I(MOD_TAG,"suit_database----test_database_sql_count---1\n");

	DbSql_PT database = wow_database_init(&gt_dbInfo);
	GREATEST_ASSERT(database);
	
	/*打开数据库*/
	ret = wow_database_open(database);
	GREATEST_ASSERT(ret == 0);
	
	/*操作方法1*/
	ret = wow_database_exec(database, "select COUNT(*) from table1 limit 1;");
	GREATEST_ASSERT(ret == 0);
	row = wow_database_result_next(database);
	GREATEST_ASSERT(row);
	/* wow_database_exec 语句获取结果都存储为text模式*/
	value = wow_database_result_value(row,0);
	GREATEST_ASSERT(database_value_int32(value) == 7);
	
	ModDebug_I(MOD_TAG,"suit_database----test_database_sql_count---2\n");
	/*操作方法2*/
	DbSqlStmt_PT statement;
	statement = wow_database_statement_init(database,"select COUNT(*) from table2 limit 1;");
	GREATEST_ASSERT(statement);
	ret = wow_database_statement_exec(database, statement); 
	GREATEST_ASSERT(ret == 0);

	row = wow_database_result_next(database);
	GREATEST_ASSERT(row);

 	value = wow_database_result_value(row, 0);
	GREATEST_ASSERT(database_value_int32(value) == 7);
	
	wow_database_statement_exit(database, statement);


	/*关闭数据库*/
	wow_database_close(database);
	
	wow_database_exit(&database);

	PASS();
}



		

TEST test_database_sql_select_01(void)
{
	int ret = 0;
	DbSqlValue_T* val_id = NULL;
	DbSqlValue_T* val_name = NULL;
	DbSqlValue_T* val_num  = NULL;
	DbSqlValue_T* val_data = NULL;
	Iterator_PT   row = NULL;
	const uint8_t* dval = NULL;
	
	ModDebug_I(MOD_TAG,"suit_database----test_database_sql_select_01---1\n");

	DbSql_PT database = wow_database_init(&gt_dbInfo);
	GREATEST_ASSERT(database);
	
	/*打开数据库*/
	ret = wow_database_open(database);
	GREATEST_ASSERT(ret == 0);
	
	/*操作方法1*/
	/* wow_database_exec 语句获取结果都存储为text模式*/
	ret = wow_database_exec(database, "select * from table1 where id=2 and name='name2';");
	GREATEST_ASSERT(ret == 0);

	row = wow_database_result_next(database);
	GREATEST_ASSERT(row);

	///<!!!注意此类使用异常 wow_database_result_value 获取为临时变量，每次获取后变更
	#if 0
		val_id	 = wow_database_result_value(row,0);
		val_name = wow_database_result_value(row,1);
		val_num  = wow_database_result_value(row,2);
		
		printf("-----------------------------\n");
		printf("[%s:%s] \n",database_value_name(val_id), database_value_text(val_id));
		printf("[%s:%s] \n",database_value_name(val_name), database_value_text(val_name));
		printf("[%s:%s] \n",database_value_name(val_num), database_value_text(val_num));
		printf("-----------------------------\n");
	#endif
	
	val_id   = wow_database_result_value(row,0);
	GREATEST_ASSERT(!strcmp("id",database_value_name(val_id)));
	GREATEST_ASSERT(database_value_int32(val_id) == 2);
	val_name = wow_database_result_value(row,1);
	GREATEST_ASSERT(!strcmp("name",database_value_name(val_name)));
	GREATEST_ASSERT(!strcmp("name2",database_value_text(val_name)));
	val_num  = wow_database_result_value(row,2);
	GREATEST_ASSERT(!strcmp("number",database_value_name(val_num)));
	///<!!!!!!!浮点型
	//GREATEST_ASSERT(abs(database_value_float(val_num)-57.127) < 0.00000001);


		
	/*操作方法2*/
	ModDebug_I(MOD_TAG,"suit_database----test_database_sql_select_01---2\n");
	DbSqlStmt_PT statement;
	statement = wow_database_statement_init(database, "select * from table2 where id=2;");
	GREATEST_ASSERT(statement);
	ret = wow_database_statement_exec(database, statement);
	GREATEST_ASSERT(ret == 0);

	row = wow_database_result_next(database);
	val_id   = wow_database_result_value(row,0);
	GREATEST_ASSERT(!strcmp("id",database_value_name(val_id)));
	GREATEST_ASSERT(database_value_int32(val_id) == 2);
	val_name = wow_database_result_value(row,1);
	GREATEST_ASSERT(!strcmp("name",database_value_name(val_name)));
	GREATEST_ASSERT(!strcmp("name2",database_value_text(val_name)));
	val_num  = wow_database_result_value(row,3);
	GREATEST_ASSERT(!strcmp("number",database_value_name(val_num)));
	///<!!!!!!!浮点型
	//GREATEST_ASSERT(abs(database_value_float(val_num)-57.127) < 0.00000001);
	val_data = wow_database_result_value(row, 2);
	GREATEST_ASSERT(!strcmp("data",database_value_name(val_data)));
	GREATEST_ASSERT(val_data->u.blob.size == 8);
	dval = database_value_blob(val_data);
	GREATEST_ASSERT(dval[0] == 0x10 && dval[1] == 0x11 && dval[2] == 0x13);
	
	wow_database_statement_exit(database, statement);


	/*操作方法3*/
	ModDebug_I(MOD_TAG,"suit_database----test_database_sql_select_01---3\n");
	statement = wow_database_statement_init(database, "select * from table2 where id=? and name=?;");
	GREATEST_ASSERT(statement);
	TestInfo_T test= {1, "name1",{}, 57.127};
	DbSqlValue_T list[2];
	database_value_set_int32(&list[0], test.id);
	database_value_set_text(&list[1], test.name,8);
	ret = wow_database_statement_bind(database, statement, list, 2);
	GREATEST_ASSERT(ret == 0);
	ret = wow_database_statement_exec(database, statement);
	GREATEST_ASSERT(ret == 0);
	
	row = wow_database_result_next(database);
	val_id   = wow_database_result_value(row,0);
	GREATEST_ASSERT(!strcmp("id",database_value_name(val_id)));
	GREATEST_ASSERT(database_value_int32(val_id) == 1);
	val_name = wow_database_result_value(row,1);
	GREATEST_ASSERT(!strcmp("name",database_value_name(val_name)));
	GREATEST_ASSERT(!strcmp("name1",database_value_text(val_name)));
	val_num  = wow_database_result_value(row,3);
	GREATEST_ASSERT(!strcmp("number",database_value_name(val_num)));
	///<!!!!!!!浮点型
	//GREATEST_ASSERT(abs(database_value_float(val_num)-57.127) < 0.00000001);
	val_data = wow_database_result_value(row, 2);
	GREATEST_ASSERT(!strcmp("data",database_value_name(val_data)));
	GREATEST_ASSERT(val_data->u.blob.size == 8);
	dval = database_value_blob(val_data);
	GREATEST_ASSERT(dval[0] == 0x00 && dval[1] == 0x01 && dval[2] == 0x03);

	wow_database_statement_exit(database, statement);


	/*关闭数据库*/
	wow_database_close(database);
	
	wow_database_exit(&database);

	PASS();
}

TEST test_database_sql_select_02(void)
{
	int ret = 0;
	
	ModDebug_I(MOD_TAG,"suit_database----test_database_sql_select_02---1\n");

	DbSql_PT database = wow_database_init(&gt_dbInfo);
	GREATEST_ASSERT(database);
	
	/*打开数据库*/
	ret = wow_database_open(database);
	GREATEST_ASSERT(ret == 0);
	
	/* wow_database_exec 语句获取结果都存储为text模式*/
	ret = wow_database_exec(database, "select * from table1 order by number desc limit 0,1;");//0---偏移参数
	GREATEST_ASSERT(ret == 0);
	sql_load_result(database);
	
	ModDebug_I(MOD_TAG,"suit_database----test_database_sql_select_02---2\n");
	ret = wow_database_exec(database, "select * from table1 where id != 3 order by number asc limit 0,1;");//0---偏移参数
	GREATEST_ASSERT(ret == 0);
	sql_load_result(database);

	ModDebug_I(MOD_TAG,"suit_database----test_database_sql_select_02---3\n");
	DbSqlStmt_PT statement;
	statement = wow_database_statement_init(database, "select * from table2 where id > 5 order by number desc;");
	GREATEST_ASSERT(statement);
	ret = wow_database_statement_exec(database, statement);
	GREATEST_ASSERT(ret == 0);
	sql_stmt_load_result(database);
	wow_database_statement_exit(database, statement);


	ModDebug_I(MOD_TAG,"suit_database----test_database_sql_select_02---4\n");
	statement = wow_database_statement_init(database, "select * from table2 where name like 'name1%';");
	GREATEST_ASSERT(statement);
	ret = wow_database_statement_exec(database, statement);
	GREATEST_ASSERT(ret == 0);
	sql_stmt_load_result(database);
	wow_database_statement_exit(database, statement);	
	
	ModDebug_I(MOD_TAG,"suit_database----test_database_sql_select_02---5\n");
	statement = wow_database_statement_init(database, "select * from table2 where id like '_7';");
	GREATEST_ASSERT(statement);
	ret = wow_database_statement_exec(database, statement);
	GREATEST_ASSERT(ret == 0);
	sql_stmt_load_result(database);
	wow_database_statement_exit(database, statement);	
	
	ModDebug_I(MOD_TAG,"suit_database----test_database_sql_select_02---6\n");
	statement = wow_database_statement_init(database, "select id,name from table2 group by name order by name asc;");
	GREATEST_ASSERT(statement);
	ret = wow_database_statement_exec(database, statement);
	GREATEST_ASSERT(ret == 0);
	sql_stmt_load_result_priv(database);
	wow_database_statement_exit(database, statement);	
	
	ModDebug_I(MOD_TAG,"suit_database----test_database_sql_select_02---7\n");
	statement = wow_database_statement_init(database, "select DISTINCT id,name from table2;");//消除所有重复的记录
	GREATEST_ASSERT(statement);
	ret = wow_database_statement_exec(database, statement);
	GREATEST_ASSERT(ret == 0);
	sql_stmt_load_result_priv(database);
	wow_database_statement_exit(database, statement);	
	
	/*关闭数据库*/
	wow_database_close(database);
	
	wow_database_exit(&database);

	PASS();
}

TEST test_database_sql_update(void)
{
	int ret = 0;
	
	ModDebug_I(MOD_TAG,"suit_database----test_database_sql_update---1\n");
	
	DbSql_PT database = wow_database_init(&gt_dbInfo);
	GREATEST_ASSERT(database);
	
	/*打开数据库*/
	ret = wow_database_open(database);
	GREATEST_ASSERT(ret == 0);
	
	/*操作方法1*/
	/* wow_database_exec 语句获取结果都存储为text模式*/
	ret = wow_database_exec(database, "update table1 set number=1234,name='name22' where id =1;");
	GREATEST_ASSERT(ret == 1);

	ModDebug_I(MOD_TAG,"suit_database----test_database_sql_update---2\n");
	/*操作方法2*/
	DbSqlStmt_PT statement;
	statement = wow_database_statement_init(database, "update table2 set number=1234,name='name22' where id =4;");
	GREATEST_ASSERT(statement);
	ret = wow_database_statement_exec(database, statement);
	GREATEST_ASSERT(ret == 1);
	wow_database_statement_exit(database, statement);

	ModDebug_I(MOD_TAG,"suit_database----test_database_sql_update---3\n");
	/*操作方法3*/
	statement = wow_database_statement_init(database, "update table2 set number=?,name=? where id=?;");
	GREATEST_ASSERT(statement);
	TestInfo_T test= {3, "name22",{}, 12.34};
	DbSqlValue_T list[3];
	database_value_set_float(&list[0], test.number);
	database_value_set_text(&list[1], test.name,8);
	database_value_set_int32(&list[2], test.id);
	ret = wow_database_statement_bind(database, statement, list, 3);
	GREATEST_ASSERT(ret == 0);
	ret = wow_database_statement_exec(database, statement);
	GREATEST_ASSERT(ret == 1);
	wow_database_statement_exit(database, statement);

	/*关闭数据库*/
	wow_database_close(database);
	
	wow_database_exit(&database);

	PASS();
}

TEST test_database_sql_delete(void)
{
	int ret = 0;

	DbSql_PT database = wow_database_init(&gt_dbInfo);
	GREATEST_ASSERT(database);
	
	/*打开数据库*/
	ret = wow_database_open(database);
	GREATEST_ASSERT(ret == 0);
	
	ModDebug_I(MOD_TAG,"suit_database----test_database_sql_delete---1\n");
	/*操作方法1*/
	/* wow_database_exec 语句获取结果都存储为text模式*/
	ret = wow_database_exec(database, "delete from table1 where id=1 and name='name22';");
	GREATEST_ASSERT(ret == 1);

	ModDebug_I(MOD_TAG,"suit_database----test_database_sql_delete---2\n");
	/*操作方法2*/
	DbSqlStmt_PT statement;
	statement = wow_database_statement_init(database, "delete from table2 where id=5;");
	GREATEST_ASSERT(statement);
	ret = wow_database_statement_exec(database, statement);
	GREATEST_ASSERT(ret == 1);
	wow_database_statement_exit(database, statement);

	ModDebug_I(MOD_TAG,"suit_database----test_database_sql_delete---3\n");
	
	/*操作方法3*/
	statement = wow_database_statement_init(database, "delete from table2 where id=? and name=?;");
	GREATEST_ASSERT(statement);
	TestInfo_T test= {2, "name2",{}, 57.127};
	DbSqlValue_T list[2];
	database_value_set_int32(&list[0], test.id);
	database_value_set_text(&list[1], test.name,8);
	//database_value_set_text(&list[1], "name2",8);  ///<此语句执行失败
	ret = wow_database_statement_bind(database, statement, list, 2);
	GREATEST_ASSERT(ret == 0);
	ret = wow_database_statement_exec(database, statement);
	GREATEST_ASSERT(ret == 1);
	wow_database_statement_exit(database, statement);

	/*关闭数据库*/
	wow_database_close(database);
	
	wow_database_exit(&database);

	PASS();
}


TEST test_database_sql_join(void)
{
	int ret = 0;
	
	ModDebug_I(MOD_TAG,"suit_database----test_database_sql_join---1\n");

	DbSql_PT database = wow_database_init(&gt_dbInfo);
	GREATEST_ASSERT(database);
	
	/*打开数据库*/
	ret = wow_database_open(database);
	GREATEST_ASSERT(ret == 0);

	DbSqlStmt_PT statement;
	statement = wow_database_statement_init(database, 
		"select name,info from table2 inner join table3 on table2.id = table3.id;");
	GREATEST_ASSERT(statement);
	ret = wow_database_statement_exec(database, statement);
	GREATEST_ASSERT(ret == 0);
	sql_stmt_load_result_ext(database);
	wow_database_statement_exit(database, statement);

	ModDebug_I(MOD_TAG,"suit_database----test_database_sql_join---2\n");
	statement = wow_database_statement_init(database, 
		"select t2.name,t3.info from table2 as t2, table3 as t3 where t2.id = t3.id;");
	GREATEST_ASSERT(statement);
	ret = wow_database_statement_exec(database, statement);
	GREATEST_ASSERT(ret == 0);
	sql_stmt_load_result_ext(database);
	wow_database_statement_exit(database, statement);

	/*关闭数据库*/
	wow_database_close(database);
	
	wow_database_exit(&database);

	PASS();
}

TEST test_database_sql_select_all(void)
{
	int ret = 0;
	
	ModDebug_I(MOD_TAG,"suit_database----test_database_sql_select_all---1\n");

	DbSql_PT database = wow_database_init(&gt_dbInfo);
	GREATEST_ASSERT(database);
	
	/*打开数据库*/
	ret = wow_database_open(database);
	GREATEST_ASSERT(ret == 0);
	
	/*操作方法1*/
	/* wow_database_exec 语句获取结果都存储为text模式*/
	ret = wow_database_exec(database, "select * from table1;");
	GREATEST_ASSERT(ret == 0);
	sql_load_result(database);


	ModDebug_I(MOD_TAG,"suit_database----test_database_sql_select_all---2\n");
	/*操作方法2*/
	DbSqlStmt_PT statement;
	statement = wow_database_statement_init(database, "select * from table2;");
	GREATEST_ASSERT(statement);
	ret = wow_database_statement_exec(database, statement);
	GREATEST_ASSERT(ret == 0);
	sql_stmt_load_result(database);
	wow_database_statement_exit(database, statement);

	/*关闭数据库*/
	wow_database_close(database);
	
	wow_database_exit(&database);
	
	PASS();
}

TEST test_database_sql_copy(void)
{
	int ret = 0;

	ModDebug_I(MOD_TAG,"suit_database----test_database_sql_copy\n");

	DbSql_PT database = wow_database_init(&gt_dbInfo);
	GREATEST_ASSERT(database);
	
	/*打开数据库*/
	ret = wow_database_open(database);
	GREATEST_ASSERT(ret == 0);
	

	ret = wow_database_exec(database, "drop table if exists table111");
	GREATEST_ASSERT(ret == 1);
	
	ret = wow_database_exec(database, "create table table111(id int, name text, number int)");
	GREATEST_ASSERT(ret == 1);

	ret = wow_database_exec(database, "insert into table111 select * from table1");
	GREATEST_ASSERT(ret == 1);

	/*关闭数据库*/
	wow_database_close(database);
	
	wow_database_exit(&database);

    PASS();
}

TEST test_database_sql_alert(void)
{
	int ret = 0;

	ModDebug_I(MOD_TAG,"suit_database----test_database_sql_alert\n");

	DbSql_PT database = wow_database_init(&gt_dbInfo);
	GREATEST_ASSERT(database);
	
	/*打开数据库*/
	ret = wow_database_open(database);
	GREATEST_ASSERT(ret == 0);
	
	ret = wow_database_exec(database, "drop table if exists table11");
	GREATEST_ASSERT(ret == 1);
	
	ret = wow_database_exec(database, "alter table table1 rename to table11");
	GREATEST_ASSERT(ret == 1);

	/*关闭数据库*/
	wow_database_close(database);
	
	wow_database_exit(&database);

    PASS();
}



SUITE(suit_database_sqlite)
{
	wow_debug_setStatus(MOD_TAG,SUIT_RUN_DEBUG_FLAG);
	
#if SUIT_RUN_ERROR_TEST
	RUN_TEST(test_database_error);
#endif

	/*创建table*/
	RUN_TEST(test_database_sql_create_drop);
	/*table插入数据*/
	RUN_TEST(test_database_sql_insert);

	/*获取table条目数*/
	RUN_TEST(test_database_sql_count);

	/*获取table内容*/
	RUN_TEST(test_database_sql_select_01);
	RUN_TEST(test_database_sql_select_02);
	RUN_TEST(test_database_sql_join);

	/*更新table内容*/
	RUN_TEST(test_database_sql_update);

	/*删除table内容*/
	RUN_TEST(test_database_sql_delete);

	/*获取table内容*/
	RUN_TEST(test_database_sql_select_all);

	/*复制table*/
	RUN_TEST(test_database_sql_copy);

	/*重命名table名称*/
	RUN_TEST(test_database_sql_alert);

}

