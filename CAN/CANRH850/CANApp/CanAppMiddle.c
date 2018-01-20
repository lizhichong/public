/** 
* @file Can_app_pro.c
* 
* @brief CAN app frame analytic code
*
* @note  analysis CAN app frame 
file layout:
	file head
	include file(standard file and others)
	constant
	internal macro	
	internal data type
	static global variable
	Local function declare
	Global function
	Local function
* 
* @author jason
* 
* @version 1.0.0 
* 
* @date 2017/7/20
* 
* modify record：
*
*/ 
/************************************************
*			Include file							*
************************************************/
#include <stdlib.h>
#include <string.h>
#include "sysconfig.h"
#include "r_uart.h"
#include "CanAppMiddle.h"
#include "can_config.h"
#include "can_app_pro.h"
#include "ComTboxApp.h"
#include "PtlTboxInfo.h"
#include "fifo.h"
#include "S401_TBox.h"
#include "Vehicle.h"
#include "Vehicle.h"
#include "can_diag_al_apl.h"

/*
* extern value
*/
extern CANCommunicationControl_STypeDef  CANDiagCCCtrl;
extern u8 NM_Stop_App;

/************************************************
*			internal macro							*
************************************************/
#define BB_MCU_DATA_FIFO_SIZE		256
static fifo_TypeDef      BBtoMCUData_Fifo;   //CAN data TX FIFO

#if CA_MCU_LIB
static u8 BBtoMCUDataBuffer[BB_MCU_DATA_FIFO_SIZE];
#endif

#define IGN_StatusIsOn()  CAN_MCU_Interface.gblIndAccOn

#define TBOX_RCV_MSG_ID_MAX_SIZE       (29u)

#define MSG_260_RX_TMO          ((s32)1000u/TIMER_INT_CYC) /*0x260 50ms */
#define MSG_266_RX_TMO          ((s32)1000u/TIMER_INT_CYC) /*0x266 40ms */
#define MSG_276_RX_TMO          ((s32)1000u/TIMER_INT_CYC) /*0x276 40ms */
#define MSG_280_RX_TMO          ((s32)1000u/TIMER_INT_CYC) /*0x280 50ms */
#define MSG_288_RX_TMO          ((s32)1000u/TIMER_INT_CYC) /*0x288 40ms */
#define MSG_2B0_RX_TMO          ((s32)1000u/TIMER_INT_CYC) /*0x2B0 50ms */
#define MSG_2D8_RX_TMO          ((s32)1000u/TIMER_INT_CYC) /*0x2D8 100ms*/
#define MSG_2E0_RX_TMO          ((s32)1000u/TIMER_INT_CYC) /*0x2E0 100ms*/
#define MSG_2F8_RX_TMO          ((s32)1000u/TIMER_INT_CYC) /*0x2F8 100ms*/
#define MSG_320_RX_TMO          ((s32)1000u/TIMER_INT_CYC) /*0x320 100ms*/
#define MSG_326_RX_TMO          ((s32)1000u/TIMER_INT_CYC) /*0x326 100ms*/
#define MSG_32A_RX_TMO          ((s32)1000u/TIMER_INT_CYC) /*0x32A 100ms*/
#define MSG_330_RX_TMO          ((s32)1000u/TIMER_INT_CYC) /*0x330 100ms*/
#define MSG_340_RX_TMO          ((s32)1000u/TIMER_INT_CYC) /*0x340 100ms*/
#define MSG_347_RX_TMO          ((s32)2500u/TIMER_INT_CYC) /*0x347 500ms*/
#define MSG_373_RX_TMO          ((s32)1500u/TIMER_INT_CYC) /*0x373 300ms*/
#define MSG_376_RX_TMO          ((s32)2500u/TIMER_INT_CYC) /*0x376 500ms*/
#define MSG_380_RX_TMO          ((s32)2500u/TIMER_INT_CYC) /*0x380 500ms*/
#define MSG_384_RX_TMO          ((s32)1000u/TIMER_INT_CYC) /*0x384 200ms*/
#define MSG_3D0_RX_TMO          ((s32)1000u/TIMER_INT_CYC) /*0x3D0 200ms*/
#define MSG_3E0_RX_TMO		  ((s32)1000u/TIMER_INT_CYC)	
#define MSG_500_RX_TMO          ((s32)5000u/TIMER_INT_CYC) /*0x500 1000ms*/
#define MSG_501_RX_TMO          ((s32)5000u/TIMER_INT_CYC) /*0x501 1000ms*/
#define MSG_510_RX_TMO          ((s32)5000u/TIMER_INT_CYC) /*0x510 1000ms*/
#define MSG_512_RX_TMO          ((s32)0u/TIMER_INT_CYC)    /*0x512 event*/
#define MSG_580_RX_TMO          ((s32)5000u/TIMER_INT_CYC) /*0x580 1000ms*/
#define MSG_600_RX_TMO          ((s32)0u/TIMER_INT_CYC)    /*0x600 event*/
#define MSG_6B6_RX_TMO          ((s32)1000u/TIMER_INT_CYC) /*0x6B6 100ms*/
#define MSG_6B7_RX_TMO          ((s32)1000u/TIMER_INT_CYC) /*0x6B7 100ms*/

#define MSG_296_TX_PERIODIC_TIME                        (100u/TIMER_INT_CYC)
#define MSG_500_TX_PERIODIC_TIME                        (1000u/TIMER_INT_CYC)
#define MSG_501_TX_PERIODIC_TIME                        (5000u/TIMER_INT_CYC)
#define MSG_6B8_TX_PERIODIC_TIME                        (100u/TIMER_INT_CYC)

#define TBOX_AudioRequest_BYTE_OFFSET                   1U
#define TBOX_AudioRequest_NoReq()                       { Tbox_296IdTxMsg.data[1u] &= (u8)(~(1<<5u|1<<6u)); }
#define TBOX_AudioRequest_ReqType1()                    { TBOX_AudioRequest_NoReq(); Tbox_296IdTxMsg.data[1u] |= (u8)(1u<<5u); }
#define TBOX_AudioRequest_Reserved0()                   { TBOX_AudioRequest_NoReq(); Tbox_296IdTxMsg.data[1u] |= (u8)(2u<<5u); }
#define TBOX_AudioRequest_Reserved1()                   { TBOX_AudioRequest_NoReq(); Tbox_296IdTxMsg.data[1u] |= (u8)(3u<<5u); }

