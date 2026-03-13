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

#include "iec104_base.h"
#include "wow_iot_errno.h"
#include "protocol/iec10x/wow_iec104s.h"


#define TAG "IEC104S"

struct __data_aligned__ iec104s_t{
	uint16_t asdu_addr;
	uint16_t back01;
	uint32_t back02;
	Iec10xState_T stat;
	Iec104Sn_T    sn;
	Iec10xTime_T	   time;
	iec10x_set_value_func_t   set_val_cb;
	iec10x_get_value_func_t   get_val_cb;
	iec10x_set_service_func_t serv_cb;
	void*			   set_val_priv;
	void*			   get_val_priv;
	void*			   serv_priv;	
	NodeListInfo_T     node_list;
	ProtoPortCtx_T*    link_ctx;
	Iec10xThreadInfo_T thread;
};

static int iec104_s_build_init_finsh(Iec104s_T* iec104s)
{

    uint8_t len = 0;
    uint8_t *ptr = NULL;
    uint32_t addr = 0;
	uint8_t  send_buf[IEC104_MAX_BUF_LEN];

	CHECK_RET_VAL_P(iec104s,-1,"param input invalid!\n");

	memset(send_buf,0,IEC104_MAX_BUF_LEN);
    Iec104Data_T* Iec104Data = (Iec104Data_T*)send_buf;
    Iec104Asdu_T* asdu = (Iec104Asdu_T*)(Iec104Data->Asdu);
    Iec104AsduInfo_T* info = (Iec104AsduInfo_T*)(asdu->_info);

    /*build head*/
    Iec104Data->Head = IEC104_HEAD;

    /*build control code*/
    Iec104Data->Ctrl.I.Type = 0;
    Iec104Data->Ctrl.I.SendSn = iec104s->sn.build_send++;
    Iec104Data->Ctrl.I.RecvSn = iec104s->sn.build_recv;

    /*build ASDU , COT ,Addr*/
    asdu->_type = IEC10x_M_EI_NA_1;
    asdu->_num._sq = 0;
    asdu->_num._num = 1;
    asdu->_reason._reason = IEC10X_COT_INIT;
    asdu->_addr = iec104s->asdu_addr;

    /*build info*/
    ptr = info->_addr;
    addr = 0;
    memcpy(ptr, &addr, 3);

    ptr = info->_element;
    info->_element[0] = 0;

    ptr++;

    /*len*/
    len = ptr - send_buf;
    Iec104Data->Len = len - 2;

    /* enqueue to the transmisson queue */
	iec104s->link_ctx->link_send(iec104s->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);
	
    return 0;
}

static int iec104_s_build_call_ack(Iec104s_T* iec104s,uint8_t qoi)
{
    uint8_t len = 0;
    uint8_t *ptr = NULL;
    uint32_t addr = 0;
	uint8_t  send_buf[IEC104_MAX_BUF_LEN];

	CHECK_RET_VAL_P(iec104s,-1,"param input invalid!\n");

	memset(send_buf,0,IEC104_MAX_BUF_LEN);
    Iec104Data_T* Iec104Data = (Iec104Data_T*)send_buf;
    Iec104Asdu_T* asdu = (Iec104Asdu_T*)(Iec104Data->Asdu);
    Iec104AsduInfo_T* info = (Iec104AsduInfo_T*)(asdu->_info);

    /*build head*/
    Iec104Data->Head = IEC104_HEAD;

    /*build control code*/
    Iec104Data->Ctrl.I.Type = 0;
    Iec104Data->Ctrl.I.SendSn = iec104s->sn.build_send++;
    Iec104Data->Ctrl.I.RecvSn = iec104s->sn.build_recv;

    /*build ASDU , COT ,Addr*/
    asdu->_type = IEC10X_C_IC_NA_1;
    asdu->_num._sq  = 0;
    asdu->_num._num = 1;
    asdu->_reason._reason = IEC10X_COT_ACTCON;
    asdu->_addr = iec104s->asdu_addr;

    /*build info*/
    ptr = info->_addr;
    addr = 0;
    memcpy(ptr, &addr, 3);

    ptr = info->_element;
    ptr[0] = qoi;

    ptr+=1;
    /*len*/
    len = ptr - send_buf;
    Iec104Data->Len = len - 2;

    /* enqueue to the transmisson queue */
	iec104s->link_ctx->link_send(iec104s->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);

    return 0;
}



//暂时支持IEC10X_M_SP_NA_1
static int iec104_s_build_yx(Iec104s_T* iec104s,uint8_t reason,Slist_T* list,int index,int count)
{
	int ret = 0;
    uint8_t len  = 0;
    uint16_t i = 0;
    uint8_t *ptr = NULL;
	uint32_t addr = 0;
	DevNodeInfo_T* node = NULL;
	DevNodeVal_T val ;
	uint8_t  send_buf[IEC104_MAX_BUF_LEN];

	CHECK_RET_VAL_P(iec104s && list,-1,"param input invalid!\n");
	
	node = wow_slist_peek_by_index(list,index);
	CHECK_RET_VAL_P(node,-1,"wow_slist_peek_by_index failed!\n");

	memset(send_buf,0,IEC104_MAX_BUF_LEN);
    Iec104Data_T* Iec104Data = (Iec104Data_T*)send_buf;
    Iec104Asdu_T* asdu = (Iec104Asdu_T*)(Iec104Data->Asdu);
    Iec104AsduInfo_T* info = (Iec104AsduInfo_T*)(asdu->_info);
	

    /*build head*/
    Iec104Data->Head = IEC104_HEAD;

    /*build control code*/
    Iec104Data->Ctrl.I.Type = 0;
    Iec104Data->Ctrl.I.SendSn = iec104s->sn.build_send++;
    Iec104Data->Ctrl.I.RecvSn = iec104s->sn.build_recv;

    /*build ASDU , COT ,Addr*/
    asdu->_type = IEC10X_M_SP_NA_1;
    asdu->_num._sq = 1;
    asdu->_num._num = count;
    asdu->_reason._reason = reason;
    asdu->_addr = iec104s->asdu_addr;

    /*build info addr*/
    ptr = info->_addr;
	addr = node->addr;
    memcpy(ptr, &addr, 3);

    /*build info value*/
    ptr = info->_element;
    for(i = 0; i < asdu->_num._num; i++)
	{
		node = wow_slist_peek_by_index(list,index+i);
		CHECK_RET_VAL_P(node,-1,"wow_slist_peek_by_index failed!\n");
		ret = iec104s->get_val_cb(iec104s->set_val_priv,node,&val);
		*ptr = (ret == 0)?val.val.u8:0xFF;
        ptr++;
    }

    /*len*/
    len = ptr - send_buf;
    Iec104Data->Len = len - 2;

    /* enqueue to the transmisson queue */
    iec104s->link_ctx->link_send(iec104s->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);

    return 0;
}

