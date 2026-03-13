#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "greatest/greatest.h"

#include "utils/wow_hex.h"

#define MOD_TAG "[hex]"


TEST test_hex_fun(void)
{
	printf(MOD_TAG"suit_hex----test_hex_fun\n");
	
	uint8_t sbyte[16] ={0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,
						0x99,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF};
	uint8_t sstr[32] = "00112233445566778899aabbccddeeff";
	
	wow_hex_print(sbyte,16);

	uint8_t dstr[33];
	memset(dstr,0,33);
	ByteArray2HexStr(sbyte,16,dstr,32);
	printf(MOD_TAG"-----------------------\n");
	printf("%s\n",dstr);
	
	uint8_t dbyte[16];
	HexStr2ByteArray(sstr,32,dbyte,16);
	printf(MOD_TAG"-----------------------\n");
	wow_hex_print(dbyte,16);

	PASS();
}


SUITE(suit_hex)
{
    RUN_TEST(test_hex_fun);
}

