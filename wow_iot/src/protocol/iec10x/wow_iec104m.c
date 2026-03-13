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
#include "utils/wow_hex.h"
#include "plugin/wow_debug.h"
#include "plugin/wow_log.h"


#include "iec104_base.h"
#include "wow_iot_errno.h"
#include "protocol/iec10x/wow_iec104m.h"

#define TAG "IEC104M"

struct __data_aligned__ iec104m_t{
	uint16_t asdu_addr;
	uint16_t back01;
	uint32_t back02;
	Iec10xState_T stat;
	Iec104Sn_T    sn;
	Iec10xTime_T	   time;
	iec10x_set_value_func_t   val_cb;
	void*			   val_priv;
	iec10x_set_service_func_t serv_cb;
	void*			   serv_priv;	
	ProtoPortCtx_T*    link_ctx;
	Iec10xThreadInfo_T thread;
};

static int iec104_m_build_u(Iec104m_T* iec104m,uint8_t UType)
{
    uint8_t len = 0;
    uint8_t *ptr = NULL;
	uint8_t  send_buf[IEC104_MAX_BUF_LEN];
	
	CHECK_RET_VAL_P(iec104m,-1,"param input invalid!\n");

	memset(send_buf,0,IEC104_MAX_BUF_LEN);
    Iec104Data_T* Iec104Data = (Iec104Data_T*)send_buf;

    /*build head*/
    Iec104Data->Head = IEC104_HEAD;

    /*build control code*/
    switch(UType){
        case IEC104_U_FUNC_STARTDT:
            Iec104Data->Ctrl.Func.Func = IEC104_U_FUNC_STARTDT;
            break;
        case IEC104_U_FUNC_STOPDT:
            Iec104Data->Ctrl.Func.Func = IEC104_U_FUNC_TESTER;
            break;
        case IEC104_U_FUNC_TESTER:
            Iec104Data->Ctrl.Func.Func = IEC104_U_FUNC_TESTER;
            break;
        default:
			ModuleErrorLog(TAG,"iec104_m_build_u_ack error  U Type(%d) \n",UType);
            return -1;
    }


    /*build ASDU , COT ,Addr*/
    ptr=Iec104Data->Asdu;

    /*build info*/

    /*len*/
    len = ptr - send_buf;
    Iec104Data->Len = len - 2;

    /* enqueue to the transmisson queue */
    iec104m->link_ctx->link_send(iec104m->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);

    return 0;
}

static int iec104_m_build_u_ack(Iec104m_T* iec104m,uint8_t UType)
{
    uint8_t len;
    uint8_t *ptr = NULL;
	uint8_t  send_buf[IEC104_MAX_BUF_LEN];
	
	CHECK_RET_VAL_P(iec104m,-1,"param input invalid!\n");

	memset(send_buf,0,IEC104_MAX_BUF_LEN);
    Iec104Data_T* Iec104Data = (Iec104Data_T*)send_buf;

    /*build head*/
    Iec104Data->Head = IEC104_HEAD;

    /*build control code*/
    switch(UType){

        case IEC104_U_FUNC_STARTDT:
            Iec104Data->Ctrl.Func.Func = IEC104_U_FUNC_STARTDT_ACK;
            break;
        case IEC104_U_FUNC_STOPDT:
            Iec104Data->Ctrl.Func.Func = IEC104_U_FUNC_STOPDT_ACK;
            break;
        case IEC104_U_FUNC_TESTER:
            Iec104Data->Ctrl.Func.Func = IEC104_U_FUNC_TESTER_ACK;
            break;
        default:
			ModuleErrorLog(TAG,"iec104_m_build_u_ack error  U Type(%d) \n",UType);
            return -1;
    }


    /*build ASDU , COT ,Addr*/
    ptr=Iec104Data->Asdu;

    /*build info*/

    /*len*/
    len = ptr - send_buf;
    Iec104Data->Len = len - 2;

    /* enqueue to the transmisson queue */
    iec104m->link_ctx->link_send(iec104m->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);

    return 0;
}


static int iec104_m_build_s_ack(Iec104m_T* iec104m)
{

    uint8_t len = 0;
    uint8_t *ptr = NULL;
	uint8_t  send_buf[IEC104_MAX_BUF_LEN];

	CHECK_RET_VAL_P(iec104m,-1,"param input invalid!\n");

	memset(send_buf,0,IEC104_MAX_BUF_LEN);
    Iec104Data_T* Iec104Data = (Iec104Data_T*)send_buf;

    /*build head*/
    Iec104Data->Head = IEC104_HEAD;

    /*build control code*/
    Iec104Data->Ctrl.S.Type1 = 1;
    Iec104Data->Ctrl.S.Type2 = 0;

    Iec104Data->Ctrl.S.Reserve = 0;
    Iec104Data->Ctrl.S.RecvSn = iec104m->sn.build_recv;

    /*build ASDU , COT ,Addr*/
    ptr=Iec104Data->Asdu;

    /*build info*/

    /*len*/
    len = ptr - send_buf;
    Iec104Data->Len = len - 2;

    /* enqueue to the transmisson queue */
    iec104m->link_ctx->link_send(iec104m->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);

    return 0;
}

static int iec104_m_asdu_call_all(Iec104m_T* iec104m,Iec104Asdu_T* Iec10x_Asdu)
{

	CHECK_RET_VAL_P(iec104m && Iec10x_Asdu,-1,"param input invalid!\n");

	uint16_t reason = Iec10x_Asdu->_reason._reason;
	Iec104AsduInfo_T* asdu_info = (Iec104AsduInfo_T*)(Iec10x_Asdu->_info);
	CHECK_RET_VAL_P(asdu_info->_addr[0] == 0 && asdu_info->_addr[1]==0 && asdu_info->_addr[2] == 0,
				-1,"call all cmd error addr!\n");
	
	switch(reason){
		case IEC10X_COT_ACTCON:
			break;
		case IEC10X_COT_INTROGEN:
		case IEC10X_COT_ACTTERM:
			if(iec104m->serv_cb){
				iec104m->serv_cb(iec104m->serv_priv,IEC10X_CALL_ALL_SUCCESS,NULL);
			}
			iec104_m_build_s_ack(iec104m);
			break;
		default:
			if(iec104m->serv_cb){
				iec104m->serv_cb(iec104m->serv_priv,IEC10X_CALL_ALL_FAILD,NULL);
			}
		    ModuleErrorLog(TAG,"iec104_m_asdu_call_all error reason(%d) \n",reason);
            break;	
	}
    return 0;
}