//暂时支持IEC10X_M_ME_NC_1/IEC10X_M_ME_NA_1
static int iec104_s_build_yc(Iec104s_T* iec104s,uint8_t reason,Slist_T* list,int index,int count)
{
	int i = 0;
	int ret = 0;
	uint8_t len = 0;
    uint8_t *ptr = NULL;
	uint32_t addr = 0;
    DevNodeInfo_T* node = NULL;
	DevNodeVal_T val ;
	uint8_t  send_buf[IEC104_MAX_BUF_LEN];
	
	CHECK_RET_VAL_P(iec104s && list,-1,"param input invalid!\n");
	
	node = wow_slist_peek_by_index(list,index);
	CHECK_RET_VAL_P(node,-1,"wow_slist_peek_by_index failed!\n");

	memset(send_buf,0,IEC104_MAX_BUF_LEN);
    Iec104Data_T* Iec104Data = (Iec104Data_T*)send_buf;
    Iec104Asdu_T* asdu = (Iec104Asdu_T*)(Iec104Data->Asdu);
    Iec104AsduInfo_T* info = (Iec104AsduInfo_T*)(asdu->_info);

    /*build head*/
    Iec104Data->Head = IEC104_HEAD;

    /*build control code*/
    Iec104Data->Ctrl.I.Type = 0;
    Iec104Data->Ctrl.I.SendSn = iec104s->sn.build_send++;
    Iec104Data->Ctrl.I.RecvSn = iec104s->sn.build_recv;

    /*build ASDU , COT ,Addr*/
	if(node->type == DATA_FLOAT){
		asdu->_type = IEC10X_M_ME_NC_1;
	}else{
		asdu->_type = IEC10X_M_ME_NA_1;
	}
    asdu->_num._sq = 1;
    asdu->_num._num = count;
    asdu->_reason._reason = reason;
    asdu->_addr = iec104s->asdu_addr;

    /*build info addr*/
    ptr = info->_addr;
	addr = node->addr;
    memcpy(ptr, &addr, 3);


    /*Build Detect value*/
    ptr = info->_element;
    for(i = 0; i < asdu->_num._num; i++){
		node = wow_slist_peek_by_index(list,index+i);
		CHECK_RET_VAL_P(node,-1,"wow_slist_peek_by_index failed!\n");
		ret = iec104s->get_val_cb(iec104s->set_val_priv,node,&val);
		switch(node->type){
			case DATA_FLOAT:
				if(ret != 0){val.val.u32 = 0xFFFFFFFF;}
				memcpy(ptr, &val.val.f, 4);
				ptr += 4;
				break;
			case DATA_SINT_16:
			case DATA_UINT_16:
			default:
				if(ret != 0){val.val.u16 = 0xFFFF;}
				memcpy(ptr, &val.val.u16, 2);
				ptr += 2;
				break;
		}
		*ptr = 0;//qds
		ptr++;
    }

    /*len*/
    len = ptr - send_buf;
    Iec104Data->Len = len - 2;

    /* enqueue to the transmisson queue */
    iec104s->link_ctx->link_send(iec104s->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);

    return 0;
}

//暂时支持IEC10X_M_IT_NA_1
static int iec104_s_build_ym(Iec104s_T* iec104s,uint8_t reason,Slist_T* list,int index,int count)
{ 
	int i = 0;
	int ret = 0;
	uint8_t len = 0;
    uint8_t *ptr = NULL;
    uint32_t addr = 0;
	DevNodeInfo_T* node = NULL;
	DevNodeVal_T val;
	uint8_t  send_buf[IEC104_MAX_BUF_LEN];
	
	CHECK_RET_VAL_P(iec104s && list,-1,"param input invalid!\n");
	
	node = wow_slist_peek_by_index(list,index);
	CHECK_RET_VAL_P(node,-1,"wow_slist_peek_head failed!\n");

	memset(send_buf,0,IEC104_MAX_BUF_LEN);
    Iec104Data_T* Iec104Data = (Iec104Data_T*)send_buf;
    Iec104Asdu_T* asdu = (Iec104Asdu_T*)(Iec104Data->Asdu);
    Iec104AsduInfo_T* info = (Iec104AsduInfo_T*)(asdu->_info);

    /*build head*/
    Iec104Data->Head = IEC104_HEAD;

    /*build control code*/
    Iec104Data->Ctrl.I.Type = 0;
    Iec104Data->Ctrl.I.SendSn = iec104s->sn.build_send++;
    Iec104Data->Ctrl.I.RecvSn = iec104s->sn.build_recv;

    /*build ASDU , COT ,Addr*/
	asdu->_type = IEC10X_M_IT_NA_1;
    asdu->_num._sq = 1;
    asdu->_num._num = count;
    asdu->_reason._reason = reason;
    asdu->_addr = iec104s->asdu_addr;

    /*build info addr*/
    ptr = info->_addr;
	addr = node->addr;
    memcpy(ptr, &addr, 3);


    /*Build Detect value*/
    ptr = info->_element;
    for(i = 0; i < asdu->_num._num; i++){
		node = wow_slist_peek_by_index(list,index+i);
		CHECK_RET_VAL_P(node,-1,"wow_slist_peek_by_index failed!\n");
		ret = iec104s->get_val_cb(iec104s->set_val_priv,node,&val);
		if(ret != 0){val.val.u32 = 0xFFFFFFFF;}
		memcpy(ptr, &val.val.u32, 4);
		ptr += 4;
		*ptr = i;
		ptr++;
    }

    /*len*/
    len = ptr - send_buf;
    Iec104Data->Len = len - 2;

    /* enqueue to the transmisson queue */
    iec104s->link_ctx->link_send(iec104s->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);

    return 0;
}

static int iec104_s_build_call_finsh(Iec104s_T* iec104s,uint8_t qoi)
{
    uint8_t len = 0, asdu_num = 0;
    uint8_t *ptr = NULL;
    uint32_t addr = 0;
	uint8_t  send_buf[IEC104_MAX_BUF_LEN];

	CHECK_RET_VAL_P(iec104s,-1,"param input invalid!\n");

	memset(send_buf,0,IEC104_MAX_BUF_LEN);
    Iec104Data_T* Iec104Data = (Iec104Data_T*)send_buf;
    Iec104Asdu_T* asdu = (Iec104Asdu_T*)(Iec104Data->Asdu);
    Iec104AsduInfo_T* info = (Iec104AsduInfo_T*)(asdu->_info);

    /*build head*/
    Iec104Data->Head = IEC104_HEAD;

    /*build control code*/
    Iec104Data->Ctrl.I.Type = 0;
    Iec104Data->Ctrl.I.SendSn = iec104s->sn.build_send++;
    Iec104Data->Ctrl.I.RecvSn = iec104s->sn.build_recv;

    /*build ASDU , COT ,Addr*/
    asdu->_type = IEC10X_C_IC_NA_1;
    asdu->_num._sq = 1;
    asdu->_num._num = asdu_num;
    asdu->_reason._reason = IEC10X_COT_ACTTERM;
    asdu->_addr = iec104s->asdu_addr;

    /*build info addr*/
    ptr = info->_addr;
    addr = IEC10X_INFO_ADDR_SIG_BASE+IEC10X_INFO_ADDR_SIG_TEMP_HX_OFF;
    memcpy(ptr, &addr, 3);

    /*Build Detect value*/
    ptr = info->_element;
    ptr[0] = qoi;

    ptr+=1;
    /*len*/
    len = ptr - send_buf;
    Iec104Data->Len = len - 2;

    /* enqueue to the transmisson queue */
    iec104s->link_ctx->link_send(iec104s->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);

    return 0;
}

static int iec104_s_build_u(Iec104s_T* iec104s,uint8_t UType)
{
    uint8_t len = 0;
    uint8_t *ptr = NULL;
	uint8_t  send_buf[IEC104_MAX_BUF_LEN];
	
	CHECK_RET_VAL_P(iec104s,-1,"param input invalid!\n");

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
			ModuleErrorLog(TAG,"iec104m_build_u_ack error  U Type(%d) \n",UType);
            return -1;
    }


    /*build ASDU , COT ,Addr*/
    ptr=Iec104Data->Asdu;

    /*build info*/

    /*len*/
    len = ptr - send_buf;
    Iec104Data->Len = len - 2;

    /* enqueue to the transmisson queue */
    iec104s->link_ctx->link_send(iec104s->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);

    return 0;
}

