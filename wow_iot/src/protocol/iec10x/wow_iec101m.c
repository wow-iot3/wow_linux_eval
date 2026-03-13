#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>


#include "prefix/wow_check.h"
#include "prefix/wow_common.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_keyword.h"

#include "utils/wow_byte_array.h"
#include "plugin/wow_debug.h"
#include "plugin/wow_log.h"

#include "iec101_base.h"
#include "wow_iot_errno.h"
#include "protocol/iec10x/wow_iec101m.h"

#define TAG "IEC101M"

struct __data_aligned__ iec101m_t{
	uint16_t asdu_addr;
	uint16_t back01;
	uint32_t back02;
	Iec10xState_T 	   stat;
	Iec10xTime_T	   time;
	iec10x_set_value_func_t   val_cb;
	void*			   val_priv;
	iec10x_set_service_func_t serv_cb;
	void*			   serv_priv;
	ProtoPortCtx_T*    link_ctx;
	ByteArray_T*	   array;
	Iec10xThreadInfo_T thread;
	int			        tack;
	Iec101AsduBack_T    back;
};


static int iec101_m_build_request_user_ack(Iec101m_T* iec101m)
{
	static uint8_t fcb = IEC101_CTRL_FCB_OPPO_NONE;

	
	uint8_t  send_buf[IEC101_STABLE_LEN];
    Iec101Data10_T* Iec101Data = (Iec101Data10_T*)send_buf;
	fcb = ~fcb;

    Iec101Data->_begin = IEC101_STABLE_BEGING;

    Iec101Data->_ctrl.down._dir = IEC101_CTRL_DIR_UP;
    Iec101Data->_ctrl.down._prm = IEC101_CTRL_PRM_MASTER;
    Iec101Data->_ctrl.down._fcb = fcb;
    Iec101Data->_ctrl.down._fcv = IEC101_CTRL_FCV_ENABLE;
    Iec101Data->_ctrl.down._func = IEC101_CTRL_REQ_USR_DATA_1;

    Iec101Data->_addr = iec101m->asdu_addr;
    Iec101Data->_cs   = send_buf[1]+send_buf[2]+send_buf[3];
    Iec101Data->_end  = IEC101_STABLE_END;

    iec101m->link_ctx->link_send(iec101m->link_ctx,send_buf, IEC101_STABLE_LEN,IEC10X_SEND_TIMEOUT);

    return 0;
}

static int iec101_m_build_request_user_data(Iec101m_T* iec101m)
{
	static uint8_t fcb = IEC101_CTRL_FCB_OPPO_NONE;

	
	uint8_t  send_buf[IEC101_STABLE_LEN];
    Iec101Data10_T* Iec101Data = (Iec101Data10_T*)send_buf;
	fcb = ~fcb;

    Iec101Data->_begin = IEC101_STABLE_BEGING;

    Iec101Data->_ctrl.down._dir = IEC101_CTRL_DIR_UP;
    Iec101Data->_ctrl.down._prm = IEC101_CTRL_PRM_MASTER;
    Iec101Data->_ctrl.down._fcb = fcb;
    Iec101Data->_ctrl.down._fcv = IEC101_CTRL_FCV_ENABLE;
    Iec101Data->_ctrl.down._func = IEC101_CTRL_REQ_USR_DATA_2;

    Iec101Data->_addr = iec101m->asdu_addr;
    Iec101Data->_cs   = send_buf[1]+send_buf[2]+send_buf[3];
    Iec101Data->_end  = IEC101_STABLE_END;

    iec101m->link_ctx->link_send(iec101m->link_ctx,send_buf, IEC101_STABLE_LEN,IEC10X_SEND_TIMEOUT);

    return 0;
}

static int iec101_m_build_request_link_status(Iec101m_T* iec101m)
{
	uint8_t  send_buf[IEC101_STABLE_LEN];
    Iec101Data10_T* Iec10x = (Iec101Data10_T*)send_buf;

    Iec10x->_begin = IEC101_STABLE_BEGING;

    /*Ctrol*/
    Iec10x->_ctrl.down._dir = IEC101_CTRL_DIR_UP;
    Iec10x->_ctrl.down._prm = IEC101_CTRL_PRM_MASTER;
    Iec10x->_ctrl.down._fcb = IEC101_CTRL_FCB_OPPO_NONE;
    Iec10x->_ctrl.down._fcv = IEC101_CTRL_FCV_DISABLE;

    Iec10x->_ctrl.up._func = IEC101_CTRL_REQ_LINK_STATUS;

    Iec10x->_addr = iec101m->asdu_addr;
    Iec10x->_cs   = send_buf[1]+send_buf[2]+send_buf[3];
    Iec10x->_end  = IEC101_STABLE_END;

    iec101m->link_ctx->link_send(iec101m->link_ctx,send_buf, IEC101_STABLE_LEN,IEC10X_SEND_TIMEOUT);

    return 0;
}

static int iec101_m_build_reset_remote_link(Iec101m_T* iec101m)
{
	uint8_t  send_buf[IEC101_STABLE_LEN];
    Iec101Data10_T* Iec10x = (Iec101Data10_T*)send_buf;


    Iec10x->_begin = IEC101_STABLE_BEGING;

    /*Ctrol*/
    Iec10x->_ctrl.down._dir = IEC101_CTRL_DIR_UP;
    Iec10x->_ctrl.down._prm = IEC101_CTRL_PRM_MASTER;
    Iec10x->_ctrl.down._fcb = IEC101_CTRL_FCB_OPPO_NONE;
    Iec10x->_ctrl.down._fcv = IEC101_CTRL_FCV_DISABLE;

    Iec10x->_ctrl.up._func = IEC101_CTRL_RESET_LINK;

    Iec10x->_addr = iec101m->asdu_addr;
    Iec10x->_cs   = send_buf[1]+send_buf[2]+send_buf[3];
    Iec10x->_end  = IEC101_STABLE_END;

    iec101m->link_ctx->link_send(iec101m->link_ctx,send_buf, IEC101_STABLE_LEN,IEC10X_SEND_TIMEOUT);

    return 0;
}


static int iec101_m_asdu_yx(Iec101m_T* iec101m,Iec101Asdu_T* Iec10x_Asdu)
{
	CHECK_RET_VAL_P(iec101m && Iec10x_Asdu,-1,"param input invalid!\n");

	int i = 0;
    uint16_t addr = 0;
    DevNodeInfo_T node; 
	DevNodeVal_T   val;
    uint8_t *ptr = NULL;

	uint8_t type = Iec10x_Asdu->_type;
    uint8_t n = Iec10x_Asdu->_num._num;
	uint8_t Sq = Iec10x_Asdu->_num._sq;
	uint16_t reason = Iec10x_Asdu->_reason._reason;
    Iec101AsduInfo_T* asdu_info = (Iec101AsduInfo_T*)(Iec10x_Asdu->_info);
	
	CHECK_RET_VAL_P(reason == IEC10X_COT_PER_CYC || reason == IEC10X_COT_INTROGEN || reason == IEC10X_COT_SPONT,-1,
			"iec101_m_asdu_yt error reason(%d) \n",Iec10x_Asdu->_reason._reason);
	
    switch(type){
       	case IEC10X_M_SP_NA_1:
		case IEC10X_M_DP_NA_1:
			if(Sq == 1){
				memset(&node,0,sizeof(DevNodeVal_T));
				node.mode = DEV_TYPE_YX;
				node.type = DATA_SINT_8;
				ptr = asdu_info->_element;
				addr = asdu_info->_addr;
				for(i = 0; i < n; i++,ptr++){
					node.addr   = addr+i;
					val.val.u8 =*ptr;
					if(iec101m->val_cb){
						iec101m->val_cb(iec101m->val_priv,&node,&val);
					}
				}			
			}else if(Sq == 0){
				memset(&node,0,sizeof(DevNodeVal_T));
				node.mode = DEV_TYPE_YX;
				node.type = DATA_SINT_8;
				ptr = Iec10x_Asdu->_info;
				for(i = 0; i < n; i++,ptr++){
					memcpy(&addr, ptr, 2);
					ptr += 2;
					node.addr   = addr;
					val.val.u8 =*ptr;
					if(iec101m->val_cb){
						iec101m->val_cb(iec101m->val_priv,&node,&val);
					}
				}
			}
			break;
		case IEC10X_M_SP_TB_1://暂不处理时标
		case IEC10X_M_DP_TB_1:
			break;	
        default:
            ModuleErrorLog(TAG,"iec101_m_asdu_yx error Type(%d) \n",type);
            return -1;
    }

	iec101_m_build_request_user_ack(iec101m);
	
    return 0;
}