static int iec104_m_asdu_call_pulse(Iec104m_T* iec104m,Iec104Asdu_T* Iec10x_Asdu)
{	
	CHECK_RET_VAL_P(iec104m && Iec10x_Asdu,-1,"param input invalid!\n");

	uint16_t reason = Iec10x_Asdu->_reason._reason;
	Iec104AsduInfo_T* asdu_info = (Iec104AsduInfo_T*)(Iec10x_Asdu->_info);
	CHECK_RET_VAL_P(asdu_info->_addr[0] == 0 && asdu_info->_addr[1]==0 && asdu_info->_addr[2] == 0,
				-1,"call all cmd error addr!\n");
	
	switch(reason){
		case IEC10X_COT_ACTCON:
			break;
		case IEC10X_COT_INTROGEN:
		case IEC10X_COT_ACTTERM:	
			if(iec104m->serv_cb){
				iec104m->serv_cb(iec104m->serv_priv,IEC10X_CALL_PULSE_SUCCESS,NULL);
			}
			iec104_m_build_s_ack(iec104m);
			break;
		default:
			if(iec104m->serv_cb){
				iec104m->serv_cb(iec104m->serv_priv,IEC10X_CALL_PULSE_FAILD,NULL);
			}
		    ModuleErrorLog(TAG,"iec104_m_asdu_call_pulse error reason(%d) \n",reason);
            break;	
	}
    return 0;
}

static int iec104_m_asdu_clock(Iec104m_T* iec104m,Iec104Asdu_T* Iec10x_Asdu)
{
	CHECK_RET_VAL_P(iec104m && Iec10x_Asdu,-1,"param input invalid!\n");

    Iec104AsduInfo_T* asdu_info = (Iec104AsduInfo_T*)(Iec10x_Asdu->_info);
	CHECK_RET_VAL_P(asdu_info->_addr[0] == 0 && asdu_info->_addr[1]==0 && asdu_info->_addr[2] == 0,
				-1,"Clock cmd error addr!\n");
	
	CP56Time2a_T cp56time2a;
    memcpy(&cp56time2a,asdu_info->_element, sizeof(CP56Time2a_T));

    switch(Iec10x_Asdu->_reason._reason){
		case IEC10X_COT_SPONT:
        case IEC10X_COT_ACT:
			if(iec104m->serv_cb){
				iec104m->serv_cb(iec104m->serv_priv,IEC10X_CLOCK_SYNC,NULL);
			}
			iec104_m_build_s_ack(iec104m);
            break;
        default:
        	ModuleErrorLog(TAG,"iec104_m_asdu_clock error reason(%d) \n",Iec10x_Asdu->_reason._reason);
            break;
    }
    return 0;
}

static int iec104_m_asdu_yx(Iec104m_T* iec104m,Iec104Asdu_T* Iec10x_Asdu)
{
	CHECK_RET_VAL_P(iec104m && Iec10x_Asdu,-1,"param input invalid!\n");

	int i = 0;
    uint32_t     addr = 0;
    DevNodeInfo_T node; 
	DevNodeVal_T  val; 
    uint8_t *ptr = NULL;

	uint8_t type = Iec10x_Asdu->_type;
    uint8_t n = Iec10x_Asdu->_num._num;
	uint8_t Sq = Iec10x_Asdu->_num._sq;
	uint16_t reason = Iec10x_Asdu->_reason._reason;
    Iec104AsduInfo_T* asdu_info = (Iec104AsduInfo_T*)(Iec10x_Asdu->_info);
	
	CHECK_RET_VAL_P(reason == IEC10X_COT_PER_CYC || reason == IEC10X_COT_INTROGEN || reason == IEC10X_COT_SPONT,-1,
			"iec104_m_asdu_yx error reason(%d) \n",Iec10x_Asdu->_reason._reason);

	/* check info addrest */
    switch(type){
       	case IEC10X_M_SP_NA_1:
		case IEC10X_M_DP_NA_1:
			if(Sq == 1){
				memset(&node,0,sizeof(DevNodeInfo_T));
				node.mode = DEV_TYPE_YX;
				node.type = DATA_SINT_8;
				ptr = asdu_info->_element;
				memcpy(&addr, asdu_info->_addr, 3);
				for(i = 0; i < n; i++,ptr++){
					node.addr   = (uint16_t)addr+i;
					val.val.u8 = *ptr;
					if(iec104m->val_cb){
						iec104m->val_cb(iec104m->val_priv,&node,&val);
					}
				}			
			}else if(Sq == 0){
				memset(&node,0,sizeof(DevNodeInfo_T));
				node.mode = DEV_TYPE_YX;
				node.type = DATA_SINT_8;
				ptr = Iec10x_Asdu->_info;
				for(i = 0; i < n; i++,ptr++){
					memcpy(&addr, ptr, 3);
					ptr += 3;
					node.addr   = (uint16_t)addr;
					val.val.u8 =*ptr;
					if(iec104m->val_cb){
						iec104m->val_cb(iec104m->val_priv,&node,&val);
					}
				}
			}
			break;
			
		case IEC10X_M_SP_TB_1://暂不处理时标
		case IEC10X_M_DP_TB_1:
			break;	
        default:
            ModuleErrorLog(TAG,"iec104_m_asdu_yx error Type(%d) \n",type);
            return -1;
    }
	
    return 0;
}

