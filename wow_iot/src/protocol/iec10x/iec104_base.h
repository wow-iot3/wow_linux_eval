#ifndef __IEC104_BASE_H_
#define __IEC104_BASE_H_


#include "iec10x_base.h"
#include "protocol/iec10x/wow_iec10x.h"
#include "protocol/wow_proto_port.h"


#define     IEC104_MAX_BUF_LEN          256
#define     IEC104_HEAD                 0X68

/* U帧处理 */
#define   IEC104_U_FUNC_STARTDT           0x07		//0B00000111
#define   IEC104_U_FUNC_STOPDT            0x13		//0B00010011
#define   IEC104_U_FUNC_TESTER            0x43		//0B01000011

#define   IEC104_U_FUNC_STARTDT_ACK       0x0b		//0B00001011
#define   IEC104_U_FUNC_STOPDT_ACK        0x23		//0B00100011
#define   IEC104_U_FUNC_TESTER_ACK        0x83		//0B10000011


#pragma pack(1)

/*asdu number*/
typedef struct {
    uint8_t _num:7;
    uint8_t _sq:1;	//是否连续
}Iec104AsduNum_T;

/*asdu reason*/
typedef   struct {
    uint16_t _reason:14;
    uint16_t _pn:1;
    uint16_t _test:1;
}Iec104AsduReason_T;

/*asdu*/
typedef struct{
    uint8_t        		 _type;
    Iec104AsduNum_T      _num;
    Iec104AsduReason_T   _reason;
    uint16_t        	 _addr;
    uint8_t        		 _info[1];
}Iec104Asdu_T;

/*asdu info*/
typedef   struct{
    uint8_t  _addr[3];
    uint8_t _element[1];
}Iec104AsduInfo_T;


/* Control filed I type */
typedef struct {
    uint32_t Type:1;
    uint32_t SendSn:15;
    uint32_t Reserve:1;
    uint32_t RecvSn:15;
}Iec104CtrlI_T;

/* Control filed S type */
typedef struct {

    uint32_t Type1:1;
    uint32_t Type2:1;
    uint32_t Reserve:15;
    uint32_t RecvSn:15;
}Iec104CtrlS_T;

/* Control filed U type */
typedef struct {

    uint32_t Type1:1;
    uint32_t Type2:1;
    uint32_t Startdt:1;
    uint32_t StartdtAck:1;
    uint32_t Stopdt:1;
    uint32_t StopdtAck:1;
    uint32_t Tester:1;
    uint32_t TesterAck:1;
    uint32_t Reserve:24;
}Iec104CtrlU_T;

/* Control filed type */
typedef struct {

    uint32_t Type1:1;
    uint32_t Type2:1;
    uint32_t Reserve:30;
}Iec104CtrlType_T;

/* Control filed type */
typedef struct {
    uint32_t Func:8;
    uint32_t Reserve:24;
}Iec104CtrlFunc_T;

typedef union{
    Iec104CtrlI_T        I;
    Iec104CtrlS_T    	 S;
    Iec104CtrlU_T     	 U;
    Iec104CtrlType_T     Type;
    Iec104CtrlFunc_T     Func;
}Iec104Ctrl_T;

typedef struct{
    uint8_t         Head;
    uint8_t         Len;
    Iec104Ctrl_T    Ctrl;
    uint8_t         Asdu[1];
}Iec104Data_T;


/*int */
typedef   struct{
    int16_t _detect;
    uint8_t _qds;
}Iec104Detect_T;

typedef   struct{
    int32_t _detect;
    uint8_t _qds;
}Iec104DetectInt32_T;

/*float*/
typedef   struct{
    float _detect;
    uint8_t _qds;
}Iec104DetectFloat_T;

/*int  sq=0 */
typedef   struct{
    uint8_t  _addr[3];
    int16_t _detect;
    uint8_t _qds;
}Iec104DetectSq0_T;

typedef   struct{
	uint8_t  _addr[3];
    int32_t _detect;
    uint8_t _qds;

}Iec104DetectInt32Sq0_T;

/*float sq=0*/
typedef   struct{
    uint8_t  _addr[3];
    float _detect;
    uint8_t _qds;
}Iec104DetectSq0Float_T;


/* length */
#define     IEC104_HEAD_LEN          (sizeof(Iec104Data_T)-1)
#define     IEC104_ASDU_LEN          (sizeof(Iec104Asdu_T)-1)
#define     IEC104_INFO_LEN          (sizeof(Iec104AsduInfo_T)-1)
#define     IEC104_DATA_LEN          (IEC104_HEAD_LEN+IEC104_ASDU_LEN+IEC104_INFO_LEN)


typedef struct{
	int build_send;
	int build_recv;
	int deal_send;
	int deal_recv;
}__attribute__((aligned(4))) Iec104Sn_T;



#endif