static int iec101_m_asdu_yc(Iec101m_T* iec101m,Iec101Asdu_T* Iec10x_Asdu)
{
	CHECK_RET_VAL_P(iec101m && Iec10x_Asdu,-1,"param input invalid!\n");

	int i = 0;
    uint16_t addr = 0;
    DevNodeInfo_T node;  
	DevNodeVal_T   val; 
    uint8_t *ptr = NULL;

	uint8_t type = Iec10x_Asdu->_type;
    uint8_t n = Iec10x_Asdu->_num._num;
	uint8_t Sq = Iec10x_Asdu->_num._sq;
	uint16_t reason = Iec10x_Asdu->_reason._reason;
    Iec101AsduInfo_T* asdu_info = (Iec101AsduInfo_T*)(Iec10x_Asdu->_info);
	
	CHECK_RET_VAL_P(reason == IEC10X_COT_PER_CYC || reason == IEC10X_COT_INTROGEN || reason == IEC10X_COT_SPONT,-1,
			"iec101_m_asdu_yt error reason(%d) \n",Iec10x_Asdu->_reason._reason);
	
	/* check info addrest */
    addr = asdu_info->_addr;
    switch(type){
		case IEC10X_M_ME_NA_1:
			if(Sq == 1){
				Iec101Detect_T* detect = NULL;
				ptr = asdu_info->_element;
				memset(&node,0,sizeof(DevNodeVal_T));
				node.mode = DEV_TYPE_YC;
				node.type = DATA_SINT_16;
				for(i=0; i<n; i++){
					detect = (Iec101Detect_T*)ptr;
					node.addr    = addr+i;
					val.val.s16 = detect->_detect;
					//qds 暂不处理
					if(iec101m->val_cb){
						iec101m->val_cb(iec101m->val_priv,&node,&val);
					}
					ptr += sizeof(Iec101Detect_T);
				}
			}else if(Sq == 0){
				Iec101DetectSq0_T* detect_Sq0 = NULL;
				ptr = Iec10x_Asdu->_info;
				memset(&node,0,sizeof(DevNodeVal_T));
				node.mode = DEV_TYPE_YC;
				node.type = DATA_SINT_16;
				for(i=0; i<n; i++){
					detect_Sq0 = (Iec101DetectSq0_T*)ptr;
					node.addr    = detect_Sq0->_addr;
					val.val.s16 = detect_Sq0->_detect;
					//qds 暂不处理
					if(iec101m->val_cb){
						iec101m->val_cb(iec101m->val_priv,&node,&val);
					}
					ptr += sizeof(Iec101DetectSq0_T);
				}
			}
			iec101m->stat.callall = IEC10X_STA_CALL_ALL_YC;
			break;
		case IEC10X_M_ME_NC_1:
			if(Sq == 1){
				Iec101DetectFloat_T* detect_f = NULL;
				ptr = asdu_info->_element;
				memset(&node,0,sizeof(DevNodeVal_T));
				node.mode = DEV_TYPE_YC;
				node.type = DATA_FLOAT;
				for(i=0; i<n; i++){
					detect_f = (Iec101DetectFloat_T*)ptr;
					node.addr    = addr+i;
					val.val.f = (float)detect_f->_detect;
					//qds 暂不处理
					if(iec101m->val_cb){
						iec101m->val_cb(iec101m->val_priv,&node,&val);
					}
					ptr += sizeof(Iec101DetectFloat_T);
				}
			}else if(Sq == 0){
				Iec101DetectSq0Float_T* detect_Sq0_f = NULL;
				ptr = Iec10x_Asdu->_info;
				memset(&node,0,sizeof(DevNodeVal_T));
				node.mode = DEV_TYPE_YC;
				node.type = DATA_FLOAT;
				for(i=0; i<n; i++){
					detect_Sq0_f = (Iec101DetectSq0Float_T*)ptr;
					node.addr = detect_Sq0_f->_addr;
					val.val.f = (float)(detect_Sq0_f->_detect);
					//qds 暂不处理
					if(iec101m->val_cb){
						iec101m->val_cb(iec101m->val_priv,&node,&val);
					}
					ptr += sizeof(Iec101DetectSq0Float_T); 
				}
			}
			break;
			
		case IEC10X_M_ME_TD_1://暂不处理时标
		case IEC10X_M_ME_TF_1:
			break;	
        default:
            ModuleErrorLog(TAG,"iec101_m_asdu_yc error Type(%d) \n",type);
            return -1;
    }
	
	iec101_m_build_request_user_ack(iec101m);
    return 0;
}

static int iec101_m_asdu_ym(Iec101m_T* iec101m,Iec101Asdu_T* Iec10x_Asdu)
{
	CHECK_RET_VAL_P(iec101m && Iec10x_Asdu,-1,"param input invalid!\n");

	int i = 0;
    uint16_t addr = 0;
    DevNodeInfo_T node;
	DevNodeVal_T  val;
    uint8_t *ptr = NULL;

	uint8_t type = Iec10x_Asdu->_type;
    uint8_t n = Iec10x_Asdu->_num._num;
	uint8_t Sq = Iec10x_Asdu->_num._sq;
	uint16_t reason = Iec10x_Asdu->_reason._reason;
    Iec101AsduInfo_T* asdu_info = (Iec101AsduInfo_T*)(Iec10x_Asdu->_info);
	
	CHECK_RET_VAL_P(reason == IEC10X_COT_REQ,-1,
			"iec101_m_asdu_yt error reason(%d) \n",Iec10x_Asdu->_reason._reason);
	
    switch(type){
		case IEC10X_M_IT_NA_1:
			if(Sq == 1){
				ptr = asdu_info->_element;
				addr = asdu_info->_addr;
				Iec101DetectInt32_T* detect = (Iec101DetectInt32_T*)ptr;
				memset(&node,0,sizeof(DevNodeVal_T));
				node.mode = DEV_TYPE_YM;
				node.type = DATA_SINT_32;
				for(i=0; i<n; i++){
					node.addr    = addr+i;
					val.val.s32 = detect->_detect;
					//qds 暂不处理
					if(iec101m->val_cb){
						iec101m->val_cb(iec101m->val_priv,&node,&val);
					}
					detect++;
				}
			}else if(Sq == 0){	
				ptr = Iec10x_Asdu->_info;
				Iec101DetectInt32Sq0_T* detect_Sq0 = (Iec101DetectInt32Sq0_T*)ptr;
				memset(&node,0,sizeof(DevNodeVal_T));
				node.mode = DEV_TYPE_YM;
				node.type = DATA_SINT_32;
				for(i=0; i<n; i++){
					val.val.s32 = detect_Sq0->_detect;
					node.addr    = detect_Sq0->_addr;
					//qds 暂不处理
					if(iec101m->val_cb){
						iec101m->val_cb(iec101m->val_priv,&node,&val);
					}	 
					detect_Sq0++; 
				}
			}
			break;
		case IEC10X_M_IT_TA_1://暂不处理时标
			break;	
        default:
            ModuleErrorLog(TAG,"iec101_m_asdu_ym error Type(%d) \n",type);
            return -1;
    }
	
	iec101_m_build_request_user_ack(iec101m);
    return 0;
}