static int iec104_m_asdu_yc(Iec104m_T* iec104m,Iec104Asdu_T* Iec10x_Asdu)
{
	CHECK_RET_VAL_P(iec104m && Iec10x_Asdu,-1,"param input invalid!\n");

	int i = 0;
    uint32_t addr = 0;
    DevNodeInfo_T node;  
	DevNodeVal_T  val;
    uint8_t *ptr = NULL;

	uint8_t type = Iec10x_Asdu->_type;
    uint8_t n = Iec10x_Asdu->_num._num;
	uint8_t Sq = Iec10x_Asdu->_num._sq;
	uint16_t reason = Iec10x_Asdu->_reason._reason;
    Iec104AsduInfo_T* asdu_info = (Iec104AsduInfo_T*)(Iec10x_Asdu->_info);
	
	CHECK_RET_VAL_P(reason == IEC10X_COT_PER_CYC || reason == IEC10X_COT_INTROGEN || reason == IEC10X_COT_SPONT,-1,
			"iec104_m_asdu_yc error reason(%d) \n",Iec10x_Asdu->_reason._reason);
	
	/* check info addrest */
    switch(type){
		case IEC10X_M_ME_NA_1:
			if(Sq == 1){
				Iec104Detect_T* detect = NULL;
				ptr = asdu_info->_element;
				memset(&node,0,sizeof(DevNodeInfo_T));
				node.mode = DEV_TYPE_YC;
				node.type = DATA_SINT_16;
				memcpy(&addr, asdu_info->_addr, 3);
				for(i=0; i<n; i++){
					detect = (Iec104Detect_T*)ptr;
					node.addr    = (uint16_t)addr+i;
					val.val.s16 = detect->_detect;
					//qds 暂不处理
					if(iec104m->val_cb){
						iec104m->val_cb(iec104m->val_priv,&node,&val);
					}
					ptr += sizeof(Iec104Detect_T);
				}
			}else if(Sq == 0){
				Iec104DetectSq0_T* detect_Sq0 = NULL;
				ptr = Iec10x_Asdu->_info;
				memset(&node,0,sizeof(DevNodeInfo_T));
				node.mode = DEV_TYPE_YC;
				node.type = DATA_SINT_16;
				for(i=0; i<n; i++){
					detect_Sq0 = (Iec104DetectSq0_T*)ptr;
					memcpy(&addr, detect_Sq0->_addr, 3);
					node.addr    = (uint16_t)addr+i;
					val.val.s16 = detect_Sq0->_detect;
					//qds 暂不处理
					if(iec104m->val_cb){
						iec104m->val_cb(iec104m->val_priv,&node,&val);
					}
					ptr += sizeof(Iec104DetectSq0_T);  
				}
			}
			break;
		case IEC10X_M_ME_NC_1:
			if(Sq == 1){
				Iec104DetectFloat_T* detect_f = NULL;
				ptr = asdu_info->_element;
				memset(&node,0,sizeof(DevNodeInfo_T));
				node.mode = DEV_TYPE_YC;
				node.type = DATA_FLOAT;
				memcpy(&addr, asdu_info->_addr, 3);
				for(i=0; i<n; i++){
					detect_f = (Iec104DetectFloat_T*)ptr;
					node.addr    = (uint16_t)addr+i;
					val.val.f = (float)detect_f->_detect;
					//qds 暂不处理
					if(iec104m->val_cb){
						iec104m->val_cb(iec104m->val_priv,&node,&val);
					}
					ptr += sizeof(Iec104DetectFloat_T);
				}
			}else if(Sq == 0){
				Iec104DetectSq0Float_T* detect_Sq0_f = NULL;
				ptr = Iec10x_Asdu->_info;
				memset(&node,0,sizeof(DevNodeInfo_T));
				node.mode = DEV_TYPE_YC;
				node.type = DATA_FLOAT;
				for(i=0; i<n; i++){
					detect_Sq0_f = (Iec104DetectSq0Float_T*)ptr;
					memcpy(&addr, detect_Sq0_f->_addr, 3);
					node.addr    = (uint16_t)addr;
					val.val.f = (float)(detect_Sq0_f->_detect);
					//qds 暂不处理
					if(iec104m->val_cb){
						iec104m->val_cb(iec104m->val_priv,&node,&val);
					}	
					ptr += sizeof(Iec104DetectSq0Float_T); 
				}
			}
			break;
			
		case IEC10X_M_ME_TD_1://暂不处理时标
		case IEC10X_M_ME_TF_1:
			break;	
        default:
            ModuleErrorLog(TAG,"iec104_m_asdu_yc error Type(%d) \n",type);
            return -1;
    }

    return 0;
}

static int iec104_m_asdu_ym(Iec104m_T* iec104m,Iec104Asdu_T* Iec10x_Asdu)
{
	CHECK_RET_VAL_P(iec104m && Iec10x_Asdu,-1,"param input invalid!\n");

	int i = 0;
    uint32_t addr = 0;
    DevNodeInfo_T node; 
	DevNodeVal_T  val;  
    uint8_t *ptr = NULL;

	uint8_t type = Iec10x_Asdu->_type;
    uint8_t n = Iec10x_Asdu->_num._num;
	uint8_t Sq = Iec10x_Asdu->_num._sq;
	uint16_t reason = Iec10x_Asdu->_reason._reason;
    Iec104AsduInfo_T* asdu_info = (Iec104AsduInfo_T*)(Iec10x_Asdu->_info);
	
	CHECK_RET_VAL_P(reason == IEC10X_COT_REQ || reason == IEC10X_COT_REQCOGEN || reason == IEC10X_COT_SPONT,-1,
					"iec104_m_asdu_ym error reason(%d) \n",Iec10x_Asdu->_reason._reason);

	/* check info addrest */
    memcpy(&addr, asdu_info->_addr, 3);
    switch(type){
		case IEC10X_M_IT_NA_1:
			if(Sq == 1){
				ptr = asdu_info->_element;
				Iec104DetectInt32_T* detect = (Iec104DetectInt32_T*)ptr;
				memset(&node,0,sizeof(DevNodeInfo_T));
				node.mode = DEV_TYPE_YM;
				node.type = DATA_SINT_32;
				for(i=0; i<n; i++){
					node.addr    = (uint16_t)addr+i;
					val.val.s32 = detect->_detect;
					//qds 暂不处理
					if(iec104m->val_cb){
						iec104m->val_cb(iec104m->val_priv,&node,&val);
					}
					detect++;
				}
			}else if(Sq == 0){	
				ptr = Iec10x_Asdu->_info;
				Iec104DetectInt32Sq0_T* detect_Sq0 = (Iec104DetectInt32Sq0_T*)ptr;
				memset(&node,0,sizeof(DevNodeInfo_T));
				node.mode = DEV_TYPE_YM;
				node.type = DATA_SINT_32;
				for(i=0; i<n; i++){
					memcpy(&addr, detect_Sq0->_addr, 3);
					node.addr    = (uint16_t)addr;
					val.val.s32 = detect_Sq0->_detect;
					//qds 暂不处理
					if(iec104m->val_cb){
						iec104m->val_cb(iec104m->val_priv,&node,&val);
					}	 
					detect_Sq0++; 
				}
			}
			break;
		case IEC10X_M_IT_TA_1://暂不处理时标
			break;	
        default:
            ModuleErrorLog(TAG,"iec104_m_asdu_ym error Type(%d) \n",type);
            return -1;
    }

    return 0;
}

