#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include "encrypt/wow_crc.h"


static void InvertUint8(uint8_t *destBuf,uint8_t *srcBuf)
{
	int i;
	uint8_t tmp[4]={0};
 
	for(i=0;i< 8;i++)
	{
		if(srcBuf[0]& (1 << i))
		tmp[0]|=1<<(7-i);
	}
	destBuf[0] = tmp[0];
	
}

static void InvertUint16(uint16_t *destBuf,uint16_t *srcBuf)
{
	int i;
	uint16_t tmp[4]={0};
 
	for(i=0;i< 16;i++)
	{
		if(srcBuf[0]& (1 << i))
		tmp[0]|=1<<(15 - i);
	}
	destBuf[0] = tmp[0];
}
static void InvertUint32(uint32_t *destBuf,uint32_t *srcBuf)
{
	int i;
	uint32_t tmp[4]={0};
	
	for(i=0;i< 32;i++)
	{
		if(srcBuf[0]& (1 << i))
		tmp[0]|=1<<(31 - i);
	}
	destBuf[0] = tmp[0];
}



uint8_t wow_crc8_add(uint8_t *pu8Data, uint32_t u32Size)
{
    uint8_t crc = 0;
    while (u32Size--)
    {
        crc += *pu8Data++;
    }
    return crc;
}

/****************************Info********************************************** 
 * Name:    CRC-8               x8+x2+x+1 
 * Width:	8 
 * Poly:    0x07 
 * Init:    0x00 
 * Refin:   False 
 * Refout:  False 
 * Xorout:  0x00 
 * Note: 
 *****************************************************************************/
uint8_t wow_crc8(uint8_t *pu8Data, uint32_t u32Size)
{
	uint8_t wCRCin = 0x00;
	uint8_t wCPoly = 0x07;
	int i = 0;
	while (u32Size--) 	
	{
		wCRCin ^= *(pu8Data++);
		for(i = 0;i < 8;i++)
		{
			if(wCRCin & 0x80)
				wCRCin = (wCRCin << 1) ^ wCPoly;
			else
				wCRCin = wCRCin << 1;
		}
	}
	return (wCRCin);
}
/****************************Info********************************************** 
 * Name:    CRC-8/ITU           x8+x2+x+1 
 * Width:	8 
 * Poly:    0x07 
 * Init:    0x00 
 * Refin:   False 
 * Refout:  False 
 * Xorout:  0x55 
 * Alias:   CRC-8/ATM 
 *****************************************************************************/
uint8_t wow_crc8_itu(uint8_t *pu8Data, uint32_t u32Size)
{
	uint8_t wCRCin = 0x00;
	uint8_t wCPoly = 0x07;
	int i = 0;
	
	while (u32Size--) 	
	{
		wCRCin ^= *(pu8Data++);
		for(i = 0;i < 8;i++)
		{
			if(wCRCin & 0x80)
				wCRCin = (wCRCin << 1) ^ wCPoly;
			else
				wCRCin = wCRCin << 1;
		}
	}
	return (wCRCin^0x55);
}
/****************************Info********************************************** 
 * Name:    CRC-8/ROHC          x8+x2+x+1 
 * Width:	8 
 * Poly:    0x07 
 * Init:    0xFF 
 * Refin:   True 
 * Refout:  True 
 * Xorout:  0x00 
 * Note: 
 *****************************************************************************/
uint8_t wow_crc8_rohc(uint8_t *pu8Data, uint32_t u32Size)
{
	uint8_t wCRCin = 0xFF;
	uint8_t wCPoly = 0x07;
	int i = 0;
	
	InvertUint8(&wCPoly,&wCPoly);
	while (u32Size--) 	
	{
		wCRCin ^= *(pu8Data++);
		for(i = 0;i < 8;i++)
		{
			if(wCRCin & 0x01)
				wCRCin = (wCRCin >> 1) ^ wCPoly;
			else
				wCRCin = wCRCin >> 1;
		}
	}
	return (wCRCin);
}

