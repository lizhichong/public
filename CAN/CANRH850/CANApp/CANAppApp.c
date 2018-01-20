/** 
* @file CANAppApp.c
* 
* @brief CAN app Task
*
* @note deal message from other moudle 
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
 The downmost code is used for system,and you can change their content except function name;
* 
* @author jason
* 
* @version 1.0.0 
* 
* @date 2017/7/20
* 
* modify record£º
*
*/  

/************************************************
*			Include file							*
************************************************/
#include <stdlib.h>
#include <string.h>
#include "sysconfig.h"
#include "base_types.h"
#include "MsgDefine.h"
#include "kernel_id.h"
#include "app.cfg"
#include "osal.h"
#include "eventDefine.h"
#include "cychdr.h"
#include "EventDefine.h"
#include "PwrManageApp.h"
#include "Debug.h"
#include "Vehicle.h"
#include "r_uart.h"
#include "CanAppApp.h"
#include "can_config.h"
#include "AccDet.h"
//#include "ComApp.h"
#include "PwrManageApp.h"
#include "Fifo.h"
#include "MsgDefine.h"
#include "CanCom.h"
#include "ComManageTbox.h"
#include "PtlTboxInfo.h"
//#include "bsp.h"
#include "S401_TBox.h"
#include "Time.h"
#include "RTC.h"
#include "CanNetworkManageMiddle.h"

extern CANCommunicationControl_STypeDef  CANDiagCCCtrl;
/************************************************
*			constant							*
************************************************/

/************************************************
*			internal macro						*
************************************************/
#define CAN_RX_DATA_FIFO_SIZE         1024
#define CAN_TX_DATA_FIFO_SIZE         512

#define CAN_TOL_CHN_NUMS   3
#define BUSOFF  (CAN_Err[CAN_USE_CHN_NUM] == 0x01)

/************************************************
*			static global variable					*
************************************************/

static fifo_TypeDef      CanRxData_Fifo;   //CAN data RX FIFO
static fifo_TypeDef      CanTxData_Fifo;   //CAN data TX FIFO


static u8  CanRxDataBuffer[CAN_RX_DATA_FIFO_SIZE];
static u8  CanTxDataBuffer[CAN_TX_DATA_FIFO_SIZE];



static u16 CanRXFrameCount;

u8 CAN_Err[CAN_TOL_CHN_NUMS];

AccOffJudge_t stAccOffJudge;

/************************************************
*			Local function declare					*
************************************************/
static void CanTxOkCallback(u8 object);
static void CanRxOkCallback(CanRxMsgTypeDef* Msg);
static void CanErrorCallback(u8 error);
void CAN_RxMsgQueue(can_frame *pCanRxData, bool Int);
Can_ReturnType CAN_TxMsgQueue(can_frame *pCanTxData);
static void CAN_RxMsgProcess(can_frame* rxmsg);
static void CanRxQueueHandle(void);
static void CanTxQueueHandle(void);
/************************************************
*			Global function						*
************************************************/
extern void CAN_TxMsgDeInit(void);
extern void CAN_MsgSendTimeInit(void);

