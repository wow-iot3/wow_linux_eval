#ifndef __MODBUS_INTERNAL_H_
#define __MODBUS_INTERNAL_H_

#include "system/wow_lock.h"
#include "protocol/wow_proto_port.h"

typedef enum{
	MODBUS_MASTER = 0x00,
	MODBUS_SLAVER = 0x01
}ModbusMode_E;

#define MSG_LENGTH_UNDEFINED -1
typedef enum {
    _STEP_FUNCTION,
    _STEP_META,
    _STEP_DATA
} Step_E;

#define MODBUS_BROADCAST_ADDRESS    0

/* It's not really the minimal length (the real one is report slave ID
 * in RTU (4 bytes)) but it's a convenient size to use in RTU or TCP
 * communications to read many values or write a single one.
 * Maximum between :
 * - HEADER_LENGTH_TCP (7) + function (1) + address (2) + number (2)
 * - HEADER_LENGTH_RTU (1) + function (1) + address (2) + number (2) + CRC (2)
 */
#define _MIN_REQ_LENGTH 12

#define _REPORT_SLAVE_ID 180

#define _MODBUS_EXCEPTION_RSP_LENGTH 5

/* Max between RTU and TCP max adu length (so TCP) */
#define MAX_MESSAGE_LENGTH 260

/* Timeouts in microsecond (0.5 s) */
#define DEFAULT_RESPONSE_TIMEOUT    100
#define DEFAULT_ERROR_TRY_TIMES     10

typedef enum {
    MODBUS_BACKEND_TYPE_RTU=0,
    MODBUS_BACKEND_TYPE_TCP
}ModbusBackendType_E;

/*
 *  ---------- Request     Indication ----------
 *  | Client | ---------------------->| Server |
 *  ---------- Confirmation  Response ----------
 */
typedef enum {
    /* Request message on the server side */
    MSG_INDICATION,
    /* Request message on the client side */
    MSG_CONFIRMATION
} MsgType_E;

/* This structure reduces the number of params in functions and so
 * optimizes the speed of execution (~ 37%). */
typedef struct _sft {
    int slave;
    int function;
    int t_id;
} sft_t;

typedef struct modbus Modbus_T;

typedef struct {
    unsigned int backend_type;
    unsigned int header_length;
    unsigned int checksum_length;
    unsigned int max_adu_length;
	
    int (*build_request_basis) (Modbus_T *ctx, int slave, int function, int addr,int nb);
    int (*build_response_basis) (sft_t *sft, uint8_t *rsp);
    int (*prepare_response_tid) (const uint8_t *req, int *req_length);
    int (*send_msg_pre) (uint8_t *req, int req_length);
    int (*check_integrity) (uint8_t *msg,const int msg_length);
    int (*check_confirmation) (const uint8_t *req,const uint8_t *rsp,int rsp_length);
} ModbusCtx_T;

struct modbus{
	uint16_t t_id;/*modbus tcp 专用数据*/
	uint32_t timeout;
	uint16_t times;
    ModbusCtx_T *ctx;
	ProtoPortCtx_T* link;
	mutex_lock_t mutex;
	uint8_t req[MAX_MESSAGE_LENGTH];
	uint8_t rsp[MAX_MESSAGE_LENGTH];
};

/* Modbus_Application_Protocol_V1_1b.pdf (chapter 6 section 1 page 12)
 * Quantity of Coils to read (2 bytes): 1 to 2000 (0x7D0)
 * (chapter 6 section 11 page 29)
 * Quantity of Coils to write (2 bytes): 1 to 1968 (0x7B0)
 */
#define MODBUS_MAX_READ_BITS              2000
#define MODBUS_MAX_WRITE_BITS             1968

/* Modbus_Application_Protocol_V1_1b.pdf (chapter 6 section 3 page 15)
 * Quantity of Registers to read (2 bytes): 1 to 125 (0x7D)
 * (chapter 6 section 12 page 31)
 * Quantity of Registers to write (2 bytes) 1 to 123 (0x7B)
 * (chapter 6 section 17 page 38)
 * Quantity of Registers to write in R/W registers (2 bytes) 1 to 121 (0x79)
 */
#define MODBUS_MAX_READ_REGISTERS          125
#define MODBUS_MAX_WRITE_REGISTERS         123
#define MODBUS_MAX_WR_WRITE_REGISTERS      121
#define MODBUS_MAX_WR_READ_REGISTERS       125


/*******************************Modbus_RTU********************************************/
#define MODBUS_RTU_HEADER_LENGTH      1
#define MODBUS_RTU_PRESET_REQ_LENGTH  6
#define MODBUS_RTU_PRESET_RSP_LENGTH  2
#define MODBUS_RTU_CHECKSUM_LENGTH    2
/* Modbus_Application_Protocol_V1_1b.pdf Chapter 4 Section 1 Page 5
 * RS232 / RS485 ADU = 253 bytes + slave (1 byte) + CRC (2 bytes) = 256 bytes
 */
#define MODBUS_RTU_MAX_ADU_LENGTH  256

/*******************************Modbus_TCP********************************************/
#define MODBUS_TCP_HEADER_LENGTH      7
#define MODBUS_TCP_PRESET_REQ_LENGTH 12
#define MODBUS_TCP_PRESET_RSP_LENGTH  8
#define MODBUS_TCP_CHECKSUM_LENGTH    0

#define MODBUS_TCP_DEFAULT_PORT   502
#define MODBUS_TCP_SLAVE         0xFF

/* Modbus_Application_Protocol_V1_1b.pdf Chapter 4 Section 1 Page 5
 * TCP MODBUS ADU = 253 bytes + MBAP (7 bytes) = 260 bytes
 */
#define MODBUS_TCP_MAX_ADU_LENGTH  260


#endif