static int iec104_s_build_u_ack(Iec104s_T* iec104s,uint8_t UType)
{
    uint8_t len = 0;
    uint8_t *ptr = NULL;
	uint8_t  send_buf[IEC104_MAX_BUF_LEN];
	
	CHECK_RET_VAL_P(iec104s,-1,"param input invalid!\n");

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
			ModuleErrorLog(TAG,"iec104_s_build_u_ack error  U Type(%d) \n",UType);
            return -1;
    }


    /*build ASDU , COT ,Addr*/
    ptr=Iec104Data->Asdu;

    /*build info*/

    /*len*/
    len = ptr - send_buf;
    Iec104Data->Len = len - 2;

    /* enqueue to the transmisson queue */
    iec104s->link_ctx->link_send(iec104s->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);

    return 0;
}


static int iec104_s_build_s_ack(Iec104s_T* iec104s)
{

    uint8_t len = 0;
    uint8_t *ptr = NULL;
	uint8_t  send_buf[IEC104_MAX_BUF_LEN];

	CHECK_RET_VAL_P(iec104s,-1,"param input invalid!\n");

	memset(send_buf,0,IEC104_MAX_BUF_LEN);
    Iec104Data_T* Iec104Data = (Iec104Data_T*)send_buf;

    /*build head*/
    Iec104Data->Head = IEC104_HEAD;

    /*build control code*/
    Iec104Data->Ctrl.S.Type1 = 1;
    Iec104Data->Ctrl.S.Type2 = 0;

    Iec104Data->Ctrl.S.Reserve = 0;
    Iec104Data->Ctrl.S.RecvSn = iec104s->sn.build_recv;

    /*build ASDU , COT ,Addr*/
    ptr=Iec104Data->Asdu;

    /*build info*/

    /*len*/
    len = ptr - send_buf;
    Iec104Data->Len = len - 2;

    /* enqueue to the transmisson queue */
     iec104s->link_ctx->link_send(iec104s->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);

    return 0;
}

#define IEC104X_YX_NODE_NUM (50)
#define IEC104X_YC_NODE_NUM (30)
#define IEC104X_YM_NODE_NUM (30)

static int iec104_s_asdu_call_all(Iec104s_T* iec104s,Iec104Asdu_T* Iec10x_Asdu)
{	
	uint32_t addr = 0;
    Iec104AsduInfo_T* asdu_info = (Iec104AsduInfo_T*)(Iec10x_Asdu->_info);
    uint8_t qoi = asdu_info->_element[0];
    
	CHECK_RET_VAL_P(iec104s && Iec10x_Asdu,-1,"param input invalid!\n");
	CHECK_RET_VAL_P(iec104s->get_val_cb,-1,"get_val_cb not found!\n");
	
    /* check info addrest */
    memcpy(&addr, asdu_info->_addr, 3);
	CHECK_RET_VAL_P(addr == 0,-1,"call cmd active error addr(%x)!\n",addr);
 	CHECK_RET_VAL_P(Iec10x_Asdu->_reason._reason == IEC10X_COT_ACT,-1,"call cmd error reason(%d)!\n",Iec10x_Asdu->_reason._reason);

    switch(qoi){
        case IEC10X_COT_INTROGEN:
	    	{
	    		int i = 0;
				int size  = 0;
				int count = 0;

				
				iec104_s_build_call_ack(iec104s,qoi);
				usleep(5*1000);

				///<遥信点
				size = wow_slist_size(iec104s->node_list.yx_list);
				for(i = 0; i < size; i += count){
					count = (size-i)>IEC104X_YX_NODE_NUM?IEC104X_YX_NODE_NUM:(size-i);
					iec104_s_build_yx(iec104s,qoi,iec104s->node_list.yx_list,i,count);
					usleep(5*1000);
				}

				///<归一化遥测点	
				size = wow_slist_size(iec104s->node_list.yc_n_list);
				for(i = 0; i < size; i += count){
					count = (size-i)>IEC104X_YX_NODE_NUM?IEC104X_YX_NODE_NUM:(size-i);
					iec104_s_build_yc(iec104s,qoi,iec104s->node_list.yc_n_list,i,count);
					usleep(5*1000);
				}

				///<短浮点遥测点
				size = wow_slist_size(iec104s->node_list.yc_f_list);
				for(i = 0; i < size; i += count){
					count = (size-i)>IEC104X_YX_NODE_NUM?IEC104X_YX_NODE_NUM:(size-i);
					iec104_s_build_yc(iec104s,qoi,iec104s->node_list.yc_f_list,i,count);
					usleep(5*1000);
				}

				iec104_s_build_call_finsh(iec104s,qoi);
				usleep(5*1000);
			}
	        break;
        case IEC10X_COT_INTRO1:
        case IEC10X_COT_INTRO2:
            break;
        default:
			ModuleErrorLog(TAG,"iec104_s_asdu_call error qoi(%d) \n",qoi);
            return -1;
    }
    
    return 0;
}


static int iec104_s_asdu_call_pulse(Iec104s_T* iec104s,Iec104Asdu_T* Iec10x_Asdu)
{
	int i = 0;
	int count = 0;
    Iec104AsduInfo_T* asdu_info = (Iec104AsduInfo_T*)(Iec10x_Asdu->_info);
    uint8_t qcc = asdu_info->_element[0];
    uint32_t addr = 0;


	CHECK_RET_VAL_P(iec104s && Iec10x_Asdu,-1,"param input invalid!\n");
	CHECK_RET_VAL_P(iec104s->get_val_cb,-1,"get_val_cb not found!\n");
	
    /* check info addrest */
    memcpy(&addr, asdu_info->_addr, 3);
	CHECK_RET_VAL_P(addr == 0,-1,"call cmd active error addr(%x)!\n",addr);
 	CHECK_RET_VAL_P(Iec10x_Asdu->_reason._reason == IEC10X_COT_ACT,-1,"call cmd error reason(%d)!\n",Iec10x_Asdu->_reason._reason);
	 
	//总请求计数量
	CHECK_RET_VAL_P((qcc & 0x3F) == 0x05,-1,"call pulse qcc error addr(%x)!\n",qcc & 0x3F);
			
	iec104_s_build_call_ack(iec104s,qcc);
	usleep(5*1000);

	int size = wow_slist_size(iec104s->node_list.ym_list);
	for(i = 0; i < size; i += count){
		count = (size-i)>IEC104X_YX_NODE_NUM?IEC104X_YX_NODE_NUM:(size-i);
		iec104_s_build_ym(iec104s,IEC10X_COT_REQCOGEN,iec104s->node_list.ym_list,i,count);
		usleep(5*1000);
	}
	

	iec104_s_build_call_finsh(iec104s,qcc);
	usleep(5*1000);
   
    return 0;
}

