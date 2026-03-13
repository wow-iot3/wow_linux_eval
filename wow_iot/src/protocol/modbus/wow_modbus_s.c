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

#include "system/wow_lock.h"
#include "system/wow_cond.h"
#include "system/wow_thread.h"
#include "protocol/modbus/wow_modbus_s.h"

extern ModbusCtx_T gt_modbusRtuCtx;
extern ModbusCtx_T gt_modbusTcpCtx;

typedef struct{
	pthread_idx_t	thread;
	uint16_t 		tflag;
	uint16_t 		bflag;
	mutex_lock_t 	lock;
	mutex_cond_t 	cond;
}__data_aligned__ ModbusSlaverThreadInfo_T ;

struct modbus_slaver_t{
	Modbus_T modbus;
	void*    priv;
	ModbusSlaverThreadInfo_T thread;
	ModbusSlaverDevCtx_T* ctx;
}__data_aligned__;

int response_exception(Modbus_T *modbus, sft_t *sft,int code)
{
    sft->function = sft->function + 0x80;
    int rsp_length = modbus->ctx->build_response_basis(sft, modbus->rsp);
    modbus->rsp[rsp_length++] = code;

    return rsp_length;

}

int modbus_s_reply(Modbus_T *modbus, uint8_t *req,int req_length)
{
	ModbusSlaver_T* s_modbus = (ModbusSlaver_T*)modbus;
	CHECK_RET_VAL_P(s_modbus && s_modbus->ctx,-1,"param input invalid!\n");

	int ret = MODBUS_ILLEGAL_DATA_VALUE;
    int offset = modbus->ctx->header_length;
    int slave  = req[offset - 1];
    int afn    = req[offset];
    uint16_t addr = (req[offset + 1] << 8) + req[offset + 2];
	int rsp_length = 0;
	
	sft_t sft;
    sft.slave = slave;
    sft.function = afn;
    sft.t_id = modbus->ctx->prepare_response_tid(req, &req_length);

	CHECK_RET_VAL_P(s_modbus->ctx && s_modbus->ctx->get_afn_addr_val,0,"get_afn_addr_val not find!\n");

    /* Data are flushed on illegal number of values errors. */
    switch (afn) {
    case MODBUS_READ_COILS:
    case MODBUS_READ_DISCRETE_INPUTS:
		{	
			int bulk   = (req[offset + 3] << 8) + req[offset + 4];
			CHECK_RET_GOTO_P(bulk >=1 && bulk <= MODBUS_MAX_READ_BITS,out,"modbus protocl not supoort len!\n");

	        rsp_length = modbus->ctx->build_response_basis(&sft, modbus->rsp);
	        modbus->rsp[rsp_length++] = Align8(bulk)/8;

			ret = s_modbus->ctx->get_afn_addr_val(s_modbus->priv,slave,afn,addr,modbus->rsp+rsp_length,bulk);
			CHECK_RET_GOTO_P(ret == 0,out,"get_afn_addr_val not find!\n");

			rsp_length += Align8(bulk)/8;
			rsp_length = modbus->ctx->send_msg_pre(modbus->rsp, rsp_length);
	    }
        break;
    case MODBUS_READ_HOLDING_REGISTERS:
    case MODBUS_READ_INPUT_REGISTERS:
		{
			int bulk   = (req[offset + 3] << 8) + req[offset + 4];
			CHECK_RET_GOTO_P(bulk >=1 && bulk <= MODBUS_MAX_READ_REGISTERS,out,"modbus protocl not supoort len!\n");

	        rsp_length = modbus->ctx->build_response_basis(&sft, modbus->rsp);
	        modbus->rsp[rsp_length++] = bulk << 1;

			ret = s_modbus->ctx->get_afn_addr_val(s_modbus->priv,slave,afn,addr,modbus->rsp+rsp_length,bulk);
			CHECK_RET_GOTO(ret == 0,out);

			rsp_length += bulk << 1;
			rsp_length = modbus->ctx->send_msg_pre(modbus->rsp, rsp_length);
   		}
        break;
    case MODBUS_WRITE_SINGLE_COIL: 
		{
			uint16_t data = (req[offset + 3] << 8) + req[offset + 4];
			CHECK_RET_GOTO_P(data == 0xFF00 || data == 0x0,out,"Illegal data value 0x%0X in write_bit request at address %0X\n",data, addr);

			uint8_t val = data ? ON : OFF;
			ret = s_modbus->ctx->set_afn_addr_val(s_modbus->priv,slave,afn,addr,&val,1);
			CHECK_RET_GOTO(ret == 0,out);

			memcpy(modbus->rsp, modbus->req, req_length);				
            rsp_length = req_length;     
    	}
        break;
    case MODBUS_WRITE_SINGLE_REGISTER: 
		{
			ret = s_modbus->ctx->set_afn_addr_val(s_modbus->priv,slave,afn,addr,req+offset+3,1);
			CHECK_RET_GOTO(ret == 0,out);
			
            memcpy(modbus->rsp, modbus->req, req_length);
			rsp_length = req_length;	 
   		}
        break;
    case MODBUS_WRITE_MULTIPLE_COILS: 
		{
	        int nb = (req[offset + 3] << 8) + req[offset + 4];
	        CHECK_RET_GOTO_P(nb >= 1 && nb <= MODBUS_MAX_WRITE_BITS,out,"modbus protocl not supoort len!\n");
			
			ret = s_modbus->ctx->set_afn_addr_val(s_modbus->priv,slave,afn,addr,req+offset+6,nb);
			CHECK_RET_GOTO(ret == 0,out);

			rsp_length = modbus->ctx->build_response_basis(&sft, modbus->rsp);
            memcpy(modbus->rsp + rsp_length, req + rsp_length, 4);
            rsp_length += 4;
			rsp_length = modbus->ctx->send_msg_pre(modbus->rsp, rsp_length);
    	}
        break;
    case MODBUS_WRITE_MULTIPLE_REGISTERS: 
		{
	        int bulk = (req[offset + 3] << 8) + req[offset + 4];
	        CHECK_RET_GOTO_P(bulk >=1 && bulk <= MODBUS_MAX_WRITE_REGISTERS,out,"modbus protocl not supoort len!\n");

			ret = s_modbus->ctx->set_afn_addr_val(s_modbus->priv,slave,afn,addr,req+offset+6,bulk);
			CHECK_RET_GOTO(ret == 0,out);

            rsp_length = modbus->ctx->build_response_basis(&sft, modbus->rsp);
            memcpy(modbus->rsp + rsp_length, req + rsp_length, 4);
            rsp_length += 4;
			rsp_length = modbus->ctx->send_msg_pre(modbus->rsp, rsp_length);
	    }
        break;
    default:
		ret = MODBUS_ILLEGAL_FUNCTION;
        goto out;
    }


    return rsp_length;  
out:
	CHECK_RET_VAL(ret != MODBUS_ERROR_MAX,0);

	rsp_length = response_exception(modbus,&sft,ret);
	rsp_length = modbus->ctx->send_msg_pre(modbus->rsp, rsp_length);
	return rsp_length;
	
}