static int iec101_m_build_yt_ack(Iec101m_T* iec101m,Iec101Asdu_T* Iec10x_Asdu)
{
	CHECK_RET_VAL_P(iec101m && Iec10x_Asdu,-1,"param input invalid!\n");

	int i = 0;
	uint8_t len = 0;
	uint8_t cs_temp = 0;
	uint8_t *ptr = NULL;
	uint8_t  send_buf[IEC101_MAX_BUF_LEN];

	CHECK_RET_VAL_P(iec101m,-1,"param input invalid!\n");
	CHECK_RET_VAL_P(iec101m->stat.init == IEC10X_STA_LINK_IDLE,-1,
					"iec101m status(%d) invalid!\n",iec101m->stat.init);

	uint8_t type = Iec10x_Asdu->_type;
	uint8_t n = Iec10x_Asdu->_num._num;
	uint8_t Sq = Iec10x_Asdu->_num._sq;
	Iec101AsduInfo_T* asdu_info = (Iec101AsduInfo_T*)(Iec10x_Asdu->_info);
		
	memset(send_buf,0,IEC101_MAX_BUF_LEN);
    Iec101Data68_T* Iec10x = (Iec101Data68_T*)send_buf;
    Iec101Asdu_T* asdu = (Iec101Asdu_T*)(Iec10x->_asdu);
    Iec101AsduInfo_T* info = (Iec101AsduInfo_T*)(asdu->_info);

	/*head*/
	Iec10x->_begin = Iec10x->_begin_cfm = IEC101_VARIABLE_BEGING;

	/*Ctrol*/
	Iec10x->_ctrl.down._dir = IEC101_CTRL_DIR_DOWN;
	Iec10x->_ctrl.down._prm = IEC101_CTRL_PRM_MASTER;
	Iec10x->_ctrl.down._fcb = IEC101_CTRL_FCB_OPPO_NONE;
	Iec10x->_ctrl.down._fcv = IEC101_CTRL_FCV_ENABLE;
	Iec10x->_ctrl.down._func = IEC101_CTRL_SEND_USR_DATA_ACK;
	
	Iec10x->_addr = iec101m->asdu_addr;

    /*build ASDU , COT ,Addr*/
    asdu->_type     = type;
    asdu->_num._sq  = Sq;
    asdu->_num._num = n;
    asdu->_reason._reason = IEC10X_COT_ACT;
    asdu->_addr = iec101m->asdu_addr;

    /*build info*/
	switch(type){
		case IEC10X_C_SE_NA_1:
			if(Sq == 1){   
				Iec101Detect_T* val = (Iec101Detect_T*)asdu_info->_element;
				for(i = 0; i < n; i++){
					val->_qds &= 0x7F;
					val++;
				}
				memcpy(info,asdu_info->_element,n*sizeof(Iec101Detect_T));
				ptr += n*sizeof(Iec101Detect_T);
			}else if(Sq == 0){
				Iec101DetectSq0_T* val = (Iec101DetectSq0_T*)asdu_info;
				for(i = 0; i < n; i++){
					val->_qds &= 0x7F;
					val++;
				}
				memcpy(info,asdu_info,n*sizeof(Iec101DetectSq0_T));
				ptr += n*sizeof(Iec101DetectSq0_T);	
			}
			break;
		case IEC10X_C_SE_NC_1:
			if(Sq == 1){
				Iec101DetectFloat_T* val = (Iec101DetectFloat_T*)asdu_info->_element;
				for(i = 0; i < n; i++){
					val->_qds &= 0x7F;
					val++;
				}
				memcpy(info,asdu_info->_element,n*sizeof(Iec101DetectFloat_T));
				ptr += n*sizeof(Iec101DetectFloat_T);
			}else if(Sq == 0){
				Iec101DetectSq0Float_T* val = (Iec101DetectSq0Float_T*)asdu_info;
				for(i = 0; i < n; i++){
					val->_qds &= 0x7F;
					val++;
				}
				memcpy(info,asdu_info,n*sizeof(Iec101DetectSq0Float_T));
				ptr += n*sizeof(Iec101DetectSq0Float_T);	
			}
			break;
		default:
			ModuleErrorLog(TAG,"iec101_m_asdu_yt error Type(%d) \n",type);
			return -1;
	}

    /*len*/
    len = ptr + 2 - send_buf;                    		/* add cs+end*/
    Iec10x->_len = Iec10x->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += send_buf[i];
    }
    send_buf[len-2] = cs_temp;
    send_buf[len-1] = IEC101_VARIABLE_END;

    
    /* enqueue to the transmisson queue */
    iec101m->link_ctx->link_send(iec101m->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);

    return 0;
}

static int iec101_m_asdu_yt(Iec101m_T* iec101m,Iec101Asdu_T* Iec10x_Asdu)
{
	CHECK_RET_VAL_P(iec101m && Iec10x_Asdu,-1,"param input invalid!\n");

	uint16_t  reason = Iec10x_Asdu->_reason._reason;
	if(Iec10x_Asdu->_reason._pn == 1){
		iec101m->stat.yt = IEC10X_YT_CMD_FAILD;
		return 0;
	}
	switch(reason){
		case IEC10X_COT_ACTCON://遥调执行
			iec101m->stat.yt = IEC10X_STA_YT_CMD_EXEC;
			iec101_m_build_yt_ack(iec101m,Iec10x_Asdu);
			break;
		case IEC10X_COT_ACTTERM:
			iec101m->stat.yt = IEC10X_STA_YT_CMD_SUCCESS;
			break;
		case IEC10X_COT_UNKNOW_TYPE:
		case IEC10X_COT_UNKNOW_COT:
		case IEC10X_COT_UNKNOW_ADDR:
		case IEC10X_COT_UNKNOW_INF:
		default:
			iec101m->stat.yt = IEC10X_YT_CMD_FAILD;
			break;
	}

    return 0;
}

static int iec101_m_build_yk_ack(Iec101m_T* iec101m,Iec101Asdu_T* Iec10x_Asdu)
{
	CHECK_RET_VAL_P(iec101m && Iec10x_Asdu,-1,"param input invalid!\n");

	int i = 0;
	uint8_t len = 0;
	uint8_t cs_temp = 0;
	uint8_t *ptr = NULL;
	uint8_t  send_buf[IEC101_MAX_BUF_LEN];
	
	CHECK_RET_VAL_P(iec101m,-1,"param input invalid!\n");
	CHECK_RET_VAL_P(iec101m->stat.init == IEC10X_STA_LINK_IDLE,-1,
					"iec101m status(%d) invalid!\n",iec101m->stat.init);

	uint8_t type = Iec10x_Asdu->_type;
	uint8_t n = Iec10x_Asdu->_num._num;
	uint8_t Sq = Iec10x_Asdu->_num._sq;
	Iec101AsduInfo_T* asdu_info = (Iec101AsduInfo_T*)(Iec10x_Asdu->_info);
		
	memset(send_buf,0,IEC101_MAX_BUF_LEN);
    Iec101Data68_T* Iec10x = (Iec101Data68_T*)send_buf;
    Iec101Asdu_T* asdu = (Iec101Asdu_T*)(Iec10x->_asdu);
    Iec101AsduInfo_T* info = (Iec101AsduInfo_T*)(asdu->_info);

	/*head*/
	Iec10x->_begin = Iec10x->_begin_cfm = IEC101_VARIABLE_BEGING;

	/*Ctrol*/
	Iec10x->_ctrl.down._dir = IEC101_CTRL_DIR_DOWN;
	Iec10x->_ctrl.down._prm = IEC101_CTRL_PRM_MASTER;
	Iec10x->_ctrl.down._fcb = IEC101_CTRL_FCB_OPPO_NONE;
	Iec10x->_ctrl.down._fcv = IEC101_CTRL_FCV_ENABLE;
	Iec10x->_ctrl.down._func = IEC101_CTRL_SEND_USR_DATA_ACK;
	
	Iec10x->_addr = iec101m->asdu_addr;

    /*build ASDU , COT ,Addr*/
    asdu->_type     = type;
    asdu->_num._sq  = Sq;
    asdu->_num._num = n;
    asdu->_reason._reason = IEC10X_COT_ACT;
    asdu->_addr = iec101m->asdu_addr;

    /*build info*/
	switch(type){
		case IEC10X_C_SC_NA_1:
		case IEC10X_C_DC_NA_1:
			if(Sq == 1){
				memcpy(info,asdu_info,n+ 2);
				ptr = info->_element;
				for(i = 0; i < n; i++){
					*ptr &=  0x7F;
					ptr++;
				}
			}else if(Sq == 0){
				memcpy(info,asdu_info,n*3);
				ptr = (uint8_t*)&(info->_addr);
				for(i = 0; i < n; i++){
					ptr += 2;
					*ptr &= 0x7F;
					ptr += 1;
				}
			}
			break;
		default:
			ModuleErrorLog(TAG,"iec101_m_asdu_yk error Type(%d) \n",type);
			return -1;
	}

    /*len*/
    len = ptr + 2 - send_buf;                    		/* add cs+end*/
    Iec10x->_len = Iec10x->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += send_buf[i];
    }
    send_buf[len-2] = cs_temp;
    send_buf[len-1] = IEC101_VARIABLE_END;

    /* enqueue to the transmisson queue */
    iec101m->link_ctx->link_send(iec101m->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);

    return 0;
}

