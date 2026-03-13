#ifndef __IEC101_BASE_H_
#define __IEC101_BASE_H_

#include "iec10x_base.h"
#include "protocol/wow_proto_port.h"
#include "protocol/iec10x/wow_iec10x.h"


#define IEC101_MAX_BUF_LEN          256

#define IEC101_HEARTBEART_TIMEOUT    (1000)	


/*	
	DIR  	PRM   FCB/ACD  FCV/DFC  功能码
	 bit7   bit6    bit5     bit4   bit3~bit0
*/
#define     IEC101_CTRL_DIR_DOWN            0x01	//1：表示报文是子站向主站传输
#define     IEC101_CTRL_DIR_UP              0x00	//0：表示报文是主站向子站传输
#define     IEC101_CTRL_FCB_OPPO_BIT        0x01	//用于每个站连续的 Send/Confirm 或者 Request/Respond 服务的变化位
#define     IEC101_CTRL_FCB_OPPO_NONE       0x00	//用于每个站连续的 Send/Confirm 或者 Request/Respond 服务的变化位
#define     IEC101_CTRL_PRM_MASTER          0x01	//1=报文从源站（启动站）发出
#define     IEC101_CTRL_PRM_SLAVE           0x00	//0=报文从被控站（响应站）发出
#define     IEC101_CTRL_FCV_ENABLE          0x01	//1=表示帧计数位 FCB 的变化有效
#define     IEC101_CTRL_FCV_DISABLE         0x00	//0=表示帧计数位 FCB 的变化无效
#define     IEC101_CTRL_ACD_WITH_DATA       0x01	//1=有 1 级数据传输的访问要求
#define     IEC101_CTRL_ACD_NONE_DATA       0x00	//0=没有 1 级数据传输的访问要求
#define     IEC101_CTRL_DFC_CAN_REC         0x00	//0=表示可以接收更多后续报文
#define     IEC101_CTRL_DFC_CANNOT_REC      0x01	//1=表示更多的后续报文将引起数据溢出

 /** down **/
#define     IEC101_CTRL_RESET_LINK          0x00		//复位远方链路
#define     IEC101_CTRL_SEND_USR_DATA_ACK   0x03		//发送/确认 用户数据
#define     IEC101_CTRL_SEND_DATA           0x04		//发送/无回答 用户数据   	无回答
#define     IEC101_CTRL_REQ_LINK_STATUS     0x09		//请求链路状态
#define     IEC101_CTRL_REQ_USR_DATA_1      0x0A		//请求 1 级用户数据
#define     IEC101_CTRL_REQ_USR_DATA_2      0x0B		//请求 2 级用户数据


/** up **/
#define     IEC101_CTRL_CONFIRM_ACK     	0x00	//ACK：肯定认可
#define     IEC101_CTRL_CONFIRM_NACK    	0x01	//NACK：报文未收到，链路忙
#define     IEC101_CTRL_RES_USR_DATA_ACK    0x08	//ACK用户数据
#define     IEC101_CTRL_RES_USR_DATA_NACK   0x09	//NACK：请求的数据无效
#define     IEC101_CTRL_RES_LINK_STATUS     0x0B	//链路状态或访问要求


#define     IEC101_STABLE_BEGING             0X10
#define     IEC101_STABLE_END                0X16
#define     IEC101_STABLE_LEN                0X06
#define     IEC101_VARIABLE_BEGING           0X68
#define     IEC101_VARIABLE_END              0X16



#define     IEC101_VARIABLE_HEAD_LEN          (sizeof(Iec101Data68_T)-1)
#define     IEC101_VARIABLE_ASDU_LEN          (sizeof(Iec101Asdu_T)-1)
#define     IEC101_VARIABLE_INFO_LEN          (sizeof(Iec101AsduInfo_T)-1)
#define     IEC101_VARIABLE_LEN               (IEC101_VARIABLE_HEAD_LEN+IEC101_VARIABLE_ASDU_LEN+IEC101_VARIABLE_INFO_LEN+2)        /*add cs+end*/

#pragma pack(1)

typedef struct{
    uint8_t _func:4;               /* function */
    uint8_t _fcv:1;
    uint8_t _fcb:1;                /* Frame calculate bit */
    uint8_t _prm:1;                /* 1:from start station, 0:from end station */
    uint8_t _dir:1;
}Iec101CtrlDown_T;

typedef struct {
    uint8_t _func:4;               /*function*/
    uint8_t _dfc:1;
    uint8_t _acd:1;
    uint8_t _prm:1;
    uint8_t _dir:1;
}Iec101CtrlUp_T;

typedef union{
    Iec101CtrlUp_T   up;
    Iec101CtrlDown_T down;
	uint8_t val;
}Iec101Ctrl_U;

typedef  struct{
    uint8_t		 _begin;
    Iec101Ctrl_U _ctrl;
    uint16_t     _addr;
    uint8_t      _cs;
    uint8_t      _end;
}Iec101Data10_T;


typedef struct{
    uint8_t _begin;
    uint8_t _len;
    uint8_t _len_cfm;
    uint8_t _begin_cfm;
    Iec101Ctrl_U _ctrl;
    uint16_t _addr;
    uint8_t _asdu[1];
}Iec101Data68_T;


/*asdu number*/
typedef struct {
    uint8_t _num:7;
    uint8_t _sq:1;	//是否连续
}Iec101AsduNum_T;

/*asdu reason*/
typedef   struct {
    uint16_t _reason:6;
    uint16_t _pn:1;
    uint16_t _test:1;
}Iec101AsduReason_T;

/*asdu info*/
typedef   struct{
    uint16_t _addr;
    uint8_t _element[1];
}Iec101AsduInfo_T;

/*asdu*/
typedef struct{
    uint8_t        		 _type;
    Iec101AsduNum_T      _num;
    Iec101AsduReason_T   _reason;
    uint16_t        	 _addr;
    uint8_t         	 _info[1];
}Iec101Asdu_T;


/*int */
typedef   struct{
    int16_t _detect;
    uint8_t _qds;
}Iec101Detect_T;

typedef   struct{
    int32_t _detect;
    uint8_t _qds;
}Iec101DetectInt32_T;

/*float*/
typedef   struct{
    float _detect;
    uint8_t _qds;
}Iec101DetectFloat_T;

/*int  sq=0 */
typedef   struct{
    uint16_t _addr;
    int16_t _detect;
    uint8_t _qds;
}Iec101DetectSq0_T;

typedef   struct{
	uint16_t _addr;
    int32_t _detect;
    uint8_t _qds;

}Iec101DetectInt32Sq0_T;

/*float sq=0*/
typedef   struct{
    uint16_t _addr;
    float _detect;
    uint8_t _qds;
}Iec101DetectSq0Float_T;

typedef struct{
	Iec101Asdu_T*	   yk_bak;
	Iec101Asdu_T*	   yt_bak;
	uint16_t  yk_size;
	uint16_t  yt_size;
}Iec101AsduBack_T;


#endif