static int iec104_s_asdu_clock(Iec104s_T* iec104s,Iec104Asdu_T* Iec10x_Asdu)
{
	CHECK_RET_VAL_P(iec104s && Iec10x_Asdu,-1,"param input invalid!\n");

    Iec104AsduInfo_T* asdu_info = (Iec104AsduInfo_T*)(Iec10x_Asdu->_info);
	CHECK_RET_VAL_P(asdu_info->_addr[0] == 0 && asdu_info->_addr[1]==0 && asdu_info->_addr[2] == 0,
				-1,"Clock cmd error addr!\n");
	
	CP56Time2a_T cp56time2a;
    memcpy(&cp56time2a,asdu_info->_element, sizeof(CP56Time2a_T));

    switch(Iec10x_Asdu->_reason._reason){
        case IEC10X_COT_ACT:
            ModuleDebugLog(TAG,"Clock cmd (20%d-%d-%d %d:%d:xx) \n",cp56time2a.year,cp56time2a.mon,cp56time2a.mon_day,cp56time2a.hour,cp56time2a.min);
			wow_time_set_cp56(&cp56time2a);
			break;
        case IEC10X_COT_SPONT:
            break;
        default:
        	ModuleErrorLog(TAG,"iec104_s_asdu_clock error reason(%d) \n",Iec10x_Asdu->_reason._reason);
            break;
    }
    return 0;
}

static int iec104_s_asdu_yt(Iec104s_T* iec104s,Iec104Asdu_T* Iec10x_Asdu)
{
	CHECK_RET_VAL_P(iec104s && Iec10x_Asdu,-1,"param input invalid!\n");

	int i = 0;
    uint32_t addr = 0;
    DevNodeInfo_T node; 
	DevNodeVal_T  val; 
    uint8_t *ptr = NULL;
	
	uint8_t type = Iec10x_Asdu->_type;
    uint8_t n = Iec10x_Asdu->_num._num;
	uint8_t Sq = Iec10x_Asdu->_num._sq;
    Iec104AsduInfo_T* asdu_info = (Iec104AsduInfo_T*)(Iec10x_Asdu->_info);
	

	CHECK_RET_VAL_P(Iec10x_Asdu->_reason._reason == IEC10X_COT_ACT,-1,
			"iec104_s_asdu_yt error reason(%d) \n",Iec10x_Asdu->_reason._reason);
	
    switch(type){
        case IEC10X_C_SE_NA_1:
            if(Sq == 1){
				Iec104Detect_T* detect = NULL;
				memcpy(&addr, asdu_info->_addr, 3);
                ptr = asdu_info->_element;
				memset(&node,0,sizeof(DevNodeInfo_T));
				node.mode = DEV_TYPE_YT;
				node.type = DATA_SINT_16;
                for(i=0; i<n; i++){
                    detect = (Iec104Detect_T*)ptr;
					node.addr    = (uint16_t)addr+i;
                    val.val.s16 = detect->_detect;
                    ptr += sizeof(Iec104Detect_T);
					if(detect->_qds & 0x80) continue; ///<选择待执行
                    if(iec104s->set_val_cb){
						iec104s->set_val_cb(iec104s->set_val_priv,&node,&val);
					}
                }
            }else if(Sq == 0){
            	Iec104DetectSq0_T* detect_Sq0 = NULL;
                ptr = Iec10x_Asdu->_info;
				memset(&node,0,sizeof(DevNodeInfo_T));
				node.mode = DEV_TYPE_YT;
				node.type = DATA_SINT_16;
                for(i=0; i<n; i++){
                    detect_Sq0 = (Iec104DetectSq0_T*)ptr;
                    memcpy(&addr, detect_Sq0->_addr, 3);
					node.addr    = (uint16_t)addr;
                    val.val.s16 = detect_Sq0->_detect;
					ptr += sizeof(Iec104DetectSq0_T);  
					if(detect_Sq0->_qds & 0x80) continue; ///<选择待执行
                   if(iec104s->set_val_cb){
						iec104s->set_val_cb(iec104s->set_val_priv,&node,&val);
					} 
                }    
            }
            break;
        case IEC10X_C_SE_NC_1:
            if(Sq == 1){
				Iec104DetectFloat_T* detect_f = NULL;
				memcpy(&addr, asdu_info->_addr, 3);
                ptr = asdu_info->_element;
				memset(&node,0,sizeof(DevNodeInfo_T));
				node.mode = DEV_TYPE_YT;
				node.type = DATA_FLOAT;
                for(i=0; i<n; i++){
                    detect_f = (Iec104DetectFloat_T*)ptr;
					node.addr  = (uint16_t)addr+i;
                    val.val.f = (float)detect_f->_detect;
                    ptr += sizeof(Iec104DetectFloat_T);
					if(detect_f->_qds & 0x80) continue; ///<选择待执行
                    if(iec104s->set_val_cb){
						iec104s->set_val_cb(iec104s->set_val_priv,&node,&val);
					}
                }
            }else if(Sq == 0){
            	Iec104DetectSq0Float_T* detect_Sq0_f = NULL;
                ptr = Iec10x_Asdu->_info;
				memset(&node,0,sizeof(DevNodeInfo_T));
				node.mode = DEV_TYPE_YT;
				node.type = DATA_FLOAT;
                for(i=0; i<n; i++){
                    detect_Sq0_f = (Iec104DetectSq0Float_T*)ptr;
                    memcpy(&addr, detect_Sq0_f->_addr, 3);
					node.addr  = (uint16_t)addr;
                    val.val.f = (float)detect_Sq0_f->_detect;
					ptr += sizeof(Iec104DetectSq0Float_T); 
					if(detect_Sq0_f->_qds & 0x80) continue; ///<选择待执行
                    if(iec104s->set_val_cb){
						iec104s->set_val_cb(iec104s->set_val_priv,&node,&val);
					}
                }
            }
            break;
        default:
            ModuleErrorLog(TAG,"iec104_s_asdu_yt error Type(%d) \n",type);
            return -1;
    }

    return 0;
}

static int iec104_s_build_yt_ack(Iec104s_T* iec104s,Iec104Asdu_T* Iec10x_Asdu)
{

	CHECK_RET_VAL_P(iec104s && Iec10x_Asdu,-1,"param input invalid!\n");

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
    Iec104Data->Ctrl.I.SendSn = iec104s->sn.build_send++;
    Iec104Data->Ctrl.I.RecvSn = iec104s->sn.build_recv;

    /*build ASDU , COT ,Addr*/
    asdu->_type = type;
    asdu->_num._sq  = Sq;
    asdu->_num._num = n;
    asdu->_reason._reason = IEC10X_COT_ACTCON;
    asdu->_addr = iec104s->asdu_addr;

    /*build info*/
	switch(type){
		case IEC10X_C_SE_NA_1:
			if(Sq == 1){   
				ptr = info->_addr;
				memcpy(ptr,asdu_info->_addr,3);
				ptr = info->_element;
				memcpy(ptr,asdu_info->_element,n*sizeof(Iec104Detect_T));
				ptr += n*sizeof(Iec104Detect_T);
			}else if(Sq == 0){
				ptr = info->_addr;
				memcpy(ptr,Iec10x_Asdu->_info,n*sizeof(Iec104DetectSq0_T));
				ptr += n*sizeof(Iec104DetectSq0_T);	
			}
			break;
		case IEC10X_C_SE_NC_1:
			if(Sq == 1){
				ptr = info->_addr;
				memcpy(ptr,asdu_info->_addr,3);
				ptr = info->_element;
				memcpy(ptr,asdu_info->_element,n*sizeof(Iec104DetectFloat_T));
				ptr += n*sizeof(Iec104DetectFloat_T);
			}else if(Sq == 0){
				ptr = info->_addr;
				memcpy(ptr,Iec10x_Asdu->_info,n*sizeof(Iec104DetectSq0Float_T));
				ptr += n*sizeof(Iec104DetectSq0Float_T);	
			}
			break;
		default:
			ModuleErrorLog(TAG,"iec104_s_asdu_yt error Type(%d) \n",type);
			return -1;
	}

    /*len*/
    len = ptr - send_buf;
    Iec104Data->Len = len - 2;
    
    /* enqueue to the transmisson queue */
    iec104s->link_ctx->link_send(iec104s->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);

    return 0;
}

