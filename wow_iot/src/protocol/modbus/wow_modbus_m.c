#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>


#include "prefix/wow_check.h"
#include "prefix/wow_common.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_keyword.h"

#include "modbus_internal.h"
#include "wow_iot_errno.h"

#include "protocol/modbus/wow_modbus_m.h"


struct modbus_master_t{
	Modbus_T modbus;
}__data_aligned__;

/*brief    计算响应长度
 *param ： ctx: Modbus_T指针
 *param ： req: 发送内容指针
 *return： 返回计算结果 >0
 */
static unsigned int compute_response_length_from_request(Modbus_T* ctx, uint8_t *req)
{
    int length;
    const int offset = ctx->ctx->header_length;

    switch (req[offset]) {
    case MODBUS_READ_COILS:
    case MODBUS_READ_DISCRETE_INPUTS: {
        /* Header + nb values (code from write_bits) */
        int nb = (req[offset + 3] << 8) | req[offset + 4];
        length = 2 + (nb / 8) + ((nb % 8) ? 1 : 0);
    }
        break;
    case MODBUS_READ_HOLDING_REGISTERS:
    case MODBUS_READ_INPUT_REGISTERS:
        /* Header + 2 * nb values */
        length = 2 + 2 * (req[offset + 3] << 8 | req[offset + 4]);
        break;
    case MODBUS_READ_EXCEPTION_STATUS:
        length = 3;
        break;
    case MODBUS_REPORT_SLAVE_ID:
        /* The response is device specific (the header provides the
           length) */
        return MSG_LENGTH_UNDEFINED;
    default:
        length = 5;
    }

    return offset + length + ctx->ctx->checksum_length;
}


static uint8_t compute_meta_length_after_function(int function,MsgType_E type)
{
    int length;

    if (type == MSG_INDICATION) 
	{
        if (function <= MODBUS_WRITE_SINGLE_REGISTER) {
            length = 4;
        } else if (function == MODBUS_WRITE_MULTIPLE_COILS ||
                   function == MODBUS_WRITE_MULTIPLE_REGISTERS) {
            length = 5;
        } else {
            /* MODBUS_FC_READ_EXCEPTION_STATUS, MODBUS_FC_REPORT_SLAVE_ID */
            length = 0;
        }
    } else {
        /* MSG_CONFIRMATION */
        switch (function) {
        case MODBUS_WRITE_SINGLE_COIL:
        case MODBUS_WRITE_SINGLE_REGISTER:
        case MODBUS_WRITE_MULTIPLE_COILS:
        case MODBUS_WRITE_MULTIPLE_REGISTERS:
            length = 4;    
            break;
        default:
            length = 1;
        }
    }

    return length;
}

/* Computes the length to read after the meta information (address, count, etc) */
static int compute_data_length_after_meta(Modbus_T* ctx, uint8_t *msg,MsgType_E type)
{
    int function = msg[ctx->ctx->header_length];
    int length;

    if (type == MSG_INDICATION) {
        switch (function) {
        case MODBUS_WRITE_MULTIPLE_COILS:
        case MODBUS_WRITE_MULTIPLE_REGISTERS:
            length = msg[ctx->ctx->header_length + 5];
            break;
        default:
            length = 0;
        }
    } else {
        /* MSG_CONFIRMATION */
        if (function <= MODBUS_READ_INPUT_REGISTERS ||
            function == MODBUS_REPORT_SLAVE_ID) {
            length = msg[ctx->ctx->header_length + 1];
        } else {
            length = 0;
        }
    }

    length += ctx->ctx->checksum_length;

    return length;
}


/*brief    接收消息(请求/响应)
 *param ： ctx: Modbus_T指针
 *param ： data: 接收内容指针
 *param ： type: MSG类型
 *return： 成功返回发送长度 失败返回-1
 */
static int modbus_receive_msg(Modbus_T* modbus, uint8_t *data, MsgType_E type)
{
	int ret = -1;
	int try = modbus->times;
	int length_to_read = 0;
    int msg_length = 0;
    Step_E step = _STEP_FUNCTION;

	memset(data,0,MAX_MESSAGE_LENGTH);
	length_to_read = modbus->ctx->header_length + 1;
    while (length_to_read != 0 && try > 0) 
	{
		ret = modbus->link->link_recv(modbus->link, data + msg_length, length_to_read,modbus->timeout);
		if(ret <= 0){
			try--;
			continue;
		}

        /* Sums bytes received */
        msg_length += ret;
        /* Computes remaining bytes */
        length_to_read -= ret;

        if (length_to_read == 0) {
            switch (step) {
            case _STEP_FUNCTION:
                /* Function code position */
                length_to_read = compute_meta_length_after_function(data[modbus->ctx->header_length],type);
                if (length_to_read != 0) {
                    step = _STEP_META;
                    break;
                } /* else switches straight to the next step */
            case _STEP_META:
                length_to_read = compute_data_length_after_meta(modbus, data, type);
                if ((msg_length + length_to_read) > (int)modbus->ctx->max_adu_length) {
                    return -1;
                }
                step = _STEP_DATA;
                break;
            default:
                break;
            }
        }
    }

	CHECK_RET_VAL_P(try,-1,"modbus recv failed!\n");
	ret = modbus->ctx->check_integrity(data, msg_length);
	CHECK_RET_VAL_P(ret == 0,-1,"modbus check crc failed!\n");
	
    return msg_length;
}