static int iec104_m_build_yt_ack(Iec104m_T* iec104m,Iec104Asdu_T* Iec10x_Asdu)
{

	CHECK_RET_VAL_P(iec104m && Iec10x_Asdu,-1,"param input invalid!\n");

	int i = 0;
    uint8_t len = 0;
    uint8_t *ptr = NULL;
	uint8_t send_buf[IEC104_MAX_BUF_LEN];

	uint8_t type = Iec10x_Asdu->_type;
	uint8_t n = Iec10x_Asdu->_num._num;
	uint8_t Sq = Iec10x_Asdu->_num._sq;
	Iec104AsduInfo_T* asdu_info = (Iec104AsduInfo_T*)(Iec10x_Asdu->_info);

	memset(send_buf,0,IEC104_MAX_BUF_LEN);
    Iec104Data_T* Iec104Data = (Iec104Data_T*)send_buf;
    Iec104Asdu_T* asdu = (Iec104Asdu_T*)(Iec104Data->Asdu);
    Iec104AsduInfo_T* info = (Iec104AsduInfo_T*)(asdu->_info);

    /*build head*/
    Iec104Data->Head = IEC104_HEAD;

    /*build control code*/
    Iec104Data->Ctrl.I.Type = 0;
    Iec104Data->Ctrl.I.SendSn = iec104m->sn.build_send++;
    Iec104Data->Ctrl.I.RecvSn = iec104m->sn.build_recv;

    /*build ASDU , COT ,Addr*/
    asdu->_type = type;
    asdu->_num._sq  = Sq;
    asdu->_num._num = n;
    asdu->_reason._reason = IEC10X_COT_ACT;
    asdu->_addr = iec104m->asdu_addr;

    /*build info*/
	ptr = info->_addr;
	switch(type){
		case IEC10X_C_SE_NA_1:
			if(Sq == 1){   
				ptr = info->_addr;
				memcpy(ptr,asdu_info->_addr,3);
				ptr = info->_element;
				Iec104Detect_T* val = (Iec104Detect_T*)asdu_info->_element;
				for(i = 0; i < n; i++){
					val->_qds &= 0x7F;
					val++;
				}
				memcpy(ptr,asdu_info->_element,n*sizeof(Iec104Detect_T));
				ptr += n*sizeof(Iec104Detect_T);
			}else if(Sq == 0){
				ptr = info->_addr;
				Iec104DetectSq0_T* val = (Iec104DetectSq0_T*)asdu_info;
				for(i = 0; i < n; i++){
					val->_qds &= 0x7F;
					val++;
				}
				memcpy(ptr,asdu_info,n*sizeof(Iec104DetectSq0_T));
				ptr += n*sizeof(Iec104DetectSq0_T);	
			}
			break;
		case IEC10X_C_SE_NC_1:
			if(Sq == 1){
				ptr = info->_addr;
				memcpy(ptr,asdu_info->_addr,3);
				ptr = info->_element;
				Iec104DetectFloat_T* val = (Iec104DetectFloat_T*)asdu_info->_element;
				for(i = 0; i < n; i++){
					val->_qds &= 0x7F;
					val++;
				}
				memcpy(ptr,asdu_info->_element,n*sizeof(Iec104DetectFloat_T));
				ptr += n*sizeof(Iec104DetectFloat_T);
			}else if(Sq == 0){
				ptr = info->_addr;
				Iec104DetectSq0Float_T* val = (Iec104DetectSq0Float_T*)asdu_info;
				for(i = 0; i < n; i++){
					val->_qds &= 0x7F;
					val++;
				}
				memcpy(ptr,asdu_info,n*sizeof(Iec104DetectSq0Float_T));
				ptr += n*sizeof(Iec104DetectSq0Float_T);	
			}
			break;
		default:
			ModuleErrorLog(TAG,"iec104_m_asdu_yt error Type(%d) \n",type);
			return -1;
	}

    /*len*/
    len = ptr - send_buf;
    Iec104Data->Len = len - 2;
    
    /* enqueue to the transmisson queue */
    iec104m->link_ctx->link_send(iec104m->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);

    return 0;
}

static int iec104_m_asdu_yt(Iec104m_T* iec104m,Iec104Asdu_T* Iec10x_Asdu)
{
	CHECK_RET_VAL_P(iec104m && Iec10x_Asdu,-1,"param input invalid!\n");
	
	uint16_t reason = Iec10x_Asdu->_reason._reason;
	switch(reason){
		case IEC10X_COT_ACTCON://遥调执行
			iec104_m_build_yt_ack(iec104m,Iec10x_Asdu);
			break;
		case IEC10X_COT_ACTTERM://执调确认
			if(iec104m->serv_cb){
				iec104m->serv_cb(iec104m->serv_priv,IEC10X_YT_CMD_SUCCESS,NULL);
			}
			break;
		case IEC10X_COT_UNKNOW_TYPE:
		case IEC10X_COT_UNKNOW_COT:
		case IEC10X_COT_UNKNOW_ADDR:
		case IEC10X_COT_UNKNOW_INF:
		default:
			if(iec104m->serv_cb){
				iec104m->serv_cb(iec104m->serv_priv,IEC10X_YT_CMD_FAILD,NULL);
			}
			break;
	}

    return 0;
}

static int iec104_m_build_yk_ack(Iec104m_T* iec104m,Iec104Asdu_T* Iec10x_Asdu)
{
	CHECK_RET_VAL_P(iec104m && Iec10x_Asdu,-1,"param input invalid!\n");
	
	int i = 0;
    uint8_t len = 0;
    uint8_t *ptr = NULL;
	uint8_t *ntr = NULL;
	uint8_t send_buf[IEC104_MAX_BUF_LEN];

	uint8_t type = Iec10x_Asdu->_type;
	uint8_t n = Iec10x_Asdu->_num._num;
	uint8_t Sq = Iec10x_Asdu->_num._sq;
	Iec104AsduInfo_T* asdu_info = (Iec104AsduInfo_T*)(Iec10x_Asdu->_info);

	memset(send_buf,0,IEC104_MAX_BUF_LEN);
    Iec104Data_T* Iec104Data = (Iec104Data_T*)send_buf;
    Iec104Asdu_T* asdu = (Iec104Asdu_T*)(Iec104Data->Asdu);
    Iec104AsduInfo_T* info = (Iec104AsduInfo_T*)(asdu->_info);

    /*build head*/
    Iec104Data->Head = IEC104_HEAD;

    /*build control code*/
    Iec104Data->Ctrl.I.Type = 0;
    Iec104Data->Ctrl.I.SendSn = iec104m->sn.build_send++;
    Iec104Data->Ctrl.I.RecvSn = iec104m->sn.build_recv;

    /*build ASDU , COT ,Addr*/
    asdu->_type = type;
    asdu->_num._sq  = Sq;
    asdu->_num._num = n;
    asdu->_reason._reason = IEC10X_COT_ACT;
    asdu->_addr = iec104m->asdu_addr;

    /*build info*/
	ptr = info->_addr;
	switch(type){
		case IEC10X_C_SC_NA_1:
		case IEC10X_C_DC_NA_1:
			if(Sq == 1){   
				ntr = asdu_info->_element;
				for(i = 0; i < n; i++){
					*ntr &=  0x7F;
					ntr++;
				}
				memcpy(info,asdu_info,n + 3);
				ptr += n+ 3;
			}else if(Sq == 0){
				ntr = asdu_info->_addr;
				for(i = 0; i < n; i++){
					ntr += 3;
					*ntr &= 0x7F;
					ntr += 1;
				}
				memcpy(info,asdu_info,n*4);
				ptr += n*4;	
			}
			break;
		default:
			ModuleErrorLog(TAG,"iec104_m_asdu_yk error Type(%d) \n",type);
			return -1;
	}

    /*len*/
    len = ptr - send_buf;
    Iec104Data->Len = len - 2;
    
    /* enqueue to the transmisson queue */
    iec104m->link_ctx->link_send(iec104m->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);

    return 0;
}