#define TBOX_ECallStatus_BYTE_OFFSET                     1U
#define TBOX_ECallStatus_Idle()                          { Tbox_296IdTxMsg.data[1u] &= (u8)(~(1<<0u|1<<1u|1<<2u));TBOX_AudioRequest_NoReq(); }
#define TBOX_ECallStatus_OnPhone()                       { TBOX_ECallStatus_Idle(); Tbox_296IdTxMsg.data[1u] |= (u8)1u;TBOX_AudioRequest_ReqType1(); }
#define TBOX_ECallStatus_Calling()                       { TBOX_ECallStatus_Idle(); Tbox_296IdTxMsg.data[1u] |= (u8)2u;TBOX_AudioRequest_ReqType1(); }
#define TBOX_ECallStatus_BeingCalled()                   { TBOX_ECallStatus_Idle(); Tbox_296IdTxMsg.data[1u] |= (u8)3u;TBOX_AudioRequest_ReqType1(); }
#define TBOX_ECallStatus_CallEnded()                     { TBOX_ECallStatus_Idle(); Tbox_296IdTxMsg.data[1u] |= (u8)4u;TBOX_AudioRequest_NoReq(); }
#define TBOX_ECallStatus_Reserved0()                     { TBOX_ECallStatus_Idle(); Tbox_296IdTxMsg.data[1u] |= (u8)5u; }
#define TBOX_ECallStatus_Reserved1()                     { TBOX_ECallStatus_Idle(); Tbox_296IdTxMsg.data[1u] |= (u8)6u; }
#define TBOX_ECallStatus_Reserved2()                     { TBOX_ECallStatus_Idle(); Tbox_296IdTxMsg.data[1u] |= (u8)7u; }

#define TBOX_BCallStatus_BYTE_OFFSET                    2U
#define TBOX_BCallStatus_Idle()                         { Tbox_296IdTxMsg.data[TBOX_BCallStatus_BYTE_OFFSET] &= (u8)(~(1<<5u|1<<6u|1<<7u));TBOX_AudioRequest_NoReq(); }
#define TBOX_BCallStatus_OnPhone()                      { TBOX_BCallStatus_Idle(); Tbox_296IdTxMsg.data[TBOX_BCallStatus_BYTE_OFFSET] |= (u8)(1u<<5);TBOX_AudioRequest_ReqType1(); }
#define TBOX_BCallStatus_Calling()                      { TBOX_BCallStatus_Idle(); Tbox_296IdTxMsg.data[TBOX_BCallStatus_BYTE_OFFSET] |= (u8)(2u<<5);TBOX_AudioRequest_ReqType1(); }
#define TBOX_BCallStatus_BeingCalled()                  { TBOX_BCallStatus_Idle(); Tbox_296IdTxMsg.data[TBOX_BCallStatus_BYTE_OFFSET] |= (u8)(3u<<5);TBOX_AudioRequest_ReqType1(); }
#define TBOX_BCallStatus_CallEnded()                    { TBOX_BCallStatus_Idle(); Tbox_296IdTxMsg.data[TBOX_BCallStatus_BYTE_OFFSET] |= (u8)(4u<<5);TBOX_AudioRequest_NoReq(); }
#define TBOX_BCallStatus_Reserved0()                    { TBOX_BCallStatus_Idle(); Tbox_296IdTxMsg.data[TBOX_BCallStatus_BYTE_OFFSET] |= (u8)(5u<<5); }
#define TBOX_BCallStatus_Reserved1()                    { TBOX_BCallStatus_Idle(); Tbox_296IdTxMsg.data[TBOX_BCallStatus_BYTE_OFFSET] |= (u8)(6u<<5); }
#define TBOX_BCallStatus_Invalid()                      { TBOX_BCallStatus_Idle(); Tbox_296IdTxMsg.data[TBOX_BCallStatus_BYTE_OFFSET] |= (u8)(7u<<5); }

/************************************************
*			constant								*
************************************************/
static const u32 TBOX_RCV_MSG_ID_LIST[TBOX_RCV_MSG_ID_MAX_SIZE] =
{
	0x260u,
	0x266u,
	0x276u,
	0X280u,
	0x288u,
	0x2B0u,
	0x2D8u,
	0x2E0u,
	0x2F8u,
	0x320u,
	0x326u,
	0x32Au,
	0x330u,
	0x340u,
	0x347u,
	0x373u,
	0X376u,
	0X380u,
	0x384u,
	0x3D0u,
	0x3E0u,
	0x500u,
	0x501u,
	0x510u,
	0x512u,
	0x580u,
	0x600u,
	0x6B6u,
	0x6B7u,
};
/************************************************
*			internal data type						*
************************************************/
typedef void (*can_rx_cb_t)(void);

