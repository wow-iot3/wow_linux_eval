#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>


#include "greatest/greatest.h"
#include "wow_test_config.h"

#include "wow_base_init.h"


/*file*/
SUITE_EXTERN(suit_dir);
SUITE_EXTERN(suit_file);
SUITE_EXTERN(suit_hfile);

/*ipc*/
SUITE_EXTERN(suit_ipc);

/*list*/
SUITE_EXTERN(suit_slist);
SUITE_EXTERN(suit_string_list);
SUITE_EXTERN(suit_kv_list);

/*network*/
SUITE_EXTERN(suit_net_iface);
SUITE_EXTERN(suit_socket);
SUITE_EXTERN(suit_tcp);
SUITE_EXTERN(suit_udp);

/*system*/
SUITE_EXTERN(suit_cond);
SUITE_EXTERN(suit_countdown);
SUITE_EXTERN(suit_lock);
SUITE_EXTERN(suit_thread);
SUITE_EXTERN(suit_time);
SUITE_EXTERN(suit_timer);

/*utils*/
SUITE_EXTERN(suit_byte_array);
SUITE_EXTERN(suit_hex);
SUITE_EXTERN(suit_ring_buff);
SUITE_EXTERN(suit_singleton);
SUITE_EXTERN(suit_str);
SUITE_EXTERN(suit_string);
SUITE_EXTERN(suit_type);

GREATEST_MAIN_DEFS(); 



int main(int argc,char* argv[])
{
    GREATEST_MAIN_BEGIN();	

	wow_base_init();

	RUN_SUITE(suit_timer);

#if 0
	/*file*/	
	RUN_SUITE(suit_dir);
	RUN_SUITE(suit_file);
	RUN_SUITE(suit_hfile);

	/*ipc*/	
	RUN_SUITE(suit_ipc);

	/*list*/
	RUN_SUITE(suit_slist);
	RUN_SUITE(suit_string_list);
	RUN_SUITE(suit_kv_list);

	/*network*/
	RUN_SUITE(suit_net_iface);
	RUN_SUITE(suit_socket);
	RUN_SUITE(suit_tcp);
	RUN_SUITE(suit_udp);

	/*system*/
	RUN_SUITE(suit_cond);
	RUN_SUITE(suit_countdown);
	RUN_SUITE(suit_lock);	
	RUN_SUITE(suit_thread);	
	RUN_SUITE(suit_time);	
	RUN_SUITE(suit_timer);	
	
	/*utils*/
	RUN_SUITE(suit_byte_array);	
	RUN_SUITE(suit_hex);
	RUN_SUITE(suit_ring_buff);
	RUN_SUITE(suit_singleton);
	RUN_SUITE(suit_str);
	RUN_SUITE(suit_string);
	RUN_SUITE(suit_type);
#endif
	//模块若使用线程函数进行测试，请保证SUIT_THREAD_RUN_TIMES_S内退出
	sleep(SUIT_THREAD_RUN_TIMES_S);

	wow_base_exit();

    GREATEST_MAIN_END();


}