static int iec104_s_asdu_yk(Iec104s_T* iec104s,Iec104Asdu_T* Iec10x_Asdu)
{

	CHECK_RET_VAL_P(iec104s && Iec10x_Asdu,-1,"param input invalid!\n");

    int i = 0;
    uint8_t *ptr = NULL;
    uint32_t addr = 0;
	uint8_t  sco  = 0;
    DevNodeInfo_T node;    
	DevNodeVal_T  val;
	
	uint8_t type = Iec10x_Asdu->_type;
	uint8_t n = Iec10x_Asdu->_num._num;
	uint8_t Sq = Iec10x_Asdu->_num._sq;
	Iec104AsduInfo_T* asdu_info = (Iec104AsduInfo_T*)(Iec10x_Asdu->_info);

	CHECK_RET_VAL_P(Iec10x_Asdu->_reason._reason == IEC10X_COT_ACT,-1,"iec104_s_asdu_yk error reason(%d) \n",Iec10x_Asdu->_reason._reason);

    switch(type){
        case IEC10X_C_SC_NA_1:
            if(Sq == 1){
				memcpy(&addr, asdu_info->_addr, 3);
                ptr = asdu_info->_element;
				memset(&node,0,sizeof(DevNodeInfo_T));
				node.mode = DEV_TYPE_YK;
				node.type = DATA_SINT_8;
                for(i=0; i<n; i++){
					uint8_t sco = *ptr;
					node.addr   = (uint16_t)addr+i;
					val.val.s8 = sco & 0x01;
					ptr++;
					if(sco & 0x80) continue; ///<选择待执行
                    if(iec104s->set_val_cb){
						iec104s->set_val_cb(iec104s->set_val_priv,&node,&val);
					}
                }
            }else if(Sq == 0){
                ptr = Iec10x_Asdu->_info;
				memset(&node,0,sizeof(DevNodeInfo_T));
				node.mode = DEV_TYPE_YK;
				node.type = DATA_SINT_8;
                for(i=0; i<n; i++){
                    memcpy(&addr, ptr, 3);
					ptr += 3;  
					sco = *ptr;
					node.addr   = (uint16_t)addr;
                    val.val.s8 = sco & 0x01;
					ptr++;
					if(sco & 0x80) continue; ///<选择待执行
                    if(iec104s->set_val_cb){
						iec104s->set_val_cb(iec104s->set_val_priv,&node,&val);
					}
                } 
            }
            break;
        case IEC10X_C_DC_NA_1:
			if(Sq == 1){
				memcpy(&addr, asdu_info->_addr, 3);
                ptr = asdu_info->_element;
				memset(&node,0,sizeof(DevNodeInfo_T));
				node.mode = DEV_TYPE_YK;
				node.type = DATA_SINT_8;
                for(i=0; i<n; i++){
					sco = *ptr;
					node.addr   = (uint16_t)addr+i;
					val.val.s8 = sco & 0x03;
					ptr++;
					if(sco & 0x80) continue; ///<选择待执行
					if(iec104s->set_val_cb){
						iec104s->set_val_cb(iec104s->set_val_priv,&node,&val);
					}
                }
            }else if(Sq == 0){
                ptr = Iec10x_Asdu->_info;
				memset(&node,0,sizeof(DevNodeInfo_T));
				node.mode = DEV_TYPE_YK;
				node.type = DATA_SINT_8;
                for(i=0; i<n; i++){
                    memcpy(&addr, ptr, 3);
					ptr += 3;
				 	uint8_t sco = *ptr;
					node.addr   = (uint16_t)addr;
                    val.val.s8 = sco & 0x03;;
					ptr++;
					if(sco & 0x80) continue; ///<选择待执行
                    if(iec104s->set_val_cb){
						iec104s->set_val_cb(iec104s->set_val_priv,&node,&val);
					}
                } 
            }
            break;
        default:
            ModuleErrorLog(TAG,"iec104_s_asdu_yk error Type(%d) \n",type);
            return -1;
    }

    return 0;
}

static int iec104_s_build_yk_ack(Iec104s_T* iec104s,Iec104Asdu_T* Iec10x_Asdu)
{
	CHECK_RET_VAL_P(iec104s && Iec10x_Asdu,-1,"param input invalid!\n");

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
    Iec104Data->Ctrl.I.SendSn = iec104s->sn.build_send++;
    Iec104Data->Ctrl.I.RecvSn = iec104s->sn.build_recv;

    /*build ASDU , COT ,Addr*/
    asdu->_type = type;
    asdu->_num._sq  = Sq;
    asdu->_num._num = n;
    asdu->_reason._reason = IEC10X_COT_ACTCON;
    asdu->_addr = iec104s->asdu_addr;

    /*build info*/
	switch(type){
		case IEC10X_C_SC_NA_1:
		case IEC10X_C_DC_NA_1:
			if(Sq == 1){   
				ptr = info->_addr;
				memcpy(ptr,asdu_info->_addr,3);
				ptr = info->_element;
				memcpy(ptr,asdu_info->_element,n);
				ptr += n;
			}else if(Sq == 0){
				ptr = info->_addr;
				memcpy(ptr,Iec10x_Asdu->_info,n*4);
				ptr += n*4;	
			}
			break;
		default:
			ModuleErrorLog(TAG,"iec104_s_asdu_yk error Type(%d) \n",type);
			return -1;
	}

    /*len*/
    len = ptr - send_buf;
    Iec104Data->Len = len - 2;
    
    /* enqueue to the transmisson queue */
    iec104s->link_ctx->link_send(iec104s->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);

    return 0;
}

static int iec104_s_deal_sn(Iec104s_T* iec104s,uint16_t SendSn, uint16_t RecvSn)
{
	CHECK_RET_VAL_P(iec104s,-1,"param input invalid!\n");

	//服务端断开重连后SN参数失效
	//CHECK_RET_VAL_P(SendSn >= iec104s->sn.deal_send && RecvSn >= iec104s->sn.deal_recv,-1,"iec104_s_deal_sn invalid sn!\n");

    iec104s->sn.build_recv = SendSn+1;

    iec104s->sn.deal_send = SendSn;
    iec104s->sn.deal_recv = RecvSn;

    //iec104s->sn.build_recv++;

    return 0;
}

static int iec104_s_frame_i_parse(Iec104s_T* iec104s,Iec104Data_T* Iec104Data, uint16_t len){

    uint8_t Type;
    uint16_t RecvSn,SendSn;

    Iec104Asdu_T* asdu = (Iec104Asdu_T*)(Iec104Data->Asdu);

    SendSn = Iec104Data->Ctrl.I.SendSn;
    RecvSn = Iec104Data->Ctrl.I.RecvSn;

    /* check asdu addrest */
    CHECK_RET_VAL_P(iec104s->asdu_addr == asdu->_addr,-1,"asdu addr(%d) invalid!\n",asdu->_addr);
	
    /* deal the receive and send serial number */
	CHECK_RET_VAL_P(iec104_s_deal_sn(iec104s,SendSn, RecvSn) == 0,-1,"deal sn faild!\n");


    Type = asdu->_type;
    switch(Type){

        case IEC10X_C_IC_NA_1:
			ModDebug_D(TAG,"Asdu Type Call All...\n");
            iec104_s_asdu_call_all(iec104s,asdu);
            break;
		case IEC10X_C_CI_NA_1:
			ModDebug_D(TAG,"Asdu Type Call Pulse...\n");
            iec104_s_asdu_call_pulse(iec104s,asdu);
            break;
        case IEC10X_C_CS_NA_1:
			ModDebug_D(TAG,"Asdu Type Clock Syc Cmd...\n");
            iec104_s_asdu_clock(iec104s,asdu);
			iec104_s_build_s_ack(iec104s);
            break;			
        case IEC10X_C_SC_NA_1:
		case IEC10X_C_DC_NA_1:
			ModDebug_D(TAG,"Asdu Type Set Cmd  Cmd...\n");
		    iec104_s_asdu_yk(iec104s,asdu);
			iec104_s_build_yk_ack(iec104s,asdu);
            break;	
        case IEC10X_C_SE_NA_1:
        case IEC10X_C_SE_NC_1:
			ModDebug_D(TAG,"Asdu Type Set Val  Cmd...\n");
            iec104_s_asdu_yt(iec104s,asdu);
			iec104_s_build_yt_ack(iec104s,asdu);
            break;
			
        default:
			ModDebug_D(TAG,"Asdu Type Error!\n");
            return -1;
    }
    return 0;
}


