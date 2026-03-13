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
#include "protocol/iec10x/wow_iec101s.h"

#define TAG "IEC101S"


struct __data_aligned__ iec101s_t{
	uint16_t asdu_addr;
	uint16_t back01;
	uint32_t back02;
	Iec10xState_T 	   stat;
	Iec10xTime_T	   time;
	iec10x_set_value_func_t   set_val_cb;
	iec10x_get_value_func_t   get_val_cb;
	iec10x_set_service_func_t serv_cb;
	void*			   set_val_priv;
	void*			   get_val_priv;
	void*			   serv_priv;
	NodeListInfo_T     node_list;
	ProtoPortCtx_T*    link_ctx;
	ByteArray_T*	   array;
	Iec10xThreadInfo_T thread;
	int			        tack;
	Iec101AsduBack_T    back;
};

static int iec101_s_build_link_status_ack(Iec101s_T* iec101s)
{
	uint8_t  send_buf[IEC101_STABLE_LEN];
    Iec101Data10_T* Iec101Data = (Iec101Data10_T*)send_buf;

    Iec101Data->_begin = IEC101_STABLE_BEGING;

    Iec101Data->_ctrl.up._dir = IEC101_CTRL_DIR_UP;
    Iec101Data->_ctrl.up._prm = IEC101_CTRL_PRM_SLAVE;
    Iec101Data->_ctrl.up._acd = IEC101_CTRL_ACD_NONE_DATA;
    Iec101Data->_ctrl.up._dfc = IEC101_CTRL_DFC_CAN_REC;
    Iec101Data->_ctrl.up._func = IEC101_CTRL_RES_LINK_STATUS;

    Iec101Data->_addr = iec101s->asdu_addr;
    Iec101Data->_cs   = send_buf[1]+send_buf[2]+send_buf[3];
    Iec101Data->_end  = IEC101_STABLE_END;

    iec101s->link_ctx->link_send(iec101s->link_ctx,send_buf, IEC101_STABLE_LEN,IEC10X_SEND_TIMEOUT);

	return 0;
}

static int iec101_s_build_request_confirm(Iec101s_T* iec101s)
{
	uint8_t  send_buf[IEC101_STABLE_LEN];
    Iec101Data10_T* Iec101Data = (Iec101Data10_T*)send_buf;


    Iec101Data->_begin = IEC101_STABLE_BEGING;

    Iec101Data->_ctrl.up._dir = IEC101_CTRL_DIR_UP;
    Iec101Data->_ctrl.up._prm = IEC101_CTRL_PRM_SLAVE;
    Iec101Data->_ctrl.up._acd = IEC101_CTRL_ACD_NONE_DATA;
    Iec101Data->_ctrl.up._dfc = IEC101_CTRL_DFC_CAN_REC;
    Iec101Data->_ctrl.up._func = IEC101_CTRL_CONFIRM_ACK;

    Iec101Data->_addr = iec101s->asdu_addr;
    Iec101Data->_cs   = send_buf[1]+send_buf[2]+send_buf[3];
    Iec101Data->_end  = IEC101_STABLE_END;

    iec101s->link_ctx->link_send(iec101s->link_ctx,send_buf, IEC101_STABLE_LEN,IEC10X_SEND_TIMEOUT);

    return 0;
}

static int iec101_s_build_request_user_nack(Iec101s_T* iec101s)
{
	uint8_t  send_buf[IEC101_STABLE_LEN];
    Iec101Data10_T* Iec101Data = (Iec101Data10_T*)send_buf;


    Iec101Data->_begin = IEC101_STABLE_BEGING;

    Iec101Data->_ctrl.up._dir = IEC101_CTRL_DIR_UP;
    Iec101Data->_ctrl.up._prm = IEC101_CTRL_PRM_SLAVE;
    Iec101Data->_ctrl.up._acd = IEC101_CTRL_ACD_NONE_DATA;
    Iec101Data->_ctrl.up._dfc = IEC101_CTRL_DFC_CAN_REC;
    Iec101Data->_ctrl.up._func = IEC101_CTRL_RES_USR_DATA_NACK;

    Iec101Data->_addr = iec101s->asdu_addr;
    Iec101Data->_cs   = send_buf[1]+send_buf[2]+send_buf[3];
    Iec101Data->_end  = IEC101_STABLE_END;

    iec101s->link_ctx->link_send(iec101s->link_ctx,send_buf, IEC101_STABLE_LEN,IEC10X_SEND_TIMEOUT);

    return 0;
}

static int iec101_s_build_request_user_ack(Iec101s_T* iec101s)
{
	uint8_t  send_buf[IEC101_STABLE_LEN];
    Iec101Data10_T* Iec101Data = (Iec101Data10_T*)send_buf;


    Iec101Data->_begin = IEC101_STABLE_BEGING;

    Iec101Data->_ctrl.up._dir = IEC101_CTRL_DIR_UP;
    Iec101Data->_ctrl.up._prm = IEC101_CTRL_PRM_SLAVE;
    Iec101Data->_ctrl.up._acd = IEC101_CTRL_ACD_WITH_DATA;
    Iec101Data->_ctrl.up._dfc = IEC101_CTRL_DFC_CAN_REC;
    Iec101Data->_ctrl.up._func = IEC101_CTRL_CONFIRM_ACK;

    Iec101Data->_addr = iec101s->asdu_addr;
    Iec101Data->_cs   = send_buf[1]+send_buf[2]+send_buf[3];
    Iec101Data->_end  = IEC101_STABLE_END;

    iec101s->link_ctx->link_send(iec101s->link_ctx,send_buf, IEC101_STABLE_LEN,IEC10X_SEND_TIMEOUT);

    return 0;
}

//暂时支持IEC10X_M_SP_NA_1
static int iec101_s_build_yx(Iec101s_T* iec101s,uint8_t reason, Slist_T* list)
{
	int i = 0;
    uint8_t len  = IEC101_STABLE_LEN;
	uint8_t *ptr = NULL;
	uint8_t cs_temp = 0;
	uint8_t  send_buf[IEC101_MAX_BUF_LEN];
	DevNodeInfo_T* node = NULL;
	DevNodeVal_T val;
		
	CHECK_RET_VAL_P(iec101s && list,-1,"param input invalid!\n");

	node = wow_slist_peek_head(list);
	CHECK_RET_VAL_P(node, -1,"wow_slist_peek_head failed!\n");
	
	memset(send_buf,0,IEC101_MAX_BUF_LEN);
    Iec101Data68_T* Iec101Data = (Iec101Data68_T*)send_buf;
    Iec101Asdu_T* asdu = (Iec101Asdu_T*)(Iec101Data->_asdu);
    Iec101AsduInfo_T* info = (Iec101AsduInfo_T*)(asdu->_info);
	
	/*head*/
	Iec101Data->_begin = Iec101Data->_begin_cfm = IEC101_VARIABLE_BEGING;

	/*Ctrol*/
	Iec101Data->_ctrl.up._dir = IEC101_CTRL_DIR_UP;
	Iec101Data->_ctrl.up._prm = IEC101_CTRL_PRM_SLAVE;
	Iec101Data->_ctrl.up._acd = IEC101_CTRL_ACD_WITH_DATA;
	Iec101Data->_ctrl.up._dfc = IEC101_CTRL_DFC_CAN_REC;
	Iec101Data->_ctrl.up._func = IEC101_CTRL_RES_USR_DATA_ACK;

	Iec101Data->_addr = iec101s->asdu_addr;
	/*asdu*/
	asdu->_type = IEC10X_M_SP_NA_1;
	asdu->_num._sq = 1;
	asdu->_num._num = wow_slist_size(list);
	asdu->_reason._pn = 0;
	asdu->_reason._test = 0;
	asdu->_reason._reason = reason;
	asdu->_addr = iec101s->asdu_addr;
	/*info*/
	info->_addr = node->addr;
	/*signal value*/
    ptr = info->_element;
    for(i=0; i < asdu->_num._num; i++)
	{
		node = wow_slist_peek_by_index(list, i);
		CHECK_RET_VAL_P(node, -1,"wow_slist_peek_head failed!\n");
		if(iec101s->get_val_cb){
			iec101s->get_val_cb(iec101s->get_val_priv,node,&val);
			*ptr = val.val.s8;
		}
		ptr++;
    }

	/*len*/
	len = ptr + 2 - send_buf;
	Iec101Data->_len = Iec101Data->_len_cfm = len-4-2;			/*-start-len-len-start	 -cs-end*/

	/*end*/
	for(i=4; i<len-2; i++){
		cs_temp += send_buf[i];
	}
	send_buf[len-2] = cs_temp;
	send_buf[len-1] = IEC101_VARIABLE_END;
	
    iec101s->link_ctx->link_send(iec101s->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);

    return 0;

}

