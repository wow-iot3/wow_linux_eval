#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>


#include "greatest/greatest.h"

#include "plugin/wow_debug.h"
#include "encrypt/wow_crc.h"

#include "wow_test_config.h"

#define MOD_TAG "crc"

#define CRC_TEST_STR "zy137xxxxxxxx"
#define CRC_TEST_LEN strlen(CRC_TEST_STR)


TEST test_crc8(void)
{
	uint8_t reval  = 0;
	ModDebug_I(MOD_TAG,"suit_crc----test_crc8\n");

	reval = wow_crc8_add((uint8_t *)CRC_TEST_STR,CRC_TEST_LEN);
	GREATEST_ASSERT(reval == 78);
	
	reval = wow_crc8((uint8_t *)CRC_TEST_STR,CRC_TEST_LEN);
	GREATEST_ASSERT(reval == 0xCF);
	
	reval = wow_crc8_itu((uint8_t *)CRC_TEST_STR,CRC_TEST_LEN);
	GREATEST_ASSERT(reval == 0x9A);
	
	reval = wow_crc8_maxim((uint8_t *)CRC_TEST_STR,CRC_TEST_LEN);
	GREATEST_ASSERT(reval == 0x33);

	reval = wow_crc8_rohc((uint8_t *)CRC_TEST_STR,CRC_TEST_LEN);
	GREATEST_ASSERT(reval == 0x1D);

	PASS();
}

TEST test_crc16(void)
{
	uint16_t reval  = 0;
	ModDebug_I(MOD_TAG,"suit_crc----test_crc16\n");

	reval = wow_crc16_ccitt((uint8_t *)CRC_TEST_STR,CRC_TEST_LEN);
	GREATEST_ASSERT(reval == 0x989E);
	
	reval = wow_crc16_dnp((uint8_t *)CRC_TEST_STR,CRC_TEST_LEN);
	GREATEST_ASSERT(reval == 0x107C);
	
	reval = wow_crc16_ibm((uint8_t *)CRC_TEST_STR,CRC_TEST_LEN);
	GREATEST_ASSERT(reval == 0x086D);
	
	reval = wow_crc16_maxim((uint8_t *)CRC_TEST_STR,CRC_TEST_LEN);
	GREATEST_ASSERT(reval == 0xF792);
	
	reval = wow_crc16_modbus((uint8_t *)CRC_TEST_STR,CRC_TEST_LEN);
	GREATEST_ASSERT(reval == 0xE36E);

	reval = wow_crc16_usb((uint8_t *)CRC_TEST_STR,CRC_TEST_LEN);
	GREATEST_ASSERT(reval == 0x1C91);

	reval = wow_crc16_x25((uint8_t *)CRC_TEST_STR,CRC_TEST_LEN);
	GREATEST_ASSERT(reval == 0x5775);

	reval = wow_crc16_xmodem((uint8_t *)CRC_TEST_STR,CRC_TEST_LEN);
	GREATEST_ASSERT(reval == 0x2411);

	PASS();
}

TEST test_crc32(void)
{
	uint32_t reval  = 0;
	ModDebug_I(MOD_TAG,"suit_crc----test_crc32\n");

	reval = wow_crc32((uint8_t *)CRC_TEST_STR,CRC_TEST_LEN);
	GREATEST_ASSERT(reval == 0xD5FC6C19);
	
	reval = wow_crc32_mpeg((uint8_t *)CRC_TEST_STR,CRC_TEST_LEN);
	GREATEST_ASSERT(reval == 0x7B53F9AE);

	PASS();
}

SUITE(suit_crc)
{
	wow_debug_setStatus(MOD_TAG,SUIT_RUN_DEBUG_FLAG);

    RUN_TEST(test_crc8);
	RUN_TEST(test_crc16);
	RUN_TEST(test_crc32);
}