static int iec104_s_frame_s_parse(Iec104s_T* iec104s,Iec104Data_T* Iec104Data, uint16_t len)
{

    return 0;
}

static int iec104_s_frame_u_parse(Iec104s_T* iec104s,Iec104Data_T* Iec104Data, uint16_t len)
{
    switch(Iec104Data->Ctrl.Func.Func){

        case IEC104_U_FUNC_STARTDT:
			ModDebug_D(TAG,"Frame function STARTDT \n");
			iec104s->stat.init = IEC10X_STA_LINK_CONNECT_OK;
            break;
        case IEC104_U_FUNC_STOPDT:
			ModDebug_D(TAG,"Frame function STOPDT \n");
            iec104s->link_ctx->link_close(iec104s->link_ctx);
		
            iec104s->stat.init = IEC10X_STA_LINK_DISCONNECT;
            break;
        case IEC104_U_FUNC_TESTER:
			ModDebug_D(TAG,"Frame function TESTER \n");
            iec104_s_build_u_ack(iec104s,IEC104_U_FUNC_TESTER);
            break;
        case IEC104_U_FUNC_TESTER_ACK:
			 iec104s->stat.test = IE10X_STA_TESTER_STOP;
			ModDebug_D(TAG,"Frame function TESTER ACK\n");
            break;
    }
    return 0;
}

static void iec104_s_frame_parse(Iec104s_T* iec104s,uint8_t *buf, uint16_t len){

    uint8_t *BufTemp = NULL;
    int16_t LenRemain,LenTmp;
    Iec104Data_T* Iec104Data = NULL;

	CHECK_RET_VOID(iec104s && buf && len > 0);

    BufTemp = buf;
    LenRemain = len;

    while(BufTemp<buf+len){

        Iec104Data = (Iec104Data_T*)BufTemp;
        if(Iec104Data->Head == IEC104_HEAD){
            LenTmp = Iec104Data->Len + 2;
			CHECK_RET_VOID_P(LenRemain >= IEC104_HEAD_LEN,"invalid frame len!\n");
			
            if(Iec104Data->Ctrl.Type.Type1 == 0){
				ModDebug_D(TAG,"Frame Type I \n");
                iec104_s_frame_i_parse(iec104s,Iec104Data, LenTmp);

            }else if(Iec104Data->Ctrl.Type.Type1 == 1 && Iec104Data->Ctrl.Type.Type2 == 0){
                ModDebug_D(TAG,"Frame Type S \n");
                iec104_s_frame_s_parse(iec104s,Iec104Data, LenTmp);

            }else if(Iec104Data->Ctrl.Type.Type1 == 1 && Iec104Data->Ctrl.Type.Type2 == 1){
                ModDebug_D(TAG,"Frame Type U \n");
                iec104_s_frame_u_parse(iec104s,Iec104Data, LenTmp);
            }
        }else{
        	ModDebug_E(TAG,"Frame Type Error! \n");
            return;
        }
        BufTemp+=LenTmp;
        LenRemain-=LenTmp;
    }
}

static void iec104_s_reset_param(void* arg)
{
	Iec104s_T* iec104s = (Iec104s_T*)arg;
	CHECK_RET_VOID_P(iec104s,"param input arg invalid!\n");

	iec104s->stat.init    = IEC10X_STA_LINK_INIT;
	iec104s->stat.callall = IEC10X_STA_CALL_ALL_IDLE;
	iec104s->stat.callpulse = IEC10X_STA_CALL_PULSE_IDLE;
	iec104s->stat.yt      = IEC10X_STA_YT_CMD_IDLE;
	iec104s->stat.yk	  = IEC10X_STA_YK_CMD_IDLE;
	iec104s->stat.test    = IE10X_STA_TESTER_START;

	iec104s->sn.build_send = 0;
	iec104s->sn.build_recv = 0;
	iec104s->sn.deal_send = -1;
	iec104s->sn.deal_recv = 0;

	iec104s->time.init = 0;
	iec104s->time.callall = 0;
	iec104s->time.callpulse = 0;
	iec104s->time.yt   = 0;
	iec104s->time.yk   = 0;
	iec104s->time.test = 0;


}

static void iec104_s_state_machine(Iec104s_T* iec104s)
{	
	/*Init link*/
	
	switch(iec104s->stat.init){
		case IEC10X_STA_LINK_INIT:
			iec104_s_reset_param(iec104s);
			iec104s->stat.init = IEC10X_STA_LINK_OPEN;
			break;

		case IEC10X_STA_LINK_CONNECT_OK:
			iec104_s_build_u_ack(iec104s,IEC104_U_FUNC_STARTDT);
			iec104_s_build_init_finsh(iec104s);
		
			iec104s->stat.init = IEC10X_STA_LINK_IDLE;
			iec104s->time.timer = wow_time_get_msec();
			if(iec104s->serv_cb){
				iec104s->serv_cb(iec104s->serv_priv,IEC10X_CONNECTED,NULL);
			}
			break;
		case IEC10X_STA_LINK_DISCONNECT:
			iec104s->stat.init = IEC10X_STA_LINK_INIT;
			if(iec104s->serv_cb){
				iec104s->serv_cb(iec104s->serv_priv,IEC10X_DISCONNECTED,NULL);
			}
			break;
		case IEC10X_STA_LINK_CLOSE:
			iec104_s_build_u(iec104s,IEC104_U_FUNC_STOPDT);
			iec104s->stat.init = IEC10X_STA_LINK_INIT;
			break;
		case IEC10X_STA_LINK_OPEN: 
		case IEC10X_STA_LINK_CONNECT:
		case IEC10X_STA_LINK_IDLE:
		default:
			break;
	}

	if(iec104s->stat.init != IEC10X_STA_LINK_IDLE) return;

	/* Tester */
	 switch(iec104s->stat.test){
		 case IE10X_STA_TESTER_START:
			 iec104s->time.test = wow_time_get_msec();
			 if(iec104s->time.test - iec104s->time.timer > IEC10X_CIRCLE_TESTER_U){
				 iec104_s_build_u(iec104s,IEC104_U_FUNC_TESTER);
				 iec104s->time.timer = iec104s->time.test;
				 iec104s->stat.test = IE10X_STA_TESTER_IDLE;	 
			 }
			 break;
		 case IE10X_STA_TESTER_IDLE:
			 if(wow_time_get_msec() - iec104s->time.test > IEC10X_TESTER_U_TIMEOUT){
				 //超时关闭
				 iec104s->link_ctx->link_close(iec104s->link_ctx);
				 iec104s->stat.init = IEC10X_STA_LINK_INIT;
			 }
			 break;
		 case IE10X_STA_TESTER_STOP:
			 iec104s->time.test = 0;
			 iec104s->stat.test = IE10X_STA_TESTER_START;	 
			 break;
		 default:
			 break;
	 }

}