/************************************************
*			Local function							*
************************************************/
static void CanTxOkCallback(u8 object)
{
	CAN_Err[CAN_USE_CHN_NUM] = 0x00;
	CAN_AppMsgTxStateConf(object);
	CAN_DIAG_TxConf(object);
	NM_TxConf(object);
}
static void CanRxOkCallback(CanRxMsgTypeDef* Msg)
{
	can_frame RxData;
	 
	if(Msg->IDE ==CAN_ID_STD)
	{
		RxData.id = Msg->StdId;
	}
	else
	{
		RxData.id = Msg->StdId;
	}
	RxData.length = Msg->DLC;
	if(RxData.length>8)RxData.length=8;
	memcpy(&RxData.data,&Msg->Data,  RxData.length);
	CAN_RxMsgQueue(&RxData,true);
}
/** 
* @Func CanErrorCallback
*
* @brief callback function when CAN control happened a error
*
* @note 
*
* @param  error  error status
*
* @return NULL
*/
static void CanErrorCallback(u8 error)
{
	if(HAL_CAN_ERROR_BOF == error)
	{
		CAN_Err[CAN_USE_CHN_NUM] = 0x01;
        DEBUG_MCU(DBG_ERROR, CAN_MODULE_ID, "Can bus off!\r\n");
	}
	else if(HAL_CAN_ERROR_LSFR == error)
	{
        DEBUG_MCU(DBG_ERROR, CAN_MODULE_ID, "Can Send Msg lost!\r\n");
	}
	
}
/** 
* @Func CAN_RxMsgQueue
*
* @brief insert receive CAN frame to CAN queue
*
* @note 
*
* @param  pCanRxData CAN frame to be inserted
			Int not used
*
* @return NULL
*/
void CAN_RxMsgQueue(can_frame *pCanRxData, bool Int)
{
	if(sizeof(can_frame) <= fifo_GetFreeSpace(&CanRxData_Fifo))
	{
		fifo_insert(&CanRxData_Fifo, (u8 *)pCanRxData, sizeof(can_frame));
		CanRXFrameCount++;
	}
	else
	{
	}	
}
Can_ReturnType CAN_TxMsgQueue(can_frame *pCanTxData)
{

	if(CAN_Diag_PhysicalResponses_ID == pCanTxData->id)
	{
		if(TRUE ==CANDiagCCCtrl.DisableDiagMsgTx) return CAN_OK;
	}else if( LocalNMID == pCanTxData->id)
	{
		if(TRUE ==CANDiagCCCtrl.DisableNMMsgTx) return CAN_OK;

	}else
	{
		if(TRUE ==CANDiagCCCtrl.DisableAppMsgTx) return CAN_OK;
	}
	if(sizeof(can_frame) <= fifo_GetFreeSpace(&CanTxData_Fifo))
	{
		loc_mtx(ID_CANTXFIFO);
		fifo_insert(&CanTxData_Fifo, (u8 *)pCanTxData, sizeof(can_frame));
		unl_mtx(ID_CANTXFIFO);
	}else
	{
		return CAN_BUSY;
	}
	return CAN_OK;
}
/** 
* @Func CAN_RxMsgProcess
*
* @brief deal with frame from CanNetwork
*
* @note 
*
* @param  rxmsg receive frame from CANNetwork;
*
* @return NULL
*/
static void CAN_RxMsgProcess(can_frame* rxmsg)
{
	u8 sendBuffer[sizeof(CanRxDataEvt_t)];
	CanRxDataEvt_t *pEvt;
	pEvt = (CanRxDataEvt_t *)sendBuffer;
	if(( rxmsg->id == CAN_Diag_PhysicalRequests_ID) ||(rxmsg->id == CAN_Diag_FunctionalRequests_ID))// Diag message
	{
		//send to Diag task
		pEvt->Super.Type = EVENT_CAN_RX_DATA;
		memcpy((u8 *)&(pEvt->frame) , (u8 *)rxmsg, sizeof(can_frame));
		SendMsgToCANDiag(sendBuffer, sizeof(sendBuffer));
	}
	else if(( rxmsg->id >=0x400u)&&(rxmsg->id < 0x500u)) //  CAN networkmanage message
	{
		//send to CANNM task
		NM_RxPro(rxmsg);
	}
	else if(OK==CAN_CheckAppMsgID(rxmsg->id))// app message
	{
		if(rxmsg->length<8)
			return; 		//CAN communication protocol define DLC = 8;

		CAN_RxAppMsgPro(rxmsg);	//deal with app message
	}
	else ;// nodeal
}
/** 
* @Func CanRxQueueHandle
*
* @brief retrieve CAN Frame from CAN receive queue;
*
* @note 
*
* @param  NULL
*
* @return NULL
*/
static void CanRxQueueHandle(void)
{
	can_frame CanRxMsgBuf;
	while(CanRXFrameCount)
	{
		CanRXFrameCount--;
		fifo_retrieve(&CanRxData_Fifo, (u8*)&CanRxMsgBuf, sizeof(can_frame));
		CAN_RxMsgProcess(&CanRxMsgBuf);
	}
}
static void CanTxQueueHandle(void)
{
	can_frame CAN_TX_MSG;
	HAL_StatusTypeDef ret = HAL_OK;

	if(fifo_GetLen(&CanTxData_Fifo) >= sizeof(can_frame))
	{
		loc_mtx(ID_CANTXFIFO);
		fifo_retrieve(&CanTxData_Fifo, (u8*)&CAN_TX_MSG, sizeof(can_frame));
		unl_mtx(ID_CANTXFIFO);
		ret = Can_TxCanMsgProcess(&CAN_TX_MSG);
		if(ret == HAL_OK)
		{

		}
		else
		{

		}			
	}
			
	
}

void CAN_ConfigInit(void)
{
	fifo_Init(&CanRxData_Fifo, CanRxDataBuffer, CAN_RX_DATA_FIFO_SIZE);
	fifo_Init(&CanTxData_Fifo, CanTxDataBuffer, CAN_TX_DATA_FIFO_SIZE);

	CAN_MsgSendTimeInit();
	
	set_CAN0_STB(DISABLE);
	CanInitSetup(CanTxOkCallback,CanRxOkCallback,CanErrorCallback); 	
}