//暂时支持IEC10X_M_ME_NC_1/IEC10X_M_ME_NA_1
static int iec101_s_build_yc(Iec101s_T* iec101s,uint8_t reason, Slist_T* list)
{
	int i = 0;
    uint8_t len = 0;
	uint8_t cs_temp = 0;
    uint8_t *ptr = NULL;
	uint8_t  send_buf[IEC101_MAX_BUF_LEN];
	DevNodeInfo_T* node = NULL;
	DevNodeVal_T val;
		
	CHECK_RET_VAL_P(iec101s && list,-1,"param input invalid!\n");

	node = wow_slist_peek_head(list);
	CHECK_RET_VAL_P(node, -1,"wow_slist_peek_head failed!\n");

	memset(send_buf,0,IEC101_MAX_BUF_LEN);
    Iec101Data68_T* Iec101Data = (Iec101Data68_T*)send_buf;
    Iec101Asdu_T* asdu = (Iec101Asdu_T*)(Iec101Data->_asdu);
    Iec101AsduInfo_T* info = (Iec101AsduInfo_T*)(asdu->_info);

	/*head*/
	Iec101Data->_begin = Iec101Data->_begin_cfm = IEC101_VARIABLE_BEGING;


	/*Ctrol*/
	Iec101Data->_ctrl.up._dir = IEC101_CTRL_DIR_UP;
	Iec101Data->_ctrl.up._prm = IEC101_CTRL_PRM_SLAVE;
	Iec101Data->_ctrl.up._acd = IEC101_CTRL_ACD_WITH_DATA;
	Iec101Data->_ctrl.up._dfc = IEC101_CTRL_DFC_CAN_REC;
	Iec101Data->_ctrl.up._func = IEC101_CTRL_RES_USR_DATA_ACK;
	
	Iec101Data->_addr = iec101s->asdu_addr;


    /*build ASDU , COT ,Addr*/
	if(node[0].type == DATA_FLOAT){
		asdu->_type = IEC10X_M_ME_NC_1;
	}else{
		asdu->_type = IEC10X_M_ME_NA_1;
	}
	asdu->_num._sq = 1;
	asdu->_num._num = wow_slist_size(list);
	asdu->_reason._pn = 0;
	asdu->_reason._test = 0;
	asdu->_reason._reason = reason;
	asdu->_addr = iec101s->asdu_addr;


    /*build info addr*/
    info->_addr = node->addr;

    /*Build Detect value*/
    ptr = info->_element;
    for(i = 0; i < asdu->_num._num; i++){
		node = wow_slist_peek_by_index(list, i);
		CHECK_RET_VAL_P(node, -1,"wow_slist_peek_head failed!\n");
		if(node->type == DATA_FLOAT){
			if(iec101s->get_val_cb){
				iec101s->get_val_cb(iec101s->get_val_priv,node,&val);
				memcpy(ptr, &val.val.f, 4);
			}
			ptr += 4;
		}else{
			if(iec101s->get_val_cb){
				iec101s->get_val_cb(iec101s->get_val_priv,node,&val);
				memcpy(ptr, &val.val.s16, 2);
			}
			ptr += 2;
		}
		*ptr = 0;//qds
		ptr++;
    }

    /*len*/
    len = ptr + 2 - send_buf;                    				 /* add cs+end*/
    Iec101Data->_len = Iec101Data->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += send_buf[i];
    }
    send_buf[len-2] = cs_temp;
    send_buf[len-1] = IEC101_VARIABLE_END;

    /* enqueue to the transmisson queue */
    iec101s->link_ctx->link_send(iec101s->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);

    return 0;
}

//暂时支持IEC10X_M_IT_NA_1
static int iec101_s_build_ym(Iec101s_T* iec101s,uint8_t reason, Slist_T* list)
{
    uint8_t len = 0;
	int i = 0;
    uint8_t *ptr = NULL;
	uint8_t cs_temp = 0;
	uint8_t  send_buf[IEC101_MAX_BUF_LEN];
	DevNodeInfo_T* node = NULL;
	DevNodeVal_T val;
	
	CHECK_RET_VAL_P(iec101s && list,-1,"param input invalid!\n");

	node = wow_slist_peek_head(list);
	CHECK_RET_VAL_P(node, -1,"wow_slist_peek_head failed!\n");


	memset(send_buf,0,IEC101_MAX_BUF_LEN);
    Iec101Data68_T* Iec101Data = (Iec101Data68_T*)send_buf;
    Iec101Asdu_T* asdu = (Iec101Asdu_T*)(Iec101Data->_asdu);
    Iec101AsduInfo_T* info = (Iec101AsduInfo_T*)(asdu->_info);

	/*head*/
	Iec101Data->_begin = Iec101Data->_begin_cfm = IEC101_VARIABLE_BEGING;


	/*Ctrol*/
	Iec101Data->_ctrl.up._dir = IEC101_CTRL_DIR_UP;
	Iec101Data->_ctrl.up._prm = IEC101_CTRL_PRM_SLAVE;
	Iec101Data->_ctrl.up._acd = IEC101_CTRL_ACD_WITH_DATA;
	Iec101Data->_ctrl.up._dfc = IEC101_CTRL_DFC_CAN_REC;
	Iec101Data->_ctrl.up._func = IEC101_CTRL_RES_USR_DATA_ACK;
	
	Iec101Data->_addr = iec101s->asdu_addr;

    /*build ASDU , COT ,Addr*/

	asdu->_type = IEC10X_M_IT_NA_1;
    asdu->_num._sq = 1;
    asdu->_num._num = wow_slist_size(list);
    asdu->_reason._reason = reason;
    asdu->_addr = iec101s->asdu_addr;

    /*build info addr*/
    info->_addr = node[0].addr;

    /*Build Detect value*/
    ptr = info->_element;
    for(i = 0; i < asdu->_num._num; i++){
		node = wow_slist_peek_by_index(list, i);
		CHECK_RET_VAL_P(node, -1,"wow_slist_peek_head failed!\n");
		if(iec101s->get_val_cb){
			iec101s->get_val_cb(iec101s->get_val_priv,node,&val);
			memcpy(ptr, &val.val.s32, 4);
		}
		ptr += 4;

		*ptr = i;
		ptr++;
    }

    /*len*/
    len = ptr + 2 - send_buf;                    				 /* add cs+end*/
    Iec101Data->_len = Iec101Data->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += send_buf[i];
    }
    send_buf[len-2] = cs_temp;
    send_buf[len-1] = IEC101_VARIABLE_END;

    /* enqueue to the transmisson queue */
    iec101s->link_ctx->link_send(iec101s->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);

    return 0;
}

static int iec101_s_build_call_ack(Iec101s_T* iec101s)
{
	int i = 0;
    uint8_t cs_temp = 0;
    uint8_t len = IEC101_STABLE_LEN;
	uint8_t  send_buf[IEC101_MAX_BUF_LEN];

	CHECK_RET_VAL_P(iec101s,-1,"param input invalid!\n");

	memset(send_buf,0,IEC101_MAX_BUF_LEN);
    Iec101Data68_T* Iec101Data = (Iec101Data68_T*)send_buf;
    Iec101Asdu_T* asdu = (Iec101Asdu_T*)(Iec101Data->_asdu);
    Iec101AsduInfo_T* info = (Iec101AsduInfo_T*)(asdu->_info);

    /*head*/
    Iec101Data->_begin = Iec101Data->_begin_cfm = IEC101_VARIABLE_BEGING;
    /*Ctrol*/
    Iec101Data->_ctrl.up._dir = IEC101_CTRL_DIR_UP;
    Iec101Data->_ctrl.up._prm = IEC101_CTRL_PRM_SLAVE;
    Iec101Data->_ctrl.up._acd = IEC101_CTRL_ACD_WITH_DATA;
    Iec101Data->_ctrl.up._dfc = IEC101_CTRL_DFC_CAN_REC;

    Iec101Data->_ctrl.up._func = IEC101_CTRL_RES_USR_DATA_ACK;

    Iec101Data->_addr = iec101s->asdu_addr;
    /*asdu*/
    asdu->_type = IEC10X_C_IC_NA_1;
    asdu->_num._sq = 0;
    asdu->_num._num = 1;
    asdu->_reason._pn = 0;
    asdu->_reason._test = 0;
    asdu->_reason._reason = IEC10X_COT_ACTCON;
    asdu->_addr = iec101s->asdu_addr;
    /*info*/
    info->_addr = IEC10X_INFO_ADDR_NONE;
    info->_element[0] = IEC10X_COT_INTROGEN;
    /*len*/
    len = IEC101_VARIABLE_LEN + asdu->_num._num;
    Iec101Data->_len = Iec101Data->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i = 4; i < len-2; i++){
        cs_temp += send_buf[i];
    }
    send_buf[len-2] = cs_temp;
    send_buf[len-1] = IEC101_VARIABLE_END;

	iec101s->link_ctx->link_send(iec101s->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);

    return 0;
}