typedef struct
{
//    CanRxMsg* pCanMsg;
    can_frame CanMsg;
    can_rx_cb_t CbFn;
    can_rx_cb_t TimeOutFn;
    s32 siTimer;
    s32 siTimeOut;
} node_record_t;
typedef struct
{
    u32 Timer;
    u32 FailedCnt;
    u32  SucceedCnt;
    CANTxMsg_ETypeDef  State;
} CANTxMsgCtrl_STypeDef;
/************************************************
*			Local function declare					*
************************************************/
node_record_t* CAN_FindNode(u32 canrxid);
static void CAN_AppMsgRxMonitor(void);
static void CAN_AppMsgRxMonitorTask(void);
void CAN_TxMsgDeInit(void);
static void CAN_TabInit(void);
void CAN_AppMsgTxProcess(void);
void MSG_ID260Pro();
void MSG_RxTimeOutID260Pro();
void MSG_ID266Pro();
void MSG_RxTimeOutID266Pro();
void MSG_ID276Pro();
void MSG_RxTimeOutID276Pro();
void MSG_ID280Pro();
void MSG_RxTimeOutID280Pro();
void MSG_ID288Pro();
void MSG_RxTimeOutID288Pro();
void MSG_ID2B0Pro();
void MSG_RxTimeOutID2B0Pro();
void MSG_ID2D8Pro();
void MSG_RxTimeOutID2D8Pro();
void MSG_ID2E0Pro();
void MSG_RxTimeOutID2E0Pro();
void MSG_ID2F8Pro();
void MSG_RxTimeOutID2F8Pro();
void MSG_ID320Pro();
void MSG_RxTimeOutID320Pro();
void MSG_ID326Pro();
void MSG_RxTimeOutID326Pro();
void MSG_ID32APro();
void MSG_RxTimeOutID32APro();
void MSG_ID330Pro();
void MSG_RxTimeOutID330Pro();
void MSG_ID340Pro();
void MSG_RxTimeOutID340Pro();
void MSG_ID347Pro();
void MSG_RxTimeOutID347Pro();
void MSG_ID373Pro();
void MSG_RxTimeOutID373Pro();
void MSG_ID376Pro();
void MSG_RxTimeOutID376Pro();
void MSG_ID380Pro();
void MSG_RxTimeOutID380Pro();
void MSG_ID384Pro();
void MSG_RxTimeOutID384Pro();
void MSG_ID3D0Pro();
void MSG_RxTimeOutID3D0Pro();
void MSG_ID3E0Pro();
void MSG_RxTimeOutID3E0Pro();
void MSG_ID500Pro();
void MSG_RxTimeOutID500Pro();
void MSG_ID501Pro();
void MSG_RxTimeOutID501Pro();
void MSG_ID510Pro();
void MSG_RxTimeOutID510Pro();
void MSG_ID512Pro();
void MSG_RxTimeOutID512Pro();
void MSG_ID580Pro();
void MSG_RxTimeOutID580Pro();
void MSG_ID600Pro();
void MSG_RxTimeOutID600Pro();
void MSG_ID6B6Pro();
void MSG_RxTimeOutID6B6Pro();
void MSG_ID6B7Pro();
void MSG_RxTimeOutID6B7Pro();
/************************************************
*			static global variable					*
************************************************/
static node_record_t* BscTab[TBOX_RCV_MSG_ID_MAX_SIZE];
static node_record_t S_Msg260Info = { {0x260u,0,{0}}, MSG_ID260Pro, MSG_RxTimeOutID260Pro, 0, MSG_260_RX_TMO };
static node_record_t S_Msg266Info = { {0x266u,0,{0}}, MSG_ID266Pro, MSG_RxTimeOutID266Pro, 0, MSG_266_RX_TMO };
static node_record_t S_Msg276Info = { {0x276u,0,{0}}, MSG_ID276Pro, MSG_RxTimeOutID276Pro, 0, MSG_276_RX_TMO };
static node_record_t S_Msg280Info = { {0x280u,0,{0}}, MSG_ID280Pro, MSG_RxTimeOutID280Pro, 0, MSG_280_RX_TMO };
static node_record_t S_Msg288Info = { {0x288u,0,{0}}, MSG_ID288Pro, MSG_RxTimeOutID288Pro, 0, MSG_288_RX_TMO };
static node_record_t S_Msg2B0Info = { {0x2b0u,0,{0}}, MSG_ID2B0Pro, MSG_RxTimeOutID2B0Pro, 0, MSG_2B0_RX_TMO };
static node_record_t S_Msg2D8Info = { {0x2d8u,0,{0}}, MSG_ID2D8Pro, MSG_RxTimeOutID2D8Pro, 0, MSG_2D8_RX_TMO };
static node_record_t S_Msg2E0Info = { {0x2e0u,0,{0}}, MSG_ID2E0Pro, MSG_RxTimeOutID2E0Pro, 0, MSG_2E0_RX_TMO };
static node_record_t S_Msg2F8Info = { {0x2f8u,0,{0}}, MSG_ID2F8Pro, MSG_RxTimeOutID2F8Pro, 0, MSG_2F8_RX_TMO };
static node_record_t S_Msg320Info = { {0x320u,0,{0}}, MSG_ID320Pro, MSG_RxTimeOutID320Pro, 0, MSG_320_RX_TMO };
static node_record_t S_Msg326Info = { {0x326u,0,{0}}, MSG_ID326Pro, MSG_RxTimeOutID326Pro, 0, MSG_326_RX_TMO };
static node_record_t S_Msg32AInfo = { {0x32Au,0,{0}}, MSG_ID32APro, MSG_RxTimeOutID32APro, 0, MSG_32A_RX_TMO };
static node_record_t S_Msg330Info = { {0x330u,0,{0}}, MSG_ID330Pro, MSG_RxTimeOutID330Pro, 0, MSG_330_RX_TMO };
static node_record_t S_Msg340Info = { {0x340u,0,{0}}, MSG_ID340Pro, MSG_RxTimeOutID340Pro, 0, MSG_340_RX_TMO };
static node_record_t S_Msg347Info = { {0x347u,0,{0}}, MSG_ID347Pro, MSG_RxTimeOutID347Pro, 0, MSG_347_RX_TMO };
static node_record_t S_Msg373Info = { {0x373u,0,{0}}, MSG_ID373Pro, MSG_RxTimeOutID373Pro, 0, MSG_373_RX_TMO };
static node_record_t S_Msg376Info = { {0x376u,0,{0}}, MSG_ID376Pro, MSG_RxTimeOutID376Pro, 0, MSG_376_RX_TMO };
static node_record_t S_Msg380Info = { {0x380u,0,{0}}, MSG_ID380Pro, MSG_RxTimeOutID380Pro, 0, MSG_380_RX_TMO };
static node_record_t S_Msg384Info = { {0x384u,0,{0}}, MSG_ID384Pro, MSG_RxTimeOutID384Pro, 0, MSG_384_RX_TMO };
static node_record_t S_Msg3D0Info = { {0x3D0u,0,{0}}, MSG_ID3D0Pro, MSG_RxTimeOutID3D0Pro, 0, MSG_3D0_RX_TMO };
static node_record_t S_Msg3E0Info = { {0x3E0u,0,{0}}, MSG_ID3E0Pro, MSG_RxTimeOutID3E0Pro, 0, MSG_3E0_RX_TMO };
static node_record_t S_Msg500Info = { {0x500u,0,{0}}, MSG_ID500Pro, MSG_RxTimeOutID500Pro, 0, MSG_500_RX_TMO };
static node_record_t S_Msg501Info = { {0x501u,0,{0}}, MSG_ID501Pro, MSG_RxTimeOutID501Pro, 0, MSG_501_RX_TMO };
static node_record_t S_Msg510Info = { {0x510u,0,{0}}, MSG_ID510Pro, MSG_RxTimeOutID510Pro, 0, MSG_510_RX_TMO };
static node_record_t S_Msg512Info = { {0x512u,0,{0}}, MSG_ID512Pro, MSG_RxTimeOutID512Pro, 0, MSG_512_RX_TMO };
static node_record_t S_Msg580Info = { {0x580u,0,{0}}, MSG_ID580Pro, MSG_RxTimeOutID580Pro, 0, MSG_580_RX_TMO };
static node_record_t S_Msg600Info = { {0x600u,0,{0}}, MSG_ID600Pro, MSG_RxTimeOutID600Pro, 0, MSG_600_RX_TMO };
static node_record_t S_Msg6B6Info = { {0x6B6u,0,{0}}, MSG_ID6B6Pro, MSG_RxTimeOutID6B6Pro, 0, MSG_6B6_RX_TMO };
static node_record_t S_Msg6B7Info = { {0x6B7u,0,{0}}, MSG_ID6B7Pro, MSG_RxTimeOutID6B7Pro, 0, MSG_6B7_RX_TMO };