static int iec101_m_asdu_yk(Iec101m_T* iec101m,Iec101Asdu_T* Iec10x_Asdu)
{
	CHECK_RET_VAL_P(iec101m && Iec10x_Asdu,-1,"param input invalid!\n");

	printf("iec101_m_asdu_yk pn:%d reason:%d\n",Iec10x_Asdu->_reason._pn,Iec10x_Asdu->_reason._reason);
	if(Iec10x_Asdu->_reason._pn == 1){
		iec101m->stat.yk = IEC10X_YK_CMD_FAILD;
		return 0;
	}
	
	switch(Iec10x_Asdu->_reason._reason){
		case IEC10X_COT_ACTCON:
			iec101m->stat.yk = IEC10X_STA_YK_CMD_EXEC;
			iec101_m_build_yk_ack(iec101m,Iec10x_Asdu);
			break;
		case IEC10X_COT_ACTTERM:
			iec101m->stat.yk = IEC10X_STA_YK_CMD_SUCCESS;
			break;
		case IEC10X_COT_UNKNOW_TYPE:
		case IEC10X_COT_UNKNOW_COT:
		case IEC10X_COT_UNKNOW_ADDR:
		case IEC10X_COT_UNKNOW_INF:
		default:
			iec101m->stat.yk = IEC10X_YK_CMD_FAILD;
			break;
	}

    return 0;
}

int iec101_m_asdu_call_all(Iec101m_T* iec101m,Iec101Asdu_T* Iec10x_Asdu)
{
    Iec101AsduInfo_T* asdu_info = (Iec101AsduInfo_T*)(Iec10x_Asdu->_info);
    uint8_t qoi = asdu_info->_element[0];

	CHECK_RET_VAL_P(asdu_info->_addr == 0,-1,"call cmd active error addr(%x)\n" ,asdu_info->_addr);

    switch(Iec10x_Asdu->_reason._reason){

        case IEC10X_COT_ACTCON:
            switch(qoi){
                case IEC10X_COT_INTROGEN:
					iec101m->stat.callall = IEC10X_STA_CALL_ALL_YX;
					iec101_m_build_request_user_ack(iec101m);
                    break;
                case IEC10X_COT_INTRO1:
                case IEC10X_COT_INTRO2:
                    break;
                default:
                	ModDebug_E(TAG,"invalid frame qoi(%d)!\n",qoi);
                    return -1;
            }
            break;
		case IEC10X_COT_ACTTERM:
			iec101m->stat.callall = IEC10X_STA_CALL_ALL_SUCCESS;
			break;
        default:
			ModDebug_E(TAG,"invalid frame reason(%d)!\n",Iec10x_Asdu->_reason._reason);
            break;
    }
    return 0;
}

int iec101_m_asdu_call_pulse(Iec101m_T* iec101m,Iec101Asdu_T* Iec10x_Asdu)
{
    Iec101AsduInfo_T* asdu_info = (Iec101AsduInfo_T*)(Iec10x_Asdu->_info);
    uint8_t qoi = asdu_info->_element[0];

	CHECK_RET_VAL_P(asdu_info->_addr == 0,-1,"call cmd active error addr(%x)\n" ,asdu_info->_addr);

    switch(Iec10x_Asdu->_reason._reason){

        case IEC10X_COT_ACTCON:
            switch(qoi){
                case IEC10X_COT_REQCOGEN:
					iec101m->stat.callpulse = IEC10X_STA_CALL_PULSE_YM;
					iec101_m_build_request_user_ack(iec101m);
                    break;
                case IEC10X_COT_REQCO1:
                case IEC10X_COT_REQCO2:
                    break;
                default:
                	ModDebug_E(TAG,"invalid frame qoi(%d)!\n",qoi);
                    return -1;
            }
            break;
		case IEC10X_COT_ACTTERM:
			iec101m->stat.callpulse = IEC10X_STA_CALL_PULSE_SUCCESS;
			break;
        default:
			ModDebug_E(TAG,"invalid frame reason(%d)!\n",Iec10x_Asdu->_reason._reason);
            break;
    }
    return 0;
}

int iec101_m_asdu_clock(Iec101m_T* iec101m,Iec101Asdu_T* Iec10x_Asdu)
{
	return 0;
}
int iec101_m_deal_10(Iec101m_T* iec101m,uint8_t *buf, uint16_t len)
{
    uint8_t cfun, sta,i;
    uint8_t cs_temp = 0;
    Iec101Data10_T* Iec10x_10 = (Iec101Data10_T*)buf;

    /* check check_sum*/
    for(i=1; i<len-2; i++){
        cs_temp += buf[i];
    }
	CHECK_RET_VAL_P(Iec10x_10->_cs == cs_temp, -1,"check sum error(%x) \n",cs_temp);
	CHECK_RET_VAL_P(Iec10x_10->_end == 0x16, -1,"iec10x_10 end error(%d) \n",Iec10x_10->_end);

    /*deal the function code*/
    cfun = Iec10x_10->_ctrl.down._func;
    sta = Iec10x_10->_ctrl.down._prm;

    if(sta == IEC101_CTRL_PRM_MASTER){
        switch(cfun){

            case IEC101_CTRL_RESET_LINK:
                ModDebug_D(TAG,"frame function reset link!\n");
                break;
            case IEC101_CTRL_SEND_DATA:
				ModDebug_D(TAG,"frame send data!\n");
                break;
            case IEC101_CTRL_REQ_LINK_STATUS:
				ModDebug_D(TAG,"frame request link status!\n");
                break;
            default:
				ModDebug_E(TAG,"invalid down frame(%d)!\n",cfun);
                break;
        }
    }else if(sta == IEC101_CTRL_PRM_SLAVE){

        switch(cfun){
            case IEC101_CTRL_RES_LINK_STATUS:
				ModDebug_D(TAG,"frame function Respon link status!\n");
				iec101_m_build_reset_remote_link(iec101m);
				iec101m->stat.init = IEC10X_STA_LINK_CONNECT_OK;
                break;
            case IEC101_CTRL_CONFIRM_ACK:
				ModDebug_D(TAG,"frame function ack confirm!\n");
				iec101m->tack = 1;
                break;
			case IEC101_CTRL_RES_USR_DATA_NACK:
				ModDebug_D(TAG,"frame function nack confirm!\n");
				break;
            default:
                ModDebug_E(TAG,"invalid up frame(%d)!\n",cfun);
                break;
        }
    }

    return 0;
}

