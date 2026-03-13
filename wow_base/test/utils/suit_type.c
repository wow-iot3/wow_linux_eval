#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "greatest/greatest.h"

#include "utils/wow_type.h"
#include "prefix/wow_common.h"

#define MOD_TAG "[type]"

TEST test_type_fun1()
{
	printf(MOD_TAG"suit_type----test_type_fun1\n");

	int ret = 0;
	uint16_t u16data = 0;
	uint32_t u32data = 0;
	uint64_t u64data = 0;
	double   ddata = 0.0;

	ret = S2toU16("0B100010",&u16data);
	GREATEST_ASSERT(ret == 0 && u16data == 34);
	
	ret = S10toU16("+2021",&u16data);
	GREATEST_ASSERT(ret == 0 && u16data == 2021);

	ret = S10toU16("-2022",&u16data);
	GREATEST_ASSERT(ret == 0 && (int16_t)u16data == -2022);

	ret = S16toU16("0x100",&u16data);
	GREATEST_ASSERT(ret == 0  &&  u16data == 0x100);


	ret = S2toU32("0B100010",&u32data);
	GREATEST_ASSERT(ret == 0 && u32data == 34);
	
	ret = S10toU32("+2021",&u32data);
	GREATEST_ASSERT(ret == 0 && u32data == 2021);

	ret = S10toU32("-2022",&u32data);
	GREATEST_ASSERT(ret == 0 && (int32_t)u32data == -2022);

	ret = S16toU32("0x100",&u32data);
	GREATEST_ASSERT(ret == 0  &&  u32data == 0x100);

	ret = S2toU64("0B100010",&u64data);
	GREATEST_ASSERT(ret == 0 && u64data == 34);
	
	ret = S10toU64("+2021",&u64data);
	GREATEST_ASSERT(ret == 0 && u64data == 2021);

	ret = S10toU64("-2022",&u64data);
	GREATEST_ASSERT(ret == 0 && (int32_t)u64data == -2022);

	ret = S16toU64("0x100",&u64data);
	GREATEST_ASSERT(ret == 0  &&  u64data == 0x100);

	ret = str2double("11.235",&ddata);
	GREATEST_ASSERT(ret == 0  &&  ABS(11.235-ddata) < 0.001);

	ret = str2double("-1.236",&ddata);
	GREATEST_ASSERT(ret == 0  &&  ABS(-1.236-ddata) < 0.001);

	PASS();
	
}

TEST test_type_fun2()
{
	printf(MOD_TAG"suit_type----test_type_fun2\n");

	char buf[32] = {0};
	uint32_t u32data = 0;
	double   ddata = 0.0;

	S32toStr(-2022,buf,32);
	StrtoS32(buf,&u32data);
	GREATEST_ASSERT((int32_t)u32data == -2022);

	U32toStr(2021,buf,32);
	StrtoS32(buf,&u32data);
	GREATEST_ASSERT(u32data == 2021);


	D64toStr(11.235,buf,32);
	StrtoD64(buf,&ddata);
	GREATEST_ASSERT(ABS(11.235-ddata) < 0.001);

	D64toStr(-1.236,buf,32);
	StrtoD64(buf,&ddata);
	GREATEST_ASSERT(ABS(-1.236-ddata) < 0.001);

	PASS();
}

TEST test_type_fun3()
{
	
	printf(MOD_TAG"suit_type----test_type_fun3\n");
	int ret = 0;
	int i = 0;
	uint8_t test_bcd[16] = {0x31,0x33,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x61,0x63,0x68,0x55};
	uint8_t data_str[32] = {0};
	uint8_t data_bcd[16] = {0};
	
	ret = BCD2String(test_bcd,16,data_str,32);
	GREATEST_ASSERT(ret == 0);
	
	GREATEST_ASSERT(STRING_EQUAL_N((const char*)data_str,"31333800000000000000000061636855",32));
	
	
	ret = String2BCD(data_str,32,data_bcd,16);
	GREATEST_ASSERT(ret == 0);
	for(i = 0; i < 16; i++){
		GREATEST_ASSERT(data_bcd[i] == test_bcd[i]);
	}
	PASS();
}



SUITE(suit_type)
{
    RUN_TEST(test_type_fun1);
    RUN_TEST(test_type_fun2);
	RUN_TEST(test_type_fun3);
}