static int iec104_m_asdu_yk(Iec104m_T* iec104m,Iec104Asdu_T* Iec10x_Asdu)
{
	CHECK_RET_VAL_P(iec104m && Iec10x_Asdu,-1,"param input invalid!\n");
	
	Iec104AsduInfo_T* asdu_info = (Iec104AsduInfo_T*)(Iec10x_Asdu->_info);
	uint8_t sco = asdu_info->_element[0];
	uint16_t  reason = Iec10x_Asdu->_reason._reason;
	switch(reason){
		case IEC10X_COT_ACTCON:
			if(sco && 0x80){//遥控执行
				iec104_m_build_yk_ack(iec104m,Iec10x_Asdu);
			}else{//执行确认
				if(iec104m->serv_cb){
					iec104m->serv_cb(iec104m->serv_priv,IEC10X_YK_CMD_SUCCESS,NULL);
				}
			}
			break;
		case IEC10X_COT_UNKNOW_TYPE:
		case IEC10X_COT_UNKNOW_COT:
		case IEC10X_COT_UNKNOW_ADDR:
		case IEC10X_COT_UNKNOW_INF:
		default:
			if(iec104m->serv_cb){
				iec104m->serv_cb(iec104m->serv_priv,IEC10X_YK_CMD_FAILD,NULL);
			}
			break;
	}

    return 0;
}




static int iec104_m_deal_sn(Iec104m_T* iec104m,uint16_t SendSn, uint16_t RecvSn)
{
	CHECK_RET_VAL_P(iec104m,-1,"param input invalid!\n");
	CHECK_RET_VAL_P(SendSn >= iec104m->sn.deal_send && RecvSn >= iec104m->sn.deal_recv,-1,
		"iec104_m_deal_sn invalid sn!\n");

    iec104m->sn.build_recv = SendSn+1;

    iec104m->sn.deal_send = SendSn;
    iec104m->sn.deal_recv = RecvSn;

    //iec104m->sn.build_recv++;

    return 0;
}

static int iec104_m_frame_i_parse(Iec104m_T* iec104m,Iec104Data_T* Iec104Data, uint16_t len){

    uint8_t Type;
    uint16_t RecvSn,SendSn;

    Iec104Asdu_T* asdu = (Iec104Asdu_T*)(Iec104Data->Asdu);

    SendSn = Iec104Data->Ctrl.I.SendSn;
    RecvSn = Iec104Data->Ctrl.I.RecvSn;

    /* check asdu addrest */
    CHECK_RET_VAL_P(iec104m->asdu_addr == asdu->_addr,-1,"asdu addr(%d) invalid!\n",asdu->_addr);
	
    /* deal the receive and send serial number */
	CHECK_RET_VAL_P(iec104_m_deal_sn(iec104m,SendSn, RecvSn) == 0,-1,"deal sn faild!\n");

    Type = asdu->_type;
    switch(Type){
		case IEC10x_M_EI_NA_1:
			ModDebug_D(TAG,"Asdu Type Init Cmd...\n");	
			iec104_m_build_s_ack(iec104m);
			break;
        case IEC10X_C_IC_NA_1:
			ModDebug_D(TAG,"Asdu Type Call All...\n");
            iec104_m_asdu_call_all(iec104m,asdu);
            break;
		case IEC10X_C_CI_NA_1:
			ModDebug_D(TAG,"Asdu Type Call Pulse...\n");
            iec104_m_asdu_call_pulse(iec104m,asdu);
            break;
        case IEC10X_C_CS_NA_1:
			ModDebug_D(TAG,"Asdu Type Clock Syc Cmd...\n");
            iec104_m_asdu_clock(iec104m,asdu);
            break;			
		

        case IEC10X_C_SC_NA_1:
		case IEC10X_C_DC_NA_1:
			ModDebug_D(TAG,"Asdu Type Set Cmd  Cmd...\n");
			iec104_m_asdu_yk(iec104m,asdu);
            break;	
        case IEC10X_C_SE_NA_1:
        case IEC10X_C_SE_NC_1:
			ModDebug_D(TAG,"Asdu Type Set Val  Cmd...\n");
			iec104_m_asdu_yt(iec104m,asdu);
            break;
		
		case IEC10X_M_SP_NA_1:
		case IEC10X_M_DP_NA_1:
		case IEC10X_M_SP_TB_1:
		case IEC10X_M_DP_TB_1:
			ModDebug_D(TAG,"Asdu Type Yx Val Cmd...\n");
			iec104_m_asdu_yx(iec104m,asdu);
			break;
		case IEC10X_M_ME_NA_1:
		case IEC10X_M_ME_NC_1:
		case IEC10X_M_ME_TD_1:
		case IEC10X_M_ME_TF_1:
			ModDebug_D(TAG,"Asdu Type Yc Val Cmd...\n");
			iec104_m_asdu_yc(iec104m,asdu);	
			break;
		case IEC10X_M_IT_NA_1:
		case IEC10X_M_IT_TA_1:
			ModDebug_D(TAG,"Asdu Type Ym Val Cmd...\n");
			iec104_m_asdu_ym(iec104m,asdu);
			break;
        default:
			ModDebug_D(TAG,"Asdu Type(%d) Error!\n",Type);
            return -1;
    }
    return 0;
}


static int iec104_m_frame_s_parse(Iec104m_T* iec104m,Iec104Data_T* Iec104Data, uint16_t len)
{
	//iec104_m_build_s_ack(iec104m);
    return 0;
}


static int iec104_m_frame_u_parse(Iec104m_T* iec104m,Iec104Data_T* Iec104Data, uint16_t len)
{
    switch(Iec104Data->Ctrl.Func.Func){
        case IEC104_U_FUNC_STARTDT_ACK:
			ModDebug_D(TAG,"Frame function STARTDT ACK\n");	
			iec104m->stat.init = IEC10X_STA_LINK_CONNECT_OK;
            break;
        case IEC104_U_FUNC_STOPDT_ACK:
			ModDebug_D(TAG,"Frame function STOPDT ACK\n");
            iec104m->stat.init = IEC10X_STA_LINK_DISCONNECT;
			iec104_m_build_u_ack(iec104m,IEC104_U_FUNC_STOPDT_ACK);
            break;
		case IEC104_U_FUNC_TESTER:
			ModDebug_D(TAG,"Frame function TESTER \n");
            iec104_m_build_u_ack(iec104m,IEC104_U_FUNC_TESTER);
            break;	
        case IEC104_U_FUNC_TESTER_ACK:
			ModDebug_D(TAG,"Frame function TESTER ACK\n");
			iec104m->stat.test = IE10X_STA_TESTER_STOP;
            break;
        default:
			ModDebug_E(TAG,"Frame function ERROR\n");
            break;
    }
    return 0;
}