int iec101_m_deal_68(Iec101m_T* iec101m,uint8_t *buf, uint16_t len)
{
    uint8_t cfun,i;
    uint8_t cs_temp = 0;
    Iec101Data68_T* Iec10x_68 = (Iec101Data68_T*)buf;
    Iec101Asdu_T* Iec10x_Asdu = (Iec101Asdu_T*)Iec10x_68->_asdu;

    /* check check_sum*/
    for(i=4; i<len-2; i++){
        cs_temp += buf[i];
    }
	CHECK_RET_VAL_P(buf[len-2] == cs_temp,-1,"iec10x_68 check sum error (%d)(%d)\n",buf[len-2],cs_temp);
	CHECK_RET_VAL_P(buf[len-1] == IEC101_VARIABLE_END,-1,"iec10x_68 end error(%d)\n",buf[len-1]);
	CHECK_RET_VAL_P(Iec10x_68->_len+6 == len,-1,"iec10x_68 rec len error(%d)(%d)\n",Iec10x_68->_len, len);
	CHECK_RET_VAL_P(Iec10x_68->_len == Iec10x_68->_len_cfm,-1,"iec10x_68 len error(%d)(%d)\n",Iec10x_68->_len, Iec10x_68->_len_cfm);

    /*deal the function code*/
    cfun = Iec10x_68->_ctrl.down._func;
    //sta = Iec10x_68->_ctrl.down._prm;

    switch(cfun){
		case IEC101_CTRL_SEND_USR_DATA_ACK:
			switch(Iec10x_Asdu->_type){
		        case IEC10X_C_CS_NA_1:
					ModDebug_D(TAG,"frame asdu type clock cmd...!\n");
		            iec101_m_asdu_clock(iec101m,Iec10x_Asdu);
		            break;
				default:
					ModDebug_E(TAG,"invalid ack frame type(%d)!\n",Iec10x_Asdu->_type);
                    break;
            }
        case IEC101_CTRL_RES_USR_DATA_ACK:
            switch(Iec10x_Asdu->_type){
				case IEC10X_M_SP_NA_1:
				case IEC10X_M_DP_NA_1:
				case IEC10X_M_SP_TB_1:
				case IEC10X_M_DP_TB_1:	
					ModDebug_D(TAG,"frame asdu type yx cmd...!\n");
					iec101_m_asdu_yx(iec101m,Iec10x_Asdu);
					break;
				case IEC10X_M_ME_NA_1:
				case IEC10X_M_ME_NC_1:
				case IEC10X_M_ME_TD_1:
				case IEC10X_M_ME_TF_1:
					ModDebug_D(TAG,"frame asdu type yc cmd...!\n");
					iec101_m_asdu_yc(iec101m,Iec10x_Asdu);
					break;
				case IEC10X_M_IT_NA_1:
				case IEC10X_M_IT_TA_1:
					ModDebug_D(TAG,"frame asdu type ym cmd...!\n");
					iec101_m_asdu_ym(iec101m,Iec10x_Asdu);
                case IEC10X_C_IC_NA_1:
					ModDebug_D(TAG,"frame asdu type call cmd...!\n");
                    iec101_m_asdu_call_all(iec101m,Iec10x_Asdu);
                    break;
				case IEC10X_C_CI_NA_1:
					ModDebug_D(TAG,"frame asdu type pulse cmd...!\n");
                    iec101_m_asdu_call_pulse(iec101m,Iec10x_Asdu);
					break;
				case IEC10X_C_SC_NA_1:
				case IEC10X_C_DC_NA_1:
					ModDebug_D(TAG,"frame asdu type yk cmd...!\n");
					iec101_m_asdu_yk(iec101m,Iec10x_Asdu);
					break;
				case IEC10X_C_SE_NA_1:
				case IEC10X_C_SE_NC_1:
					ModDebug_D(TAG,"frame asdu type yk cmd...!\n");
					iec101_m_asdu_yt(iec101m,Iec10x_Asdu);
					break;
                default:
					ModDebug_E(TAG,"invalid ack frame type(%d)!\n",Iec10x_Asdu->_type);
                    break;
            }
            break;
        default:
			ModDebug_E(TAG,"invalid frame function code(%d)!\n",cfun);
            break;
    }

    return 0;
}


static void iec101_m_reset_param(Iec101m_T* iec101m)
{
	CHECK_RET_VOID_P(iec101m,"param input arg invalid!\n");

	iec101m->tack = 0;
	
	iec101m->stat.init    = IEC10X_STA_LINK_INIT;
	iec101m->stat.callall = IEC10X_STA_CALL_ALL_IDLE;
	iec101m->stat.callpulse = IEC10X_STA_CALL_PULSE_IDLE;
	iec101m->stat.yt      = IEC10X_STA_YT_CMD_IDLE;
	iec101m->stat.yk	  = IEC10X_STA_YK_CMD_IDLE;
	iec101m->stat.test    = IE10X_STA_TESTER_START;
}

static void iec101_m_state_machine_work(Iec101m_T* iec101m)
{
	switch(iec101m->stat.init){
		case IEC10X_STA_LINK_INIT:
			iec101_m_reset_param(iec101m);
			iec101m->stat.init = IEC10X_STA_LINK_OPEN;
			break;
		case IEC10X_STA_LINK_OPEN:
			iec101_m_build_request_link_status(iec101m);
			iec101m->stat.init = IEC10X_STA_LINK_CONNECT;
			break;
		case IEC10X_STA_LINK_CONNECT:
			break;
		case IEC10X_STA_LINK_CONNECT_OK:
			if(iec101m->tack == 1){
				iec101m->tack = 0;
				if(iec101m->serv_cb){
					iec101m->serv_cb(iec101m->val_priv,IEC10X_CONNECTED,NULL);
				}
				iec101m->stat.init = IEC10X_STA_LINK_IDLE;
			}
			break;
		case IEC10X_STA_LINK_DISCONNECT:
			iec101m->stat.init = IEC10X_STA_LINK_INIT;
			iec101m->link_ctx->link_close(iec101m->link_ctx);
			if(iec101m->serv_cb){
				iec101m->serv_cb(iec101m->val_priv,IEC10X_DISCONNECTED,NULL);
			}		
			break;
		case IEC10X_STA_LINK_CLOSE:
			iec101m->stat.init = IEC10X_STA_LINK_INIT;
			break;
		case IEC10X_STA_LINK_IDLE:
			if(wow_time_get_msec() - iec101m->time.init > IEC101_HEARTBEART_TIMEOUT){
				iec101_m_build_request_user_data(iec101m);
				iec101m->time.init = wow_time_get_msec();
			}
			break;
		default:
			break;
	}
}


static void iec101_m_state_machine_callall(Iec101m_T* iec101m)
{
	switch(iec101m->stat.callall){
		case IEC10X_STA_CALL_ALL_IDLE:
			break;
		case IEC10X_STA_CALL_ALL_START:
			iec101m->time.callall = wow_time_get_msec();
			iec101m->stat.callall = IEC10X_STA_CALL_ALL_CONFIRM;
			break;
		case IEC10X_STA_CALL_ALL_CONFIRM:
			if(iec101m->tack == 1){
				iec101_m_build_request_user_ack(iec101m);
				iec101m->tack = 0;
			}
			if(wow_time_get_msec() - iec101m->time.callall > IEC10X_CALL_ALL_TIMEOUT){
				iec101m->stat.callall = IEC10X_CALL_ALL_FAILD;
			}
			break;
		case IEC10X_STA_CALL_ALL_YX:
		case IEC10X_STA_CALL_ALL_YC:
			if(wow_time_get_msec() - iec101m->time.callall > IEC10X_CALL_ALL_TIMEOUT){
				iec101m->stat.callall = IEC10X_CALL_ALL_FAILD;
			}
			break;
		case IEC10X_STA_CALL_ALL_SUCCESS:
			iec101_m_build_request_user_ack(iec101m);
			iec101m->stat.callall = IEC10X_STA_CALL_ALL_IDLE;	
			if(iec101m->serv_cb){
				iec101m->serv_cb(iec101m->val_priv,IEC10X_CALL_ALL_SUCCESS,NULL);
			}
			break;
		case IEC10X_STA_CALL_ALL_FAILD:
			iec101m->stat.callall = IEC10X_STA_CALL_ALL_IDLE;
			if(iec101m->serv_cb){
				iec101m->serv_cb(iec101m->val_priv,IEC10X_CALL_ALL_FAILD,NULL);
			}
			break;
	}

}


static void iec101_m_state_machine_callpulse(Iec101m_T* iec101m)
{
	switch(iec101m->stat.callpulse){
		case IEC10X_STA_CALL_PULSE_IDLE:
			break;
		case IEC10X_STA_CALL_PULSE_START:
			iec101m->time.callpulse = wow_time_get_msec();
			iec101m->stat.callpulse = IEC10X_STA_CALL_PULSE_CONFIRM;
			break;
		case IEC10X_STA_CALL_PULSE_CONFIRM:
			if(iec101m->tack == 1){
				iec101_m_build_request_user_ack(iec101m);
				iec101m->tack = 0;
			}
			if(wow_time_get_msec() - iec101m->time.callpulse > IEC10X_CALL_PULSE_TIMEOUT){
				iec101m->stat.callpulse = IEC10X_CALL_PULSE_FAILD;
			}
			break;
		case IEC10X_STA_CALL_PULSE_YM:
			if(wow_time_get_msec() - iec101m->time.callpulse > IEC10X_CALL_PULSE_TIMEOUT){
				iec101m->stat.callpulse = IEC10X_CALL_PULSE_FAILD;
			}
			break;
		case IEC10X_STA_CALL_PULSE_SUCCESS:
			iec101_m_build_request_user_ack(iec101m);
			iec101m->stat.callpulse = IEC10X_STA_CALL_PULSE_IDLE;
			if(iec101m->serv_cb){
				iec101m->serv_cb(iec101m->val_priv,IEC10X_CALL_PULSE_SUCCESS,NULL);
			}
			iec101m->time.init = wow_time_get_msec();
			break;
		case IEC10X_STA_CALL_PULSE_FAILD:
			iec101m->stat.callpulse = IEC10X_STA_CALL_PULSE_IDLE;
			if(iec101m->serv_cb){
				iec101m->serv_cb(iec101m->val_priv,IEC10X_CALL_PULSE_FAILD,NULL);
			}
			break;
	}

}