static node_record_t* TBOX_RX_MSG_INFO_PTR_LIST[TBOX_RCV_MSG_ID_MAX_SIZE] =
{
	&S_Msg260Info,
	&S_Msg266Info,
	&S_Msg276Info,
	&S_Msg280Info,
	&S_Msg288Info,
	&S_Msg2B0Info,
	&S_Msg2D8Info,
	&S_Msg2E0Info,
	&S_Msg2F8Info,
	&S_Msg320Info,
	&S_Msg326Info,
	&S_Msg32AInfo,
	&S_Msg330Info,
	&S_Msg340Info,
	&S_Msg347Info,
	&S_Msg373Info,
	&S_Msg376Info,
	&S_Msg380Info,
	&S_Msg384Info,
	&S_Msg3D0Info,
	&S_Msg3E0Info,
	&S_Msg500Info,
	&S_Msg501Info,
	&S_Msg510Info,
	&S_Msg512Info,
	&S_Msg580Info,
	&S_Msg600Info,
	&S_Msg6B6Info,
	&S_Msg6B7Info,
};

can_frame Tbox_296IdTxMsg;  /* 100ms  */
can_frame Tbox_6B8IdTxMsg;  /* 100ms *///add by yubao
can_frame Tbox_500IdTxMsg;  /* 1000ms */
can_frame Tbox_501IdTxMsg;  /* 5000ms */

CANTxMsgCtrl_STypeDef    Tbox_296IdTxCtrl= {0};
CANTxMsgCtrl_STypeDef    Tbox_6B8IdTxCtrl= {0};
CANTxMsgCtrl_STypeDef    Tbox_500IdTxCtrl= {0};
CANTxMsgCtrl_STypeDef    Tbox_501IdTxCtrl= {0};

can_frame Tbox_395IdTxMsg;  /* event  */
can_frame Tbox_555IdTxMsg;  /* event  */
CANTxMsgCtrl_STypeDef    Tbox_395IdTxCtrl= {0};
CANTxMsgCtrl_STypeDef    Tbox_555IdTxCtrl= {0};

