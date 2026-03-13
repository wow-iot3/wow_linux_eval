#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "prefix/wow_check.h"
#include "modbus_internal.h"


#include "encrypt/wow_crc.h"

static int modbus_rtu_build_request_basis(Modbus_T *modbus,
								int slave,int function,int addr, int nb)
{
    modbus->req[0] = slave;
    modbus->req[1] = function;
    modbus->req[2] = addr >> 8;
    modbus->req[3] = addr & 0x00ff;
    modbus->req[4] = nb >> 8;
    modbus->req[5] = nb & 0x00ff;

    return MODBUS_RTU_PRESET_REQ_LENGTH;
}

static int modbus_rtu_build_response_basis(sft_t *sft, uint8_t *rsp)
{
    rsp[0] = sft->slave;
    rsp[1] = sft->function;

    return MODBUS_RTU_PRESET_RSP_LENGTH;
}

static int modbus_rtu_prepare_response_tid(const uint8_t *req, int *req_length)
{
    return 0;
}

static int modbus_rtu_send_msg_pre(uint8_t *req, int req_length)
{
    uint16_t crc = wow_crc16_modbus(req, req_length);

	//低字节在前
    req[req_length++] = (uint8_t)(crc & 0x00FF);
	req[req_length++] = (uint8_t)(crc >> 8);
	
    return req_length;
}

static int modbus_rtu_check_confirmation( const uint8_t *req,const uint8_t *rsp, int rsp_length)
{
	CHECK_RET_VAL(rsp_length >= 1,-1);

    CHECK_RET_VAL(req[0] == rsp[0], -1);
    return 0;
}

/* The check_crc16 function shall return 0 is the message is ignored and the
   message length if the CRC is valid. Otherwise it shall return -1 and set
   errno to EMBBADCRC. */
static int modbus_rtu_check_integrity(uint8_t *msg,const int msg_length)
{
	CHECK_RET_VAL(msg_length >= 2,-1);

    uint16_t crc_calculated = wow_crc16_modbus(msg, msg_length-2);
    uint16_t crc_received   = (msg[msg_length-1] << 8) | msg[msg_length-2];

	return (crc_calculated == crc_received)?0:-1;
}


const ModbusCtx_T gt_modbusRtuCtx = {
    MODBUS_BACKEND_TYPE_RTU,
    MODBUS_RTU_HEADER_LENGTH,
    MODBUS_RTU_CHECKSUM_LENGTH,
    MODBUS_RTU_MAX_ADU_LENGTH,
    
    modbus_rtu_build_request_basis,
    modbus_rtu_build_response_basis,
    modbus_rtu_prepare_response_tid,
    modbus_rtu_send_msg_pre,
    modbus_rtu_check_integrity,
    modbus_rtu_check_confirmation,
};