static void iec101_m_state_machine_yt(Iec101m_T* iec101m)
{
	
	switch(iec101m->stat.yt){
		case IEC10X_STA_YT_CMD_IDLE:
			break;
		case IEC10X_STA_YT_CMD_START:
			iec101m->time.yt = wow_time_get_msec();
			iec101m->stat.yt = IEC10X_STA_YT_CMD_CONFIRM;
			break;
		case IEC10X_STA_YT_CMD_CONFIRM:
			if(iec101m->tack == 1){
				iec101_m_build_request_user_ack(iec101m);
				iec101m->tack = 0;
			}
			if(wow_time_get_msec() - iec101m->time.yt > IEC10X_YT_CMD_TIMEOUT){
				iec101m->stat.yt = IEC10X_STA_YT_CMD_FAILD;
			}
			break;
		case IEC10X_STA_YT_CMD_EXEC:
			if(iec101m->tack == 1){
				iec101_m_build_request_user_ack(iec101m);
				iec101m->tack = 0;
			}
			if(wow_time_get_msec() - iec101m->time.yt > IEC10X_YT_CMD_TIMEOUT){
				iec101m->stat.yt = IEC10X_STA_YT_CMD_FAILD;
			}
			break;
		case IEC10X_STA_YT_CMD_SUCCESS:
			iec101m->stat.yt = IEC10X_STA_YT_CMD_IDLE;
			if(iec101m->serv_cb){
				iec101m->serv_cb(iec101m->val_priv,IEC10X_YT_CMD_SUCCESS,NULL);
			}
			break;
		case IEC10X_STA_YT_CMD_FAILD:
			iec101m->stat.yt = IEC10X_STA_YT_CMD_IDLE;	
			if(iec101m->serv_cb){
				iec101m->serv_cb(iec101m->val_priv,IEC10X_YT_CMD_FAILD,NULL);
			}
			break;
	}

}

static void iec101_m_state_machine_yk(Iec101m_T* iec101m)
{
	switch(iec101m->stat.yk){
		case IEC10X_STA_YK_CMD_IDLE:
			break;
		case IEC10X_STA_YK_CMD_START:
			iec101m->time.yk = wow_time_get_msec();
			iec101m->stat.yk = IEC10X_STA_YK_CMD_CONFIRM;
			break;
		case IEC10X_STA_YK_CMD_CONFIRM:
			if(iec101m->tack == 1){
				iec101_m_build_request_user_ack(iec101m);
				iec101m->tack = 0;
			}
			if(wow_time_get_msec() - iec101m->time.yk > IEC10X_YK_CMD_TIMEOUT){
				iec101m->stat.yk = IEC10X_STA_YK_CMD_FAILD;
			}
			break;
		case IEC10X_STA_YK_CMD_EXEC:
			if(iec101m->tack == 1){
				iec101_m_build_request_user_ack(iec101m);
				iec101m->tack = 0;
			}
			if(wow_time_get_msec() - iec101m->time.yk > IEC10X_YK_CMD_TIMEOUT){
				iec101m->stat.yk = IEC10X_STA_YK_CMD_FAILD;
			}
			break;
		case IEC10X_STA_YK_CMD_SUCCESS:
			iec101m->stat.yk = IEC10X_STA_YK_CMD_IDLE;	
			if(iec101m->serv_cb){
				iec101m->serv_cb(iec101m->val_priv,IEC10X_YK_CMD_SUCCESS,NULL);
			}
			break;
		case IEC10X_STA_YK_CMD_FAILD:
			iec101m->stat.yk = IEC10X_STA_YK_CMD_IDLE;	
			if(iec101m->serv_cb){
				iec101m->serv_cb(iec101m->val_priv,IEC10X_YK_CMD_FAILD,NULL);
			}
			break;
	}

}

static void iec101_m_state_machine(Iec101m_T* iec101m)
{	
	iec101_m_state_machine_work(iec101m);
	
	if(iec101m->stat.init != IEC10X_STA_LINK_IDLE) return;
	
	iec101_m_state_machine_callall(iec101m);
	iec101_m_state_machine_callpulse(iec101m);
	iec101_m_state_machine_yt(iec101m);
	iec101_m_state_machine_yk(iec101m);

}

static  void iec101_m_frame_parse(Iec101m_T* iec101m)
{
	int index_10 = -1;
	int index_68 = -1;

	index_10 = wow_byte_array_find(iec101m->array,IEC101_STABLE_BEGING);
	index_68 = wow_byte_array_find(iec101m->array,IEC101_VARIABLE_BEGING);

	CHECK_RET_VOID(index_10 >= 0 || index_68 >= 0);

	if(index_10 < 0 && index_68 >= 0){
exec_68:		
		wow_byte_array_remove(iec101m->array,0,index_68);
		CHECK_RET_VOID(wow_byte_array_size(iec101m->array) >= IEC101_STABLE_LEN);
		
		uint8_t* data = wow_byte_array_data(iec101m->array);
		uint8_t tlen = data[1];
		CHECK_RET_VOID(wow_byte_array_size(iec101m->array) >= tlen + IEC101_STABLE_LEN);
		
		if(data[tlen + IEC101_STABLE_LEN -1] == IEC101_VARIABLE_END){
			iec101_m_deal_68(iec101m,data ,tlen + IEC101_STABLE_LEN);
			wow_byte_array_remove(iec101m->array,0,tlen + IEC101_STABLE_LEN);
		}else{
			wow_byte_array_remove(iec101m->array,0,1);
		}
						
	}else if(index_10 >= 0  && index_68 < 0){
exec_10:	
		wow_byte_array_remove(iec101m->array,0,index_10);
		CHECK_RET_VOID(wow_byte_array_size(iec101m->array) >= IEC101_STABLE_LEN);
		uint8_t* data = wow_byte_array_data(iec101m->array);
		
		if(data[IEC101_STABLE_LEN -1] == IEC101_VARIABLE_END){
			iec101_m_deal_10(iec101m,data,IEC101_STABLE_LEN);
			wow_byte_array_remove(iec101m->array,0,IEC101_STABLE_LEN);
		}else{
			wow_byte_array_remove(iec101m->array,0,1);
		}
	}else if(index_10 >= 0  && index_68 >= 0){
		if(index_10 < index_68){
			goto exec_10;
		}else{
			goto exec_68;
		}
	}else{
		return;
	}

}



static void* iec101_m_exec_thread(void* arg)
{
	Iec101m_T* iec101m = (Iec101m_T*)arg;
	CHECK_RET_VAL_P(iec101m,NULL,"param input arg invalid!\n");


	int rlen = 0;
	uint8_t recv_buf[1500]={0};

	while(iec101m->thread.tflag){
		rlen = iec101m->link_ctx->link_recv(iec101m->link_ctx,recv_buf,1500,IEC10X_RECV_TIMEOUT);
		if(rlen > 0){
			wow_byte_array_append_ndata(iec101m->array,recv_buf,rlen);
		}
		
		iec101_m_frame_parse(iec101m);


		iec101_m_state_machine(iec101m);
	}
	
	wow_mutex_lock(&iec101m->thread.lock);
	wow_mutex_cond_signal(&iec101m->thread.cond);
	wow_mutex_unlock(&iec101m->thread.lock);

	return NULL;
}


/*brief    Iec101主设备初始化
 *param ： u16Addr: 公共地址 
 *param ： ptUartPar:  硬件参数
 *return： 成功返回iec04m操作标识 失败返回NULL
 */
