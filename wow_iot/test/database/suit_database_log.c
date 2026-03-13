#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "greatest/greatest.h"
#include "wow_test_config.h"

#include "prefix/wow_common.h"

#include "plugin/wow_debug.h"
#include "system/wow_time.h"
#include "database/wow_database_log.h"

#define MOD_TAG "database_Log"
#define SQL_LOG_FILE_TEST "/log/sql_log_test.db3"


static SqlLog_T* gpt_sql_log = NULL;

static bool sql_log_entry_data_cb (void* parameter, const char* dataRef,uint8_t* data, int dataSize, uint8_t reasonCode)
{
	if(reasonCode == 0){
		char buff[256] = {0};
		dataSize = MIN2(dataSize, 256);
		memcpy(buff,data,dataSize);
		printf("----dataRef:%s data(%d):%s\n",dataRef,dataSize,buff);
	}else if(reasonCode == 1){
		//int index = 0;
		//char2int(data,&index);
		printf("----dataRef:%s data(%d):%02x%02x%02x%02x\n",dataRef,dataSize,data[0],data[1],data[2],data[3]);
	}

	
    return true;
}

TEST test_database_log_test_01(void)
{	
	int i = 0;
	int ret = 0;
	int64_t index = 0;
	int64_t timestap = 0;
	uint8_t data[128] = {0};
	
	gpt_sql_log = wow_sql_log_init(WOW_TEST_FILE_STRING(SQL_LOG_FILE_TEST));
	GREATEST_ASSERT(gpt_sql_log);

	ret = wow_sql_log_SetMaxEntrySize(gpt_sql_log,30);
	GREATEST_ASSERT(ret == 0);
	
	for(i = 1; i < 35; i++){
		index = wow_sql_log_addEntry(gpt_sql_log,wow_time_get_msec());
		GREATEST_ASSERT(index >= 0);

		memset(data,0,sizeof(data));
		memcpy(data,"AABBCCDD",8);
		ret = wow_sql_log_addEntryData(gpt_sql_log,index,"test01",data,8,0);
		GREATEST_ASSERT(ret == 0);
		
		memset(data,0,sizeof(data));
		memcpy(data,"ZZXXCCVV",8);
		ret = wow_sql_log_addEntryData(gpt_sql_log,index,"test02",data,8,0);
		GREATEST_ASSERT(ret == 0);
		
		memset(data,0,sizeof(data));
		memcpy(data,&index,4);
		ret = wow_sql_log_addEntryData(gpt_sql_log,index,"test03",data,4,1);
		GREATEST_ASSERT(ret == 0);

		usleep(100*1000);
	}
	ret = wow_sql_log_getEntry(gpt_sql_log,0,wow_time_get_msec(),sql_log_entry_data_cb,NULL);
	GREATEST_ASSERT(ret == 0);

	ret = wow_sql_log_getOldestEntries(gpt_sql_log,&index,&timestap);
	printf("wow_sql_log_getOldestEntries index:%ld timestap:%ld\n",index,timestap);
	ret = wow_sql_log_getEntryData(gpt_sql_log,index, sql_log_entry_data_cb,NULL);
	GREATEST_ASSERT(ret == 0);
	ret = wow_sql_log_getNewestEntries(gpt_sql_log,&index,&timestap);
	printf("wow_sql_log_getNewestEntries index:%ld timestap:%ld\n",index,timestap);
	ret = wow_sql_log_getEntryData(gpt_sql_log,index, sql_log_entry_data_cb,NULL);
	GREATEST_ASSERT(ret == 0);

	wow_sql_log_exit(gpt_sql_log);
	PASS();
}



SUITE(suit_database_log)
{
	wow_debug_setStatus(MOD_TAG,SUIT_RUN_DEBUG_FLAG);

	RUN_TEST(test_database_log_test_01);

}