static int iec101_s_build_call_finsh(Iec101s_T* iec101s,uint8_t qoi)
{
	int i = 0;
	uint8_t cs_temp = 0;
    uint8_t len = IEC101_STABLE_LEN;
	uint8_t  send_buf[IEC101_MAX_BUF_LEN];

	CHECK_RET_VAL_P(iec101s,-1,"param input invalid!\n");

	memset(send_buf,0,IEC101_MAX_BUF_LEN);
    Iec101Data68_T* Iec101Data = (Iec101Data68_T*)send_buf;
    Iec101Asdu_T* asdu = (Iec101Asdu_T*)(Iec101Data->_asdu);
    Iec101AsduInfo_T* info = (Iec101AsduInfo_T*)(asdu->_info);

    /*head*/
    Iec101Data->_begin = Iec101Data->_begin_cfm = IEC101_VARIABLE_BEGING;
    /*Ctrol*/
    Iec101Data->_ctrl.up._dir = IEC101_CTRL_DIR_UP;
    Iec101Data->_ctrl.up._prm = IEC101_CTRL_FCB_OPPO_NONE;
    Iec101Data->_ctrl.up._acd = IEC101_CTRL_ACD_NONE_DATA;
    Iec101Data->_ctrl.up._dfc = IEC101_CTRL_DFC_CAN_REC;
    Iec101Data->_ctrl.up._func = IEC101_CTRL_RES_USR_DATA_ACK;

    Iec101Data->_addr = iec101s->asdu_addr;
    /*asdu*/
    asdu->_type = IEC10X_C_IC_NA_1;
    asdu->_num._sq = 0;
    asdu->_num._num = 1;
    asdu->_reason._pn = 0;
    asdu->_reason._test = 0;
    asdu->_reason._reason = IEC10X_COT_ACTTERM;
    asdu->_addr = iec101s->asdu_addr;
    /*info*/
    info->_addr = IEC10X_INFO_ADDR_NONE;
    info->_element[0] = qoi;
    /*len*/
    len = IEC101_VARIABLE_LEN + asdu->_num._num;
    Iec101Data->_len = Iec101Data->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i = 4; i < len-2; i++){
        cs_temp += send_buf[i];
    }
    send_buf[len-2] = cs_temp;
    send_buf[len-1] = IEC101_VARIABLE_END;

	iec101s->link_ctx->link_send(iec101s->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);

    return 0;

}



static int iec101_s_asdu_yt(Iec101s_T* iec101s,Iec101Asdu_T* Iec10x_Asdu,uint16_t len)
{
	CHECK_RET_VAL_P(iec101s && Iec10x_Asdu,-1,"param input invalid!\n");

	int i = 0;
    uint16_t addr = 0;
    DevNodeInfo_T node;     
	DevNodeVal_T  val;
	
	uint8_t type = Iec10x_Asdu->_type;
    uint8_t n = Iec10x_Asdu->_num._num;
	uint8_t Sq = Iec10x_Asdu->_num._sq;
    Iec101AsduInfo_T* asdu_info = (Iec101AsduInfo_T*)(Iec10x_Asdu->_info);
	
	CHECK_RET_VAL_P(Iec10x_Asdu->_reason._reason == IEC10X_COT_ACT,-1,
			"iec101_s_asdu_yt error reason(%d) \n",Iec10x_Asdu->_reason._reason);
	
    switch(type){
        case IEC10X_C_SE_NA_1:
            if(Sq == 1){
				addr = asdu_info->_addr;
				Iec101Detect_T* detect = (Iec101Detect_T*)asdu_info->_element;
				memset(&node,0,sizeof(DevNodeVal_T));
				node.mode = DEV_TYPE_YT;
				node.type = DATA_SINT_16;
                for(i=0; i<n; i++,detect++){
					node.addr    = addr+i;
                    val.val.s16 = detect->_detect;
					if(detect->_qds & 0x80){
						goto out;
					}else if(iec101s->set_val_cb){
						 iec101s->set_val_cb(iec101s->set_val_priv,&node,&val);
					}
                }
				iec101s->stat.yk = IEC10X_STA_YK_CMD_EXEC;
            }else if(Sq == 0){
				Iec101DetectSq0_T* detect_Sq0 = (Iec101DetectSq0_T*)Iec10x_Asdu->_info;
				memset(&node,0,sizeof(DevNodeVal_T));
				node.mode = DEV_TYPE_YT;
				node.type = DATA_SINT_16;
                for(i=0; i<n; i++,detect_Sq0++){
					node.addr    = detect_Sq0->_addr;
                    val.val.s16 = detect_Sq0->_detect;
					if(detect_Sq0->_qds & 0x80){
						goto out;
					}else if(iec101s->set_val_cb){
						iec101s->set_val_cb(iec101s->set_val_priv,&node,&val);
					}
                } 
				iec101s->stat.yk = IEC10X_STA_YK_CMD_EXEC;
            }
            break;
        case IEC10X_C_SE_NC_1:
            if(Sq == 1){
				addr = asdu_info->_addr;
                Iec101DetectFloat_T* detect_f = (Iec101DetectFloat_T*)asdu_info->_element;
				memset(&node,0,sizeof(DevNodeVal_T));
				node.mode = DEV_TYPE_YT;
				node.type = DATA_FLOAT;
                for(i=0; i<n; i++, detect_f++){
					node.addr  = addr+i;
                    val.val.f = (float)detect_f->_detect; 
					if(detect_f->_qds & 0x80){
						goto out;
					}else if(iec101s->set_val_cb){
						iec101s->set_val_cb(iec101s->set_val_priv,&node,&val);
					}
                }
				iec101s->stat.yk = IEC10X_STA_YK_CMD_EXEC;
            }else if(Sq == 0){
                Iec101DetectSq0Float_T* detect_Sq0_f = (Iec101DetectSq0Float_T*)Iec10x_Asdu->_info;
				memset(&node,0,sizeof(DevNodeVal_T));
				node.mode = DEV_TYPE_YT;
				node.type = DATA_FLOAT;
                for(i=0; i<n; i++, detect_Sq0_f++){
					node.addr    = detect_Sq0_f->_addr;
                    val.val.s16 = (float)detect_Sq0_f->_detect;
					if(detect_Sq0_f->_qds & 0x80){
						goto out;
					}else if(iec101s->set_val_cb){
						iec101s->set_val_cb(iec101s->set_val_priv,&node,&val);
					}
                }
				iec101s->stat.yk = IEC10X_STA_YK_CMD_EXEC;
            }
            break;
        default:
            ModuleErrorLog(TAG,"iec101_s_asdu_yt error Type(%d) \n",type);
			if(iec101s->serv_cb){
				iec101s->serv_cb(iec101s->serv_priv,IEC10X_YT_CMD_FAILD,NULL);
			}
            return -1;
    }
out:
	iec101s->stat.yt = IEC10X_STA_YT_CMD_START;
	iec101s->back.yt_size = len - 8;
	iec101s->back.yt_bak = malloc(iec101s->back.yt_size);
	memcpy(iec101s->back.yt_bak,Iec10x_Asdu,iec101s->back.yt_size);

    return 0;
}

static int iec101_s_build_yt_ack(Iec101s_T* iec101s,uint8_t reason)
{
	int i = 0;
    uint8_t cs_temp = 0;
    uint8_t len = IEC101_STABLE_LEN;
	uint8_t  send_buf[IEC101_MAX_BUF_LEN];

	CHECK_RET_VAL_P(iec101s,-1,"param input invalid!\n");

	memset(send_buf,0,IEC101_MAX_BUF_LEN);
    Iec101Data68_T* Iec101Data = (Iec101Data68_T*)send_buf;
    Iec101Asdu_T* asdu = (Iec101Asdu_T*)(Iec101Data->_asdu);

    /*head*/
    Iec101Data->_begin = Iec101Data->_begin_cfm = IEC101_VARIABLE_BEGING;
    /*Ctrol*/
    Iec101Data->_ctrl.up._dir = IEC101_CTRL_DIR_UP;
    Iec101Data->_ctrl.up._prm = IEC101_CTRL_PRM_SLAVE;
    Iec101Data->_ctrl.up._acd = IEC101_CTRL_ACD_WITH_DATA;
    Iec101Data->_ctrl.up._dfc = IEC101_CTRL_DFC_CAN_REC;

    Iec101Data->_ctrl.up._func = IEC101_CTRL_RES_USR_DATA_ACK;

    Iec101Data->_addr = iec101s->asdu_addr;

	memcpy(asdu,iec101s->back.yt_bak,iec101s->back.yt_size);
    /*asdu*/
    asdu->_reason._reason = reason;

    /*len*/
    len = IEC101_VARIABLE_LEN + asdu->_num._num;
    Iec101Data->_len = Iec101Data->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i = 4; i < len-2; i++){
        cs_temp += send_buf[i];
    }
    send_buf[len-2] = cs_temp;
    send_buf[len-1] = IEC101_VARIABLE_END;

	iec101s->link_ctx->link_send(iec101s->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);

    return 0;
}