/************************************************
*			Global function						*
************************************************/
/** 
* @Func CAN_AppInit
*
* @brief init CanAppMiddle.c
*
* @note if you use this file,you should call CAN_AppInit at initialize
*
* @param  NULL
*
* @return NULL
*/
void CAN_AppInit(void)
{
	CAN_TabInit();
	
#if CA_MCU_LIB
	fifo_Init(&BBtoMCUData_Fifo, BBtoMCUDataBuffer, BB_MCU_DATA_FIFO_SIZE);
	Runnable_Init();
#else
	//init can_app_pro.c
	CAN_AppProcInit();
#endif
}
/** 
* @Func CAN_AppMsgPeriodSend
*
* @brief a cycle calling task
*
* @note you need to calling this function TIMER_INT_CYC cycle 
*
* @param  NULL
*
* @return NULL
*/
void CAN_AppMsgPeriodSend(void)
{
	#if !CA_MCU_LIB
	//can_app_pro.c task, 
	CanAppProcTask();
	#endif
	if(1 == NM_Stop_App)
	{
		return;
	}
	
	if(!CANDiagCCCtrl.DisableAppMsgTx)
	{
		CAN_AppMsgTxProcess();
	}
	CAN_AppMsgRxMonitorTask();


}
/** 
* @Func CAN_AppMsgTxStateConf
*
* @brief TX OK state change
*
* @note you should call this function in TXOK callback function,
*
* @param  ucPara
*	this param show you whitch ID message send success
*
* @return NULL
*/
void CAN_AppMsgTxStateConf(u8 ucPara)
	{
	if( 1 == ucPara )
	{
		Tbox_296IdTxCtrl.State=CANTX_SUCCEED;
		Tbox_296IdTxCtrl.SucceedCnt++;
		Tbox_296IdTxCtrl.FailedCnt=0;
	}
	if( 5 == ucPara )
	{
		Tbox_555IdTxCtrl.State=CANTX_SUCCEED;
		Tbox_555IdTxCtrl.SucceedCnt++;
		Tbox_555IdTxCtrl.FailedCnt=0;
	}
	if( 7 == ucPara )
	{
		Tbox_6B8IdTxCtrl.State=CANTX_SUCCEED;
		Tbox_6B8IdTxCtrl.SucceedCnt++;
		Tbox_6B8IdTxCtrl.FailedCnt=0;
	}
}
u8 CAN_CheckAppMsgID(u32 id)
{
    u8 i;
    for(i=0; i<TBOX_RCV_MSG_ID_MAX_SIZE; i++)
    {
        if( id == TBOX_RCV_MSG_ID_LIST[i])
        {
            return OK;
        }
    }
    return ERROR;
}
void CAN_RxAppMsgPro(can_frame *rxframe)
{
    node_record_t* pNode ;
   pNode = CAN_FindNode(rxframe->id);

    if( (node_record_t*)0 != pNode )
    {
        memcpy( (u8*)&(pNode->CanMsg.data[0u]), (u8*)&(rxframe->data[0u]), 8u );
        pNode->siTimer = pNode->siTimeOut;

        if( (void*)0 != pNode->CbFn )
        {
            (*pNode->CbFn)();
        }
    }
}
void CAN_COMRx4GmsgProcess(Message_t *pMsg)
{
	TboxPtlDataEvtSturt *pEvt;
	pEvt = (TboxPtlDataEvtSturt *)pMsg;

	if(NULL != pEvt)
	{
	#if CA_MCU_LIB
		u16 length;
		length = pEvt->Data[3] + 1;
		if(fifo_GetFreeSpace(&BBtoMCUData_Fifo)>length)
		{
			fifo_insert(&BBtoMCUData_Fifo, &pMsg[3], length);
		}
	#else
		Car_Contrl_CmdRev(&pEvt->Data[0]);
	#endif		
	}

}
void CAN_RxCallTypeHandle(Message_t *pMsg)
{
    VehSendCallTypeEvt_t * pEvt = NULL;

	pEvt = ( VehSendCallTypeEvt_t *)pMsg;

	if(NULL != pEvt)
	{
		if((ECALLING == pEvt->CallSrc)
	   		||(BECALLED == pEvt->CallSrc)
	   		||(NOCALLING == pEvt->CallSrc))
		{
			switch(pEvt->CallStatus)
			{
				case CALLIDLE:
					TBOX_ECallStatus_Idle();
					break;
				case CALLONLINE:
					TBOX_ECallStatus_OnPhone();
					break;
				case CALLING:
					TBOX_ECallStatus_Calling();
					break;
				case CALLED:
					TBOX_ECallStatus_BeingCalled();
					break;
				case CALLEND:
					TBOX_ECallStatus_CallEnded();
					break;
				default:
					TBOX_ECallStatus_Idle();
					break;			
			}
   		}
		else if(BCALLING == pEvt->CallSrc)
		{
			switch(pEvt->CallStatus)
			{
				case CALLIDLE:
					TBOX_BCallStatus_Idle();
					break;
				case CALLONLINE:
					TBOX_BCallStatus_OnPhone();
					break;
				case CALLING:
					TBOX_BCallStatus_Calling();
					break;
				case CALLED:
					TBOX_BCallStatus_BeingCalled();
					break;
				case CALLEND:
					TBOX_BCallStatus_CallEnded();
					break;
				default:
					TBOX_BCallStatus_Idle();
					break;						
			}
		}
	}
}
/*
* for CA mcu calling
*/
void RTC_SetAlarm(RTC_AlarmCfg_t *config)
{
	
}
//receive message
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_IP_260_Buff_CANRX_IP_260_Buff(void)
{
	return (u8*)&(S_Msg260Info.CanMsg.data[0u]);
}
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_PEPS_266_Buff_CANRX_PEPS_266_Buff(void)
{
	return (u8*)&(S_Msg266Info.CanMsg.data[0u]);
}
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_276_Buff_CANRX_GW_276_Buff(void)
{
	return (u8*)&(S_Msg276Info.CanMsg.data[0u]);
}
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_IP_280_Buff_CANRX_IP_280_Buff(void)
{
	return (u8*)&(S_Msg280Info.CanMsg.data[0u]);
}
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_BCM_288_Buff_CANRX_BCM_288_Buff(void)
{
	return (u8*)&(S_Msg288Info.CanMsg.data[0u]);
}
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_2B0_Buff_CANRX_GW_2B0_Buff(void)
{
	return (u8*)&(S_Msg2B0Info.CanMsg.data[0u]);
}
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_2D8_Buff_CANRX_GW_2D8_Buff(void)
{
	return (u8*)&(S_Msg2D8Info.CanMsg.data[0u]);
}
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_2E0_Buff_CANRX_GW_2E0_Buff(void)
{
	return (u8*)&(S_Msg2E0Info.CanMsg.data[0u]);
}
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_HU_2F8_Buff_CANRX_HU_2F8_Buff(void)
{
	return (u8*)&(S_Msg2B0Info.CanMsg.data[0u]);
}
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_320_Buff_CANRX_GW_320_Buff(void)
{
	return (u8*)&(S_Msg320Info.CanMsg.data[0u]);
}
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_326_Buff_CANRX_GW_326_Buff(void)
{
	return (u8*)&(S_Msg326Info.CanMsg.data[0u]);
}
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_32A_Buff_CANRX_GW_32A_Buff(void)
{
	return (u8*)&(S_Msg32AInfo.CanMsg.data[0u]);
}
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_AC_330_Buff_CANRX_AC_330_Buff(void)
{
	return (u8*)&(S_Msg330Info.CanMsg.data[0u]);
}
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_AC_340_Buff_CANRX_AC_340_Buff(void)
{
	return (u8*)&(S_Msg340Info.CanMsg.data[0u]);
}
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_BCM_347_Buff_CANRX_BCM_347_Buff(void)
{
	return (u8*)&(S_Msg347Info.CanMsg.data[0u]);
}
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_373_Buff_CANRX_GW_373_Buff(void)
{
	return (u8*)&(S_Msg373Info.CanMsg.data[0u]);
}
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_376_Buff_CANRX_GW_376_Buff(void)
{
	return (u8*)&(S_Msg376Info.CanMsg.data[0u]);
}
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_IP_380_Buff_CANRX_IP_380_Buff(void)
{
	return (u8*)&(S_Msg380Info.CanMsg.data[0u]);
}
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_BCM_384_Buff_CANRX_BCM_384_Buff(void)
{
	return (u8*)&(S_Msg384Info.CanMsg.data[0u]);
}
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_3D0_Buff_CANRX_GW_3D0_Buff(void)
{
	return (u8*)&(S_Msg2B0Info.CanMsg.data[0u]);
}
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_3E0_Buff_CANRX_GW_3E0_Buff(void)
{
	return (u8*)&(S_Msg3E0Info.CanMsg.data[0u]);
}
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_HU_500_Buff_CANRX_HU_500_Buff(void)
{
	return (u8*)&(S_Msg500Info.CanMsg.data[0u]);
}
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_HU_501_Buff_CANRX_HU_501_Buff(void)
{
	return (u8*)&(S_Msg501Info.CanMsg.data[0u]);
}
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_510_Buff_CANRX_GW_510_Buff(void)
{
	return (u8*)&(S_Msg510Info.CanMsg.data[0u]);
}
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_PEPS_512_Buff_CANRX_PEPS_512_Buff()
{
	return (u8*)&(S_Msg512Info.CanMsg.data[0u]);
}
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_580_Buff_CANRX_GW_580_Buff(void)
{
	return (u8*)&(S_Msg580Info.CanMsg.data[0u]);
}
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_600_Buff_CANRX_GW_600_Buff(void)
{
	return (u8*)&(S_Msg600Info.CanMsg.data[0u]);
}
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_6B6_Buff_CANRX_GW_6B6_Buff(void)
{
	return (u8*)&(S_Msg6B6Info.CanMsg.data[0u]);
}
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_6B7_Buff_CANRX_GW_6B7_Buff(void)
{
	return (u8*)&(S_Msg6B7Info.CanMsg.data[0u]);
}
//send message
void Rte_IWrite_Fun_TboxLogic_Call_CANTX_TBox_296_Buff_CANTX_TBox_296_Buff(u8* u)
{	
	u8 notchange1;
	u8 notchange2;
	notchange1 = Tbox_296IdTxMsg.data[1];
	notchange2 = Tbox_296IdTxMsg.data[2];
	memcpy( (u8*)&(Tbox_296IdTxMsg.data[0u]), u, 8u );
	Tbox_296IdTxMsg.data[1] |= (notchange1 & 0x67);
	Tbox_296IdTxMsg.data[2] |= (notchange2 & 0xe0);
}