static void* iec104_s_exec_thread(void* arg)
{
	Iec104s_T* iec104s = (Iec104s_T*)arg;
	CHECK_RET_VAL_P(iec104s,NULL,"param input arg invalid!\n");
	
	uint8_t recv_buf[1500]={0};
	int rlen = 0;

	while(iec104s->thread.tflag){
		iec104_s_state_machine(iec104s);
		
		rlen = iec104s->link_ctx->link_recv(iec104s->link_ctx,recv_buf,1500,IEC10X_RECV_TIMEOUT);
		if(rlen > 0){
			iec104_s_frame_parse(iec104s,recv_buf,rlen);
		}
	}

	wow_mutex_lock(&iec104s->thread.lock);
	wow_mutex_cond_signal(&iec104s->thread.cond);
	wow_mutex_unlock(&iec104s->thread.lock);	

	return NULL;
}




/*brief    Iec104从设备初始化
 *param ： u16Addr: 公共地址 
 *param ： ptEthPar:  硬件参数
 *return： 成功返回iec04s操作标识 失败返回NULL
 */
__EX_API__ Iec104s_T* wow_iec104_s_init(uint16_t u16Addr,ProtoEthParam_T* ptEthPar)
{
	CHECK_RET_VAL_ERRNO_P(ptEthPar,-PARAM_INPUT_DATA_IS_NULL, "param input data invalid!\n");

	Iec104s_T* iec104s = CALLOC(1, Iec104s_T);
	CHECK_RET_VAL_ERRNO_P(iec104s,-SYSTEM_MALLOC_FAILED,"malloc iec104s false!\n");
	
	/*参数初始化*/
	iec104s->asdu_addr = u16Addr;
	iec104_s_reset_param(iec104s);
	iec104s->node_list.yx_list = wow_slist_create();
	CHECK_RET_GOTO_ERRNO_P(iec104s->node_list.yx_list,out,-SLIST_CREATE_FAILED,"wow_slist_create faild!\n");
	iec104s->node_list.yc_n_list = wow_slist_create();
	CHECK_RET_GOTO_ERRNO_P(iec104s->node_list.yc_n_list,out,-SLIST_CREATE_FAILED,"wow_slist_create faild!\n");
	iec104s->node_list.yc_f_list = wow_slist_create();
	CHECK_RET_GOTO_ERRNO_P(iec104s->node_list.yc_f_list,out,-SLIST_CREATE_FAILED,"wow_slist_create faild!\n");
	iec104s->node_list.ym_list = wow_slist_create();
	CHECK_RET_GOTO_ERRNO_P(iec104s->node_list.ym_list,out,-SLIST_CREATE_FAILED,"wow_slist_create faild!\n");

	/*建立连接*/
	iec104s->link_ctx = proto_port_eth_slave_init(ptEthPar);
	CHECK_RET_GOTO_ERRNO_P(iec104s->link_ctx,out,-PROTO_NET_INIT_ERROR,"iec10x port init faild!\n");
	proto_port_eth_set_connected_cb(iec104s->link_ctx,iec104_s_reset_param,iec104s);
	
	iec104s->thread.tflag = 1;
	wow_mutex_lock_init(&iec104s->thread.lock);
	wow_mutex_cond_init(&iec104s->thread.cond);

	return iec104s;
out:
	FREE(iec104s);
	return NULL;
}

/*brief    退出Iec104从设备
 *param ： pptIec104s:iec04s操作标识
 *return： 无
 */
__EX_API__ void wow_iec104_s_exit(Iec104s_T** pptIec104s)
{
	CHECK_RET_VOID(pptIec104s && *pptIec104s);

	Iec104s_T* iec104s= (Iec104s_T*)*pptIec104s;
	
	wow_mutex_lock(&iec104s->thread.lock);
	iec104s->thread.tflag= 0;
	wow_mutex_cond_wait(&iec104s->thread.lock,&iec104s->thread.cond,-1);
	wow_mutex_unlock(&iec104s->thread.lock);
	
	proto_port_eth_slave_exit(&iec104s->link_ctx);

	if(iec104s->node_list.yx_list){
		wow_slist_destroy(&iec104s->node_list.yx_list,NULL);
	}
	if(iec104s->node_list.yc_n_list){
		wow_slist_destroy(&iec104s->node_list.yc_n_list,NULL);
	}
	if(iec104s->node_list.yc_f_list){
		wow_slist_destroy(&iec104s->node_list.yc_f_list,NULL);
	}
	if(iec104s->node_list.ym_list){
		wow_slist_destroy(&iec104s->node_list.ym_list,NULL);
	}

	FREE(iec104s);
	*pptIec104s = NULL;
}


/*brief    启动Iec104从设备
 *param ： ptIec104s:iec04s操作标识
 *return： 成功返回0 失败返回-1
 */
__EX_API__ int  wow_iec104_s_start(Iec104s_T* ptIec104s)
{
	CHECK_RET_VAL_P(ptIec104s,-PARAM_INPUT_STRUCT_IS_NULL,"param input strcuct invalid!\n");

	int ret = ptIec104s->link_ctx->link_open(ptIec104s->link_ctx);
	CHECK_RET_VAL_P(ret == 0,-PROTO_LINK_OPEN_ERROR,"link_open failed!\n");
	
	ptIec104s->thread.thread = wow_thread_create("iec104s",iec104_s_exec_thread,ptIec104s);
	CHECK_RET_VAL_P(ptIec104s->thread.thread,-THREAD_CREATE_FAILED,"wow_thread_create failed!\n");

	return 0;
}

/*brief    Iec104设置数据节点信息
 *param ： ptIec104s:iec04s操作标识
 *param ： ptNode:  节点信息
 *param ： nCount: 节点个数
 *return： 成功返回0 失败返回-1
 */
__EX_API__ int wow_iec104_s_set_node_info(Iec104s_T* ptIec104s,DevNodeInfo_T* ptNode,int nCount)
{
	int i = 0;
	int ret = 0;

	CHECK_RET_VAL_P(ptIec104s,-PARAM_INPUT_STRUCT_IS_NULL,"param input strcuct invalid!\n");

	for(i = 0; i < nCount;i++){
		switch(ptNode[i].mode){
			case DEV_TYPE_YX:///<遥信点
				ret = wow_slist_insert_tail(ptIec104s->node_list.yx_list,&ptNode[i]);
				break;
			case DEV_TYPE_YC:///<遥测点
				{
					if(ptNode[i].type == DATA_FLOAT){
						ret = wow_slist_insert_tail(ptIec104s->node_list.yc_f_list,&ptNode[i]);
					}else{
						ret = wow_slist_insert_tail(ptIec104s->node_list.yc_n_list,&ptNode[i]);					}
				}
				break;
			case DEV_TYPE_YM:///<遥脉点

				ret = wow_slist_insert_tail(ptIec104s->node_list.ym_list,&ptNode[i]);
				break;
		}

		CHECK_RET_VAL_P(ret == 0,-SLIST_INSERT_FAILED,"wow_slist_insert_tail failed!\n");
	}

	return 0;
}