/****************************Info********************************************** 
 * Name:    CRC-8/MAXIM         x8+x5+x4+1 
 * Width:	8 
 * Poly:    0x31 
 * Init:    0x00 
 * Refin:   True 
 * Refout:  True 
 * Xorout:  0x00 
 * Alias:   DOW-CRC,CRC-8/IBUTTON 
 * Use:     Maxim(Dallas)'s some devices,e.g. DS18B20 
 *****************************************************************************/ 
uint8_t wow_crc8_maxim(uint8_t *pu8Data, uint32_t u32Size)
{
	uint8_t wCRCin = 0x00;
	uint8_t wCPoly = 0x31;
	int i = 0;
	
	InvertUint8(&wCPoly,&wCPoly);
	while (u32Size--) 	
	{
		wCRCin ^= *(pu8Data++);
		for(i = 0;i < 8;i++)
		{
			if(wCRCin & 0x01)
				wCRCin = (wCRCin >> 1) ^ wCPoly;
			else
				wCRCin = wCRCin >> 1;
		}
	}
	return (wCRCin);
}

/****************************Info********************************************** 
 * Name:    CRC-16/CCITT        x16+x12+x5+1 
 * Width:	16
 * Poly:    0x1021 
 * Init:    0x0000 
 * Refin:   True 
 * Refout:  True 
 * Xorout:  0x0000 
 * Alias:   CRC-CCITT,CRC-16/CCITT-TRUE,CRC-16/KERMIT 
 *****************************************************************************/ 
uint16_t wow_crc16_ccitt(uint8_t *pu8Data, uint32_t u32Size)
{
	uint16_t wCRCin = 0x0000;
	uint16_t wCPoly = 0x1021;
	int i = 0;
	
	InvertUint16(&wCPoly,&wCPoly);
	while (u32Size--) 	
	{
		wCRCin ^= *(pu8Data++);
		for(i = 0;i < 8;i++)
		{
			if(wCRCin & 0x01)
				wCRCin = (wCRCin >> 1) ^ wCPoly;
			else
				wCRCin = wCRCin >> 1;
		}
	}
	return (wCRCin);
}


/****************************Info********************************************** 
 * Name:    CRC-16/XMODEM       x16+x12+x5+1 
 * Width:	16 
 * Poly:    0x1021 
 * Init:    0x0000 
 * Refin:   False 
 * Refout:  False 
 * Xorout:  0x0000 
 * Alias:   CRC-16/ZMODEM,CRC-16/ACORN 
 *****************************************************************************/ 
uint16_t wow_crc16_xmodem(uint8_t *pu8Data, uint32_t u32Size)
{
	uint16_t wCRCin = 0x0000;
	uint16_t wCPoly = 0x1021;
	int i = 0;
	
	while (u32Size--) 	
	{
		wCRCin ^= (*(pu8Data++) << 8);
		for(i = 0;i < 8;i++)
		{
			if(wCRCin & 0x8000)
				wCRCin = (wCRCin << 1) ^ wCPoly;
			else
				wCRCin = wCRCin << 1;
		}
	}
	return (wCRCin);
}
/****************************Info********************************************** 
 * Name:    CRC-16/X25          x16+x12+x5+1 
 * Width:	16 
 * Poly:    0x1021 
 * Init:    0xFFFF 
 * Refin:   True 
 * Refout:  True 
 * Xorout:  0XFFFF 
 * Note: 
 *****************************************************************************/
uint16_t wow_crc16_x25(uint8_t *pu8Data, uint32_t u32Size)
{
	uint16_t wCRCin = 0xFFFF;
	uint16_t wCPoly = 0x1021;
	int i = 0;
	
	InvertUint16(&wCPoly,&wCPoly);
	while (u32Size--) 	
	{
		wCRCin ^= *(pu8Data++);
		for(i = 0;i < 8;i++)
		{
			if(wCRCin & 0x01)
				wCRCin = (wCRCin >> 1) ^ wCPoly;
			else
				wCRCin = wCRCin >> 1;
		}
	}
	return (wCRCin^0xFFFF);
}