static void iec104_m_frame_parse(Iec104m_T* iec104m,uint8_t *buf, uint16_t len){

    uint8_t *BufTemp = NULL;
    int16_t LenRemain,LenTmp;
    Iec104Data_T* Iec104Data = NULL;

	CHECK_RET_VOID(iec104m && buf && len > 0);

    BufTemp = buf;
    LenRemain = len;
	
    while(BufTemp<buf+len){

        Iec104Data = (Iec104Data_T*)BufTemp;
        if(Iec104Data->Head == IEC104_HEAD){
            LenTmp = Iec104Data->Len + 2;
			CHECK_RET_VOID_P(LenRemain >= IEC104_HEAD_LEN,"invalid frame len!\n");

            if(Iec104Data->Ctrl.Type.Type1 == 0){
				ModDebug_D(TAG,"Frame Type I \n");
                iec104_m_frame_i_parse(iec104m,Iec104Data, LenTmp);
				iec104m->time.timer = wow_time_get_msec();
            }else if(Iec104Data->Ctrl.Type.Type1 == 1 && Iec104Data->Ctrl.Type.Type2 == 0){
                ModDebug_D(TAG,"Frame Type S \n");
                iec104_m_frame_s_parse(iec104m,Iec104Data, LenTmp);
				iec104m->time.timer = wow_time_get_msec();
            }else if(Iec104Data->Ctrl.Type.Type1 == 1 && Iec104Data->Ctrl.Type.Type2 == 1){
                ModDebug_D(TAG,"Frame Type U \n");
                iec104_m_frame_u_parse(iec104m,Iec104Data, LenTmp);
				iec104m->time.timer = wow_time_get_msec();
            }
        }else{
        	ModDebug_E(TAG,"Frame Type Error! \n");
            return;
        }
        BufTemp+=LenTmp;
        LenRemain-=LenTmp;
    }
}


static void iec104_m_reset_param(Iec104m_T* iec104m)
{
	CHECK_RET_VOID_P(iec104m,"param input arg invalid!\n");

	iec104m->stat.init    = IEC10X_STA_LINK_INIT;
	iec104m->stat.callall = IEC10X_STA_CALL_ALL_IDLE;
	iec104m->stat.callpulse = IEC10X_STA_CALL_PULSE_IDLE;
	iec104m->stat.yt      = IEC10X_STA_YT_CMD_IDLE;
	iec104m->stat.yk	  = IEC10X_STA_YK_CMD_IDLE;
	iec104m->stat.test    = IE10X_STA_TESTER_START;

	iec104m->sn.build_send = 0;
	iec104m->sn.build_recv = 0;
	iec104m->sn.deal_send = -1;
	iec104m->sn.deal_recv = 0;

	iec104m->time.init = 0;
	iec104m->time.callall = 0;
	iec104m->time.callpulse = 0;
	iec104m->time.yt   = 0;
	iec104m->time.yk   = 0;
	iec104m->time.test = 0;

}


static void iec104_m_state_machine(Iec104m_T* iec104m)
{	
	/*Init link*/
	switch(iec104m->stat.init){
		case IEC10X_STA_LINK_INIT:
			iec104_m_reset_param(iec104m);
			iec104m->stat.init = IEC10X_STA_LINK_OPEN;
			break;
		case IEC10X_STA_LINK_OPEN:
			if(iec104m->link_ctx->link_open(iec104m->link_ctx) == 0){
				iec104m->stat.init = IEC10X_STA_LINK_CONNECT;
				iec104m->time.timer = wow_time_get_msec();
				iec104_m_build_u(iec104m,IEC104_U_FUNC_STARTDT);
			}
			break;
		case IEC10X_STA_LINK_CONNECT:
			if(wow_time_get_msec() - iec104m->time.init > IEC10X_CONNECT_TIMEOUT){
				//超时关闭
				iec104m->link_ctx->link_close(iec104m->link_ctx);
				iec104m->stat.init = IEC10X_STA_LINK_INIT;
			}
			break;
		case IEC10X_STA_LINK_CONNECT_OK:
			iec104m->time.init = 0;
			iec104m->stat.init = IEC10X_STA_LINK_IDLE;
			iec104m->time.timer = wow_time_get_msec();
			if(iec104m->serv_cb){
				iec104m->serv_cb(iec104m->serv_priv,IEC10X_CONNECTED,NULL);
			}		
			break;
			
		case IEC10X_STA_LINK_DISCONNECT:
			iec104m->link_ctx->link_close(iec104m->link_ctx);
			iec104m->stat.init = IEC10X_STA_LINK_INIT;
			if(iec104m->serv_cb){
				iec104m->serv_cb(iec104m->serv_priv,IEC10X_DISCONNECTED,NULL);
			}
			break;
		case IEC10X_STA_LINK_CLOSE:
			iec104_m_build_u(iec104m,IEC104_U_FUNC_STOPDT);
			iec104m->stat.init = IEC10X_STA_LINK_INIT;
			break;

		case IEC10X_STA_LINK_IDLE:
		default:
			break;
	}
	
	
	if(iec104m->stat.init != IEC10X_STA_LINK_IDLE) return;

    /* Tester */
    switch(iec104m->stat.test){
        case IE10X_STA_TESTER_START:
			iec104m->time.test = wow_time_get_msec();
			if(iec104m->time.test - iec104m->time.timer > IEC10X_CIRCLE_TESTER_U){
				iec104_m_build_u(iec104m,IEC104_U_FUNC_TESTER);
				iec104m->time.timer = iec104m->time.test;
				iec104m->stat.test = IE10X_STA_TESTER_IDLE; 	
			}
            break;
		case IE10X_STA_TESTER_IDLE:
			if(wow_time_get_msec() - iec104m->time.test > IEC10X_TESTER_U_TIMEOUT){
				//超时关闭
				iec104m->link_ctx->link_close(iec104m->link_ctx);
				iec104m->stat.init = IEC10X_STA_LINK_INIT;
			}
			break;
        case IE10X_STA_TESTER_STOP:
			iec104m->time.test = 0;
			iec104m->stat.test = IE10X_STA_TESTER_START;	
            break;
        default:
            break;
    }
}

static void* iec104_m_exec_thread(void* arg)
{
	Iec104m_T* iec104m = (Iec104m_T*)arg;
	CHECK_RET_VAL_P(iec104m,NULL,"param input arg invalid!\n");
	
	uint8_t recv_buf[1500]={0};
	int rlen = 0;
	
	while(iec104m->thread.tflag){
		rlen = iec104m->link_ctx->link_recv(iec104m->link_ctx,recv_buf,1500,IEC10X_RECV_TIMEOUT);
		if(rlen > 0){
			iec104_m_frame_parse(iec104m,recv_buf,rlen);
		}

		iec104_m_state_machine(iec104m);
		usleep(10*1000);
	}
	
	wow_mutex_lock(&(iec104m->thread.lock));
	wow_mutex_cond_signal(&(iec104m->thread.cond));
	wow_mutex_unlock(&(iec104m->thread.lock));

	return NULL;
}


/*brief    Iec104主设备初始化
 *param ： u16Addr: 公共地址 
 *param ： ptEthPar:  硬件参数
 *return： 成功返回iec04m操作标识 失败返回NULL
 */
