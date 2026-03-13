#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "wow_base_init.h"
#include "wow_hal_init.h"
#include "wow_iot_init.h"

#include "greatest/greatest.h"
#include "wow_test_config.h"

/*algorithm*/
SUITE_EXTERN(suit_bitmap);
SUITE_EXTERN(suit_bloomfilter);

/*config*/
SUITE_EXTERN(suit_ini);
SUITE_EXTERN(suit_json);
SUITE_EXTERN(suit_xml);

/*database*/
SUITE_EXTERN(suit_database_sqlite);

/*encrypt*/
SUITE_EXTERN(suit_crc);
SUITE_EXTERN(suit_rsa);

/*hash*/
SUITE_EXTERN(suit_hash);
SUITE_EXTERN(suit_hash_map);
SUITE_EXTERN(suit_hash_set);

/*network*/
SUITE_EXTERN(suit_ftp);
SUITE_EXTERN(suit_ntpdate);

/*plugin*/
SUITE_EXTERN(suit_shell);
SUITE_EXTERN(suit_log);

/*protocol*/
SUITE_EXTERN(suit_modbus_m);
SUITE_EXTERN(suit_modbus_s);
SUITE_EXTERN(suit_iec104m);
SUITE_EXTERN(suit_iec104s);

GREATEST_MAIN_DEFS(); 


int main(int argc,char* argv[])
{
    GREATEST_MAIN_BEGIN();	

	int ret = -1;

	ret = wow_base_init();
	GREATEST_ASSERT(ret == 0);
	ret = wow_hal_init();
	GREATEST_ASSERT(ret == 0);
	ret = wow_iot_init();
	GREATEST_ASSERT(ret == 0);

	/*encrypt*/
	RUN_SUITE(suit_rsa);

#if 0
	/*algorithm*/
	RUN_SUITE(suit_bitmap);
	RUN_SUITE(suit_bloomfilter);

	/*config*/
	RUN_SUITE(suit_ini);
	RUN_SUITE(suit_json);
	RUN_SUITE(suit_xml);

	/*database*/
	RUN_SUITE(suit_database_sqlite);

	/*encrypt*/
	RUN_SUITE(suit_crc);

	/*hash*/
	RUN_SUITE(suit_hash);
	RUN_SUITE(suit_hash_map);
	RUN_SUITE(suit_hash_set);

	/*plugin*/
	RUN_SUITE(suit_shell);
	RUN_SUITE(suit_log);

	/*protocol*/
	RUN_SUITE(suit_modbus_m);
	RUN_SUITE(suit_modbus_s);
	RUN_SUITE(suit_iec104m);
	RUN_SUITE(suit_iec104s);

	/*network*/
	RUN_SUITE(suit_ftp);
	RUN_SUITE(suit_ntpdate);

	/*shmem*/
	RUN_SUITE(suit_shmem);
#endif

	//模块若使用线程函数进行测试，请保证SUIT_THREAD_RUN_TIMES内退出
	sleep(SUIT_THREAD_RUN_TIMES_S);

	ret = wow_iot_exit();
	GREATEST_ASSERT(ret == 0);
	ret = wow_hal_exit();
	GREATEST_ASSERT(ret == 0);
	ret = wow_base_exit();
	GREATEST_ASSERT(ret == 0);

    GREATEST_MAIN_END();

}