/****************************Info********************************************** 
 * Name:    CRC-16/MODBUS       x16+x15+x2+1 
 * Width:	16 
 * Poly:    0x8005 
 * Init:    0xFFFF 
 * Refin:   True 
 * Refout:  True 
 * Xorout:  0x0000 
 * Note: 
 *****************************************************************************/
uint16_t wow_crc16_modbus(uint8_t *pu8Data, uint32_t u32Size)
{
	uint16_t wCRCin = 0xFFFF;
	uint16_t wCPoly = 0x8005;
	int i = 0;
	
	InvertUint16(&wCPoly,&wCPoly);
	while (u32Size--) 	
	{
		wCRCin ^= *(pu8Data++);
		for(i = 0;i < 8;i++)
		{
			if(wCRCin & 0x01)
				wCRCin = (wCRCin >> 1) ^ wCPoly;
			else
				wCRCin = wCRCin >> 1;
		}
	}
	return (wCRCin);
}
/****************************Info********************************************** 
 * Name:    CRC-16/IBM          x16+x15+x2+1 
 * Width:	16 
 * Poly:    0x8005 
 * Init:    0x0000 
 * Refin:   True 
 * Refout:  True 
 * Xorout:  0x0000 
 * Alias:   CRC-16,CRC-16/ARC,CRC-16/LHA 
 *****************************************************************************/ 
uint16_t wow_crc16_ibm(uint8_t *pu8Data, uint32_t u32Size)
{
	uint16_t wCRCin = 0x0000;
	uint16_t wCPoly = 0x8005;
	int i = 0;
	
	InvertUint16(&wCPoly,&wCPoly);
	while (u32Size--) 	
	{
		wCRCin ^= *(pu8Data++);
		for(i = 0;i < 8;i++)
		{
			if(wCRCin & 0x01)
				wCRCin = (wCRCin >> 1) ^ wCPoly;
			else
				wCRCin = wCRCin >> 1;
		}
	}
	return (wCRCin);
}

/****************************Info********************************************** 
 * Name:    CRC-16/MAXIM        x16+x15+x2+1 
 * Width:	16 
 * Poly:    0x8005 
 * Init:    0x0000 
 * Refin:   True 
 * Refout:  True 
 * Xorout:  0xFFFF 
 * Note: 
 *****************************************************************************/
uint16_t wow_crc16_maxim(uint8_t *pu8Data, uint32_t u32Size)
{
	uint16_t wCRCin = 0x0000;
	uint16_t wCPoly = 0x8005;
	int i = 0;
	
	InvertUint16(&wCPoly,&wCPoly);
	while (u32Size--) 	
	{
		wCRCin ^= *(pu8Data++);
		for(i = 0;i < 8;i++)
		{
			if(wCRCin & 0x01)
				wCRCin = (wCRCin >> 1) ^ wCPoly;
			else
				wCRCin = wCRCin >> 1;
		}
	}
	return (wCRCin^0xFFFF);
}

/****************************Info********************************************** 
 * Name:    CRC-16/USB          x16+x15+x2+1 
 * Width:	16 
 * Poly:    0x8005 
 * Init:    0xFFFF 
 * Refin:   True 
 * Refout:  True 
 * Xorout:  0xFFFF 
 * Note: 
 *****************************************************************************/ 
uint16_t wow_crc16_usb(uint8_t *pu8Data, uint32_t u32Size)
{
	uint16_t wCRCin = 0xFFFF;
	uint16_t wCPoly = 0x8005;
	int i = 0;
	
	InvertUint16(&wCPoly,&wCPoly);
	while (u32Size--) 	
	{
		wCRCin ^= *(pu8Data++);
		for(i = 0;i < 8;i++)
		{
			if(wCRCin & 0x01)
				wCRCin = (wCRCin >> 1) ^ wCPoly;
			else
				wCRCin = wCRCin >> 1;
		}
	}
	return (wCRCin^0xFFFF);
}