__EX_API__ Iec104m_T* wow_iec104_m_init(uint16_t u16Addr,ProtoEthParam_T* ptEthPar)
{
	CHECK_RET_VAL_ERRNO_P(ptEthPar,-PARAM_INPUT_DATA_IS_NULL, "param input data invalid!\n");

	wow_debug_setStatus(TAG,0);
	
	Iec104m_T* iec104m = CALLOC(1, Iec104m_T);
	CHECK_RET_VAL_ERRNO_P(iec104m,-SYSTEM_MALLOC_FAILED,"malloc iec104m faild!\n");
	
	/*参数初始化*/
	iec104m->asdu_addr = u16Addr;	
	iec104_m_reset_param(iec104m);
	
	/*建立连接*/
	iec104m->link_ctx = proto_port_eth_master_init(ptEthPar);
	CHECK_RET_GOTO_ERRNO_P(iec104m->link_ctx,out,-PROTO_NET_INIT_ERROR,"iec10x port init faild!\n");
	
	/*IEC104状态机*/
	iec104m->thread.tflag = 1;
	wow_mutex_lock_init(&(iec104m->thread.lock));
	wow_mutex_cond_init(&(iec104m->thread.cond));

	return iec104m;
out:
	FREE(iec104m);
	return NULL;
}

/*brief    启动Iec104主设备
 *param ： ptIec104m:iec04m操作标识
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_iec104_m_start(Iec104m_T* ptIec104m)
{
	CHECK_RET_VAL_P(ptIec104m,-PARAM_INPUT_STRUCT_IS_NULL,"param input strcuct invalid!\n");

	ptIec104m->thread.thread	= wow_thread_create("iec104m",iec104_m_exec_thread,ptIec104m);
	CHECK_RET_VAL_P(ptIec104m->thread.thread,-THREAD_CREATE_FAILED,"wow_thread_create failed!\n");

	return 0;
}

/*brief    Iec104设置更新数据节点参数回调函数
 *param ： ptIec104m:iec04m操作标识
 *param ： fCallBack:   回调函数 
 *param ： pArg:  回调函数私有变量
 *return： 成功返回0 失败返回<0
 */
int wow_iec104_m_set_value_handle(Iec104m_T* ptIec104m,iec10x_set_value_func_t fCallBack,void* pArg)
{
	CHECK_RET_VAL_P(ptIec104m,-PARAM_INPUT_STRUCT_IS_NULL,"param input strcuct invalid!\n");
	CHECK_RET_VAL_P(fCallBack,-PARAM_INPUT_FUNC_IS_NULL,"param input func invalid!\n");

	ptIec104m->val_cb   = fCallBack;
	ptIec104m->val_priv = pArg;

	return 0;
}

/*brief    Iec104设置更新服务状态回调函数
 *param ： ptIec104m:iec04m操作标识
 *param ： fCallBack:   回调函数 
 *param ： pArg:  回调函数私有变量
 *return： 成功返回0 失败返回<0
 */
int wow_iec104_m_set_service_handle(Iec104m_T* ptIec104m,iec10x_set_service_func_t fCallBack,void* pArg)
{
	CHECK_RET_VAL_P(ptIec104m,-PARAM_INPUT_STRUCT_IS_NULL,"param input strcuct invalid!\n");
	CHECK_RET_VAL_P(fCallBack,-PARAM_INPUT_FUNC_IS_NULL,"param input func invalid!\n");

	ptIec104m->serv_cb   = fCallBack;
	ptIec104m->serv_priv = pArg;

	return 0;
}

/*brief    退出Iec104主设备
 *param ： pptIec104M:iec04m操作标识
 *return： 无
 */
__EX_API__ void wow_iec104_m_exit(Iec104m_T** pptIec104m)
{
	CHECK_RET_VOID(pptIec104m && *pptIec104m);

	Iec104m_T* iec104m= (Iec104m_T*)*pptIec104m;
	
	//安全退出线程
	wow_mutex_lock(&(iec104m->thread.lock));
	iec104m->thread.tflag= 0;
	wow_mutex_cond_wait(&(iec104m->thread.lock),&(iec104m->thread.cond),-1);
	wow_mutex_unlock(&(iec104m->thread.lock));

	proto_port_eth_master_exit(&iec104m->link_ctx);

	FREE(iec104m);
	*pptIec104m = NULL;
}


/*brief    iec04总召
 *param ： ptIec104m:iec04m操作标识
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_iec104_m_build_call_all(Iec104m_T* ptIec104m)
{
	int ret = 0;
    uint8_t len = 0;
    uint8_t *ptr = NULL;
    uint32_t addr = 0;
	uint8_t  send_buf[IEC104_MAX_BUF_LEN]={0};

	CHECK_RET_VAL_P(ptIec104m,-PARAM_INPUT_STRUCT_IS_NULL,"param input strcuct invalid!\n");
	CHECK_RET_VAL_P(ptIec104m->stat.init == IEC10X_STA_LINK_IDLE,-IEC10X_STATUS_ERROR,"iec104m status(%d) invalid!\n",ptIec104m->stat.init);

	memset(send_buf,0,IEC104_MAX_BUF_LEN);
    Iec104Data_T* Iec104Data = (Iec104Data_T*)send_buf;
    Iec104Asdu_T* asdu = (Iec104Asdu_T*)(Iec104Data->Asdu);
    Iec104AsduInfo_T* info = (Iec104AsduInfo_T*)(asdu->_info);

    /*build head*/
    Iec104Data->Head = IEC104_HEAD;

    /*build control code*/
    Iec104Data->Ctrl.I.Type = 0;
    Iec104Data->Ctrl.I.SendSn = ptIec104m->sn.build_send++;
    Iec104Data->Ctrl.I.RecvSn = ptIec104m->sn.build_recv;

    /*build ASDU , COT ,Addr*/
    asdu->_type = IEC10X_C_IC_NA_1;
    asdu->_num._sq  = 0;
    asdu->_num._num = 1;
    asdu->_reason._reason = IEC10X_COT_ACT;
    asdu->_addr = ptIec104m->asdu_addr;

    /*build info*/
    ptr = info->_addr;
    addr = 0;
    memcpy(ptr, &addr, 3);

    ptr = info->_element;
    ptr[0] = IEC10X_COT_INTROGEN;

    ptr+=1;
    /*len*/
    len = ptr - send_buf;
    Iec104Data->Len = len - 2;

    /* enqueue to the transmisson queue */
	ret = ptIec104m->link_ctx->link_send(ptIec104m->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);
	return (ret == len)?0:-1;
}