static void CAN_RecAccStateHandle(const Message_t *pMsg)
{
	VehicleIOSendAccEvt_t *pEvt = (VehicleIOSendAccEvt_t *)pMsg;
	if(pEvt->AccOn == ACC_ON)
	{
		CAN_MCU_Interface.gblIndAccOn = true;
	}else
	{
		CAN_MCU_Interface.gblIndAccOn = false;
	}
}

static void CAN_RecPwrModeHandle(const Message_t *pMsg)
{
	PwrManageModeNf_t *PwrMode = (PwrManageModeNf_t *)pMsg;
	switch(PwrMode->PMState)
	{
		case PWR_MODE_START:
		{

		}break;
		case PWR_MODE_RUNNING:
		{
			
		}break;
		case PWR_MODE_PREPSLEEP:
		{
			if(true == gblCanSleepFlag)
			{
				SendCANSleepMsg(CAN_NM_ST_SLEEP);
			}
			else
			{
				SendCANSleepMsg(CAN_NM_ST_WAKEUP);
			}
			
		}break;
		case PWR_MODE_STANDBY:
		{
			
		}break;

		default :break;
	}
}

static void CAN_RecCallStateHandle(const Message_t *pMsg)
{
	PtlCallStatusEvt_t * pEvt = (PtlCallStatusEvt_t *)pMsg;

	CAN_MCU_Interface.gCallStatus = pEvt->CallStatus;
}

void Can_AccOffTimeOutInit(void)
{
	#define TMER_40S        (40000)
	
	stAccOffJudge.TimeOutValue = TMER_40S;
	stAccOffJudge.TimeCnt = 0;
	stAccOffJudge.AccOffTimeOutFlag = false;
}

static void CAN_AccOffTimeOutJudge(void)
{	
	if(FALSE == CAN_MCU_Interface.gblIndAccOn)
	{
		stAccOffJudge.TimeCnt += 10;
		if(stAccOffJudge.TimeCnt > stAccOffJudge.TimeOutValue)
		{
			stAccOffJudge.TimeCnt = 0;
			stAccOffJudge.AccOffTimeOutFlag = true;
		}
	}
	else
	{
		stAccOffJudge.TimeCnt = 0; 
		stAccOffJudge.AccOffTimeOutFlag = false;
	}
}


/*end by jason*/
/***************************define by system*************************************/
s32 SendMsgToCANApp(u8* MsgData, u16 MsgLen)
{
    ER ret = E_OK;


    ret = OS_MQSend(TASK_NAME(CANApp), MsgData, MsgLen);
    return ret;
}

void CANApp_Func(void)
{
	u32 tSignals;
	static u8 TimeCnt = 0;

	/* wait for events indefinitely and clear after end of function */
	tSignals = cfThreadWaitWithTimeout((EVENT_GLOBAL_Watchdog | 
	                            EVENT_CANApp_TIMER_5MS_EVENT),
	                            OS_WAITFOREVER);

	if(tSignals & EVENT_CANApp_TIMER_5MS_EVENT)
	{	
		
		CanRxQueueHandle();
		Process_can_NM();
		CanTxQueueHandle();

		TimeCnt++;
		if(TimeCnt & 0x01)
		{
			#if CA_MCU_LIB
			Fun_TboxLogic_Call();
			#endif
			CAN_AccOffTimeOutJudge();//10ms
			CAN_AppMsgPeriodSend(); //10ms
		}
		
	}
}


void CANApp_MsgHandle(void* pbuf, u16 buflen)
{
	Message_t *pMsg_t;
	pMsg_t = (Message_t *)pbuf;
	switch(pMsg_t->Type)
	{
		case EVENT_CAN_RX_4G_DATA:
		{
			CAN_COMRx4GmsgProcess(pMsg_t);
		}break;
		
		case EVENT_VEHICLEIO_SEND_CALLTYPE_NF:
		{
			CAN_RxCallTypeHandle(pMsg_t);
		}break;
		
		case EVENT_VEHICLEIO_SEND_ACC_DATA:
		{
			CAN_RecAccStateHandle(pMsg_t);
     	}break;
				
		case EVENT_PWR_SEND_PM_MODE_NF:
		{
			CAN_RecPwrModeHandle(pMsg_t);
		}break;

		case EVENT_TBOX_SEND_CALLSTATUS:
		{
			CAN_RecCallStateHandle(pMsg_t);
		}break;

		case EVENT_COM_RECV_4GMODULE_READY:
		{
			PowerOn_Sync_St();
		}break;
		
		default: 
		 break;
	}
}



TASK_INIT(CANApp)
{
	CAN_TxMsgDeInit();
	CAN_ConfigInit();
	CAN_AppInit();
	Can_AccOffTimeOutInit();
	Process_can_NM();
	enableCycleEvent(TASK_NAME(CANApp));
}

TASK_CREATE(CANApp)
{
	cfThreadCreate(TASK_NAME(CANApp), CANApp_Func, CANApp_MsgHandle);
}
