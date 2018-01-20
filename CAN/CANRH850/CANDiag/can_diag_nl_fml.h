#ifndef CAN_DIAG_NL_H
#define CAN_DIAG_NL_H
#include "Can_config.h"

typedef enum
{
    NL_PhysicalAddr = 0, //
    NL_FunctionalAddr,  //
} Nl_TAtype_ETypeDef; //

/*Network layer Protocol Data Unit*/
typedef struct
{
    u8 Mtype;//Message type--diagnostics, remote diagnostics
    u8 N_SA;//network source address
    u8 N_TA;//network target address
    u8 N_TAtype;//network target address type---physical, functional
    u8 N_AE;// network address extension
    u8  *pdata;//data
    u16 Datalen;//data length
} Nl_PDU_STypeDef;
/////////////////////////////////////////////////////////////////////////////////
/*change the para of network layer ,Reserved*/
typedef enum
{
    NL_RESULT_CHGPARA_OK = 0,
    NL_RESULT_CHGPARA_RX_ON,
    NL_RESULT_CHGPARA_WRONG_PARAMETER,
    NL_RESULT_CHGPARA_WRONG_VALUE,
} Nl_ResultChangeParameter_ETypeDef;

/*NL ind or confirm AL*/
typedef enum
{
    NL_IND_AL_NULL = 0,
    NL_IND_AL_RX_FF,
    NL_IND_AL_RX_UNEXP_PDU,
    NL_IND_AL_N_INVALID_FS,
    NL_ACK_AL_SENDSUCESS,
    NL_ACK_AL_SENDFAILED,
} Nl_Com_Al_ETypeDef;
///////////////////////////////////////////////////////////////////////////////
//define by MCU Type: u8 u16 u32
typedef  u32    nl_timer_t;
typedef  u32    nl_bit_t;
///////////////////////////////////////////////////////////////////////////////
typedef enum
{
    NL_Tx_ST_Idle = 0,//
    NL_Tx_ST_DlyToTx,//wait time to tx
    NL_Tx_ST_Txing,//
    NL_Tx_ST_TxingWaitIsr,//
} Nl_Tx_ST_ETypeDef; //
///////////////////////////////////////////////////////////////////////////////
//R--Receiver; ST--status; EType--EnumType;SType--StructType
//reciver Progress def
typedef enum
{
    NL_R_ST_Idle = 0,//
    NL_R_ST_WaitTxFC,
    NL_R_ST_TxFC,
    NL_R_ST_WaitRxCF,
    NL_R_ST_Err,
} Nl_Receiver_ST_ETypeDef; //

typedef struct
{
    Nl_Receiver_ST_ETypeDef Status;
    nl_timer_t Timer;//
    //tx
    nl_timer_t TxFailedTimer;//
    Nl_Tx_ST_ETypeDef  TxStatus;
    nl_bit_t   NeedTx          : 1;
    //nl_bit_t   reTx                 :1;
    //
    nl_bit_t   Sn          : 4;
    nl_bit_t   Fs          : 2;
    u8    BsCnt;
    u8    WftCnt;
    u8    STmin;
    u16 ReadIndex;              //
    u16 WriteIndex;             //
    Nl_PDU_STypeDef  N_PDU;  //
} Nl_Receiver_Ctrl_STypeDef;
///////////////////////////////////////////////////////////////////////////////
//S--Sender; ST--status; EType--EnumType;SType--StructType
//Sender Progress def
typedef enum
{
    NL_S_ST_Idle = 0,
    NL_S_ST_WaitTxSF = 1,
    NL_S_ST_TxSF = 2,
    NL_S_ST_WaitTxFF = 3,
    NL_S_ST_TxFF = 4,
    NL_S_ST_WaitRxFC = 5,
    NL_S_ST_WaitTxCF = 6,
    NL_S_ST_TxCF = 7,
    NL_S_ST_WaitTxCFOver = 8,
    NL_S_ST_TxCFOver = 9,
    NL_S_ST_Err = 10,
} Nl_Sender_ST_ETypeDef;

typedef struct
{
    Nl_Sender_ST_ETypeDef Status;
    nl_timer_t Timer;
    //tx
    nl_timer_t TxFailedTimer;
    Nl_Tx_ST_ETypeDef  TxStatus;
    nl_bit_t   NeedTx          : 1;
    //nl_bit_t   reTx            :1;

    nl_bit_t   NoBsFlag     : 1;
    nl_bit_t   Sn          : 4;
    nl_bit_t   Fs          : 2;
    u8    BsCnt;
    u8    WftCnt;
    u8    STmin;
    u16 ReadIndex;              //
    u16 WriteIndex;             //
    Nl_PDU_STypeDef  N_PDU;  //
} Nl_Sender_Ctrl_STypeDef;
///////////////////////////////////////////////////////////////////////////////
typedef struct
{
    u32  RxNPDUidPhys;
    u32  RxNPDUidFunc;
    u32  TxNPDUidDiag;
    nl_timer_t N_ArTimer;//R  time
    nl_timer_t N_BrTimer;//R  time
    nl_timer_t N_CrTimer;//R  time
    nl_timer_t N_AsTimer;//S  time
    nl_timer_t N_BsTimer;//S time
    nl_timer_t N_CsTimer;//S  time
    u8  BsSet;
    u8  STminSet;
} CAN_Nl_Init_STypeDef;

typedef struct
{
    //Link data layer_Protocol Data Unit
    u8  RxLData[8];// Rx buff
    u8  TxLData[8];// Tx  buff  prepare
    NL_MSG_SType LData_txmsg;
    //Networklayer Protocol Data Unit
    CAN_Nl_Init_STypeDef    ParaDefault;
    Nl_Receiver_Ctrl_STypeDef        RCtrl;//
    Nl_Sender_Ctrl_STypeDef     SCtrl;//
} NL_Process_STypeDef;
////////////////////////////////////////////////////////////////////////////////
#if 0
// CallBack pointer function definition
typedef u8 (*pCallBack_RequestSend) (NL_MSG_SType *);
typedef u8 (*pCallBack_Indication) (NL_Process_STypeDef *);
//pCallBack_Req_confirm
//pCallBack_Response
//pCallBack_Rsp_confirm
//pCallBack_Confirm

typedef struct
{
    pCallBack_RequestSend CAN_NLTXmsg;
    pCallBack_Indication   CAN_NLIndAL; /*Pointer to CallBack function ()  to notify upper layer*/

} NL_Service_STypeDef;
#endif
////////////////////////////////////////////////////////////////////////////////
//Net Layer PowerOn Init
void CAN_Nl_Initx(NL_Process_STypeDef *NlPROx, CAN_Nl_Init_STypeDef *ParaInit, u8 *pRbuff, u8 *pSBuff);
//CAN Net Layer Rx CANData
void CAN_Nl_RxProcess(NL_Process_STypeDef *NlPROx, NL_MSG_SType *rxmsg);

u8 CAN_Nl_AlPDUDataRequest(NL_Process_STypeDef *NlPROx, u8 *pApdu, u16 length);
//Timer
void CAN_Nl_Timer_Process(NL_Process_STypeDef *NlPROx);
//CAN TX ok ISR/task call
void CAN_Nl_TxOk(NL_Process_STypeDef *NlPROx);
//
void CAN_Nl_TxFailed(NL_Process_STypeDef* NlPROx);
//
void CAN_Nl_CancelTx(NL_Process_STypeDef* NlPROx);

#endif