static int check_confirmation(Modbus_T* ctx, uint8_t *req,
								  		uint8_t *rsp, int rsp_len)
{
	int ret = -1;
	int rsp_len_cal;
	const int offset = ctx->ctx->header_length;
	const int function = rsp[offset];
	int req_nb_value;
	int rsp_nb_value;

	CHECK_RET_VAL_P(function == req[offset],-1,
		"Received function not corresponding to the request (0x%X != 0x%X)\n",function, req[offset]);
	
	ret = ctx->ctx->check_confirmation(req, rsp,rsp_len);
	CHECK_RET_VAL_P(ret == 0,-1,"pre check confirmation failed!\n");
	

	rsp_len_cal = compute_response_length_from_request(ctx, req);
	CHECK_RET_VAL_P((rsp_len == rsp_len_cal ||rsp_len_cal == MSG_LENGTH_UNDEFINED),
						-1,"pre check confirmation failed!\n");

	/* Check the number of values is corresponding to the request */
	switch (function) {
	case MODBUS_READ_COILS:
	case MODBUS_READ_DISCRETE_INPUTS:
		/* Read functions, 8 values in a byte (nb
		 * of values in the request and byte count in
		 * the response. */
		req_nb_value = (req[offset + 3] << 8) + req[offset + 4];
		req_nb_value = (req_nb_value / 8) + ((req_nb_value % 8) ? 1 : 0);
		rsp_nb_value = rsp[offset + 1];
		break;
	case MODBUS_READ_HOLDING_REGISTERS:
	case MODBUS_READ_INPUT_REGISTERS:
		/* Read functions 1 value = 2 bytes */
		req_nb_value = ((req[offset + 3] << 8) + req[offset + 4])*2;
		rsp_nb_value = rsp[offset + 1];
		break;
	case MODBUS_WRITE_MULTIPLE_COILS:
	case MODBUS_WRITE_MULTIPLE_REGISTERS:
		/* N Write functions */
		req_nb_value = (req[offset + 3] << 8) + req[offset + 4];
		rsp_nb_value = (rsp[offset + 3] << 8) | rsp[offset + 4];
		break;
	case MODBUS_REPORT_SLAVE_ID:
		/* Report slave ID (bytes received) */
		req_nb_value = rsp_nb_value = rsp[offset + 1];
		break;
	default:
		/* 1 Write functions & others */
		req_nb_value = rsp_nb_value = 1;
	}
	CHECK_RET_VAL_P(req_nb_value == rsp_nb_value, -1,
						"Quantity not corresponding to the request (%d != %d)\n",rsp_nb_value, req_nb_value);

	return rsp_nb_value;

}




extern ModbusCtx_T gt_modbusRtuCtx;
extern ModbusCtx_T gt_modbusTcpCtx;

/*brief    打开modbus-rtu主设备
 *param ： ptUartPara: 设备端硬件参数
 *return： 成功返回modbus操作符 失败返回NULL
 */