/* Computes the length to read after the function received */
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

	CHECK_RET_VAL(try,-1);

	//wow_hex_print(data,msg_length);
	ret = modbus->ctx->check_integrity(data, msg_length);
	if(ret != 0){
		while(1){
			ret = modbus->link->link_recv(modbus->link,modbus->req,MAX_MESSAGE_LENGTH,10);
			if(ret <= 0) break;
		}	
		memset(modbus->req,0,MAX_MESSAGE_LENGTH);
		return WOW_SUCCESS;
	}
	
    return msg_length;
}

static void* modbus_s_exec_thread(void* arg)
{
	ModbusSlaver_T* s_modbus = (ModbusSlaver_T*)arg;
	CHECK_RET_VAL_P(s_modbus,NULL,"param input arg invalid!\n");

	int rlen = 0;

	while(s_modbus->thread.tflag){

		usleep(50*1000);
		
		wow_mutex_lock(&(s_modbus->modbus.mutex));
	
		rlen  = modbus_receive_msg(&(s_modbus->modbus), s_modbus->modbus.req, MSG_INDICATION);
		CHECK_RET_GOTO(rlen > 0, out);
		//wow_hex_print(s_modbus->modbus,req, rlen);
	
		rlen = modbus_s_reply(&(s_modbus->modbus),s_modbus->modbus.req,rlen);
		CHECK_RET_GOTO(rlen > 0, out);
	
		s_modbus->modbus.link->link_send(s_modbus->modbus.link, s_modbus->modbus.rsp, rlen,s_modbus->modbus.timeout);
		//wow_hex_print(s_modbus->modbus.rsp, rlen);
out:
		wow_mutex_unlock(&(s_modbus->modbus.mutex));
	}
	
	wow_mutex_lock(&s_modbus->thread.lock);
	wow_mutex_cond_signal(&s_modbus->thread.cond);
	wow_mutex_unlock(&s_modbus->thread.lock);

	return NULL;
}