void  Rte_IWrite_Fun_TboxLogic_Call_CANTX_TBox_555_Buff_CANTX_TBox_555_Buff(u8* u)
{		
	memcpy( (u8*)&(Tbox_555IdTxMsg.data[0u]), u, 8u );
}
void Rte_IWrite_Fun_TboxLogic_Call_gTbox_555_SendFlag_gTbox_555_SendFlag(u8 u)
{
	Can_ReturnType ucRet = CAN_BUSY;
	
	if(u == 1)
	{
		Tbox_555IdTxCtrl.Timer = 0;

		ucRet = CAN_TxMsgQueue(&Tbox_555IdTxMsg);

		if( CAN_BUSY == ucRet )
		{
			Tbox_555IdTxCtrl.State=CANTX_WAIT;
		}else
		{
			Tbox_555IdTxCtrl.State=CANTX_ING;
		}
	}
}
void Rte_IWrite_Fun_TboxLogic_Call_CANTX_Test_6B8_Buff_CANTX_Test_6B8_Buff(u8* u)
{
	memcpy( (u8*)&(Tbox_6B8IdTxMsg.data[0u]), u, 8u );
}
void Rte_IWrite_Fun_TboxLogic_Call_CANTX_TBox_601_Buff_CANTX_TBox_601_Buff(u8* u)
{}
u8 *Rte_IRead_Fun_TboxLogic_Call_CANRX_HU_580_Buff_CANRX_HU_580_Buff()
{
	return (u8*)&(S_Msg580Info.CanMsg.data[0u]);;
}
void Rte_IWrite_Fun_TboxLogic_Call_CANTX_TBox_6C9_Buff_CANTX_TBox_6C9_Buff(u8* u)
{}
u8 *Rte_IRead_Fun_TboxLogic_Call_CANRX_AD_6C6_Buff_CANRX_AD_6C6_Buff()
{
	return (u8*)&(S_Msg580Info.CanMsg.data[0u]);;
}
u8 *Rte_IRead_Fun_TboxLogic_Call_CANRX_Test_6B7_Buff_CANRX_Test_6B7_Buff()
{
	return (u8*)&(S_Msg580Info.CanMsg.data[0u]);;
}
void Rte_IWrite_Fun_TboxLogic_Call_CANTX_TBox_600_Buff_CANTX_TBox_600_Buff(u8* u)
{}
void Rte_IWrite_Fun_TboxLogic_Call_CANTX_TBox_6C8_Buff_CANTX_TBox_6C8_Buff(u8* u)
{

}
u8 *Rte_IRead_Fun_TboxLogic_Call_CANRX_AD_6C7_Buff_CANRX_AD_6C7_Buff()
{
	return (u8*)&(S_Msg580Info.CanMsg.data[0u]);;
}
u8 *Rte_IRead_Fun_TboxLogic_Call_CANRX_Test_6B6_Buff_CANRX_Test_6B6_Buff()
{
	return (u8*)&(S_Msg580Info.CanMsg.data[0u]);;
}
u8 *Rte_IRead_Fun_TboxLogic_Call_CANRX_BCM_5FF_Buff_CANRX_BCM_5FF_Buff()
{
	return (u8*)&(S_Msg580Info.CanMsg.data[0u]);;
}

