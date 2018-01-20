#include "kernel_id.h"
#include "app.cfg"
#include "base_types.h"
#include "osal.h"
#include "eventDefine.h"
#include "cychdr.h"
#include "taskHandl.h"
#include "MsgDefine.h"
#include "CANDiagApp.h"
#include "NVM_if.h"
#include "can_config.h"
#include "Can_diag_al_apl.h"
#include "ComManageTbox.h"


CAN_MCU_IF_STypeDef CAN_MCU_Interface;
CAN_GPRS_IF_STypeDef CAN_GPRS_Interface;

/************************************************
*			static global variable				*
************************************************/
u8 CANDiagRecvBuf[64];
static u32 timecount;

/************************************************
*			Global function						*
************************************************/
void InitCANDiageepRom0(void)
{
}

void InitCANDiageepRom1(void)
{
}

void InitCANDiageepRom2(void)
{
}

void InitCANDiageepRom3(void)
{
}
void initCANRRAM(void)
{
    BackUpRAMPara_STypeDef* pData = NVM_GetStartAddress(NVM_RRAMNAME(CANapp));

	pData->VIN_Match_Status[0] = 2;
	pData->EngineSt = 0;
	pData->FuelPercent = 0;
	pData->awake_src = 0;
	
}
void ReadCANappRRam()
{
	BackUpRAMPara_STypeDef* pData = NVM_GetStartAddress(NVM_RRAMNAME(CANapp));

	initCANBackUpData(pData);
}
void SystemTick()
{
	timecount+=5;
}
u32 GetSystemTick()
{
	return timecount;
}

/************************************************
*			Local function						*
************************************************/
static void SendCanDiagInitDataTo4G(void);

/***************************define by system*************************************/
s32 SendMsgToCANDiag(u8* MsgData, u16 MsgLen)
{
    ER ret = E_OK;


    ret = OS_MQSend(TASK_NAME(CANDiag), MsgData, MsgLen);
    return ret;
}



void CANDiag_Func(void)
{
    u32 tSignals;

    /* wait for events indefinitely and clear after end of function */
    tSignals = cfThreadWaitWithTimeout((EVENT_GLOBAL_Watchdog | 
                                        EVENT_CANDiag_TIMER_5MS_EVENT |
                                        EVENT_CANDiag_TIMER_10MS_EVENT),
                                       OS_WAITFOREVER);

    if(tSignals & EVENT_CANDiag_TIMER_5MS_EVENT)
    {
      	SystemTick();
    }
	else if(tSignals & EVENT_CANDiag_TIMER_10MS_EVENT)
    {
    	CAN_DIAG_Timer_Process();
    }
}


void CANDiag_MsgHandle(void* pbuf, u16 buflen)
{
	Message_t *pMsg_t;
	pMsg_t = (Message_t *)pbuf;
	
	if(pMsg_t != 0)
	{
		switch(pMsg_t->Type)
		{
			case EVENT_CAN_RX_DATA: 
			{	
				CanRxDataEvt_t *pEvt;
				pEvt = (CanRxDataEvt_t *)pbuf;
				Can_Dll_Process(&pEvt->frame,0);
			}break;

			case EVENT_NVM_SEND_EEPROMREADY_NF:
			{
			 	PwrOnScanSystermPara();		    
			}break;

			case EVENT_COM_RECV_4GMODULE_READY:
    		{
    			SendCanDiagInitDataTo4G();
    		}break;
			
			case EVENT_COM_SEND_CANDIAG_DATA:
			{
				CanDiagRec4GDiagDataHandle(pMsg_t);
			}break;

			case EVENT_TBOX_SEND_KEY_CMD:
			{
				CanDiagRecKeyEvtHandle(pMsg_t);
			}break;

			case EVENT_DIAG_MIAN_VOLT_STATE:
			{
				CanDiagRecMainVoltErrStateHandle(pMsg_t);
			}break;

			case EVENT_DIAG_GPS_ANT_VOLT_STATE:
			{
				CanDiagRecGpsErrStateHandle(pMsg_t);
			}break;

			case EVENT_DIAG_MIC_VOLT_STATE:
			{
				CanDiagRecMicErrStateHandle(pMsg_t);
			}break;

			case EVENT_DIAG_ECALL_KEY_STATE:
			{
				CanDiagRecEcallErrStateHandle(pMsg_t);
			}break;
			
			default:
				break;
		}
	}

}

TASK_INIT(CANDiag)
{
	ReadCANappRRam();
    CAN_DIAG_Init();
    enableCycleEvent(TASK_NAME(CANDiag));
}

TASK_CREATE(CANDiag)
{
	cfThreadCreate(TASK_NAME(CANDiag), CANDiag_Func, CANDiag_MsgHandle);
}


static void SendCanDiagInitDataTo4G(void)
{
	CanDiagSendParkModeTo4G();
	CanDiagSendTUIDTo4G();
	CanDiagSendHardVersionTo4G();
	CanDiagSendVinTo4G();
	CanDiagSendMcuSoftWareTo4G();
}