/*brief    打开modbus-rtu从设备
 *param ： ptUartPara: 设备端硬件参数
 *param ： ptDevCtx  : 参数读取/设置回调函数 
 *param ： pArg:  回调函数私有变量 
 *return： 成功返回modbus操作符失败返回NULL
 */
__EX_API__ ModbusSlaver_T* wow_modbus_s_rtu_open(ProtoUartParam_T* ptUartPara,ModbusSlaverDevCtx_T* ptDevCtx,void* pArg)
{
	int ret = -1;
	
	CHECK_RET_VAL_ERRNO_P(ptUartPara,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");	
	
	ModbusSlaver_T *s_modbus = CALLOC(1,ModbusSlaver_T);
	CHECK_RET_VAL_ERRNO_P(s_modbus,-SYSTEM_MALLOC_FAILED,"malloc ModbusMaster_T failed!\n");
	
    s_modbus->modbus.ctx     = &gt_modbusRtuCtx;
	s_modbus->modbus.timeout = DEFAULT_RESPONSE_TIMEOUT;
	s_modbus->modbus.times   = DEFAULT_ERROR_TRY_TIMES;
	s_modbus->ctx            = ptDevCtx;
	s_modbus->priv           = pArg;
	
	wow_mutex_lock_init(&s_modbus->modbus.mutex);

	s_modbus->modbus.link = proto_port_uart_init(ptUartPara);
	CHECK_RET_GOTO_ERRNO_P(s_modbus->modbus.link, out1,-MODBUS_USART_INIT_ERROR, "proto_port_uart_init failed!\n");
	ret = s_modbus->modbus.link->link_open(s_modbus->modbus.link);
	CHECK_RET_GOTO_ERRNO_P(ret == 0, out2, -MODBUS_USART_OPEN_ERROR,"proto_port_uart link_open failed!\n");

	s_modbus->thread.tflag = 1;
	wow_mutex_lock_init(&s_modbus->thread.lock);
	wow_mutex_cond_init(&s_modbus->thread.cond);
	s_modbus->thread.thread  = wow_thread_create("modbus_s",modbus_s_exec_thread,s_modbus);
	CHECK_RET_GOTO_ERRNO_P(s_modbus->thread.thread, out2,-SYSTEM_THREAD_CREATE_FAILED, "wow_thread_create failed!\n");

    return (ModbusSlaver_T*)s_modbus;

out2:
	proto_port_uart_exit(&(s_modbus->modbus.link));
out1:
	FREE(s_modbus);
	return NULL;
}


/*brief    打开modbus-tcp从设备
 *param ： ptEthPara: 设备端硬件参数
 *param ： ptDevCtx : 参数读取/设置回调函数 
 *param ： pArg:  回调函数私有变量 
 *return： 成功返回modbus操作符失败返回NULL
 */
__EX_API__ ModbusSlaver_T* wow_modbus_s_tcp_open(ProtoEthParam_T* ptEthPara,ModbusSlaverDevCtx_T* ptDevCtx,void* pArg)
{
	CHECK_RET_VAL_ERRNO_P(ptEthPara,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid!\n");	

	int ret = 0;
	ModbusSlaver_T *s_modbus = CALLOC(1,ModbusSlaver_T);
	CHECK_RET_VAL_ERRNO_P(s_modbus,-SYSTEM_MALLOC_FAILED,"malloc ModbusMaster_T failed!\n");
	
	s_modbus->modbus.ctx	 = &gt_modbusTcpCtx;
	s_modbus->modbus.timeout = DEFAULT_RESPONSE_TIMEOUT;
	s_modbus->modbus.times	 = DEFAULT_ERROR_TRY_TIMES;
	s_modbus->ctx			 = ptDevCtx;
	s_modbus->priv           = pArg;

	wow_mutex_lock_init(&s_modbus->modbus.mutex);

	s_modbus->modbus.link = proto_port_eth_slave_init(ptEthPara);
	CHECK_RET_GOTO_ERRNO_P(s_modbus->modbus.link, out1, -MODBUS_ETH_INIT_ERROR,"proto_port_eth_master_init failed!\n");
	ret = s_modbus->modbus.link->link_open(s_modbus->modbus.link);
	CHECK_RET_GOTO_ERRNO_P(ret == 0, out2, -MODBUS_ETH_OPEN_ERROR, "proto_port_eth link_open failed!\n");

	s_modbus->thread.tflag = 1;
	wow_mutex_lock_init(&s_modbus->thread.lock);
	wow_mutex_cond_init(&s_modbus->thread.cond);
	s_modbus->thread.thread  = wow_thread_create("modbus_s",modbus_s_exec_thread,s_modbus);
	CHECK_RET_GOTO_ERRNO_P(s_modbus->thread.thread, out2,-SYSTEM_THREAD_CREATE_FAILED, "wow_thread_create failed!\n");

	return (ModbusSlaver_T*)s_modbus;
	
out2:
	proto_port_eth_slave_exit(&(s_modbus->modbus.link));
out1:
	FREE(s_modbus);
	return NULL;

}

/*brief    关闭modbus-rtu主设备
 *param ： pptModbusCtx: 设备操作符 
 *return： 无
 */
__EX_API__ void wow_modbus_s_rtu_close(ModbusSlaver_T** pptModbusCtx)
{
	CHECK_RET_VOID(pptModbusCtx && *pptModbusCtx);
	ModbusSlaver_T* s_modbus = (ModbusSlaver_T*)*pptModbusCtx;

	s_modbus->thread.tflag = 0;
	wow_thread_cancel(s_modbus->thread.thread);

	sleep(5);

	wow_mutex_lock_exit(&s_modbus->modbus.mutex);
    s_modbus->modbus.link->link_close(s_modbus->modbus.link);
	proto_port_uart_exit(&(s_modbus->modbus.link));
	
	FREE(*pptModbusCtx);
	*pptModbusCtx = NULL;
}

/*brief    关闭modbus-tcp从设备
 *param ： pptModbusCtx: 从设备操作符 
 *return： 无
 */
__EX_API__ void wow_modbus_s_tcp_close(ModbusSlaver_T** pptModbusCtx)
{
	CHECK_RET_VOID(pptModbusCtx && *pptModbusCtx);
	ModbusSlaver_T* s_modbus = (ModbusSlaver_T*)*pptModbusCtx;

	s_modbus->thread.tflag = 0;
	wow_thread_cancel(s_modbus->thread.thread);
	
	sleep(5);

	wow_mutex_lock_exit(&s_modbus->modbus.mutex);
    s_modbus->modbus.link->link_close(s_modbus->modbus.link);
	proto_port_eth_slave_exit(&(s_modbus->modbus.link));
	
	FREE(*pptModbusCtx);
	*pptModbusCtx = NULL;
}