static int iec101_s_asdu_yk(Iec101s_T* iec101s,Iec101Asdu_T* Iec10x_Asdu,uint16_t len)
{

	CHECK_RET_VAL_P(iec101s && Iec10x_Asdu,-1,"param input invalid!\n");

    int i = 0;
    uint8_t *ptr = NULL;
    uint16_t addr = 0;
    DevNodeInfo_T node;     
	DevNodeVal_T val;

	uint8_t type = Iec10x_Asdu->_type;
	uint8_t n = Iec10x_Asdu->_num._num;
	uint8_t Sq = Iec10x_Asdu->_num._sq;
	Iec101AsduInfo_T* asdu_info = (Iec101AsduInfo_T*)(Iec10x_Asdu->_info);

	CHECK_RET_VAL_P(Iec10x_Asdu->_reason._reason == IEC10X_COT_ACT,-1,"iec101_s_asdu_yk error reason(%d) \n",Iec10x_Asdu->_reason._reason);

	if(asdu_info->_element[0] & 0x80){
		iec101s->stat.yk = IEC10X_STA_YK_CMD_START;
		iec101s->back.yk_size = len - 8;
		iec101s->back.yk_bak = malloc(iec101s->back.yk_size);
		memcpy(iec101s->back.yk_bak,Iec10x_Asdu,iec101s->back.yk_size);
		return 0;
	}else{
		iec101s->stat.yk = IEC10X_STA_YK_CMD_EXEC;
	}
	

    switch(type){
        case IEC10X_C_SC_NA_1:
            if(Sq == 1){
				memset(&node,0,sizeof(DevNodeVal_T));
				node.mode = DEV_TYPE_YK;
				node.type = DATA_SINT_8;
			    addr = asdu_info->_addr;
                ptr = asdu_info->_element;
                for(i=0; i<n; i++,ptr++){
					node.addr   = addr+i;
					val.val.s8 = *ptr & 0x01;
					if(((*ptr) & 0x80) == 0 && iec101s->set_val_cb){
						iec101s->set_val_cb(iec101s->set_val_priv,&node,&val);
					} 
                }
            }else if(Sq == 0){
            	memset(&node,0,sizeof(DevNodeVal_T));
				node.mode = DEV_TYPE_YK;
				node.type = DATA_SINT_8;
                ptr = Iec10x_Asdu->_info;
                for(i=0; i<n; i++,ptr++){
					memcpy(&addr, ptr, 2);
					ptr += 2; 
					node.addr   = addr;
                    val.val.s8 = *ptr & 0x01;;
					if(((*ptr) & 0x80) == 0 && iec101s->set_val_cb){
						iec101s->set_val_cb(iec101s->set_val_priv,&node,&val);
					} 
                } 
            }
            break;
        case IEC10X_C_DC_NA_1:
			if(Sq == 1){
				node.mode = DEV_TYPE_YK;
				node.type = DATA_SINT_8;
				addr = asdu_info->_addr;
                ptr = asdu_info->_element;
                for(i=0; i<n; i++,ptr++){
					node.addr   = addr+i;
					val.val.s8 = *ptr & 0x03;
					if(((*ptr) & 0x80) == 0 && iec101s->set_val_cb){
						iec101s->set_val_cb(iec101s->set_val_priv,&node,&val);
					} 
                }
            }else if(Sq == 0){
            	node.mode = DEV_TYPE_YK;
				node.type = DATA_SINT_8;
                ptr = Iec10x_Asdu->_info;
                for(i=0; i<n; i++,ptr++){
					memcpy(&addr, ptr, 2);
					ptr += 2;  
					node.addr   = addr;
                    val.val.s8 = *ptr & 0x03;;
					if(((*ptr) & 0x80) == 0 && iec101s->set_val_cb){
						iec101s->set_val_cb(iec101s->set_val_priv,&node,&val);
					} 
                } 
            }
            break;
        default:
			ModuleErrorLog(TAG,"iec101_s_asdu_yk error Type(%d) \n",type);
			if(iec101s->serv_cb){
				iec101s->serv_cb(iec101s->serv_priv,IEC10X_YK_CMD_FAILD,NULL);
			}
            return -1;
    }

    return 0;
}


static int iec101_s_build_yk_ack(Iec101s_T* iec101s,uint8_t reason)
{
	int i = 0;
    uint8_t cs_temp = 0;
    uint8_t len = IEC101_STABLE_LEN;
	uint8_t  send_buf[IEC101_MAX_BUF_LEN];

	CHECK_RET_VAL_P(iec101s,-1,"param input invalid!\n");

	memset(send_buf,0,IEC101_MAX_BUF_LEN);
    Iec101Data68_T* Iec101Data = (Iec101Data68_T*)send_buf;
    Iec101Asdu_T* asdu = (Iec101Asdu_T*)(Iec101Data->_asdu);

    /*head*/
    Iec101Data->_begin = Iec101Data->_begin_cfm = IEC101_VARIABLE_BEGING;
    /*Ctrol*/
    Iec101Data->_ctrl.up._dir = IEC101_CTRL_DIR_UP;
    Iec101Data->_ctrl.up._prm = IEC101_CTRL_PRM_SLAVE;
    Iec101Data->_ctrl.up._acd = IEC101_CTRL_ACD_WITH_DATA;
    Iec101Data->_ctrl.up._dfc = IEC101_CTRL_DFC_CAN_REC;

    Iec101Data->_ctrl.up._func = IEC101_CTRL_RES_USR_DATA_ACK;

    Iec101Data->_addr = iec101s->asdu_addr;

	memcpy(asdu,iec101s->back.yk_bak,iec101s->back.yk_size);
    /*asdu*/
    asdu->_reason._reason = reason;

    /*len*/
    len = IEC101_VARIABLE_LEN + asdu->_num._num;
    Iec101Data->_len = Iec101Data->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i = 4; i < len-2; i++){
        cs_temp += send_buf[i];
    }
    send_buf[len-2] = cs_temp;
    send_buf[len-1] = IEC101_VARIABLE_END;

	iec101s->link_ctx->link_send(iec101s->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);

    return 0;
}



static int iec101_s_asdu_call_all(Iec101s_T* iec101s,Iec101Asdu_T* Iec10x_Asdu)
{
    Iec101AsduInfo_T* asdu_info = (Iec101AsduInfo_T*)(Iec10x_Asdu->_info);
    uint8_t qoi = asdu_info->_element[0];

	CHECK_RET_VAL_P(asdu_info->_addr == 0,-1,"call cmd active error addr(%x)\n" ,asdu_info->_addr);
 	CHECK_RET_VAL_P(Iec10x_Asdu->_reason._reason == IEC10X_COT_ACT,-1,
		"call cmd error reason(%d)!\n",Iec10x_Asdu->_reason._reason);

    switch(qoi){
        case IEC10X_COT_INTROGEN:
			iec101s->stat.callall = IEC10X_STA_CALL_ALL_START;
            break;
        case IEC10X_COT_INTRO1:
        case IEC10X_COT_INTRO2:
            break;
        default:
        	ModDebug_E(TAG,"invalid frame qoi(%d)!\n",qoi);
            return -1;
    }

    return 0;
}

static int iec101_s_asdu_call_pulse(Iec101s_T* iec101s,Iec101Asdu_T* Iec10x_Asdu)
{
    Iec101AsduInfo_T* asdu_info = (Iec101AsduInfo_T*)(Iec10x_Asdu->_info);
    uint8_t qcc = asdu_info->_element[0];

	CHECK_RET_VAL_P(asdu_info->_addr == 0,-1,"call cmd active error addr(%x)\n" ,asdu_info->_addr);
 	CHECK_RET_VAL_P(Iec10x_Asdu->_reason._reason == IEC10X_COT_ACT,-1,
		"call cmd error reason(%d)!\n",Iec10x_Asdu->_reason._reason);

    switch(qcc){
        case IEC10X_COT_REQCOGEN:
			iec101s->stat.callall = IEC10X_STA_CALL_PULSE_START;
			
            break;
        case IEC10X_COT_REQCO1:
        case IEC10X_COT_REQCO2:
            break;
        default:
        	ModDebug_E(TAG,"invalid frame qcc(%d)!\n",qcc);
            return -1;
    }

    return 0;
}


int iec101_s_asdu_clock(Iec101s_T* iec101s,Iec101Asdu_T* Iec10x_Asdu)
{
	iec101_s_build_request_confirm(iec101s);

	return 0;
}
int iec101_s_deal_10(Iec101s_T* iec101s,uint8_t *buf, uint16_t len)
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
				iec101_s_build_request_confirm(iec101s);
                break;
            case IEC101_CTRL_SEND_DATA:
				ModDebug_D(TAG,"frame send data!\n");
                break;
            case IEC101_CTRL_REQ_LINK_STATUS:
				ModDebug_D(TAG,"frame request link status!\n");
				iec101_s_build_link_status_ack(iec101s);
				iec101s->stat.init = IEC10X_STA_LINK_CONNECT_OK;
                break;
			case IEC101_CTRL_REQ_USR_DATA_1:
				iec101s->tack = 1;
				break;
			case IEC101_CTRL_REQ_USR_DATA_2:
				iec101_s_build_request_user_nack(iec101s);
				break;
            default:
				ModDebug_E(TAG,"invalid down frame(%d)!\n",cfun);
                break;
        }
    }else if(sta == IEC101_CTRL_PRM_SLAVE){

        switch(cfun){
            case IEC101_CTRL_CONFIRM_ACK:
				ModDebug_D(TAG,"frame function requst confirm!\n");
                break;
            default:
                ModDebug_E(TAG,"invalid up frame(%d)!\n",cfun);
                break;
        }
    }

    return 0;
}