/*brief    iec04电度量总召
 *param ： ptIec104m:iec04m操作标识
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_iec104_m_build_call_pulse(Iec104m_T* ptIec104m)
{
	int ret = 0;
    uint8_t len = 0;
    uint8_t *ptr = NULL;
    uint32_t addr = 0;
	uint8_t  send_buf[IEC104_MAX_BUF_LEN]={0};

	CHECK_RET_VAL_P(ptIec104m,-PARAM_INPUT_STRUCT_IS_NULL,"param input strcuct invalid!\n");
	CHECK_RET_VAL_P(ptIec104m->stat.init == IEC10X_STA_LINK_IDLE,-IEC10X_STATUS_ERROR,"iec104m status(%d) invalid!\n",ptIec104m->stat.init);

	memset(send_buf,0,IEC104_MAX_BUF_LEN);
    Iec104Data_T* Iec104Data = (Iec104Data_T*)send_buf;
    Iec104Asdu_T* asdu = (Iec104Asdu_T*)(Iec104Data->Asdu);
    Iec104AsduInfo_T* info = (Iec104AsduInfo_T*)(asdu->_info);

    /*build head*/
    Iec104Data->Head = IEC104_HEAD;

    /*build control code*/
    Iec104Data->Ctrl.I.Type = 0;
    Iec104Data->Ctrl.I.SendSn = ptIec104m->sn.build_send++;
    Iec104Data->Ctrl.I.RecvSn = ptIec104m->sn.build_recv;

    /*build ASDU , COT ,Addr*/
    asdu->_type = IEC10X_C_CI_NA_1;
    asdu->_num._sq  = 0;
    asdu->_num._num = 1;
    asdu->_reason._reason = IEC10X_COT_ACT;
    asdu->_addr = ptIec104m->asdu_addr;

    /*build info*/
    ptr = info->_addr;
    addr = 0;
    memcpy(ptr, &addr, 3);
	
	//默认QCC-0x05 读取计量 0x45冻结累加量  0x45冻结增量
    ptr = info->_element;
    ptr[0] = 0x05;

    ptr+=1;
    /*len*/
    len = ptr - send_buf;
    Iec104Data->Len = len - 2;

    /* enqueue to the transmisson queue */
	ret = ptIec104m->link_ctx->link_send(ptIec104m->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);
	return (ret == len)?0:-1;
}


/*brief    iec04遥控
 *param ： ptIec104m:iec04m操作标识
 *param ： ptNode:遥控节点信息
 *param ： u8Count:遥控节点个数
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_iec104_m_build_yk(Iec104m_T* ptIec104m,DevNodeCtrl_T* ptNode,uint8_t u8Count)
{
    uint8_t len = 0,  i;
    uint8_t *ptr  = NULL;
    uint32_t addr = 0;
	uint8_t  send_buf[IEC104_MAX_BUF_LEN];

	CHECK_RET_VAL_P(ptIec104m,-PARAM_INPUT_STRUCT_IS_NULL,"param input strcuct invalid!\n");
	CHECK_RET_VAL_P(ptNode && u8Count > 0,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(ptIec104m->stat.init == IEC10X_STA_LINK_IDLE,-IEC10X_STATUS_ERROR,"iec104m status(%d) invalid!\n",ptIec104m->stat.init);

	memset(send_buf,0,IEC104_MAX_BUF_LEN);
    Iec104Data_T* Iec104Data = (Iec104Data_T*)send_buf;
     Iec104Asdu_T* asdu = ( Iec104Asdu_T*)(Iec104Data->Asdu);
    Iec104AsduInfo_T* info = (Iec104AsduInfo_T*)(asdu->_info);

    /*build head*/
    Iec104Data->Head = IEC104_HEAD;

    /*build control code*/
    Iec104Data->Ctrl.I.Type = 0;
    Iec104Data->Ctrl.I.SendSn = ptIec104m->sn.build_send++;
    Iec104Data->Ctrl.I.RecvSn = ptIec104m->sn.build_recv;
	
    /*build ASDU , COT ,Addr*/
	asdu->_type = IEC10X_C_SC_NA_1;
    asdu->_num._sq  = 0;
    asdu->_num._num = u8Count;
    asdu->_reason._reason = IEC10X_COT_ACT;
    asdu->_addr = ptIec104m->asdu_addr;

	ptr = info->_addr;
	for(i = 0; i < u8Count;i++)
	{
		/*build info addr*/
		addr = (uint32_t)ptNode[i].info.addr;
		memcpy(ptr, &addr, 3);
		ptr += 3;

		/*build info value*/
		*ptr = 0x80 | ptNode[i].value.val.s8;
		ptr++;
	}

    /*len*/
    len = ptr - send_buf;
    Iec104Data->Len = len - 2;

    /* enqueue to the transmisson queue */
	ptIec104m->link_ctx->link_send(ptIec104m->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);

    return 0;
}

/*brief    iec04遥调
 *param ： ptIec104m:iec04m操作标识
 *param ： ptNode:遥调节点信息
 *param ： u8Count:遥调节点个数
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_iec104_m_build_yt(Iec104m_T* ptIec104m,DevNodeCtrl_T* ptNode, uint8_t u8Count)
{
	int i = 0;
    uint8_t len = 0;
    uint8_t *ptr = NULL;
    uint32_t addr = 0;
	uint8_t  send_buf[IEC104_MAX_BUF_LEN];

	CHECK_RET_VAL_P(ptIec104m,-PARAM_INPUT_STRUCT_IS_NULL,"param input strcuct invalid!\n");
	CHECK_RET_VAL_P(ptNode && u8Count > 0,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(ptIec104m->stat.init == IEC10X_STA_LINK_IDLE,-IEC10X_STATUS_ERROR,"iec104m status(%d) invalid!\n",ptIec104m->stat.init);

	memset(send_buf,0,IEC104_MAX_BUF_LEN);
    Iec104Data_T* Iec104Data = (Iec104Data_T*)send_buf;
    Iec104Asdu_T* asdu = ( Iec104Asdu_T*)(Iec104Data->Asdu);
    Iec104AsduInfo_T* info = (Iec104AsduInfo_T*)(asdu->_info);

    /*build head*/
    Iec104Data->Head = IEC104_HEAD;

    /*build control code*/
    Iec104Data->Ctrl.I.Type = 0;
    Iec104Data->Ctrl.I.SendSn = ptIec104m->sn.build_send++;
    Iec104Data->Ctrl.I.RecvSn = ptIec104m->sn.build_recv;
        
    /*build ASDU , COT ,Addr*/
	if(ptNode[0].info.type == DATA_FLOAT){
		asdu->_type = IEC10X_C_SE_NC_1;
	}else{
		asdu->_type = IEC10X_C_SE_NA_1;
	}

    asdu->_num._sq = 0;
    asdu->_num._num = u8Count;
    asdu->_reason._reason = IEC10X_COT_ACT;
    asdu->_addr = ptIec104m->asdu_addr;

	ptr = info->_addr;
	for(i = 0; i < u8Count;i++)
	{
		/*build info addr*/
		addr = (uint32_t)ptNode[i].info.addr;
		memcpy(ptr, &addr, 3);
		ptr += 3;

		/*build info value*/	
		if(ptNode[0].info.type == DATA_FLOAT){
			memcpy(ptr, &ptNode[i].value.val.f, 4);
			ptr += 4;
		}else{
			memcpy(ptr, &ptNode[i].value.val.s16, 2);
			ptr += 2;
		}

		*ptr = 0x80 | 0x00; //qds=0x00
		ptr++;
	}

    /*len*/
    len = ptr - send_buf;
    Iec104Data->Len = len - 2;

    /* enqueue to the transmisson queue */
	ptIec104m->link_ctx->link_send(ptIec104m->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);

    return 0;
}