///function
u8 tmp_DIDF1F1[6]={0xff, 0xff, 0xff, 0xff, 0xff, 0xff};	//system config
u8* Rte_IRead_Fun_TboxLogic_Call_EEPROMRX_F1F1_Buff_EEPROMRX_F1F1_Buff(void)
{
	SystemParaReadEeprom(member_offset(EepromPara_STypeDef,LocalFuncBitConf),tmp_DIDF1F1,sizeof(tmp_DIDF1F1));
	return tmp_DIDF1F1;
}
u8 tmp_DIDF190[17]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};	//VIN
u8* Rte_IRead_Fun_TboxLogic_Call_EEPROMRX_F190_Buff_EEPROMRX_F190_Buff(void)
{
	SystemParaReadEeprom(member_offset(EepromPara_STypeDef,CAR_VinNumber),tmp_DIDF190,sizeof(tmp_DIDF190));
	return tmp_DIDF190;
}
u8 AppAllowSleepFlag;
void Rte_IWrite_Fun_TboxLogic_Call_gAppAllowSleepFlag_gAppAllowSleepFlag(u8 u)
{
/*
0X0: AllowSleep
0X1: Not AllowSleep
*/
	AppAllowSleepFlag = u;		//you need add this flag to your sleep control.
}
u8 CrashOutputStatus;
void Rte_IWrite_Fun_TboxLogic_Call_CANSignal_SRS_CrashOutputStatus_CANSignal_SRS_CrashOutputStatus(u8 u)
{
/*
0X0: No Crash
0X1: crash
0x2~0xF: NotUsed
*/
	CrashOutputStatus=u;
}
void Rte_IWrite_Fun_TboxLogic_Call_CANSignal_SRS_CrashOutputStatusCheckSum_CANSignal_SRS_CrashOutputStatusCheckSum(u8 u)
{
/*
Checksum=(SRS_CrashOutputStatus) XOR 0xF
*/
	if((CrashOutputStatus^0x0f) == u)		//check OK
	{
		if(CrashOutputStatus == 1)
		{
			//happend a crash
		}else
		{
			//No crash
		}
	}else
	{
		//check invalid
	}
}
u32 Rte_IRead_Fun_TboxLogic_Call_SystemMs_SystemMs(void)
{
	
	//TBD ,return a SystemMs.
	return 0;
}
u8 SaveBuff[100];
u8* Rte_IRead_Fun_TboxLogic_Call_AppSave_ReadBuff_AppSave_ReadBuff(void)
{
	//we need allocate 100 byte eeprom or datastorage to CA mcu
	//TBD
	return &SaveBuff[0];
}
void Rte_IWrite_Fun_TboxLogic_Call_AppSave_WriteBuff_AppSave_WriteBuff(u8* u)
{
	//save CA mcu data to eeprom or datastorage
	//TBD
	//memcpy(&SaveBuff[0], u, 100);
}
u8 Rte_IRead_Fun_TboxLogic_Call_BatteryValue_BatteryValue(void)
{
/*
放大系数为 10，例如实际电压为10.1V，传送值为101
*/
	//return a BatteryValue
	return 120;
}
u8 DID_F1F5_BUFF[8];
u8* Rte_IRead_Fun_TboxLogic_Call_EEPROMRX_F1F5_Buff_EEPROMRX_F1F5_Buff(void)
{
	return DID_F1F5_BUFF;
}
u8 DID_F1F6_BUFF[16];
u8* Rte_IRead_Fun_TboxLogic_Call_EEPROMRX_F1F6_Buff_EEPROMRX_F1F6_Buff(void)
{
	return DID_F1F6_BUFF;
}
void Rte_IWrite_Fun_TboxLogic_Call_EEPROMTX_F1F5_Buff_EEPROMTX_F1F5_Buff(u8* u)
{
	
}
void Rte_IWrite_Fun_TboxLogic_Call_EEPROMTX_F1F6_Buff_EEPROMTX_F1F6_Buff(u8* u)
{
	
}
u8 LocalTime[6] = {17, 9, 15, 10, 53, 00};
u8* Rte_IRead_Fun_TboxLogic_Call_LocalTime_LocalTime(void)
{
/*
byte0：表示年（偏
移量 2000，即 0
代表 2000 年）
byte1：表示月
byte2：表示日
byte3：表示时
byte4：表示分
byte5：表示秒
*/
	return &LocalTime[0];
}
u8 Rte_IRead_Fun_TboxLogic_Call_EEPROMRX_F1F0_Buff_EEPROMRX_F1F0_Buff(void)
{
	return 0;
}
u8 Rte_IRead_Fun_TboxLogic_Call_CrashSignal_Crashsignal(void)
{
/*
0x00：碰撞信号无效
0x01：碰撞信号有效
*/
	//singal from IO 
	return 0;
}
u8 DID_F210_Buff[64];
u8* Rte_IRead_Fun_TboxLogic_Call_EEPROMRX_F210_Buff_EEPROMRX_F210_Buff(void)
{
	return DID_F210_Buff;
}
void Rte_IWrite_Fun_TboxLogic_Call_EEPROMTX_F22A_Buff_EEPROMTX_F22A_Buff(u8 u)
{
/*
0x00:匹配成功
0x01:匹配失败
*/	
	//VIN match status
}
void Rte_IWrite_Fun_TboxLogic_Call_RAMTX_F1FD_Buff_RAMTX_F1FD_Buff(u8 u)
{
/*
0x00:网络检测失败
0x01:网络检测成功
*/	
	//initial value is 1
}
void Rte_IWrite_Fun_TboxLogic_Call_Led_ReqControl_Led_ReqControl(u8 u)
{
/*
0x00:Led 灭
0x01:Led 亮
0x02:Led 闪烁
*/	
	//led control request
}
u8 Rte_IRead_Fun_TboxLogic_Call_parkmode_parkmode(void)
{
/*
0： Park Mode 开启（未激活）
1： Park Mode 关闭（已激活）
*/	
	return 1;
}
void Rte_IWrite_Fun_TboxLogic_Call_CANSignal_HU_BcallReq_CANSignal_HU_BcallReq(u8 u)
{
/*
0x0=no request;
0x1=call up(拨电话） ;
0x2 =hangup(挂电话）
0x3=Reserved;
*/
	//B-call request from HU
}
u8 Rte_IRead_Fun_TboxLogic_Call_WakeUp_Source_WakeUp_Source(void)
{
/*
1： 4G 网络唤醒（包括短信、电话）
2： SOS 按键唤醒
3： CAN 唤醒
4： SRS 唤醒
5： RTC 唤醒
6：电源档位不为OFF 档唤醒
7： G-SENSOR 信号唤醒
*/	
	//wakeup reason
	return 6;
}
///Uart comm
u8 receiveData[256];
u8 *Rte_IRead_Fun_TboxLogic_Call_UARTRX_Buff_UARTRX_Buff(void)
{
	//receive msg from BB.To achieve this ,you need build a fifo to save msg from BB,
	//and when CA mcu call this function,return a msg retrieve from fifo
	 uint8_T *data;
	if(fifo_GetLen(&BBtoMCUData_Fifo) > 0)
	{
		fifo_retrieve(&BBtoMCUData_Fifo, &receiveData[0], 1);
		if(fifo_GetLen(&BBtoMCUData_Fifo) >= receiveData[0])
		{
			fifo_retrieve(&BBtoMCUData_Fifo, &receiveData[1], receiveData[0]);
			data = &receiveData[0];
		}else
		{
			//length error
			fifo_Reset(&BBtoMCUData_Fifo);
			data = NULL_PTR;
		}	
	}
	else
	{
	    //TRACE_RPMSG("M3printf: there is no data from r4\r\n");
		data = NULL_PTR;
	}
	return data;
}

void Rte_IWrite_Fun_TboxLogic_Call_UARTTX_Buff_UARTTX_Buff(u8* u)
{
	u32 senddatabuffer[(270+3)/sizeof(u32)];
	
	u8 Length = 0;

	TboxPtlDataEvtSturt *pEvt;

	if(u != NULL)
	{
	  	Length = u[0];
		if(Length != 0)
		{
			pEvt = (TboxPtlDataEvtSturt *)senddatabuffer;
			pEvt->Head.Msg.Type = EVENT_COM_SEND_DATA_TO_4GMODULE;
			pEvt->Data[0] = CMD_TBOX_INFO;
			pEvt->Data[1] = UNVARTRAN_INFO;
			pEvt->Data[2] = 0x00;
			pEvt->Data[3] = UNVAR_TRAN_CID;
			pEvt->Data[4] = 0;
			pEvt->Data[5] = 0;
			pEvt->Data[6] = 0;
			pEvt->Head.Length = Length + (4 + 4);
			memcpy(&pEvt->Data[7], u, Length+1);
			SendMsgToComTbox((u8*)senddatabuffer, sizeof(TboxPtlDataEvtSturt) + pEvt->Head.Length);
			Length = 0;
		}
		else
		{
		}
	}
	else
	{
	}
	return ;
}

/* end for CA mcu calling*/
/************************************************
*			Local function							*
************************************************/
node_record_t* CAN_FindNode(u32 canrxid)
{
    u8  i = 0u;
    node_record_t* pNode ;
		pNode=NULL_PTR;
    for(i=0; i<TBOX_RCV_MSG_ID_MAX_SIZE; i++)
    {
        pNode= BscTab[i];

        if( (node_record_t*)0u != pNode )
        {
            if( canrxid == pNode->CanMsg.id )
            {
                return pNode;
            }
        }
    }
    return pNode;
}
static void CAN_AppMsgRxMonitor(void)
{
    node_record_t* pNode = (node_record_t*)0u;
    u8 i = 0u;

    for( i = 0u; i <TBOX_RCV_MSG_ID_MAX_SIZE; i++ )
    {
        pNode = BscTab[i];

        if( (node_record_t*)0 != pNode )
        {
            pNode->siTimer--;
            if( pNode->siTimer < 0 )
            {
                pNode->siTimer = pNode->siTimeOut;
                if( (void*)0 != pNode->CbFn )
                {
                    (*pNode->TimeOutFn)();
                }
            }
        }
    }
}
static void CAN_AppMsgRxMonitorTask(void)
{
	static u32 s_uiIgnStOnTimer  = 0u;
	if(FALSE==IGN_StatusIsOn())		//TBD only do this after acc on 1s later,now we do not have acc status
	{
		s_uiIgnStOnTimer++;
		if( s_uiIgnStOnTimer > (1000u/TIMER_INT_CYC) )   //1000ms
		{
			CAN_AppMsgRxMonitor();
		}
	}
	else
	{

	}
}