__EX_API__ ModbusMaster_T* wow_modbus_m_rtu_open(ProtoUartParam_T* ptUartPara)
{
	CHECK_RET_VAL_ERRNO_P(ptUartPara,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");

	int ret = 0;
	ModbusMaster_T *m_modbus = CALLOC(1,ModbusMaster_T);
	CHECK_RET_VAL_ERRNO_P(m_modbus,-SYSTEM_MALLOC_FAILED,"malloc ModbusMaster_T failed!\n");

  	m_modbus->modbus.ctx     = &gt_modbusRtuCtx;
	m_modbus->modbus.timeout = DEFAULT_RESPONSE_TIMEOUT;
	m_modbus->modbus.times   = DEFAULT_ERROR_TRY_TIMES;

	m_modbus->modbus.link = proto_port_uart_init(ptUartPara);
	CHECK_RET_GOTO_ERRNO_P(m_modbus->modbus.link, out1,-MODBUS_USART_INIT_ERROR, "proto_port_uart_init failed!\n");

	ret = m_modbus->modbus.link->link_open(m_modbus->modbus.link);
	CHECK_RET_GOTO_ERRNO_P(ret == 0, out2, -MODBUS_USART_OPEN_ERROR, "proto_port_uart_ link_open failed!\n");

    return (ModbusMaster_T*)m_modbus;
	
out2:
	proto_port_uart_exit(&(m_modbus->modbus.link));
out1:
	FREE(m_modbus);
	return NULL;
}

/*brief    打开modbus-tcp主设备
 *param ： ptEthPara: 设备端硬件参数
 *return： 成功返回modbus操作符 失败返回NULL
 */
__EX_API__ ModbusMaster_T* wow_modbus_m_tcp_open(ProtoEthParam_T* ptEthPara)
{
	CHECK_RET_VAL_ERRNO_P(ptEthPara,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");	

	int ret = 0;
	ModbusMaster_T *m_modbus = CALLOC(1,ModbusMaster_T);
	CHECK_RET_VAL_ERRNO_P(m_modbus,-SYSTEM_MALLOC_FAILED,"malloc ModbusMaster_T failed!\n");

	m_modbus->modbus.ctx	  = &gt_modbusTcpCtx;
	m_modbus->modbus.timeout = DEFAULT_RESPONSE_TIMEOUT;
	m_modbus->modbus.times	  = DEFAULT_ERROR_TRY_TIMES;
	
	m_modbus->modbus.link = proto_port_eth_master_init(ptEthPara);
	CHECK_RET_GOTO_ERRNO_P(m_modbus->modbus.link, out1,-MODBUS_ETH_INIT_ERROR, "proto_port_eth_master_init failed!\n");

	ret = m_modbus->modbus.link->link_open(m_modbus->modbus.link);
	CHECK_RET_GOTO_ERRNO_P(ret == 0, out2, -MODBUS_ETH_OPEN_ERROR, "proto_port_eth link_open failed!\n");

	return (ModbusMaster_T*)m_modbus;

out2:
	proto_port_eth_master_exit(&(m_modbus->modbus.link));
out1:
	FREE(m_modbus);
	return NULL;

}

/*brief    设置modbus主采集超时时间
 *param ： ptModbusCtx: 从设备操作符 
 *param ： u32Msec: 采集超时时间(ms)  默认500ms
 *return： 成功返回0 失败返回-1
 */
int wow_modbus_m_set_timeout(ModbusMaster_T* ptModbusCtx, uint32_t u32Msec)
{
	CHECK_RET_VAL_P(ptModbusCtx,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	
	ptModbusCtx->modbus.timeout = u32Msec;

	return 0;
}

/*brief    modbus读取操作
 *param ： ptModbusCtx:   设备操作符 
 *param ： nSlave:   从设备地址
 *param ： eAfn:     采集功能码
 *param ： u32Addr:  采集地址
 *param ： u32Size:  采集长度
 *param ： pu8Data:  采集内容存储
 *return： 成功返回0 接收失败返回-1 发送失败返回-2 
 */
__EX_API__ int  wow_modbus_m_read_data(ModbusMaster_T* ptModbusCtx,int nSlave,ModbusFunctionMode_E eAfn,
										uint32_t u32Addr, uint32_t u32Size, uint8_t* pu8Data)
{
	CHECK_RET_VAL_P(ptModbusCtx,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(nSlave > 0 && nSlave <= 247,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

	int ret = -1;
	int req_len = 0;
	Modbus_T* m_modbus = (Modbus_T*)&ptModbusCtx->modbus;

	switch(eAfn){
		case MODBUS_READ_COILS:
		case MODBUS_READ_DISCRETE_INPUTS:
			CHECK_RET_VAL_P(u32Size <= MODBUS_MAX_READ_BITS,-1,"param input nb invalid!\n");
			break;
		case MODBUS_READ_HOLDING_REGISTERS:
		case MODBUS_READ_INPUT_REGISTERS:
			CHECK_RET_VAL_P(u32Size <= MODBUS_MAX_READ_REGISTERS,-1,"param input nb invalid !\n");
			break;
		default:
			return -1;		
	}

	
	//组帧
    req_len = m_modbus->ctx->build_request_basis(m_modbus,nSlave, eAfn, u32Addr, u32Size);
	req_len = m_modbus->ctx->send_msg_pre(m_modbus->req, req_len);

	//wow_hex_print(m_modbus->req, req_len);
	
	//发送
    ret = m_modbus->link->link_send(m_modbus->link, m_modbus->req, req_len,m_modbus->timeout);
	CHECK_RET_VAL_P(ret == req_len, -2,"send msg failed!\n");

	//接收
	ret = modbus_receive_msg(m_modbus, m_modbus->rsp, MSG_CONFIRMATION);
	CHECK_RET_VAL_P(ret > 0, -1,"receive msg failed!\n");

	//check
	ret = check_confirmation(m_modbus, m_modbus->req, m_modbus->rsp, ret);
	CHECK_RET_VAL_P(ret > 0, -1,"check confirmation failed!\n");


	memcpy(pu8Data,m_modbus->rsp+m_modbus->ctx->header_length + 2,ret);
	
    return WOW_SUCCESS;
}
										
/*brief    modbus写入操作
 *param ： ptModbusCtx:	 设备操作符 
 *param ： nSlave:   从设备地址
 *param ： eAfn:	  采集功能码
 *param ： u32Addr:  采集地址
 *param ： u32Size:  采集长度
 *param ： pu8Data:  采集内容存储
 *return： 成功返回0 失败返回-1
 */
__EX_API__ int wow_modbus_m_write_data(ModbusMaster_T* ptModbusCtx,int nSlave,ModbusFunctionMode_E eAfn,
										uint32_t u32Addr, uint32_t u32Size, uint8_t* pu8Data)
{
	CHECK_RET_VAL_P(ptModbusCtx,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");
	CHECK_RET_VAL_P(nSlave > 0 && nSlave <= 247,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");

	int ret = -1;
	int req_len = 0;
	int byte_count = 0;
	Modbus_T* m_modbus = (Modbus_T*)&ptModbusCtx->modbus;
	
	switch(eAfn){
		case MODBUS_WRITE_SINGLE_COIL:
    		req_len = m_modbus->ctx->build_request_basis(m_modbus,nSlave, eAfn, u32Addr, *(uint16_t*)pu8Data ? 0xFF00 : 0);
			break;
		case MODBUS_WRITE_SINGLE_REGISTER:
			{
				uint16_t val = (uint16_t)(pu8Data[0]*256 + pu8Data[1]);
				req_len = m_modbus->ctx->build_request_basis(m_modbus,nSlave, eAfn, u32Addr, val);
			}
			break;
		case MODBUS_WRITE_MULTIPLE_COILS:
			{
			    req_len = m_modbus->ctx->build_request_basis(m_modbus, nSlave,eAfn, u32Addr, u32Size);
				byte_count = (u32Size / 8) + ((u32Size % 8) ? 1 : 0);
				m_modbus->req[req_len++] = byte_count;
				memcpy(m_modbus->req+req_len,(uint8_t*)pu8Data,byte_count);	
				req_len += byte_count;
			}
			break;
		case MODBUS_WRITE_MULTIPLE_REGISTERS:
			{
				req_len = m_modbus->ctx->build_request_basis(m_modbus, nSlave,eAfn, u32Addr, u32Size);
				m_modbus->req[req_len++] = u32Size * 2;
				memcpy(m_modbus->req+req_len,(uint8_t*)pu8Data,u32Size*2);
				req_len += u32Size*2;
			}
			break;
		default:
			return -1;
	}

	
	req_len = m_modbus->ctx->send_msg_pre(m_modbus->req, req_len);
	
	//发送	
	ret = m_modbus->link->link_send(m_modbus->link, m_modbus->req, req_len,m_modbus->timeout);
	CHECK_RET_VAL_P(ret == req_len , -1,"send msg failed!\n");

	//wow_hex_print(m_modbus->req,req_len);
	
	//接收
	ret = modbus_receive_msg(m_modbus, m_modbus->rsp, MSG_CONFIRMATION);
	CHECK_RET_VAL_P(ret > 0, -1,"receive msg failed!\n");

	//check
	ret = check_confirmation(m_modbus, m_modbus->req, m_modbus->rsp, ret);
	CHECK_RET_VAL_P(ret > 0, -1,"check confirmation failed!\n");

	return WOW_SUCCESS;
}

/*brief    关闭modbus主设备
 *param ： pptModbusCtx: 设备操作符 
 *return： 无
 */
__EX_API__ void wow_modbus_m_rtu_close(ModbusMaster_T** pptModbusCtx)
{
	CHECK_RET_VOID(pptModbusCtx && *pptModbusCtx);
	ModbusMaster_T* m_modbus = (ModbusMaster_T*)*pptModbusCtx;
    

    m_modbus->modbus.link->link_close(m_modbus->modbus.link);
	proto_port_uart_exit(&(m_modbus->modbus.link));
	
	FREE(m_modbus);
	*pptModbusCtx = NULL;
}

/*brief    关闭modbus主设备
 *param ： pptModbusCtx: 设备操作符 
 *return： 无
 */
__EX_API__ void wow_modbus_m_tcp_close(ModbusMaster_T** pptModbusCtx)
{
	CHECK_RET_VOID(pptModbusCtx && *pptModbusCtx);
	ModbusMaster_T* m_modbus = (ModbusMaster_T*)*pptModbusCtx;
    

    m_modbus->modbus.link->link_close(m_modbus->modbus.link);
	proto_port_eth_master_exit(&(m_modbus->modbus.link));
	
	FREE(m_modbus);
	*pptModbusCtx = NULL;
}

