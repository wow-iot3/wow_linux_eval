#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "prefix/wow_check.h"
#include "modbus_internal.h"



static int modbus_tcp_build_request_basis(Modbus_T *modbus, 
								int slave,int function,int addr, int nb)
{
    /* Increase transaction ID */
    if (modbus->t_id < MAXU16){
		 modbus->t_id++;
	}else{
		 modbus->t_id = 0;
	}
       
    modbus->req[0] = modbus->t_id >> 8;
    modbus->req[1] = modbus->t_id & 0x00ff;

    /* Protocol Modbus */
    modbus->req[2] = 0;
    modbus->req[3] = 0;

    /* Length will be defined later by set_req_length_tcp at offsets 4 and 5 */
	
    modbus->req[6]  = slave;
    modbus->req[7]  = function;
    modbus->req[8]  = addr >> 8;
    modbus->req[9]  = addr & 0x00ff;
    modbus->req[10] = nb >> 8;
    modbus->req[11] = nb & 0x00ff;

    return MODBUS_TCP_PRESET_REQ_LENGTH;
}


static int modbus_tcp_build_response_basis(sft_t *sft, uint8_t *rsp)
{
    /* Extract from MODBUS Messaging on TCP/IP Implementation
       Guide V1.0b (page 23/46):
       The transaction identifier is used to associate the future
       response with the request. */
    rsp[0] = sft->t_id >> 8;
    rsp[1] = sft->t_id & 0x00ff;

    /* Protocol Modbus */
    rsp[2] = 0;
    rsp[3] = 0;

    /* Length will be set later by send_msg (4 and 5) */

    /* The slave ID is copied from the indication */
    rsp[6] = sft->slave;
    rsp[7] = sft->function;

    return MODBUS_TCP_PRESET_RSP_LENGTH;
}


static int modbus_tcp_prepare_response_tid(const uint8_t *req, int *req_length)
{
    return (req[0] << 8) + req[1];
}

static int modbus_tcp_send_msg_pre(uint8_t *req, int req_length)
{
    /* Substract the header length to the message length */
    int mbap_length = req_length - 6;

    req[4] = mbap_length >> 8;
    req[5] = mbap_length & 0x00FF;

    return req_length;
}

static int modbus_tcp_check_integrity(uint8_t *msg, const int msg_length)
{
    return 0;
}

static int modbus_tcp_check_confirmation(const uint8_t *req,const uint8_t *rsp, int rsp_length)
{
	CHECK_RET_VAL(rsp_length >= 4,-1);
	
    /* Check transaction ID */
	CHECK_RET_VAL(req[0] == rsp[0] && req[1] == rsp[1],-1);

    /* Check protocol ID */
    CHECK_RET_VAL(rsp[2] == 0x0 && rsp[3] == 0x0,-1);
    return 0;
}
											  
const ModbusCtx_T gt_modbusTcpCtx = {
    MODBUS_BACKEND_TYPE_TCP,
    MODBUS_TCP_HEADER_LENGTH,
    MODBUS_TCP_CHECKSUM_LENGTH,
    MODBUS_TCP_MAX_ADU_LENGTH,
    
    modbus_tcp_build_request_basis,
    modbus_tcp_build_response_basis,
    modbus_tcp_prepare_response_tid,
    modbus_tcp_send_msg_pre,
    modbus_tcp_check_integrity,
    modbus_tcp_check_confirmation,
};