/*brief    Iec104设置更新数据节点参数回调函数
 *param ： ptIec104s:iec04s操作标识
 *param ： fCallBack:   回调函数 
 *param ： pArg:  回调函数私有变量
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_iec104_s_set_value_handle(Iec104s_T* ptIec104s,iec10x_set_value_func_t fCallBack,void* pArg)
{
	CHECK_RET_VAL_P(ptIec104s,-PARAM_INPUT_STRUCT_IS_NULL,"param input strcuct invalid!\n");
	CHECK_RET_VAL_P(fCallBack,-PARAM_INPUT_FUNC_IS_NULL,"param input func invalid!\n");

	ptIec104s->set_val_cb   = fCallBack;
	ptIec104s->set_val_priv = pArg;

	return 0;
}

__EX_API__ int wow_iec104_s_get_value_handle(Iec104s_T* ptIec104s,iec10x_get_value_func_t fCallBack,void* pArg)
{
	CHECK_RET_VAL_P(ptIec104s,-PARAM_INPUT_STRUCT_IS_NULL,"param input strcuct invalid!\n");
	CHECK_RET_VAL_P(fCallBack,-PARAM_INPUT_FUNC_IS_NULL,"param input func invalid!\n");

	ptIec104s->get_val_cb   = fCallBack;
	ptIec104s->get_val_priv = pArg;

	return 0;
}

/*brief    Iec104设置更新服务状态回调函数
 *param ： ptIec104s:iec04s操作标识
 *param ： fCallBack:   回调函数 
 *param ： pArg:  回调函数私有变量
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_iec104_s_set_service_handle(Iec104s_T* ptIec104s,iec10x_set_service_func_t fCallBack,void* pArg)
{
	CHECK_RET_VAL_P(ptIec104s,-PARAM_INPUT_STRUCT_IS_NULL,"param input strcuct invalid!\n");
	CHECK_RET_VAL_P(fCallBack,-PARAM_INPUT_FUNC_IS_NULL,"param input func invalid!\n");

	ptIec104s->serv_cb   = fCallBack;
	ptIec104s->serv_priv = pArg;

	return 0;
}

/*brief    iec04遥信突变
 *param ： iec104s:iec04s操作标识
 *param ： ptNode:遥信节点信息
 *param ： u8Count:遥信节点个数
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_iec104_s_build_yx_spon(Iec104s_T* ptIec104s,DevNodeCtrl_T* ptNode,uint8_t u8Count)
{
    uint8_t len = 0,  i;
    uint8_t *ptr  = NULL;
    uint32_t addr = 0;
	uint8_t  send_buf[IEC104_MAX_BUF_LEN];
	
	CHECK_RET_VAL_P(ptIec104s,-PARAM_INPUT_STRUCT_IS_NULL,"param input strcuct invalid!\n");
	CHECK_RET_VAL_P(ptNode && u8Count > 0,-PARAM_INPUT_STRUCT_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(ptIec104s->stat.init == IEC10X_STA_LINK_IDLE,-IEC10X_STATUS_ERROR,"iec101s status(%d) invalid!\n",ptIec104s->stat.init);


	memset(send_buf,0,IEC104_MAX_BUF_LEN);
    Iec104Data_T* Iec104Data = (Iec104Data_T*)send_buf;
    Iec104Asdu_T* asdu = (Iec104Asdu_T*)(Iec104Data->Asdu);
    Iec104AsduInfo_T* info = (Iec104AsduInfo_T*)(asdu->_info);

    /*build head*/
    Iec104Data->Head = IEC104_HEAD;

    /*build control code*/
    Iec104Data->Ctrl.I.Type = 0;
    Iec104Data->Ctrl.I.SendSn = ptIec104s->sn.build_send++;
    Iec104Data->Ctrl.I.RecvSn = ptIec104s->sn.build_recv;
	
    /*build ASDU , COT ,Addr*/
    if(ptNode[0].value.time.year == 0){
		asdu->_type = IEC10X_M_SP_NA_1;
	} else{
		asdu->_type = IEC10X_M_SP_TB_1;
	}
	
	
    asdu->_num._sq  = 0;
    asdu->_num._num = u8Count;
    asdu->_reason._reason = IEC10X_COT_SPONT;
    asdu->_addr = ptIec104s->asdu_addr;

	ptr = info->_addr;
	for(i = 0; i < u8Count;i++)
	{
		/*build info addr*/
		addr = ptNode[i].info.addr;
		memcpy(ptr, &addr, 3);
		ptr += 3;

		/*build info value*/
		*ptr = ptNode[i].value.val.s8;
		ptr++;
		
		/*build timesapce value*/
		if(ptNode[i].value.time.year > 0){
			memcpy(ptr, &ptNode[i].value.time, sizeof(CP56Time2a_T));
			ptr += sizeof(CP56Time2a_T);
		}
	}

    /*len*/
    len = ptr - send_buf;
    Iec104Data->Len = len - 2;

    /* enqueue to the transmisson queue */
	ptIec104s->link_ctx->link_send(ptIec104s->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);

    return 0;
}

/*brief    iec04遥测突变
 *param ： ptIec104s:iec04s操作标识
 *param ： ptNode:遥测节点信息
 *param ： u8Count:遥测节点个数
 *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_iec104_s_build_yc_spon(Iec104s_T* ptIec104s,DevNodeCtrl_T* ptNode, uint8_t u8Count)
{
	int i = 0;
    uint8_t len = 0;
    uint8_t *ptr = NULL;
    uint32_t addr = 0;
	uint8_t  send_buf[IEC104_MAX_BUF_LEN];

	CHECK_RET_VAL_P(ptIec104s,-PARAM_INPUT_STRUCT_IS_NULL,"param input strcuct invalid!\n");
	CHECK_RET_VAL_P(ptNode && u8Count > 0,-PARAM_INPUT_STRUCT_IS_NULL,"param input data invalid!\n");
	CHECK_RET_VAL_P(ptIec104s->stat.init == IEC10X_STA_LINK_IDLE,-IEC10X_STATUS_ERROR,"iec101s status(%d) invalid!\n",ptIec104s->stat.init);


	memset(send_buf,0,IEC104_MAX_BUF_LEN);
    Iec104Data_T* Iec104Data = (Iec104Data_T*)send_buf;
    Iec104Asdu_T* asdu = (Iec104Asdu_T*)(Iec104Data->Asdu);
    Iec104AsduInfo_T* info = (Iec104AsduInfo_T*)(asdu->_info);

    /*build head*/
    Iec104Data->Head = IEC104_HEAD;

    /*build control code*/
    Iec104Data->Ctrl.I.Type = 0;
    Iec104Data->Ctrl.I.SendSn = ptIec104s->sn.build_send++;
    Iec104Data->Ctrl.I.RecvSn = ptIec104s->sn.build_recv;
        

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
    asdu->_reason._reason = IEC10X_COT_SPONT;
    asdu->_addr = ptIec104s->asdu_addr;

	ptr = info->_addr;
	for(i = 0; i < u8Count;i++)
	{
		/*build info addr*/
		addr = ptNode[i].info.addr;
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

		*ptr = 0x00; ///qds =0x00
		ptr++;
		
		/*build timesapce value*/
		if(ptNode[0].value.time.year > 0){
			memcpy(ptr, &ptNode[i].value.time, sizeof(CP56Time2a_T));
			ptr += sizeof(CP56Time2a_T);
		}
	}

    /*len*/
    len = ptr - send_buf;
    Iec104Data->Len = len - 2;

    /* enqueue to the transmisson queue */
	ptIec104s->link_ctx->link_send(ptIec104s->link_ctx,send_buf, len,IEC10X_SEND_TIMEOUT);

    return 0;
}