__EX_API__ Iec101m_T* wow_iec101_m_init(uint16_t u16Addr,ProtoUartParam_T* ptUartPar)
{
	CHECK_RET_VAL_ERRNO_P(ptUartPar,-PARAM_INPUT_DATA_IS_NULL, "param input data invalid!\n");
	
	Iec101m_T* iec101m = CALLOC(1, Iec101m_T);
	CHECK_RET_VAL_ERRNO_P(iec101m,-SYSTEM_MALLOC_FAILED,"malloc iec101m faild!\n");
	
	iec101m->array = wow_byte_array_new(2048);
	CHECK_RET_GOTO_ERRNO_P(iec101m->array,out1,-BYTE_ARRAY_CREATE_ERRNOR,"wow_byte_array_new faild!\n");
	
	/*参数初始化*/
	iec101_m_reset_param(iec101m);
	iec101m->asdu_addr = u16Addr;
	
	/*建立连接*/
	iec101m->link_ctx = proto_port_uart_init(ptUartPar);
	CHECK_RET_GOTO_ERRNO_P(iec101m->link_ctx,out2,-PROTO_UART_INIT_ERROR,"iec10x port init faild!\n");
	
	/*IEC101状态机*/
	iec101m->thread.tflag   = 1;
	wow_mutex_lock_init(&iec101m->thread.lock);
	wow_mutex_cond_init(&iec101m->thread.cond);

	return iec101m;
out2:
	wow_byte_array_free(&iec101m->array);	
out1:
	FREE(iec101m);
	return NULL;
}


/*brief    退出Iec101主设备
 *param ： pptIec101m:iec04m操作标识
 *return： 无
 */
__EX_API__ void wow_iec101_m_exit(Iec101m_T** pptIec101m)
{
	CHECK_RET_VOID(pptIec101m && *pptIec101m);

	Iec101m_T* iec101m = (Iec101m_T*)*pptIec101m;
		
	//安全退出线程
	wow_mutex_lock(&iec101m->thread.lock);
	iec101m->thread.tflag= 0;
	wow_mutex_cond_wait(&iec101m->thread.lock,&iec101m->thread.cond,-1);
	wow_mutex_unlock(&iec101m->thread.lock);

	proto_port_uart_exit(&iec101m->link_ctx);

	wow_byte_array_free(&iec101m->array);

	FREE(iec101m);
	*pptIec101m  = NULL;
}

/*brief    启动Iec101主设备
 *param ： pptIec101m:iec04m操作标识
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_iec101_m_start(Iec101m_T* ptIec101m)
{
	CHECK_RET_VAL_P(ptIec101m,-PARAM_INPUT_STRUCT_IS_NULL,"param input strcuct invalid!\n");

	int ret = ptIec101m->link_ctx->link_open(ptIec101m->link_ctx);
	CHECK_RET_VAL_P(ret == 0,-PROTO_LINK_OPEN_ERROR,"link_open failed!\n");

	ptIec101m->thread.thread	= wow_thread_create("iec101m",iec101_m_exec_thread,ptIec101m);
	CHECK_RET_VAL_P(ptIec101m->thread.thread,-THREAD_CREATE_FAILED,"wow_thread_create failed!\n");

	return 0;
}

/*brief    Iec101设置更新数据节点参数回调函数
 *param ： ptIec101m:iec04m操作标识
 *param ： fCallBack:   回调函数 
 *param ： pArg:  回调函数私有变量
 *return： 成功返回0 失败返回<0
 */
int wow_iec101_m_set_value_handle(Iec101m_T* ptIec101m,iec10x_set_value_func_t fCallBack,void* pArg)
{
	CHECK_RET_VAL_P(ptIec101m,-PARAM_INPUT_STRUCT_IS_NULL,"param input strcuct invalid!\n");
	CHECK_RET_VAL_P(fCallBack,-PARAM_INPUT_FUNC_IS_NULL,"param input func invalid!\n");

	ptIec101m->val_cb   = fCallBack;
	ptIec101m->val_priv = pArg;

	return 0;
}

/*brief    Iec101设置更新服务状态回调函数
 *param ： ptIec101m:iec04m操作标识
 *param ： fCallBack:   回调函数 
 *param ： priv:  回调函数私有变量
 *return： 成功返回0 失败返回<0
 */
int wow_iec101_m_set_service_handle(Iec101m_T* ptIec101m,iec10x_set_service_func_t fCallBack,void* pArg)
{
	CHECK_RET_VAL_P(ptIec101m,-PARAM_INPUT_STRUCT_IS_NULL,"param input strcuct invalid!\n");
	CHECK_RET_VAL_P(fCallBack,-PARAM_INPUT_FUNC_IS_NULL,"param input func invalid!\n");

	ptIec101m->serv_cb   = fCallBack;
	ptIec101m->serv_priv = pArg;

	return 0;
}

/*brief    iec04总召
 *param ： ptIec101m:iec04m操作标识
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_iec101_m_build_call_all(Iec101m_T* ptIec101m)
{
 	uint16_t len = IEC101_STABLE_LEN;
    uint8_t cs_temp = 0,i;
	uint8_t  send_buf[IEC101_MAX_BUF_LEN];

	CHECK_RET_VAL_P(ptIec101m,-PARAM_INPUT_STRUCT_IS_NULL,"param input strcuct invalid!\n");
	
    Iec101Data68_T* Iec10x = (Iec101Data68_T*)send_buf;
    Iec101Asdu_T* asdu = (Iec101Asdu_T*)(Iec10x->_asdu);
    Iec101AsduInfo_T* info = (Iec101AsduInfo_T*)(asdu->_info);

    /*head*/
    Iec10x->_begin = Iec10x->_begin_cfm = IEC101_VARIABLE_BEGING;

    /*Ctrol*/
    Iec10x->_ctrl.down._dir = IEC101_CTRL_DIR_DOWN;
    Iec10x->_ctrl.down._prm = IEC101_CTRL_PRM_MASTER;
    Iec10x->_ctrl.down._fcb = IEC101_CTRL_FCB_OPPO_NONE;
    Iec10x->_ctrl.down._fcv = IEC101_CTRL_FCV_ENABLE;

    Iec10x->_ctrl.down._func = IEC101_CTRL_SEND_USR_DATA_ACK;

    Iec10x->_addr = ptIec101m->asdu_addr;
    /*asdu*/
    asdu->_type = IEC10X_C_IC_NA_1;
    asdu->_num._num = 1;
    asdu->_reason._reason = IEC10X_COT_ACT;
    asdu->_addr = ptIec101m->asdu_addr;
    /*info*/
    info->_addr = IEC10X_INFO_ADDR_NONE;
    info->_element[0] = IEC10X_COT_INTROGEN;

    /*len*/
    len = IEC101_VARIABLE_LEN + asdu->_num._num;
    Iec10x->_len = Iec10x->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += send_buf[i];
    }
    send_buf[len-2] = cs_temp;
    send_buf[len-1] = IEC101_VARIABLE_END;
	
    /* enqueue to the transmisson queue */
	ptIec101m->link_ctx->link_send(ptIec101m->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);

	ptIec101m->stat.callall = IEC10X_STA_CALL_ALL_START;

    return 0;
}

/*brief    iec04电度量总召
 *param ： ptIec101m:iec04m操作标识
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_iec101_m_build_call_pulse(Iec101m_T* ptIec101m)
{
	uint16_t len = IEC101_STABLE_LEN;
	uint8_t cs_temp = 0,i;
	uint8_t  send_buf[IEC101_MAX_BUF_LEN];

	CHECK_RET_VAL_P(ptIec101m,-PARAM_INPUT_STRUCT_IS_NULL,"param input strcuct invalid!\n");
	
	Iec101Data68_T* Iec10x = (Iec101Data68_T*)send_buf;
	Iec101Asdu_T* asdu = (Iec101Asdu_T*)(Iec10x->_asdu);
	Iec101AsduInfo_T* info = (Iec101AsduInfo_T*)(asdu->_info);

	/*head*/
	Iec10x->_begin = Iec10x->_begin_cfm = IEC101_VARIABLE_BEGING;

	/*Ctrol*/
	Iec10x->_ctrl.down._dir = IEC101_CTRL_DIR_DOWN;
	Iec10x->_ctrl.down._prm = IEC101_CTRL_PRM_MASTER;
	Iec10x->_ctrl.down._fcb = IEC101_CTRL_FCB_OPPO_NONE;
	Iec10x->_ctrl.down._fcv = IEC101_CTRL_FCV_ENABLE;

	Iec10x->_ctrl.down._func = IEC101_CTRL_SEND_USR_DATA_ACK;

	Iec10x->_addr = ptIec101m->asdu_addr;
	/*asdu*/
	asdu->_type = IEC10X_C_CI_NA_1;
	asdu->_num._num = 1;
	asdu->_reason._reason = IEC10X_COT_ACT;
	asdu->_addr = ptIec101m->asdu_addr;
	/*info*/
	info->_addr = IEC10X_INFO_ADDR_NONE;
	info->_element[0] = IEC10X_COT_REQCOGEN;

	/*len*/
	len = IEC101_VARIABLE_LEN + asdu->_num._num;
	Iec10x->_len = Iec10x->_len_cfm = len-4-2;			/*-start-len-len-start	 -cs-end*/

	/*end*/
	for(i=4; i<len-2; i++){
		cs_temp += send_buf[i];
	}
	send_buf[len-2] = cs_temp;
	send_buf[len-1] = IEC101_VARIABLE_END;
	
	/* enqueue to the transmisson queue */
	ptIec101m->link_ctx->link_send(ptIec101m->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);

	ptIec101m->stat.callpulse = IEC10X_STA_CALL_PULSE_START;

	return 0;

}