int iec101_s_deal_68(Iec101s_T* iec101s,uint8_t *buf, uint16_t len)
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
				case IEC10X_C_IC_NA_1:
					ModDebug_D(TAG,"frame asdu type call all cmd...!\n");	
		            iec101_s_asdu_call_all(iec101s,Iec10x_Asdu);
		            break;	
				case IEC10X_C_CI_NA_1:
					ModDebug_D(TAG,"frame asdu type call pulse cmd...!\n");	
		            iec101_s_asdu_call_pulse(iec101s,Iec10x_Asdu);
		
				case IEC10X_C_CD_NA_1:
					ModDebug_D(TAG,"frame asdu type delay cmd...!\n");
					iec101_s_build_request_confirm(iec101s);
					break;
				case IEC10X_C_SC_NA_1:
				case IEC10X_C_DC_NA_1:
					ModDebug_D(TAG,"frame asdu yk cmd...!\n");
					iec101_s_asdu_yk(iec101s,Iec10x_Asdu,len);
					break;
				case IEC10X_C_SE_NA_1:
				case IEC10X_C_SE_NC_1:
					ModDebug_D(TAG,"frame asdu yt cmd...!\n");
					iec101_s_asdu_yt(iec101s,Iec10x_Asdu,len);
					break;
				default:
					ModDebug_E(TAG,"invalid ack frame type(%d)!\n",Iec10x_Asdu->_type);
                    break;
            }
			break;
		case IEC101_CTRL_RES_USR_DATA_ACK:
			switch(Iec10x_Asdu->_type){
				case IEC10X_C_CS_NA_1:
					ModDebug_D(TAG,"frame asdu type clock cmd...!\n");
					iec101_s_asdu_clock(iec101s,Iec10x_Asdu);
					break;		
			}
			break;
        default:
			ModDebug_E(TAG,"invalid frame function code(%d)!\n",cfun);
            break;
    }

    return 0;
}


static void iec101_s_reset_param(Iec101s_T* iec101s)
{
	CHECK_RET_VOID_P(iec101s,"param input arg invalid!\n");

	iec101s->tack = 0;

	iec101s->stat.init    = IEC10X_STA_LINK_INIT;
	iec101s->stat.callall = IEC10X_STA_CALL_ALL_IDLE;
	iec101s->stat.callpulse = IEC10X_STA_CALL_PULSE_IDLE;
	iec101s->stat.yt      = IEC10X_STA_YT_CMD_IDLE;
	iec101s->stat.yk	  = IEC10X_STA_YK_CMD_IDLE;
	iec101s->stat.test    = IE10X_STA_TESTER_START;
}

static void iec101_s_state_machine_work(Iec101s_T* iec101s)
{
	switch(iec101s->stat.init){
		case IEC10X_STA_LINK_INIT:
			iec101_s_reset_param(iec101s);
			iec101s->stat.init = IEC10X_STA_LINK_OPEN;
			break;
		case IEC10X_STA_LINK_OPEN:
			iec101s->stat.init = IEC10X_STA_LINK_CONNECT;
			break;
		case IEC10X_STA_LINK_CONNECT:
			break;
		case IEC10X_STA_LINK_CONNECT_OK:
			iec101s->stat.init = IEC10X_STA_LINK_IDLE;
			if(iec101s->serv_cb){
				iec101s->serv_cb(iec101s->serv_priv,IEC10X_CONNECTED,NULL);
			}
			break;
		case IEC10X_STA_LINK_DISCONNECT:
			iec101s->link_ctx->link_close(iec101s->link_ctx);
			iec101s->stat.init = IEC10X_STA_LINK_INIT;
			if(iec101s->serv_cb){
				iec101s->serv_cb(iec101s->serv_priv,IEC10X_DISCONNECTED,NULL);
			}
			break;
		case IEC10X_STA_LINK_CLOSE:
			iec101s->stat.init = IEC10X_STA_LINK_INIT;
			break;
		case IEC10X_STA_LINK_IDLE:
		default:
			break;
	}

}

static void iec101_s_state_machine_callall(Iec101s_T* iec101s)
{

	switch(iec101s->stat.callall){
			case IEC10X_STA_CALL_ALL_IDLE:
				break;
			case IEC10X_STA_CALL_ALL_START:
				iec101s->time.callall = wow_time_get_msec();
				iec101_s_build_request_user_ack(iec101s);
				iec101s->stat.callall = IEC10X_STA_CALL_ALL_CONFIRM;
				break;
			case IEC10X_STA_CALL_ALL_CONFIRM:
				if(iec101s->tack == 1){
					iec101_s_build_call_ack(iec101s);
					iec101s->tack = 0;
					iec101s->stat.callall = IEC10X_STA_CALL_ALL_YX;
				}
				if(wow_time_get_msec() - iec101s->time.callall > IEC10X_CALL_ALL_TIMEOUT){
					iec101s->stat.callall = IEC10X_STA_CALL_ALL_FAILD;
				}
				break;
			case IEC10X_STA_CALL_ALL_YX:
				if(iec101s->tack == 1){
					int i = 0;
					int size = wow_slist_size(iec101s->node_list.yx_list);
					for(i = 0; i < size; i++){
						Slist_T* list = wow_slist_peek_by_index(iec101s->node_list.yx_list,i);
						iec101_s_build_yx(iec101s,IEC10X_COT_INTROGEN,list);
						usleep(5*1000);
					}
					iec101s->stat.callall = IEC10X_STA_CALL_ALL_YC; 
					iec101s->tack = 0;
				}
				if(wow_time_get_msec() - iec101s->time.callall > IEC10X_CALL_ALL_TIMEOUT){
					iec101s->stat.callall = IEC10X_STA_CALL_ALL_FAILD;
				}
				break;
			case IEC10X_STA_CALL_ALL_YC:
				if(iec101s->tack == 1){
					int i = 0;
					int size = 0;
					
					size = wow_slist_size(iec101s->node_list.yc_n_list);
					for(i = 0; i < size; i++){
						Slist_T* list = wow_slist_peek_by_index(iec101s->node_list.yc_n_list,i);
						iec101_s_build_yc(iec101s,IEC10X_COT_INTROGEN,list);
						usleep(5*1000);
					}
					
					size = wow_slist_size(iec101s->node_list.yc_f_list);
					for(i = 0; i < size; i++){
						Slist_T* list = wow_slist_peek_by_index(iec101s->node_list.yc_f_list,i);
						iec101_s_build_yc(iec101s,IEC10X_COT_INTROGEN,list);
						usleep(5*1000);
					}

					iec101s->stat.callall = IEC10X_STA_CALL_ALL_SUCCESS;	
					iec101s->tack = 0;
				}
				if(wow_time_get_msec() - iec101s->time.callall > IEC10X_CALL_ALL_TIMEOUT){
					iec101s->stat.callall = IEC10X_STA_CALL_ALL_FAILD;
				}
				break;		
			case IEC10X_STA_CALL_ALL_SUCCESS:
				if(iec101s->tack == 1){
					iec101_s_build_call_finsh(iec101s,IEC10X_COT_INTROGEN);
					if(iec101s->serv_cb){
						iec101s->serv_cb(iec101s->serv_priv,IEC10X_CALL_ALL_SUCCESS,NULL);
					}
					iec101s->tack = 0;
					iec101s->stat.callall = IEC10X_STA_CALL_ALL_IDLE;	
				}

				break;
			case IEC10X_STA_CALL_ALL_FAILD:
				if(iec101s->serv_cb){
					iec101s->serv_cb(iec101s->serv_priv,IEC10X_CALL_ALL_FAILD,NULL);
				}
				iec101s->stat.callall = IEC10X_STA_CALL_ALL_IDLE;
				break;
		}

}

static void iec101_s_state_machine_callpulse(Iec101s_T* iec101s)
{
	switch(iec101s->stat.callpulse){
		case IEC10X_STA_CALL_PULSE_IDLE:
			break;
		case IEC10X_STA_CALL_PULSE_START:
			iec101s->time.callpulse = wow_time_get_msec();
			iec101_s_build_request_user_ack(iec101s);
			iec101s->stat.callpulse = IEC10X_STA_CALL_PULSE_CONFIRM;
			break;
		case IEC10X_STA_CALL_PULSE_CONFIRM:
			if(iec101s->tack == 1){
				iec101_s_build_call_ack(iec101s);
				iec101s->tack = 0;
				iec101s->stat.callpulse = IEC10X_STA_CALL_PULSE_YM;
			}
			if(wow_time_get_msec() - iec101s->time.callpulse > IEC10X_CALL_ALL_TIMEOUT){
				iec101s->stat.callpulse = IEC10X_STA_CALL_PULSE_IDLE;
			}
			break;
		case IEC10X_STA_CALL_PULSE_YM:
			if(iec101s->tack == 1){
				int i = 0;
				int size = wow_slist_size(iec101s->node_list.ym_list);
				for(i = 0; i < size; i++){
					Slist_T* list = wow_slist_peek_by_index(iec101s->node_list.ym_list,i);
					iec101_s_build_ym(iec101s,IEC10X_COT_REQCOGEN,list);
					usleep(5*1000);
				}

				iec101s->stat.callpulse = IEC10X_STA_CALL_PULSE_SUCCESS; 
				iec101s->tack = 0;
			}
			if(wow_time_get_msec() - iec101s->time.callpulse > IEC10X_CALL_ALL_TIMEOUT){
				iec101s->stat.callall = IEC10X_STA_CALL_PULSE_IDLE;
			}
			break;

		case IEC10X_STA_CALL_PULSE_SUCCESS:
			if(iec101s->tack == 1){
				iec101_s_build_call_finsh(iec101s,IEC10X_COT_INTROGEN);
				if(iec101s->serv_cb){
					iec101s->serv_cb(iec101s->serv_priv,IEC10X_CALL_PULSE_SUCCESS,NULL);
				}
				iec101s->tack = 0;
				iec101s->stat.callpulse = IEC10X_STA_CALL_PULSE_IDLE;	
			}
			break;
		case IEC10X_STA_CALL_PULSE_FAILD:
			break;
	}

}