/****************************Info********************************************** 
 * Name:    CRC-16/DNP          x16+x13+x12+x11+x10+x8+x6+x5+x2+1 
 * Width:	16 
 * Poly:    0x3D65 
 * Init:    0x0000 
 * Refin:   True 
 * Refout:  True 
 * Xorout:  0xFFFF 
 * Use:     M-Bus,ect. 
 *****************************************************************************/  
uint16_t wow_crc16_dnp(uint8_t *pu8Data, uint32_t u32Size)
{
	uint16_t wCRCin = 0x0000;
	uint16_t wCPoly = 0x3D65;
	int i = 0;
	
	InvertUint16(&wCPoly,&wCPoly);
	while (u32Size--) 	
	{
		wCRCin ^= *(pu8Data++);
		for(i = 0;i < 8;i++)
		{
			if(wCRCin & 0x01)
				wCRCin = (wCRCin >> 1) ^ wCPoly;
			else
				wCRCin = (wCRCin >> 1);
		}
	}
	return (wCRCin^0xFFFF);
}

uint16_t wow_crc16_add(uint8_t *pu8Data, uint32_t u32Size)
{
    uint16_t crc = 0;
    while (u32Size--)
    {
        crc += *pu8Data++;
    }
    return crc;
}

/****************************Info********************************************** 
 * Name:    CRC-32  x32+x26+x23+x22+x16+x12+x11+x10+x8+x7+x5+x4+x2+x+1 
 * Width:	32 
 * Poly:    0x4C11DB7 
 * Init:    0xFFFFFFF 
 * Refin:   True 
 * Refout:  True 
 * Xorout:  0xFFFFFFF 
 * Alias:   CRC_32/ADCCP 
 * Use:     WinRAR,ect. 
 *****************************************************************************/  
uint32_t wow_crc32(uint8_t *pu8Data, uint32_t u32Size)
{
	uint32_t wCRCin = 0xFFFFFFFF;
	uint32_t wCPoly = 0x04C11DB7;
 	int i = 0;
	
	InvertUint32(&wCPoly,&wCPoly);
	while (u32Size--) 	
	{
		wCRCin ^= *(pu8Data++);
		for(i = 0;i < 8;i++)
		{
			if(wCRCin & 0x01)
				wCRCin = (wCRCin >> 1) ^ wCPoly;
			else
				wCRCin = wCRCin >> 1;
		}
	}
	return (wCRCin ^ 0xFFFFFFFF) ;
}

/****************************Info********************************************** 
 * Name:    CRC-32/MPEG-2  x32+x26+x23+x22+x16+x12+x11+x10+x8+x7+x5+x4+x2+x+1 
 * Width:	32 
 * Poly:    0x4C11DB7 
 * Init:    0xFFFFFFF 
 * Refin:   False 
 * Refout:  False 
 * Xorout:  0x0000000 
 * Note: 
 *****************************************************************************/ 
uint32_t wow_crc32_mpeg(uint8_t *pu8Data, uint32_t u32Size)
{
	uint32_t wCRCin = 0xFFFFFFFF;
	uint32_t wCPoly = 0x04C11DB7;
	uint32_t wChar = 0;
	int i = 0;
	
	while (u32Size--) 	
	{
		wChar = *(pu8Data++);
		wCRCin ^= (wChar << 24);
		for(i = 0;i < 8;i++)
		{
			if(wCRCin & 0x80000000)
				wCRCin = (wCRCin << 1) ^ wCPoly;
			else
				wCRCin = wCRCin << 1;
		}
	}
	return (wCRCin) ;
}