/*brief    iec04遥控
 *param ： ptIec101m:iec04m操作标识
 *param ： node:遥控节点信息
 *param ： count:遥控节点个数
 *return： 成功返回0 失败返回-1
 */
__EX_API__ int wow_iec101_m_build_yk(Iec101m_T* ptIec101m,DevNodeCtrl_T* ptNode, uint8_t u8Count)
{
	int i = 0;
	uint8_t len = 0;
	uint8_t cs_temp = 0;
	uint8_t *ptr = NULL;
	uint8_t  send_buf[IEC101_MAX_BUF_LEN];

	CHECK_RET_VAL_P(ptIec101m,-PARAM_INPUT_STRUCT_IS_NULL,"param input strcuct invalid!\n");
	CHECK_RET_VAL_P(ptNode && u8Count > 0,-PARAM_INPUT_STRUCT_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(ptIec101m->stat.init == IEC10X_STA_LINK_IDLE,-IEC10X_STATUS_ERROR,
					"iec101m status(%d) invalid!\n",ptIec101m->stat.init);

	memset(send_buf,0,IEC101_MAX_BUF_LEN);
    Iec101Data68_T* Iec10x = (Iec101Data68_T*)send_buf;
    Iec101Asdu_T* asdu = (Iec101Asdu_T*)(Iec10x->_asdu);
    Iec101AsduInfo_T* info = (Iec101AsduInfo_T*)(asdu->_info);

	/*head*/
	Iec10x->_begin = Iec10x->_begin_cfm = IEC101_VARIABLE_BEGING;

	/*Ctrol*/
	Iec10x->_ctrl.down._dir = IEC101_CTRL_DIR_DOWN;
	Iec10x->_ctrl.down._prm = IEC101_CTRL_PRM_MASTER;
	Iec10x->_ctrl.down._fcb = IEC101_CTRL_FCB_OPPO_NONE;
	Iec10x->_ctrl.down._fcv = IEC101_CTRL_FCV_ENABLE;

	Iec10x->_ctrl.down._func = IEC101_CTRL_SEND_USR_DATA_ACK;


	Iec10x->_addr = ptIec101m->asdu_addr;
	/*build ASDU , COT ,Addr*/
	asdu->_type = IEC10X_C_SC_NA_1;
	asdu->_num._sq  = 0;
	asdu->_num._num = u8Count;
	asdu->_reason._pn = 0;
	asdu->_reason._test = 0;
	asdu->_reason._reason = IEC10X_COT_ACT;
	asdu->_addr = ptIec101m->asdu_addr;

	ptr = (uint8_t*)&(info->_addr);
	for(i = 0; i < u8Count;i++)
	{
		/*build info addr*/
		memcpy(ptr, &ptNode[i].info.addr, 2);
		ptr += 2;

		/*build info value*/
		*ptr = 0x80 | ptNode[i].value.val.s8;
		ptr++;
	}

    /*len*/
    len = ptr + 2 - send_buf;                    		/* add cs+end*/
    Iec10x->_len = Iec10x->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += send_buf[i];
    }
    send_buf[len-2] = cs_temp;
    send_buf[len-1] = IEC101_VARIABLE_END;

	/* enqueue to the transmisson queue */
	ptIec101m->link_ctx->link_send(ptIec101m->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);

	ptIec101m->stat.yk = IEC10X_STA_YK_CMD_START;
	return 0;
}


/*brief    iec04遥调
 *param ： ptIec101m:iec04m操作标识
 *param ： ptNode:遥调节点信息
 *param ： u8Count:遥调节点个数
 *return： 成功返回0 失败返回-1
 */
__EX_API__ int wow_iec101_m_build_yt(Iec101m_T* ptIec101m,DevNodeCtrl_T* ptNode, uint8_t u8Count)
{
	int i = 0;
	uint8_t len = 0;
	uint8_t cs_temp = 0;
	uint8_t *ptr = NULL;
	uint8_t  send_buf[IEC101_MAX_BUF_LEN];

	CHECK_RET_VAL_P(ptIec101m,-PARAM_INPUT_STRUCT_IS_NULL,"param input strcuct invalid!\n");
	CHECK_RET_VAL_P(ptNode && u8Count > 0,-PARAM_INPUT_STRUCT_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(ptIec101m->stat.init == IEC10X_STA_LINK_IDLE,-IEC10X_STATUS_ERROR,
					"iec101m status(%d) invalid!\n",ptIec101m->stat.init);

	memset(send_buf,0,IEC101_MAX_BUF_LEN);
    Iec101Data68_T* Iec10x = (Iec101Data68_T*)send_buf;
    Iec101Asdu_T* asdu = (Iec101Asdu_T*)(Iec10x->_asdu);
    Iec101AsduInfo_T* info = (Iec101AsduInfo_T*)(asdu->_info);

	/*head*/
	Iec10x->_begin = Iec10x->_begin_cfm = IEC101_VARIABLE_BEGING;

	/*Ctrol*/
	Iec10x->_ctrl.down._dir = IEC101_CTRL_DIR_DOWN;
	Iec10x->_ctrl.down._prm = IEC101_CTRL_PRM_MASTER;
	Iec10x->_ctrl.down._fcb = IEC101_CTRL_FCB_OPPO_NONE;
	Iec10x->_ctrl.down._fcv = IEC101_CTRL_FCV_ENABLE;
	Iec10x->_ctrl.down._func = IEC101_CTRL_SEND_USR_DATA_ACK;

	Iec10x->_addr = ptIec101m->asdu_addr;

	/*build ASDU , COT ,Addr*/
	if(ptNode[0].info.type == DATA_FLOAT){
		asdu->_type = IEC10X_C_SE_NC_1;
	}else{
		asdu->_type = IEC10X_C_SE_NA_1;
	}
	asdu->_num._sq = 0;
	asdu->_num._num = u8Count;
	asdu->_reason._pn = 0;
	asdu->_reason._test = 0;
	asdu->_reason._reason = IEC10X_COT_ACT;
	asdu->_addr = ptIec101m->asdu_addr;

	ptr = (uint8_t*)&(info->_addr);
	for(i = 0; i < u8Count;i++)
	{
		/*build info addr*/
		memcpy(ptr, &ptNode[i].info.addr, 2);
		ptr += 2;

		/*build info value*/	
		if(ptNode[0].info.type == DATA_FLOAT){
			memcpy(ptr, &ptNode[i].value.val.f, 4);
			ptr += 4;
		}else{
			memcpy(ptr, &ptNode[i].value.val.s16, 2);
			ptr += 2;
		}

		*ptr = 0x80 | 0x00; //qds =0x00
		ptr++;

	}

    /*len*/
    len = ptr + 2 - send_buf;                    		/* add cs+end*/
    Iec10x->_len = Iec10x->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += send_buf[i];
    }
    send_buf[len-2] = cs_temp;
    send_buf[len-1] = IEC101_VARIABLE_END;

	/* enqueue to the transmisson queue */
	ptIec101m->link_ctx->link_send(ptIec101m->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);

	ptIec101m->stat.yt = IEC10X_STA_YT_CMD_START;

	return 0;

}

