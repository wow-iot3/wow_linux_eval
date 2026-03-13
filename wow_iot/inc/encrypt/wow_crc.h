#ifndef __WOW_IOT_ENCRYPT_WOW_CRC_H_
#define __WOW_IOT_ENCRYPT_WOW_CRC_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*brief    crc-8计算
 *param ： pu8Data   ：输入需计算源数据
 *param ： u32Size   ：原数据长度
 *return： 成功返回计算值 
 */
uint8_t wow_crc8_add(uint8_t *pu8Data, uint32_t u32Size); 
uint8_t wow_crc8(uint8_t *pu8Data, uint32_t u32Size);
uint8_t wow_crc8_itu(uint8_t *pu8Data, uint32_t u32Size);
uint8_t wow_crc8_rohc(uint8_t *pu8Data, uint32_t u32Size);
uint8_t wow_crc8_maxim(uint8_t *pu8Data, uint32_t u32Size);


/*brief    crc-16计算
 *param ： pu8Data   ：输入需计算源数据
 *param ： u32Size   ：原数据长度
 *return： 成功返回计算值 
 */
uint16_t wow_crc16_add(uint8_t *pu8Data, uint32_t u32Size); 
uint16_t wow_crc16_ccitt(uint8_t *pu8Data, uint32_t u32Size);
uint16_t wow_crc16_xmodem(uint8_t *pu8Data, uint32_t u32Size);
uint16_t wow_crc16_x25(uint8_t *pu8Data, uint32_t u32Size);
uint16_t wow_crc16_modbus(uint8_t *pu8Data, uint32_t u32Size);
uint16_t wow_crc16_ibm(uint8_t *pu8Data, uint32_t u32Size);
uint16_t wow_crc16_maxim(uint8_t *pu8Data, uint32_t u32Size);
uint16_t wow_crc16_usb(uint8_t *pu8Data, uint32_t u32Size);
uint16_t wow_crc16_dnp(uint8_t *pu8Data, uint32_t u32Size);

/*brief    crc-32计算
 *param ： pu8Data   ：输入需计算源数据
 *param ： u32Size   ：原数据长度
 *return： 成功返回计算值 
 */
uint32_t wow_crc32(uint8_t *pu8Data, uint32_t u32Size);
uint32_t wow_crc32_mpeg(uint8_t *pu8Data, uint32_t u32Size);

#ifdef __cplusplus
}
#endif

#endif