static void iec101_s_state_machine_yt(Iec101s_T* iec101s)
{
	switch(iec101s->stat.yt){
		case IEC10X_STA_YT_CMD_IDLE:
			break;
		case IEC10X_STA_YT_CMD_START:
			iec101s->time.yt = wow_time_get_msec();
		
			iec101_s_build_request_user_ack(iec101s);
			iec101s->stat.yt = IEC10X_STA_YT_CMD_CONFIRM;
	
			if(wow_time_get_msec() - iec101s->time.yt > IEC10X_YT_CMD_TIMEOUT){
				iec101s->stat.yt = IEC10X_STA_YT_CMD_FAILD;
			}
			break;
		case IEC10X_STA_YT_CMD_CONFIRM:
			if(iec101s->tack == 1){
				iec101_s_build_yt_ack(iec101s,IEC10X_COT_ACTCON);
				iec101s->tack = 0;
			}
			if(wow_time_get_msec() - iec101s->time.yt > IEC10X_YT_CMD_TIMEOUT){
				iec101s->stat.yt = IEC10X_STA_YT_CMD_FAILD;
			}
			break;
		case IEC10X_STA_YT_CMD_EXEC:
			iec101_s_build_request_user_ack(iec101s);
			iec101s->stat.yt = IEC10X_STA_YT_CMD_SUCCESS;
			iec101s->tack = 0;
			if(wow_time_get_msec() - iec101s->time.yt > IEC10X_YT_CMD_TIMEOUT){
				iec101s->stat.yt = IEC10X_STA_YT_CMD_FAILD;
			}
			break;	
		case IEC10X_STA_YT_CMD_SUCCESS:
			if(iec101s->tack == 1){
				iec101_s_build_yt_ack(iec101s,IEC10X_COT_ACTTERM);
				if(iec101s->serv_cb){
					iec101s->serv_cb(iec101s->serv_priv,IEC10X_YT_CMD_SUCCESS,NULL);
				}
				iec101s->tack = 0;
			}		
			iec101s->stat.yt = IEC10X_STA_YT_CMD_IDLE;				
			break;
		case IEC10X_STA_YT_CMD_FAILD:
			if(iec101s->serv_cb){
				iec101s->serv_cb(iec101s->serv_priv,IEC10X_YT_CMD_FAILD,NULL);
			}
			iec101s->stat.yt = IEC10X_STA_YT_CMD_IDLE;	
			break;
	}

}


static void iec101_s_state_machine_yk(Iec101s_T* iec101s)
{
	switch(iec101s->stat.yk){
		case IEC10X_STA_YK_CMD_IDLE:
			break;
		case IEC10X_STA_YK_CMD_START:
			iec101s->time.yk = wow_time_get_msec();
		
			iec101_s_build_request_user_ack(iec101s);
			iec101s->stat.yk = IEC10X_STA_YK_CMD_CONFIRM;

			if(wow_time_get_msec() - iec101s->time.yk > IEC10X_YK_CMD_TIMEOUT){
				iec101s->stat.yk = IEC10X_STA_YK_CMD_FAILD;
			}
			break;
		case IEC10X_STA_YK_CMD_CONFIRM:
			if(iec101s->tack == 1){
				iec101_s_build_yk_ack(iec101s,IEC10X_COT_ACTCON);
				iec101s->tack = 0;
			}
			if(wow_time_get_msec() - iec101s->time.yk > IEC10X_YK_CMD_TIMEOUT){
				iec101s->stat.yk = IEC10X_STA_YK_CMD_FAILD;
			}
			break;
		case IEC10X_STA_YK_CMD_EXEC:
			iec101_s_build_request_user_ack(iec101s);
			iec101s->stat.yt = IEC10X_STA_YK_CMD_SUCCESS;
			iec101s->tack = 0;
			if(wow_time_get_msec() - iec101s->time.yk > IEC10X_YK_CMD_TIMEOUT){
				iec101s->stat.yk = IEC10X_STA_YK_CMD_FAILD;
			}
			break;	
		case IEC10X_STA_YK_CMD_SUCCESS:
			if(iec101s->tack == 1){	
				iec101_s_build_yk_ack(iec101s,IEC10X_COT_ACTTERM);
				if(iec101s->back.yk_bak){
					free(iec101s->back.yk_bak);
					iec101s->back.yk_bak = NULL;
				}
				if(iec101s->serv_cb){
					iec101s->serv_cb(iec101s->serv_priv,IEC10X_YK_CMD_SUCCESS,NULL);
				}
				iec101s->tack = 0;
			}
			iec101s->stat.yk = IEC10X_STA_YK_CMD_IDLE;				

			break;
		case IEC10X_STA_YK_CMD_FAILD:
			if(iec101s->back.yk_bak){
				free(iec101s->back.yk_bak);
				iec101s->back.yk_bak = NULL;
			}
			if(iec101s->serv_cb){
				iec101s->serv_cb(iec101s->serv_priv,IEC10X_YK_CMD_FAILD,NULL);
			}
			iec101s->stat.yk = IEC10X_STA_YK_CMD_IDLE;	
			break;
	}

}



static void iec101_s_state_machine(Iec101s_T* iec101s)
{		
	iec101_s_state_machine_work(iec101s);
	
	if(iec101s->stat.init != IEC10X_STA_LINK_IDLE) return;
	
	iec101_s_state_machine_callall(iec101s);
	iec101_s_state_machine_callpulse(iec101s);
	iec101_s_state_machine_yt(iec101s);
	iec101_s_state_machine_yk(iec101s);
}