void CAN_MsgSendTimeInit(void)
{
	Tbox_296IdTxCtrl.Timer = MSG_296_TX_PERIODIC_TIME-2;  //next 10ms period send
}

void CAN_TxMsgDeInit(void)
{	
	u8 i;

	Tbox_555IdTxMsg.id	   = 0x555u;
	Tbox_555IdTxMsg.length = 8u;
	for(i = 0;i < 8;i++)
	{
		Tbox_555IdTxMsg.data[i] = 0;
	}

	Tbox_296IdTxMsg.id     = 0x296u;
	Tbox_296IdTxMsg.length = 8u;
	for(i = 0;i < 8;i++)
	{
		Tbox_296IdTxMsg.data[i] = 0;
	}

	Tbox_6B8IdTxMsg.id     = 0x6B8u;
	Tbox_6B8IdTxMsg.length = 8u;
	for(i = 0;i < 8;i++)
	{
		Tbox_6B8IdTxMsg.data[i] = 0;
	}
}
static void CAN_TabInit(void)
{
    u8 i = 0u;

    for( i = 0u; i < TBOX_RCV_MSG_ID_MAX_SIZE; i++ )
    {
        BscTab[i] = (node_record_t*)0u;
    }
    for( i = 0u; i < TBOX_RCV_MSG_ID_MAX_SIZE; i++ )
    {
        BscTab[i] = TBOX_RX_MSG_INFO_PTR_LIST[i];
    }
}
/*
* 10ms
*/
void CAN_AppMsgTxProcess(void)
{
	Can_ReturnType ret = CAN_BUSY;
	Tbox_296IdTxCtrl.Timer++;
	
	if((Tbox_296IdTxCtrl.Timer >= MSG_296_TX_PERIODIC_TIME)
	   ||(Tbox_296IdTxCtrl.State == CANTX_WAIT) 
	   )
	{
		Tbox_296IdTxCtrl.Timer=0;
	
		ret = CAN_TxMsgQueue(&Tbox_296IdTxMsg);

		if( CAN_BUSY == ret )
		{
			Tbox_296IdTxCtrl.State=CANTX_WAIT;
		}
		else
		{
			Tbox_296IdTxCtrl.State=CANTX_ING;
		}
	}
}
void MSG_ID260Pro()
{
	DTCRxCANID260Process();
}
void MSG_RxTimeOutID260Pro()
{
	
}
void MSG_ID266Pro()
{
	DTCRxCANID266Process();
}
void MSG_RxTimeOutID266Pro()
{
	
}
void MSG_ID276Pro()
{
	
}
void MSG_RxTimeOutID276Pro()
{
	
}
void MSG_ID280Pro()
{
	DTCRxCANID280Process();
}
void MSG_RxTimeOutID280Pro()
{
	
}
void MSG_ID288Pro()
{
	DTCRxCANID288Process();
}
void MSG_RxTimeOutID288Pro()
{
	
}
void MSG_ID2B0Pro()
{
	DTCRxCANID2B0Process();
}
void MSG_RxTimeOutID2B0Pro()
{
	
}
void MSG_ID2D8Pro()
{
	
}
void MSG_RxTimeOutID2D8Pro()
{
	
}
void MSG_ID2E0Pro()
{
	
}
void MSG_RxTimeOutID2E0Pro()
{
	
}
void MSG_ID2F8Pro()
{
	DTCRxCANID2F8Process();
}
void MSG_RxTimeOutID2F8Pro()
{
	
}
void MSG_ID320Pro()
{
	DTCRxCANID320Process();
}
void MSG_RxTimeOutID320Pro()
{
	
}
void MSG_ID326Pro()
{
	DTCRxCANID326Process();
}
void MSG_RxTimeOutID326Pro()
{
	
}
void MSG_ID32APro()
{
	DTCRxCANID32AProcess();
}
void MSG_RxTimeOutID32APro()
{
	
}
void MSG_ID330Pro()
{
	DTCRxCANID330Process();
}
void MSG_RxTimeOutID330Pro()
{
	
}
void MSG_ID340Pro()
{
	
}
void MSG_RxTimeOutID340Pro()
{
	
}
void MSG_ID347Pro()
{
	DTCRxCANID347Process();
}
void MSG_RxTimeOutID347Pro()
{
	
}
void MSG_ID373Pro()
{
	
}
void MSG_RxTimeOutID373Pro()
{
	
}
void MSG_ID376Pro()
{
	
}
void MSG_RxTimeOutID376Pro()
{
	
}
void MSG_ID380Pro()
{
	DTCRxCANID380Process();
}
void MSG_RxTimeOutID380Pro()
{
	
}
void MSG_ID384Pro()
{
	DTCRxCANID384Process();
}
void MSG_RxTimeOutID384Pro()
{
	
}
void MSG_ID3D0Pro()
{
	
}
void MSG_RxTimeOutID3D0Pro()
{
	
}
void MSG_ID3E0Pro()
{
	
}
void MSG_RxTimeOutID3E0Pro()
{
	
}
void MSG_ID500Pro()
{
	DTCRxCANID500Process();
}
void MSG_RxTimeOutID500Pro()
{
	
}
void MSG_ID501Pro()
{
	DTCRxCANID501Process();
}
void MSG_RxTimeOutID501Pro()
{
	
}
void MSG_ID510Pro()
{
	
}
void MSG_RxTimeOutID510Pro()
{
	
}
void MSG_ID512Pro()
{
	DTCRxCANID512Process();
}
void MSG_RxTimeOutID512Pro()
{
	
}
void MSG_ID580Pro()
{
	
}
void MSG_RxTimeOutID580Pro()
{
	
}
void MSG_ID600Pro()
{
	
}
void MSG_RxTimeOutID600Pro()
{
	
}
void MSG_ID6B6Pro()
{
	
}
void MSG_RxTimeOutID6B6Pro()
{
	
}
void MSG_ID6B7Pro()
{
	
}
void MSG_RxTimeOutID6B7Pro()
{
	
}
