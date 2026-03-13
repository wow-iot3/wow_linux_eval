#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "greatest/greatest.h"

#include "wow_test_config.h"

#include "network/wow_net_iface.h"

#define MOD_TAG "[eth_iface]"

#if SUIT_RUN_ERROR_TEST
TEST test_net_iface_error(void)
{
	int ret = 0;
	printf(MOD_TAG"suit_net_iface ----- test_net_iface_error\n");
	
	ret = wow_eth_checkIfaceStatus("eth300");
	GREATEST_ASSERT(ret != 0);
	PASS();
}
#endif

TEST suit_net_iface_test(void)
{
	printf(MOD_TAG"suit_net_iface----suit_net_iface_test\n");

	int ret = 0;
	int i = 0;
	int status = 0;
	char ip[16] = {0};
	char mask[16] = {0};
	char mac[32] = {0};
	char board[16] = {0};
	size_t tx_byte = 0;
	size_t rx_byte = 0;
	double s_upload;
	double s_dnload;
	StringList_T* list = NULL;

	list = wow_eth_getIfaceName();
	GREATEST_ASSERT(list);
	

	for(i = 0; i < wow_stringlist_size(list); i++){

		status = wow_eth_checkIfaceRunning(wow_stringlist_data(list,i));

		ret = wow_eth_getIfaceIp(wow_stringlist_data(list,i),ip);
		GREATEST_ASSERT(ret == 0);

		ret = wow_eth_getIfaceMask(wow_stringlist_data(list,i),mask);
		GREATEST_ASSERT(ret == 0);

		ret = wow_eth_getIfaceMac(wow_stringlist_data(list,i),mac);
		GREATEST_ASSERT(ret == 0);

		ret = wow_eth_getIfacBroardcast(wow_stringlist_data(list,i),board);
		GREATEST_ASSERT(ret == 0);

		ret = wow_eth_getIfaceRtx(wow_stringlist_data(list,i),&tx_byte,&rx_byte);
		GREATEST_ASSERT(ret == 0);
		
		ret = wow_eth_getIfacSpeed(wow_stringlist_data(list,i),&s_upload,&s_dnload);
		GREATEST_ASSERT(ret == 0);

		printf("%s status:%d ip:%s mask:%s broard:%s mac:%s tx_byte:%zu rx_byte:%zu,s_upload:%8.2lfKB/s s_dnload:%8.2lfKB/s\n",
					wow_stringlist_data(list,i),status,ip,mask,board,mac,tx_byte,rx_byte,s_upload,s_dnload);
		ret = wow_eth_checkIfaceStatus(wow_stringlist_data(list,i));
		GREATEST_ASSERT(ret == 0);
	}

	wow_stringlist_free(&list);

	
	PASS();
}


SUITE(suit_net_iface)
{
#if SUIT_RUN_ERROR_TEST
	RUN_TEST(test_net_iface_error);
#endif
    RUN_TEST(suit_net_iface_test);
}