static  void iec101_s_frame_parse(Iec101s_T* iec101s)
{
	int index_10 = -1;
	int index_68 = -1;

	index_10 = wow_byte_array_find(iec101s->array,IEC101_STABLE_BEGING);
	index_68 = wow_byte_array_find(iec101s->array,IEC101_VARIABLE_BEGING);

	CHECK_RET_VOID(index_10 >= 0 || index_68 >= 0);

	if(index_10 < 0 && index_68 >= 0){
exec_68:		
		wow_byte_array_remove(iec101s->array,0,index_68);
		CHECK_RET_VOID(wow_byte_array_size(iec101s->array) >= IEC101_STABLE_LEN);
		
		uint8_t* data = wow_byte_array_data(iec101s->array);
		uint8_t tlen = data[1];
		CHECK_RET_VOID(wow_byte_array_size(iec101s->array) >= tlen + IEC101_STABLE_LEN);
		
		if(data[tlen + IEC101_STABLE_LEN -1] == IEC101_VARIABLE_END){
			iec101_s_deal_68(iec101s,data ,tlen + IEC101_STABLE_LEN);
			wow_byte_array_remove(iec101s->array,0,tlen + IEC101_STABLE_LEN);
		}else{
			wow_byte_array_remove(iec101s->array,0,1);
		}
						
	}else if(index_10 >= 0  && index_68 < 0){
exec_10:	
		wow_byte_array_remove(iec101s->array,0,index_10);
		CHECK_RET_VOID(wow_byte_array_size(iec101s->array) >= IEC101_STABLE_LEN);
		uint8_t* data = wow_byte_array_data(iec101s->array);
		
		if(data[IEC101_STABLE_LEN -1] == IEC101_VARIABLE_END){
			iec101_s_deal_10(iec101s,data,IEC101_STABLE_LEN);
			wow_byte_array_remove(iec101s->array,0,IEC101_STABLE_LEN);
		}else{
			wow_byte_array_remove(iec101s->array,0,1);
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



static void* iec101_s_exec_thread(void* arg)
{
	Iec101s_T* iec101s = (Iec101s_T*)arg;
	CHECK_RET_VAL_P(iec101s,NULL,"param input arg invalid!\n");


	int rlen = 0;
	uint8_t recv_buf[1500]={0};

	while(iec101s->thread.tflag){
		rlen = iec101s->link_ctx->link_recv(iec101s->link_ctx,recv_buf,1500,IEC10X_RECV_TIMEOUT);
		if(rlen > 0){
			wow_byte_array_append_ndata(iec101s->array,recv_buf,rlen);
		}
		
		iec101_s_frame_parse(iec101s);


		iec101_s_state_machine(iec101s);
	}
	
	wow_mutex_lock(&iec101s->thread.lock);
	wow_mutex_cond_signal(&iec101s->thread.cond);
	wow_mutex_unlock(&iec101s->thread.lock);

	return NULL;
}


/*brief    Iec101从设备初始化
 *param ： u16Addr: 公共地址 
 *param ： ptUartPar:  硬件参数
 *return： 成功返回iec101s操作标识 失败返回NULL
 */
Iec101s_T*  wow_iec101_s_init(uint16_t u16Addr,ProtoUartParam_T* ptUartPar)
{
	CHECK_RET_VAL_ERRNO_P(ptUartPar,-PARAM_INPUT_DATA_IS_NULL, "param input data invalid!\n");
		
	Iec101s_T* iec101s = CALLOC(1, Iec101s_T);
	CHECK_RET_VAL_ERRNO_P(iec101s,-SYSTEM_MALLOC_FAILED,"malloc iec101s faild!\n");
	
	/*参数初始化*/
	iec101_s_reset_param(iec101s);
	iec101s->asdu_addr = u16Addr;
	iec101s->node_list.yx_list = wow_slist_create();
	CHECK_RET_GOTO_ERRNO_P(iec101s->node_list.yx_list,out1,-SLIST_CREATE_FAILED,"wow_slist_create faild!\n");
	iec101s->node_list.yc_n_list = wow_slist_create();
	CHECK_RET_GOTO_ERRNO_P(iec101s->node_list.yc_n_list,out1,-SLIST_CREATE_FAILED,"wow_slist_create faild!\n");
	iec101s->node_list.yc_f_list = wow_slist_create();
	CHECK_RET_GOTO_ERRNO_P(iec101s->node_list.yc_f_list,out1,-SLIST_CREATE_FAILED,"wow_slist_create faild!\n");
	iec101s->node_list.ym_list = wow_slist_create();
	CHECK_RET_GOTO_ERRNO_P(iec101s->node_list.ym_list,out1,-SLIST_CREATE_FAILED,"wow_slist_create faild!\n");

	iec101s->array = wow_byte_array_new(2048);
	CHECK_RET_GOTO_ERRNO_P(iec101s->array,out1,-BYTE_ARRAY_CREATE_ERRNOR,"wow_byte_array_new faild!\n");

	/*建立连接*/
	iec101s->link_ctx = proto_port_uart_init(ptUartPar);
	CHECK_RET_GOTO_ERRNO_P(iec101s->link_ctx,out2,-PROTO_UART_INIT_ERROR,"Iec101Data port init faild!\n");
	
	/*IEC101状态机*/
	iec101s->thread.tflag = 1;
	wow_mutex_lock_init(&iec101s->thread.lock);
	wow_mutex_cond_init(&iec101s->thread.cond);

	return iec101s;
out2:
	wow_byte_array_free(&iec101s->array);
out1:
	FREE(iec101s);
	return NULL;
}

static void free_node_list(void *data)
{
	CHECK_RET_VOID(data);
	wow_slist_destroy((Slist_T**)&data,NULL);
}


/*brief    退出Iec101从设备
 *param ： pptIec101S:iec101s操作标识
 *return： 无
 */
__EX_API__ void wow_iec101_s_exit(Iec101s_T** pptIec101s)
{
	CHECK_RET_VOID(pptIec101s && *pptIec101s);

	Iec101s_T* iec101s= (Iec101s_T*)*pptIec101s;

	//安全退出线程
	wow_mutex_lock(&iec101s->thread.lock);
	iec101s->thread.tflag= 0;
	wow_mutex_cond_wait(&iec101s->thread.lock,&iec101s->thread.cond,-1);
	wow_mutex_unlock(&iec101s->thread.lock);

	proto_port_uart_exit(&iec101s->link_ctx);
	
	if(iec101s->node_list.yx_list){
		wow_slist_destroy(&iec101s->node_list.yx_list,free_node_list);
	}
	if(iec101s->node_list.yc_n_list){
		wow_slist_destroy(&iec101s->node_list.yc_n_list,free_node_list);
	}
	if(iec101s->node_list.yc_f_list){
		wow_slist_destroy(&iec101s->node_list.yc_f_list,free_node_list);
	}
	if(iec101s->node_list.ym_list){
		wow_slist_destroy(&iec101s->node_list.ym_list,free_node_list);
	}

	wow_byte_array_free(&iec101s->array);
	
	FREE(iec101s);
	*pptIec101s = NULL;
}

/*brief    启动Iec101从设备
 *param ： ptIec101s:iec101s操作标识
 *return： 成功返回0 失败返回-1
 */
__EX_API__ int  wow_iec101_s_start(Iec101s_T* ptIec101s)
{
	CHECK_RET_VAL_P(ptIec101s,-PARAM_INPUT_STRUCT_IS_NULL,"param input strcuct invalid!\n");

	int ret = ptIec101s->link_ctx->link_open(ptIec101s->link_ctx);
	CHECK_RET_VAL_P(ret == 0,-PROTO_LINK_OPEN_ERROR,"link_open failed!\n");

	ptIec101s->thread.thread	= wow_thread_create("iec101s",iec101_s_exec_thread,ptIec101s);
	CHECK_RET_VAL_P(ptIec101s->thread.thread,-THREAD_CREATE_FAILED,"wow_thread_create failed!\n");

	return 0;
}

/*brief    Iec101设置数据节点信息
 *param ： ptIec101s:iec101s操作标识
 *param ： ptNode:  节点信息
 *param ： nCount: 节点个数
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_iec101_s_set_node_info(Iec101s_T* ptIec101s,DevNodeInfo_T* ptNode,int nCount)
{
	int i = 0;
	int baddr = -1;
	int ret = 0;
	Slist_T* list = NULL;

	CHECK_RET_VAL_P(ptIec101s,-PARAM_INPUT_STRUCT_IS_NULL,"param input strcuct invalid!\n");

	///<遥信点
	for(i = 0; i < nCount;i++){
		CHECK_RET_CONTINUE(ptNode[i].mode == DEV_TYPE_YX);
		if(baddr == ptNode[i].addr + 1){
			
			ret = wow_slist_insert_tail(list,&ptNode[i]);
			CHECK_RET_VAL_P(ret == 0,-SLIST_CREATE_FAILED,"wow_slist_insert_tail failed!\n");
			if(wow_slist_size(list) >= 200) goto here1;
		}else{
here1:			
			list = wow_slist_create();
			CHECK_RET_VAL_P(list,-1,"wow_slist_create failed!\n");
			ret = wow_slist_insert_tail(ptIec101s->node_list.yx_list,list);
			CHECK_RET_VAL_P(ret == 0,-SLIST_INSERT_FAILED,"wow_slist_insert_tail failed!\n");
		}
		baddr = ptNode[i].addr;
	}


	///<归一化遥测点
	baddr = -1;
	for(i = 0; i < nCount;i++){
		CHECK_RET_CONTINUE(ptNode[i].mode == DEV_TYPE_YC && ptNode[i].type != DATA_FLOAT);
		if(baddr == ptNode[i].addr + 1){
			ret = wow_slist_insert_tail(list,&ptNode[i]);
			CHECK_RET_VAL_P(ret == 0,-SLIST_CREATE_FAILED,"wow_slist_insert_tail failed!\n");
			if(wow_slist_size(list) >= 100) goto here2;
		}else{
here2:			
			list = wow_slist_create();
			CHECK_RET_VAL_P(list,-1,"wow_slist_create failed!\n");
			ret = wow_slist_insert_tail(ptIec101s->node_list.yc_n_list,list);
			CHECK_RET_VAL_P(ret == 0,-SLIST_INSERT_FAILED,"wow_slist_insert_tail failed!\n");
		}
		baddr = ptNode[i].addr;
	}

	///<短浮点遥测点
	baddr = -1;
	for(i = 0; i < nCount;i++){
		CHECK_RET_CONTINUE(ptNode[i].mode == DEV_TYPE_YC && ptNode[i].type == DATA_FLOAT);
		if(baddr == ptNode[i].addr + 1){
			ret = wow_slist_insert_tail(list,&ptNode[i]);
			CHECK_RET_VAL_P(ret == 0,-SLIST_CREATE_FAILED,"wow_slist_insert_tail failed!\n");
			if(wow_slist_size(list) >= 50) goto here3;
		}else{
here3:			
			list = wow_slist_create();
			CHECK_RET_VAL_P(list,-1,"wow_slist_create failed!\n");
			ret = wow_slist_insert_tail(ptIec101s->node_list.yc_f_list,list);
			CHECK_RET_VAL_P(ret == 0,-SLIST_INSERT_FAILED,"wow_slist_insert_tail failed!\n");
		}
		baddr = ptNode[i].addr;
	}

	///<遥脉点
	baddr = -1;
	for(i = 0; i < nCount;i++){
		CHECK_RET_CONTINUE(ptNode[i].mode == DEV_TYPE_YM);
		if(baddr == ptNode[i].addr + 1){
			ret = wow_slist_insert_tail(list,&ptNode[i]);
			CHECK_RET_VAL_P(ret == 0,-SLIST_INSERT_FAILED,"wow_slist_insert_tail failed!\n");
			if(wow_slist_size(list) >= 50) goto here4;
		}else{
here4:			
			list = wow_slist_create();
			CHECK_RET_VAL_P(list,-SLIST_CREATE_FAILED,"wow_slist_create failed!\n");
			ret = wow_slist_insert_tail(ptIec101s->node_list.ym_list,list);
			CHECK_RET_VAL_P(ret == 0,-SLIST_INSERT_FAILED,"wow_slist_insert_tail failed!\n");
		}
		baddr = ptNode[i].addr;
	}

	return 0;
}

/*brief    Iec101设置更新数据节点参数回调函数
 *param ： ptIec101s:iec101s操作标识
 *param ： fCallBack:   回调函数 
 *param ： pArg:  回调函数私有变量
 *return： 成功返回0 失败返回-1
 */
__EX_API__ int wow_iec101_s_set_value_handle(Iec101s_T* ptIec101s,iec10x_set_value_func_t fCallBack,void* pArg)
{
	CHECK_RET_VAL_P(ptIec101s,-PARAM_INPUT_STRUCT_IS_NULL,"param input strcuct invalid!\n");
	CHECK_RET_VAL_P(fCallBack,-PARAM_INPUT_FUNC_IS_NULL,"param input func invalid!\n");

	ptIec101s->set_val_cb   = fCallBack;
	ptIec101s->set_val_priv = pArg;

	return 0;
}
__EX_API__ int wow_iec101_s_get_value_handle(Iec101s_T* ptIec101s,iec10x_set_value_func_t fCallBack,void* pArg)
{
	CHECK_RET_VAL_P(ptIec101s,-PARAM_INPUT_STRUCT_IS_NULL,"param input strcuct invalid!\n");
	CHECK_RET_VAL_P(fCallBack,-PARAM_INPUT_FUNC_IS_NULL,"param input func invalid!\n");

	ptIec101s->get_val_cb   = fCallBack;
	ptIec101s->get_val_priv = pArg;

	return 0;
}


/*brief    Iec101设置更新服务状态回调函数
 *param ： iec101s:iec101s操作标识
 *param ： fCallBack:   回调函数 
 *param ： pArg:  回调函数私有变量
 *return： 成功返回0 失败返回-1
 */
__EX_API__ int wow_iec101_s_set_service_handle(Iec101s_T* ptIec101s,iec10x_set_service_func_t fCallBack,void* pArg)
{
	CHECK_RET_VAL_P(ptIec101s,-PARAM_INPUT_STRUCT_IS_NULL,"param input strcuct invalid!\n");
	CHECK_RET_VAL_P(fCallBack,-PARAM_INPUT_FUNC_IS_NULL,"param input func invalid!\n");

	ptIec101s->serv_cb   = fCallBack;
	ptIec101s->serv_priv = pArg;

	return 0;
}

/*brief    iec04遥信突变
 *param ： ptIec101s:iec101s操作标识
 *param ： ptNode:遥信节点信息
 *param ： u8Count:遥信节点个数
 *return： 成功返回0 失败返回-1
 */
__EX_API__ int wow_iec101_s_build_yx_spon(Iec101s_T* ptIec101s,DevNodeCtrl_T* ptNode,uint8_t u8Count)
{
	int i = 0;
    uint8_t len = IEC101_STABLE_LEN;
    uint8_t *ptr  = NULL;
	uint8_t cs_temp = 0;
	uint8_t  send_buf[IEC101_MAX_BUF_LEN];
	
	CHECK_RET_VAL_P(ptIec101s,-PARAM_INPUT_STRUCT_IS_NULL,"param input strcuct invalid!\n");
	CHECK_RET_VAL_P(ptNode && u8Count > 0,-PARAM_INPUT_STRUCT_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(ptIec101s->stat.init == IEC10X_STA_LINK_IDLE,-IEC10X_STATUS_ERROR,"iec101s status(%d) invalid!\n",ptIec101s->stat.init);
	
	/*init struct*/
    Iec101Data68_T* Iec101Data = (Iec101Data68_T*)send_buf;
    Iec101Asdu_T* asdu = (Iec101Asdu_T*)(Iec101Data->_asdu);
    Iec101AsduInfo_T* info = (Iec101AsduInfo_T*)(asdu->_info);

	/*head*/
	Iec101Data->_begin = Iec101Data->_begin_cfm = IEC101_VARIABLE_BEGING;

	/*Ctrol*/
	Iec101Data->_ctrl.up._dir = IEC101_CTRL_DIR_UP;
	Iec101Data->_ctrl.up._prm = IEC101_CTRL_PRM_SLAVE;
	Iec101Data->_ctrl.up._acd = IEC101_CTRL_ACD_NONE_DATA;
	Iec101Data->_ctrl.up._dfc = IEC101_CTRL_DFC_CAN_REC;
	Iec101Data->_ctrl.up._func = IEC101_CTRL_RES_USR_DATA_ACK;

	Iec101Data->_addr = ptIec101s->asdu_addr;
	/*asdu*/
	if(ptNode[0].value.time.year >  0){
		asdu->_type = IEC10X_M_SP_NA_1;
	} else{
		asdu->_type = IEC10X_M_SP_TB_1;
	}
	
	asdu->_num._sq = 0;
	asdu->_num._num = u8Count;
	asdu->_reason._pn = 0;
	asdu->_reason._test = 0;
	asdu->_reason._reason = IEC10X_COT_SPONT;
	asdu->_addr = ptIec101s->asdu_addr;
	
	ptr = (uint8_t*)&(info->_addr);
	for(i = 0; i < u8Count;i++)
	{
		/*build info addr*/
		memcpy(ptr, &ptNode[i].info.addr, 2);
		ptr += 2;

		/*build info value*/
		*ptr = ptNode[i].value.val.s8;
		ptr++;
		
		/*build timesapce value*/
		if(ptNode[0].value.time.year >  0){
			memcpy(ptr, &ptNode[i].value.time, sizeof(CP56Time2a_T));
			ptr += sizeof(CP56Time2a_T);
		}
	}
	
	/*len*/
	len = ptr + 2 - send_buf;
	Iec101Data->_len = Iec101Data->_len_cfm = len-4-2; 		 /*-start-len-len-start   -cs-end*/

	/*end*/
	for(i=4; i<len-2; i++){
		cs_temp += send_buf[i];
	}
	send_buf[len-2] = cs_temp;
	send_buf[len-1] = IEC101_VARIABLE_END;

	ptIec101s->link_ctx->link_send(ptIec101s->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);
	
    return 0;

}


/*brief    iec04遥测突变
 *param ： ptIec101s:iec101s操作标识
 *param ： ptNode:遥测节点信息
 *param ： u8Count:遥测节点个数
 *return： 成功返回0 失败返回-1
 */
__EX_API__ int wow_iec101_s_build_yc_spon(Iec101s_T* ptIec101s,DevNodeCtrl_T* ptNode, uint8_t u8Count)
{
	int i = 0;
	uint8_t len = 0;
	uint8_t cs_temp = 0;
	uint8_t *ptr = NULL;
	uint8_t  send_buf[IEC101_MAX_BUF_LEN];

	CHECK_RET_VAL_P(ptIec101s,-PARAM_INPUT_STRUCT_IS_NULL,"param input strcuct invalid!\n");
	CHECK_RET_VAL_P(ptNode && u8Count > 0,-PARAM_INPUT_STRUCT_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(ptIec101s->stat.init == IEC10X_STA_LINK_IDLE,-IEC10X_STATUS_ERROR,"iec101s status(%d) invalid!\n",ptIec101s->stat.init);

	memset(send_buf,0,IEC101_MAX_BUF_LEN);
    Iec101Data68_T* Iec101Data = (Iec101Data68_T*)send_buf;
    Iec101Asdu_T* asdu = (Iec101Asdu_T*)(Iec101Data->_asdu);
    Iec101AsduInfo_T* info = (Iec101AsduInfo_T*)(asdu->_info);

	/*head*/
	Iec101Data->_begin = Iec101Data->_begin_cfm = IEC101_VARIABLE_BEGING;

	/*Ctrol*/
	Iec101Data->_ctrl.up._dir = IEC101_CTRL_DIR_UP;
	Iec101Data->_ctrl.up._prm = IEC101_CTRL_PRM_SLAVE;
	Iec101Data->_ctrl.up._acd = IEC101_CTRL_ACD_NONE_DATA;
	Iec101Data->_ctrl.up._dfc = IEC101_CTRL_DFC_CAN_REC;
	Iec101Data->_ctrl.up._func = IEC101_CTRL_RES_USR_DATA_ACK;

	Iec101Data->_addr = ptIec101s->asdu_addr;
	/*build ASDU , COT ,Addr*/
	if(ptNode[0].info.type == DATA_FLOAT){
		if(ptNode[0].value.time.year > 0){
			asdu->_type = IEC10X_M_ME_TF_1;
		}else{
			asdu->_type = IEC10X_M_ME_NC_1;
		}
	}else{
		if(ptNode[0].value.time.year > 0){
			asdu->_type = IEC10X_M_ME_TD_1;
		}else{
			asdu->_type = IEC10X_M_ME_NA_1;
		}
	}

	asdu->_num._sq = 0;
	asdu->_num._num = u8Count;
	asdu->_reason._pn = 0;
	asdu->_reason._test = 0;
	asdu->_reason._reason = IEC10X_COT_SPONT;
	asdu->_addr = ptIec101s->asdu_addr;

	ptr = (uint8_t*)&(info->_addr);
	for(i = 0; i < u8Count;i++)
	{
		/*build info addr*/
		memcpy(ptr, &ptNode[i].info.addr, 2);
		ptr += 2;

		/*build info value*/	
		if(ptNode[i].info.type == DATA_FLOAT){
			memcpy(ptr, &ptNode[i].value.val.f, 4);
			ptr += 4;
		}else{
			memcpy(ptr, &ptNode[i].value.val.s16, 2);
			ptr += 2;
		}

		*ptr = 0x00; //qds = 0x00
		ptr++;
		
		/*build timesapce value*/
		if(ptNode[0].value.time.year){
			memcpy(ptr, &ptNode[i].value.time, sizeof(CP56Time2a_T));
			ptr += sizeof(CP56Time2a_T);
		}
	}

    /*len*/
    len = ptr + 2 - send_buf;                    			    /* add cs+end*/
    Iec101Data->_len = Iec101Data->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += send_buf[i];
    }
    send_buf[len-2] = cs_temp;
    send_buf[len-1] = IEC101_VARIABLE_END;

	/* enqueue to the transmisson queue */
	ptIec101s->link_ctx->link_send(ptIec101s->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);
	
	return 0;
}

