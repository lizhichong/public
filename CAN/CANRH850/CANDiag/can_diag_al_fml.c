/** 
* Copyright (C), CHINATSP,www.chinatsp.com.  All rights reserved.
*
* @file can_diag_al_fml.c
* 
* @Description Diagnostics on Controller Area Networks, Application layer, ISO 15765-3
*
* @author jason
* 
* @version v1.0.0 
* 
* @date 2017/7/20
* 
* History��
*
*/  
/************************************************
*			Include file							*
************************************************/
#include <stdlib.h>
#include <limits.h>
#include "Can_config.h"
#include "Debug.h"

/************************************************
*			constant								*
************************************************/

/************************************************
*			internal macro							*
************************************************/
/*Uart debug Print information*/
#define  TRACE_AFL    
 /*Uart debug Not Print information*/
//#define  TRACE_AFL   TRACE_NOTHING 

#define  StartS3SeverTimer()   {AlPROx->Timer=AL_S3sever_MAX;AlPROx->TimerStart = true;}
#define  StopS3SeverTimer()   {AlPROx->TimerStart = false;AlPROx->Timer=0;}
//#define  StartConnetS3SeverTimer()   {AlPROx->Timer=AL_S3sever_MAX;AlPROx->TimerStart = 1;}
#define CAN_Al_GetDefaultTimerCount()       GetSystemTick()

#define CAN_Al_ErrSendNRC(x)  {S_AL_ReportMsg[0u] = 0x7f;S_AL_ReportMsg[1u] =pdata[0];S_AL_ReportMsg[2u]=x;ucRet=CAN_AL_TxDataRequest(NlPROx,&S_AL_ReportMsg[0],3);\
                                             AlPROx->AlTxSid = S_AL_ReportMsg[1];AlPROx->AlTxReason =S_AL_ReportMsg[2];}
/************************************************
*			internal data type						*
************************************************/

/************************************************
*			static global variable					*
************************************************/
static u8 S_AL_ReportMsg[NL_TXBUFFMAXSIZE] = {0};

static u32 S_uiSeed = 0u;
static u8 S_ucTesterKey[4u];
/************************************************
*			Local function declare					*
************************************************/
void CAN_Al_ResetAllCtrlValue(Al_Ctrl_STypeDef *AlPROx);
void CAN_Al_ChangeDsc(Al_Ctrl_STypeDef *AlPROx);
u8 CAN_Al_DiagnosticSessionControl_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len);
u8 CAN_Al_EcuReset_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len);
static u32 CAN_AL_GetSecuritySeed(void);
static u8 CAN_Al_ChkSecurityKey(void);
u8 CAN_Al_SecurityAccess_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len);
static u8 CAN_Al_CommunicationControl_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len);
u8 CAN_Al_SID_TesterPresent_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len);
u8 CAN_Al_SID_ControlDTCSetting_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len);
u8 CAN_Al_SID_ReadDataByIdentifier_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len);
u8 CAN_Al_ReadMemoryByAddress_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len);
u8 CAN_Al_SID_WriteDataByIdentifier_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len);
u8 CAN_Al_SID_ClearDiagnosticInformation_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len);
u8 CAN_Al_SID_ReadDTCInformation_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len);
u8 CAN_Al_InputOutputControlByIdentifier_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len);
u8 CAN_Al_SID_RoutineControl_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len);
u8 CAN_Al_RequestDownload_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len);
u8 CAN_Al_TransferData_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len);
u8 CAN_Al_RequestTransferExit_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len);
static u8 CAN_Al_Ack0x11_Process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len);
u8  CAN_AL_TxDataRequest(NL_Process_STypeDef* NlPROx,u8* pData, u16 DataLen );

u8 CAN_Al_DiagnosticFunctionRequest_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len);
u8 CAN_Al_DiagnosticWriteEEPROM_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len);
u8 CAN_Al_DiagnosticReadEEPROM_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len);
u8 CAN_Al_DiagnosticTransferFile_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len);

/************************************************
*			Global function						*
************************************************/
/** 
* @Func CAN_Al_Ctrl_Init
*
* @brief AL init function

* @note 
*
* @param   Al_Ctrl_STypeDef* AlPROx :  struct pointer
*
* @return NULL
*/
void CAN_Al_Ctrl_Init(Al_Ctrl_STypeDef* AlPROx)
{
    AlPROx->TimerStart=false;
    AlPROx->Timer       = 0;
    AlPROx->DsTypes   =  AL_DEFAULT_SESSION;
    AlPROx->DsTypesNext = AL_NULL_SESSION;
    AlPROx->DsTypesTimeOver = AL_NULL_SESSION;
    AlPROx->AlTxSid = 0;
    AlPROx->AlTxReason = 0;

#if 0
    AlPROx->Security.TimerStart = 1;
    AlPROx->Security.Timer = (10000 / AL_BASICTIME);
#else
    AlPROx->Security.TimerStart=false;
    AlPROx->Security.Timer=0;
#endif
    AlPROx->Security.RequestSeed = false;
    AlPROx->Security.AttemptsNumber = 0;
    AlPROx->Security.Status    = AL_SECURITY_LOCK;

    AlPROx->CCCtrlType = AL_CommunicationControl_enableRxAndTx;
    AlPROx->CCComType = AL_CommunicationControl_APPNM;
    AlPROx->DTCSetType = AL_ControlDTCSetting_ON;
    AlPROx->TPS3OverFlag = FALSE;
	UdsDtcCtrlIsOffFlag=false;
	CAN_Sid_SetIdAppAndNMCtrlType (AL_CommunicationControl_enableRxAndTx, AL_CommunicationControl_APPNM); //

}
/** 
* @Func CAN_Al_Timer_Process
*
* @brief AL timer process
*
* @note  called by Diag timer process
*
* @param   Al_Ctrl_STypeDef* AlPROx :  struct pointer
*
* @return NULL
*/
void CAN_Al_Timer_Process(Al_Ctrl_STypeDef* AlPROx)
{
    if(0 != AlPROx->TimerStart)
    {
        if(AlPROx->Timer > 0) AlPROx->Timer--;
        if(0 == AlPROx->Timer)
        {
            //CAN_Al_ResetAllCtrlValue();
            //CAN_Al_Ctrl_Init(AlPROx);
            if(AL_DEFAULT_SESSION != AlPROx->DsTypes)
            {
                AlPROx->DsTypesTimeOver = AL_DEFAULT_SESSION ;
                CAN_Al_ChangeDsc(AlPROx);
            }

            CAN_MCU_Interface.gblDiagWorking=FALSE;
            StopS3SeverTimer();

            DEBUG_MCU(DBG_INFO,CAN_MODULE_ID," CanDiag : S3TimeOver \r\n");
        }
    }
    if(0!=AlPROx->Security.TimerStart)
    {
        if(AlPROx->Security.Timer > 0) AlPROx->Security.Timer--;
        if(0 == AlPROx->Security.Timer)
        {
            //AlPROx->Security.AttemptsNumber=0;
            AlPROx->Security.TimerStart=false;
            //AlPROx->Security.RequestSeed=0;
        }
    }
}
/** 
* @Func CAN_Al_NlUSData_con
*
* @brief NL<======>AL 
*
* @note  
*
* @param   NL_Process_STypeDef *NlPROx :  struct pointer
*			Nl_Com_Al_ETypeDef reason: NL send season
*
* @return NULL
*/
void CAN_Al_NlUSData_con(NL_Process_STypeDef *NlPROx, Nl_Com_Al_ETypeDef reason)
{
    Al_Ctrl_STypeDef* AlPROx;
    // add
    AlPROx =  &AL_Process[0];

    if(NL_IND_AL_RX_FF == reason)
    {
        CAN_MCU_Interface.gblDiagWorking=TRUE;
        StartS3SeverTimer();
#ifdef DEBUG_PRINT_DETAIL
        DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"CanDiag : S3TimerReStart rx  ff. \r\n");
#endif // DEBUG_PRINT_DETAIL
        // StopS3SeverTimer();
    }
    else if(NL_ACK_AL_SENDSUCESS == reason)
    {
        if(AlPROx->AlTxReason!=0x78)
        {
            StartS3SeverTimer();
            // TRACE_AFL("\r\n##CYZ## : S3TimeStart ack. \r\n");
        }
        if(SID_DiagnosticSessionControl==AlPROx->AlTxSid)
        {
            CAN_Al_ChangeDsc(AlPROx);
#ifdef DEBUG_PRINT_DETAIL
            DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"CanDiag : dsctype--%2x,  Security---%2x, sid--%2x \r\n",AlPROx->DsTypes,AlPROx->Security.Status,AlPROx->AlTxSid);
#endif // DEBUG_PRINT_DETAIL
        }
        AlPROx->AlTxSid = 0;
        AlPROx->AlTxReason = 0;
    }
    else if(NL_ACK_AL_SENDFAILED == reason)
    {
        StartS3SeverTimer();
        DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"CanDiag : S3TimeStart  ack failed. \r\n");
        AlPROx->AlTxSid = 0;
        AlPROx->AlTxReason = 0;
    }
    else if(NL_IND_AL_RX_UNEXP_PDU == reason)
    {
        StartS3SeverTimer();
        DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"CanDiag : Diag data rx err!\r\n");
    }
}
/** 
* @Func CAN_Al_NlReciveDataInd
*
* @brief AL receive NL send data
*
* @note  called by NL
*
* @param   NL_Process_STypeDef *NlPROx :  struct pointer
*			u8* pdata: point to receive data
*			u16 len : receive len
*
* @return NULL
*/
void CAN_Al_NlReciveDataInd(NL_Process_STypeDef* NlPROx,u8* pdata, u16 len)
{
    u8 uiSid;
#ifdef DEBUG_PRINT_DETAIL
	u16 i;
#endif // DEBUG_PRINT_DETAIL
    Al_Ctrl_STypeDef* AlPROx;
    if((len == 0) || (len > NL_RXBUFFMAXSIZE) )return;

    // add
    AlPROx = &AL_Process[0];

    CAN_MCU_Interface.gblDiagWorking=TRUE;
    StartS3SeverTimer();

    //StopS3SeverTimer();
 
#ifdef DEBUG_PRINT_DETAIL
    if(pdata[0]!=0x3e)
    {
       DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"CanDiag : RX A_PDU: ");
        for(i=0; i<len; i++)
        {
            DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"%02x  ",pdata[i]);
        }
        DEBUG_MCU(DBG_INFO,CAN_MODULE_ID, "\r\n");
    }
#endif // DEBUG_PRINT_DETAIL

    uiSid=pdata[0];
    switch(uiSid)
    {
	
#if CAN_YUANTEL_HOST_ENABLE  //zhouxb 20171018 ��Ϸ��񣬲��߼��
		case SID_FunctionRequest:
			CAN_Al_DiagnosticFunctionRequest_process(AlPROx,NlPROx,pdata,len);
			break;
		case SID_WriteEEPROM:
			CAN_Al_DiagnosticWriteEEPROM_process(AlPROx,NlPROx,pdata,len);
			break;
		case SID_ReadEEPROM:
			CAN_Al_DiagnosticReadEEPROM_process(AlPROx,NlPROx,pdata,len);
			break;
		case SID_TransferFile:
			CAN_Al_DiagnosticTransferFile_process(AlPROx,NlPROx,pdata,len);
			break;
#endif
	    case SID_DiagnosticSessionControl:
	        CAN_Al_DiagnosticSessionControl_process(AlPROx,NlPROx,pdata,len);
	        break;
	    case SID_EcuReset:
	        CAN_Al_EcuReset_process(AlPROx,NlPROx,pdata,len);
	        break;
	    case SID_SecurityAccess:
	        CAN_Al_SecurityAccess_process(AlPROx,NlPROx,pdata,len);
	        break;
	    case SID_CommunicationControl:
	        CAN_Al_CommunicationControl_process(AlPROx,NlPROx,pdata,len);
	        break;
	    case SID_TesterPresent:
	        CAN_Al_SID_TesterPresent_process(AlPROx,NlPROx,pdata,len);
	        break;
	    case SID_ControlDTCSetting:
	        CAN_Al_SID_ControlDTCSetting_process(AlPROx,NlPROx,pdata,len);
	        break;
	    case SID_ReadDataByIdentifier:
	        CAN_Al_SID_ReadDataByIdentifier_process(AlPROx,NlPROx,pdata,len);
	        break;
	    case SID_ReadMemoryByAddress:
	    	//CAN_Al_ReadMemoryByAddress_process(AlPROx,NlPROx,pdata,len);
			CAN_Al_Ack0x11_Process(AlPROx,NlPROx,pdata,len);
	        break;
	    case SID_ReadDataByPeriodicIdentifier:
	        CAN_Al_Ack0x11_Process(AlPROx,NlPROx,pdata,len);
	        break;
	    case SID_WriteDataByIdentifier:
	        CAN_Al_SID_WriteDataByIdentifier_process(AlPROx,NlPROx,pdata,len);
	        break;
	    case SID_WriteMemoryByAddress:
	        CAN_Al_Ack0x11_Process(AlPROx,NlPROx,pdata,len);
	        break;
	    case SID_DynamicallyDefineDataIdentifier:
	        CAN_Al_Ack0x11_Process(AlPROx,NlPROx,pdata,len);
	        break;
	    case SID_ClearDiagnosticInformation:
	        CAN_Al_SID_ClearDiagnosticInformation_process(AlPROx,NlPROx,pdata,len);
	        break;
	    case SID_ReadDTCInformation:
	        CAN_Al_SID_ReadDTCInformation_process(AlPROx,NlPROx,pdata,len);
	        break;
	    case SID_InputOutputControlByIdentifier:
	        CAN_Al_InputOutputControlByIdentifier_process(AlPROx,NlPROx,pdata,len);
	        break;
	    case SID_RoutineControl:
	        CAN_Al_SID_RoutineControl_process(AlPROx,NlPROx,pdata,len);
	        break;
	    case SID_RequestDownload:
			//CAN_Al_RequestDownload_process(AlPROx,NlPROx,pdata,len);
	        CAN_Al_Ack0x11_Process(AlPROx,NlPROx,pdata,len);
	        break;
	    case SID_TransferData:
			//CAN_Al_TransferData_process(AlPROx,NlPROx,pdata,len);
	        CAN_Al_Ack0x11_Process(AlPROx,NlPROx,pdata,len);
	        break;
	    case SID_RequestTransferExit:
			//CAN_Al_RequestTransferExit_process(AlPROx,NlPROx,pdata,len);
	        CAN_Al_Ack0x11_Process(AlPROx,NlPROx,pdata,len);
	        break;

	    default:
	        CAN_Al_Ack0x11_Process(AlPROx,NlPROx,pdata,len);
	        break;
    }
}

/************************************************
*			Local function							*
************************************************/
void CAN_Al_ResetAllCtrlValue(Al_Ctrl_STypeDef *AlPROx)
{
    AlPROx->Security.TimerStart = false;
    AlPROx->Security.Timer = 0;
    AlPROx->Security.RequestSeed = false;
    AlPROx->Security.AttemptsNumber = 0;
    AlPROx->Security.Status    = AL_SECURITY_LOCK;
    //CommunicationControl (0x28) ControlDTCSetting(0x85) reset
    AlPROx->CCCtrlType = AL_CommunicationControl_enableRxAndTx;
    AlPROx->CCComType = AL_CommunicationControl_APPNM;
    AlPROx->DTCSetType = AL_ControlDTCSetting_ON;
    UdsDtcCtrlIsOffFlag=false;
	CAN_Sid_SetIdAppAndNMCtrlType (AL_CommunicationControl_enableRxAndTx, AL_CommunicationControl_APPNM); //
}
void CAN_Al_ChangeDsc(Al_Ctrl_STypeDef *AlPROx)
{
    if(AL_DEFAULT_SESSION == AlPROx->DsTypesTimeOver  )//S3 time over
    {
        CAN_Al_Ctrl_Init(AlPROx);
        CAN_Al_ResetAllCtrlValue(AlPROx);
        AlPROx->TPS3OverFlag = TRUE;
    }
    else
    {
        AlPROx->AlTxSid = 0;
        AlPROx->AlTxReason = 0;
        AlPROx->TPS3OverFlag = FALSE;
        //timer  not  reset
        if(AL_DEFAULT_SESSION == AlPROx->DsTypesNext )
        {
            if(AL_DEFAULT_SESSION ==  AlPROx->DsTypes)
            {
                //Reset All
                CAN_Al_ResetAllCtrlValue(AlPROx);
                AlPROx->DsTypesNext = AL_NULL_SESSION;
                AlPROx->DsTypes = AL_DEFAULT_SESSION;
                AlPROx->DsTypesTimeOver = AL_NULL_SESSION;

                //AlPROx->Security.RequestSeed = 0;
                // AlPROx->Security.AttemptsNumber = 0;
                AlPROx->Security.Status    = AL_SECURITY_LOCK;
            }
            else
            {
                //Reset All
                CAN_Al_ResetAllCtrlValue(AlPROx);
                AlPROx->DsTypesNext = AL_NULL_SESSION;
                AlPROx->DsTypes = AL_DEFAULT_SESSION;
                AlPROx->DsTypesTimeOver = AL_NULL_SESSION;
                //  AlPROx->AlTxSid = 0;
                //  AlPROx->AlTxReason = 0;

                //AlPROx->Security.RequestSeed = 0;
                // AlPROx->Security.AttemptsNumber = 0;
                AlPROx->Security.RequestSeed = false;
                AlPROx->Security.Status = AL_SECURITY_LOCK;
                //ReadDataByPeriodicIdentifier

            }
        }
        else if(AL_PROGRAMMING_SESSION == AlPROx->DsTypesNext)
        {
            if(AL_DEFAULT_SESSION ==  AlPROx->DsTypes)
            {
                AlPROx->DsTypes =  AL_PROGRAMMING_SESSION;
                AlPROx->DsTypesNext = AL_NULL_SESSION;
            }
            else
            {
                AlPROx->Security.TimerStart = false;
                AlPROx->Security.Timer = 0;
                AlPROx->Security.RequestSeed    = false;
                AlPROx->Security.AttemptsNumber = 0;
                AlPROx->Security.Status    = AL_SECURITY_LOCK;
                AlPROx->DsTypes =  AL_PROGRAMMING_SESSION;
                AlPROx->DsTypesNext = AL_NULL_SESSION;
            }
        }
        else if(AL_EXTEND_DIAGNOSTIC_SESSION == AlPROx->DsTypesNext)
        {
            if(AL_DEFAULT_SESSION ==  AlPROx->DsTypes)
            {
                AlPROx->DsTypes =  AL_EXTEND_DIAGNOSTIC_SESSION;
                AlPROx->DsTypesNext = AL_NULL_SESSION;
            }
            else
            {
                AlPROx->Security.TimerStart = false;
                AlPROx->Security.Timer = 0;
                AlPROx->Security.RequestSeed    = false;
                AlPROx->Security.AttemptsNumber = 0;
                AlPROx->Security.Status    = AL_SECURITY_LOCK;
                AlPROx->DsTypes =  AL_EXTEND_DIAGNOSTIC_SESSION;
                AlPROx->DsTypesNext = AL_NULL_SESSION;
            }
        }
        else;
    }
}
/////////////////////////////////////////////////////////////////

#if CAN_YUANTEL_HOST_ENABLE
u8 CAN_Al_DiagnosticFunctionRequest_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len)
{
	u8 ucRet;
	u8 ucReuslt;

	ucRet = MCURET_OK;
	
	ucReuslt = CAN_Al_DiagnosticFunctionRequest(&pdata[1]);

	if(ucReuslt==0)
	{
		S_AL_ReportMsg[0u] = pdata[0];
		S_AL_ReportMsg[1u] = pdata[1];
		ucRet = CAN_AL_TxDataRequest(NlPROx,&S_AL_ReportMsg[0],2);
		AlPROx->AlTxSid = pdata[0];
		AlPROx->AlTxReason = 0;
		return ucRet;
	}
	else
	{
		CAN_Al_ErrSendNRC (ucReuslt);//NRC
		return ucRet;
	}
}

u8 CAN_Al_DiagnosticWriteEEPROM_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len)
{
	u8 ucRet;
	u8 ucReuslt;

	ucRet = MCURET_OK;

	ucReuslt = CAN_Al_DiagnosticWriteEEPROM(pdata, len);

	if(ucReuslt==0)
	{
		S_AL_ReportMsg[0u] = pdata[0];
		S_AL_ReportMsg[1u] = pdata[1];
		ucRet=CAN_AL_TxDataRequest(NlPROx,&S_AL_ReportMsg[0],2);
		AlPROx->AlTxSid = pdata[0];
		AlPROx->AlTxReason = 0;
		return ucRet;
	}
	else
	{
		CAN_Al_ErrSendNRC (ucReuslt);//NRC
		return ucRet;
	}
}

u8 CAN_Al_DiagnosticReadEEPROM_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len)
{
	u16  i,usDidNum;
    u16 rIndex,wIndex;
    u8 ucRet;

    ucRet = MCURET_OK;

    //requestOutOfRange
    usDidNum = (len - 1);

    //func  process
    S_AL_ReportMsg[0u] = pdata[0];
    rIndex=1;
    wIndex=1;
    for(i=0; i<usDidNum; i++)
    {
        if(MCURET_ERR == CAN_Al_DiagnosticReadEEPROM(pdata, rIndex, &S_AL_ReportMsg[wIndex], &wIndex))
        {
            CAN_Al_ErrSendNRC (0x31);//NRC
            return ucRet;
        }
        rIndex++;
    }

    ucRet=CAN_AL_TxDataRequest(NlPROx,&S_AL_ReportMsg[0],wIndex);
    AlPROx->AlTxSid = pdata[0];
    AlPROx->AlTxReason =0;
    return ucRet;
}

u8 CAN_Al_DiagnosticTransferFile_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len)
{
	u8 ucRet;
	u8 ucReuslt;

	ucRet = MCURET_OK;

	ucReuslt = CAN_Al_DiagnosticTransferFile(pdata, len);

	if(ucReuslt==0)
	{
		S_AL_ReportMsg[0u] = 0x04;//pdata[0];
		S_AL_ReportMsg[1u] = pdata[1];
		ucRet=CAN_AL_TxDataRequest(NlPROx,&S_AL_ReportMsg[0],2);
		AlPROx->AlTxSid = pdata[0];
		AlPROx->AlTxReason = 0;
	}
	else
	{
		CAN_Al_ErrSendNRC (ucReuslt);//NRC
	}
	return ucRet;
}
#endif

/*
* SID 0x10 process
*/
u8 CAN_Al_DiagnosticSessionControl_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len)
{
    u8 ucRet;
    u8 ucSubFunction;
    u8 ucSuppressPositiveResponseMsgInd;
    ucRet = MCURET_OK;
    ucSubFunction = pdata[1] & (~Bit_SuppressPositiveResponseMsgInd);
    ucSuppressPositiveResponseMsgInd = pdata[1] & Bit_SuppressPositiveResponseMsgInd;
    //NRC 11   12  13

	    //incorrectMessageLengthOrInvalidFormat
    if(len < 2)
    {
        CAN_Al_ErrSendNRC (0x13);//NRC
        return ucRet;
    }
	
    //subfuntionNotSupported
    if((ucSubFunction != 1)&&(ucSubFunction != 2) && (ucSubFunction != 3))
    {
        CAN_Al_ErrSendNRC (0x12);//NRC
        return ucRet;
    }
    //incorrectMessageLengthOrInvalidFormat
    if(len != 2)
    {
        CAN_Al_ErrSendNRC (0x13);//NRC
        return ucRet;
    }
#if 1 //openby nongyubao 2016/12/3
    //default to program
    if(AL_DEFAULT_SESSION==AlPROx->DsTypes)
    {
        if(AL_PROGRAMMING_SESSION==ucSubFunction)
        {
            CAN_Al_ErrSendNRC (0x7e);//NRC
            AlPROx->TimerStart=false;
            return ucRet;
        }
    }
    //program to extend
    else if(AL_PROGRAMMING_SESSION==AlPROx->DsTypes)
    {
        if(AL_EXTEND_DIAGNOSTIC_SESSION == ucSubFunction)
        {
            CAN_Al_ErrSendNRC (0x7e);//NRC
            AlPROx->TimerStart=false;
            return ucRet;
        }
    }
#endif
	// NRC 0x22  The ECU internal conditions do not allow the change of diagnostic Session.
	// 1) The system is currently in initialization process.

    //func  process
    AlPROx->DsTypesNext = (Al_Diagnostic_Session_ETypeDef)ucSubFunction;
    //check suppressPosRspMsgIndicationBit
    if(ucSuppressPositiveResponseMsgInd != 0)
    {
        ucRet = MCURET_OK;
        return ucRet;
    }


	if(AL_PROGRAMMING_SESSION == AlPROx->DsTypesNext)
	{
		if(NlPROx->RCtrl.N_PDU.Mtype == NL_FunctionalAddr)
	    {
	    	CAN_Al_ErrSendNRC (0x12);//NRC
	        return ucRet;
	    }
		
		ucRet = MCURET_OK;
		
		#define APP_NEED_CAN_UPDATAS	0x5b5b5b5b
		WriteUpDateFlag(APP_NEED_CAN_UPDATAS);
		
		CAN_Al_ErrSendNRC (0x78);//NRC
		CAN_Sid_ResetEcu (0x01);
		
        return ucRet;
	}
		
    //msg is right send answer
    S_AL_ReportMsg[0] = pdata[0] + 0x40;
    S_AL_ReportMsg[1] = pdata[1];
    S_AL_ReportMsg[2]=  GetU16HighU8(AL_P2sever_MAX_TIME);
    S_AL_ReportMsg[3]=  GetU16LowU8(AL_P2sever_MAX_TIME);
    S_AL_ReportMsg[4]=  GetU16HighU8(AL_P2_xsever_MAX_TIME);
    S_AL_ReportMsg[5]=  GetU16LowU8(AL_P2_xsever_MAX_TIME);
    ucRet=CAN_AL_TxDataRequest(NlPROx,&S_AL_ReportMsg[0],6);
    AlPROx->AlTxSid = pdata[0];
    AlPROx->AlTxReason =0;
    return ucRet;
}
/*
* SID 0x11 process
*/
u8 CAN_Al_EcuReset_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len)
{
    u8 ucRet;
    u8 ucSubFunction;
    //u8 ucSuppressPositiveResponseMsgInd;
    ucRet = MCURET_OK;
    //ucSubFunction = pdata[1] & (~Bit_SuppressPositiveResponseMsgInd);
    ucSubFunction = pdata[1] ;
    //ucSuppressPositiveResponseMsgInd = pdata[1] & Bit_SuppressPositiveResponseMsgInd;
    //NRC 11    13  12 22
	if(len < 2)
    {
        CAN_Al_ErrSendNRC (0x13);//NRC
        return ucRet;
    }

    //subfuntionNotSupported
    if((ucSubFunction != 1) && (ucSubFunction != 3))
    {
        CAN_Al_ErrSendNRC (0x12);//NRC
        return ucRet;
    }
    //incorrectMessageLengthOrInvalidFormat
    if(len != 2)
    {
        CAN_Al_ErrSendNRC (0x13);//NRC
        return ucRet;
    }
#if 0
    //incorrectMessageLengthOrInvalidFormat-- length  less
    if(len<2)
    {
        CAN_Al_ErrSendNRC (0x13);//NRC
        //AlPROx->Security.Status    = AL_SECURITY_LOCK;
        return ucRet;
    }
    //subfuntionNotSupported
    if((ucSubFunction<1)||(ucSubFunction>3))
    {
        CAN_Al_ErrSendNRC (0x12);//NRC
        //AlPROx->Security.Status    = AL_SECURITY_LOCK;
        return ucRet;
    }
    //incorrectMessageLengthOrInvalidFormat
    if(len != 2)
    {
        CAN_Al_ErrSendNRC (0x13);//NRC
        //AlPROx->Security.Status    = AL_SECURITY_LOCK;
        return ucRet;
    }
#endif // DIAG_UDS_NORMAL

    //NRC 0x22  The criteria for the ECUReset request are not met

    //func  process
    CAN_Sid_ResetEcu (ucSubFunction);//Dly  xx ms  reset
    //复位
    AlPROx->DsTypes   =  AL_DEFAULT_SESSION;
    AlPROx->DsTypesNext = AL_DEFAULT_SESSION;
    //AlPROx->DsTypesTimeOver = AL_NULL_SESSION;
    AlPROx->Security.Status    = AL_SECURITY_LOCK;

    //check suppressPosRspMsgIndicationBit   ??????
    //if(ucSuppressPositiveResponseMsgInd !=0)
    //{
    //    ucRet = MCURET_OK;
    //    return ucRet;
    //}
    //msg is right send answer
    S_AL_ReportMsg[0] = pdata[0]+0x40;
    S_AL_ReportMsg[1] = pdata[1];
    ucRet=CAN_AL_TxDataRequest(NlPROx,&S_AL_ReportMsg[0],2);
    AlPROx->AlTxSid = pdata[0];
    AlPROx->AlTxReason =0;
    return ucRet;
}
static u32 CAN_AL_GetSecuritySeed(void)
{
    u32 timenow = 0;
	u32 temp=0;
	u8 i;
    timenow = CAN_Al_GetDefaultTimerCount();
    srand(timenow);
	for(i=0;i<3;i++) // 3 max 3 times
	{
		temp= rand();
		if((temp!=0)&&(temp!=0xffffffff)) //seed not to 0 or 0xffffffff
		{
			break;
		}
		else
		{
		 if(i==2) temp= 0x12345678;//force to value
		}
	}
    return temp;
}
////////////////////////////////////////////////////////

static u8 CAN_Al_ChkSecurityKey(void)
{
    u8 i;
    u32 uiEcuKey = 0u;
    u32  uitemp=0;
    u32 uiSeed2  = S_uiSeed;
    for( i = 0u; i < 16u; i++ )
    {
        if( (((u32)0x01u)&(uiSeed2 >> i)) != (((u32)0x01u)&(uiSeed2 >> (31u-i))) )
        {
            if( ((u32)0x01u)&(uiSeed2>>i) )
            {
                uiSeed2 &= (~((u32)1u << i));
                uiSeed2 |= ((u32)1u << (31u-i));
            }
            else
            {
                uiSeed2 &= (~((u32)1u << (31u-i)));
                uiSeed2 |= ((u32)1u << i);
            }
        }
    }
    uiEcuKey = (S_uiSeed^0x1275f334)+(uiSeed2^0x1275f334);
    // uitemp=CYZ_FourU8ToU32( S_ucTesterKey[0], S_ucTesterKey[1], S_ucTesterKey[2], S_ucTesterKey[3]);
    uitemp=FourU8ToU32( S_ucTesterKey[3], S_ucTesterKey[2], S_ucTesterKey[1], S_ucTesterKey[0]);

#ifdef DEBUG_PRINT_DETAIL
    DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"CanDiag  RX uckey: %8x --123---SubKey %8x .\r\n",uitemp,uiEcuKey);
 #endif
    if(uitemp == uiEcuKey )
    {
        return MCURET_OK;
    }

    return MCURET_ERR;
}
/*
* SID 0x27 process
*/
u8 CAN_Al_SecurityAccess_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len)
{
    u8 ucRet;
    u8 ucSubFunction;
    //u8 ucSuppressPositiveResponseMsgInd;
    ucRet = MCURET_OK;
    //ucSubFunction = pdata[1] &(~Bit_SuppressPositiveResponseMsgInd);
    // ucSuppressPositiveResponseMsgInd=pdata[1] & Bit_SuppressPositiveResponseMsgInd;
    ucSubFunction = pdata[1] ;
    //NRC 11  7f  12 13 24 37 36 35

    //AlPROx->Security.Status    = AL_SECURITY_LOCK;
    //serviceNotSupported  (Functional Communication)
    if(NL_FunctionalAddr == NlPROx->RCtrl.N_PDU.N_TAtype)
    {
        CAN_Al_ErrSendNRC (0x11);//NRC
        return ucRet;
    }

    //serviceNotSupportedInActiveSession
    if(AL_DEFAULT_SESSION==AlPROx->DsTypes)
    {
        CAN_Al_ErrSendNRC (0x7f);//NRC
        return ucRet;
    }

	//incorrectMessageLengthOrInvalidFormat-- length  less
    if(len<2)
    {
        CAN_Al_ErrSendNRC (0x13);//NRC
        return ucRet;
    }
    //subfuntionNotSupported //11 12 not support
    if((ucSubFunction < 1) || (ucSubFunction > 2))
    {
        CAN_Al_ErrSendNRC (0x12);//NRC
        return ucRet;
    }
	#if 0
    //incorrectMessageLengthOrInvalidFormat-- length  less
    if(len<2)
    {
        CAN_Al_ErrSendNRC (0x13);//NRC
        //AlPROx->Security.Status    = AL_SECURITY_LOCK;
        return ucRet;
    }
	#endif
    if(((ucSubFunction==1)&&(len!=2) )||((ucSubFunction==2) &&(len !=6)) ) //长安密钥等级0 为4字节
    {
        CAN_Al_ErrSendNRC (0x13);//NRC
        return ucRet;
    }


    if(1==ucSubFunction)
    {
        //check suppressPosRspMsgIndicationBit
        //    if(ucSuppressPositiveResponseMsgInd !=0)//????????
        //   {
        //        ucRet=MCURET_OK;
        //        return ucRet;
        //   }
		if(0 != AlPROx->Security.TimerStart)
		{
			if(AlPROx->Security.Timer > 0) //NRC 0x37
			{
				CAN_Al_ErrSendNRC (0x37); //NRC
				return ucRet;
			}
		}
        AlPROx->Security.RequestSeed = true;
        S_AL_ReportMsg[0] = pdata[0]+0x40;
        S_AL_ReportMsg[1] = pdata[1];
        S_uiSeed =CAN_AL_GetSecuritySeed();//CAN_Al_GetDefaultTimerCount();
        
        // security unlock   request  seed =0
        if(AL_SECURITY_UNLOCK == AlPROx->Security.Status)
        {
            S_uiSeed=0;
			AlPROx->Security.RequestSeed = false;
        }

        S_AL_ReportMsg[2u] = (u8)(S_uiSeed>>24u);
        S_AL_ReportMsg[3u] = (u8)(S_uiSeed>>16u);
        S_AL_ReportMsg[4u] = (u8)(S_uiSeed>>8u);
        S_AL_ReportMsg[5u] = (u8)(S_uiSeed>>0u);
        ucRet=CAN_AL_TxDataRequest(NlPROx,&S_AL_ReportMsg[0],6);
        AlPROx->AlTxSid = pdata[0];
        AlPROx->AlTxReason =0;
		#if 0  ///for test get key
		CAN_Al_ChkSecurityKey();
		#endif
        return    ucRet;
    }
    else// if(2==ucSubFunction)
    {
        if(0==AlPROx->Security.RequestSeed)//NRC 0x24
        {
            CAN_Al_ErrSendNRC (0x24);//NRC
            AlPROx->Security.Status    = AL_SECURITY_LOCK;
            return ucRet;
        }
        if(0!=AlPROx->Security.TimerStart)
        {
            if(AlPROx->Security.Timer>0)//NRC 0x37
            {
                CAN_Al_ErrSendNRC (0x37);//NRC
                return ucRet;
            }
        }
	#if 0	// liubo move it to down 20161218
        AlPROx->Security.AttemptsNumber++;
        if( AlPROx->Security.AttemptsNumber > 10u )//NRC 0x36
        {
            CAN_Al_ErrSendNRC (0x36);//NRC
            AlPROx->Security.Status    = AL_SECURITY_LOCK;
            return ucRet;
        }
	#endif

        S_ucTesterKey[3] = pdata[2];
        S_ucTesterKey[2] = pdata[3];
        S_ucTesterKey[1] = pdata[4];
        S_ucTesterKey[0] = pdata[5];
        if(MCURET_OK == CAN_Al_ChkSecurityKey())//ok
        {
            AlPROx->Security.AttemptsNumber=0;
            AlPROx->Security.Status    = AL_SECURITY_UNLOCK;
			//check suppressPosRspMsgIndicationBit
			//if(ucSuppressPositiveResponseMsgInd !=0)//????????
			//{
				//ucRet=MCURET_OK;
				//return ucRet;
			//}
            S_AL_ReportMsg[0u] =pdata[0]+0x40;
            S_AL_ReportMsg[1u] = pdata[1];
            ucRet=CAN_AL_TxDataRequest(NlPROx,&S_AL_ReportMsg[0],2);
            AlPROx->AlTxSid = pdata[0];
            AlPROx->AlTxReason =0;
            return ucRet;
        }
        else
        {
			AlPROx->Security.AttemptsNumber++;
			if( AlPROx->Security.AttemptsNumber > 10u )//NRC 0x36
			{
				CAN_Al_ErrSendNRC (0x36);//NRC
				AlPROx->Security.Status    = AL_SECURITY_LOCK;
				return ucRet;
			}		
            if(AlPROx->Security.AttemptsNumber < 3)//NRC 0x35
            {
                AlPROx->Security.Status    = AL_SECURITY_LOCK;
                CAN_Al_ErrSendNRC (0x35);//NRC
                return ucRet;
            }
            if(AlPROx->Security.AttemptsNumber >= 3)
            {
                //start 10S timer
                if(false == AlPROx->Security.TimerStart)
                {
                    AlPROx->Security.TimerStart = true;
                    AlPROx->Security.Timer=(10000/AL_BASICTIME);
                }
            }
        }
    }
    return ucRet;

}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static u8 CAN_Al_CommunicationControl_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len)
{
    u8 ucRet;
    u8 ucSubFunction;
    u8 ucSuppressPositiveResponseMsgInd;
    ucRet = MCURET_OK;
    ucSubFunction = pdata[1] & (~Bit_SuppressPositiveResponseMsgInd);
    ucSuppressPositiveResponseMsgInd = pdata[1] & Bit_SuppressPositiveResponseMsgInd;
    //NRC
    //serviceNotSupportedInActiveSession
    if(AL_DEFAULT_SESSION==AlPROx->DsTypes)
    {
        CAN_Al_ErrSendNRC (0x7f);//NRC
        return ucRet;
    }
#if 0
    //serviceNotSupportedInActiveSession
    if(AL_PROGRAMMING_SESSION==AlPROx->DsTypes)
    {
        CAN_Al_ErrSendNRC (0x7f);//NRC
        return ucRet;
    }
#endif 
    if(len < 3)
    {
        CAN_Al_ErrSendNRC (0x13);//NRC
        return ucRet;
    }

    //subfuntionNotSupported
    if((ucSubFunction!=0)&&(ucSubFunction!=3))
    {
        CAN_Al_ErrSendNRC (0x12);//NRC
        return ucRet;
    }
    //incorrectMessageLengthOrInvalidFormat
    if(len != 3)
    {
        CAN_Al_ErrSendNRC (0x13);//NRC
        return ucRet;
    }
    // requestOutOfRange
    if((0 == pdata[2] ) ||(pdata[2] > 3)) //NRC 0x31
    {
        CAN_Al_ErrSendNRC (0x31);//NRC
        return ucRet;
    }

    //func  process
    AlPROx->CCCtrlType=(Al_CCCtrl_ETypeDef)ucSubFunction;
    AlPROx->CCComType=(Al_CCCom_ETypeDef)pdata[2];

    CAN_Sid_SetIdAppAndNMCtrlType (ucSubFunction, pdata[2]); //
    //check suppressPosRspMsgIndicationBit
    if(ucSuppressPositiveResponseMsgInd !=0)
    {
        ucRet = MCURET_OK;
        return ucRet;
    }
    //msg is right send answer
    S_AL_ReportMsg[0] = pdata[0]+0x40;
    S_AL_ReportMsg[1] = pdata[1];
    ucRet=CAN_AL_TxDataRequest(NlPROx,&S_AL_ReportMsg[0],2);
    AlPROx->AlTxSid = pdata[0];
    AlPROx->AlTxReason =0;
    return ucRet;

}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
u8 CAN_Al_SID_TesterPresent_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len)
{
    u8 ucRet;
    u8 ucSubFunction;
    u8 ucSuppressPositiveResponseMsgInd;
    ucRet = MCURET_OK;
    ucSubFunction = pdata[1] & (~Bit_SuppressPositiveResponseMsgInd);
    ucSuppressPositiveResponseMsgInd = pdata[1] & Bit_SuppressPositiveResponseMsgInd;
    //NRC

	    //incorrectMessageLengthOrInvalidFormat
    if(len < 2)
    {
        CAN_Al_ErrSendNRC (0x13);//NRC
        return ucRet;
    }
    //subfuntionNotSupported
    if(ucSubFunction > 0)
    {
        CAN_Al_ErrSendNRC (0x12);//NRC
        return ucRet;
    }

    //incorrectMessageLengthOrInvalidFormat
    if(len != 2)
    {
        CAN_Al_ErrSendNRC (0x13);//NRC
        return ucRet;
    }
#if 0
    if(AL_DEFAULT_SESSION == AlPROx->DsTypes)
    {
        if(AlPROx->TPS3OverFlag==FALSE)
        {
            CAN_Al_ErrSendNRC (0x7f);//NRC
        }
        return ucRet;
    }
#endif
    //func  process timer reset??
    AlPROx->Timer = AL_P2_xsever_MAX;
    //check suppressPosRspMsgIndicationBit
    if(ucSuppressPositiveResponseMsgInd !=0)
    {
        ucRet = MCURET_OK;
        return ucRet;
    }
#if 0
    //other  no return  特意这样写直? ?  ?  后跟甲方? ? ?下再?
    ucRet = MCURET_OK;
    return ucRet;

    // a functionally addressed request under valid conditions No response expected
    if(NL_FunctionalAddr == NlPROx->RCtrl.NPDU.N_TAtype)
    {
        ucRet = MCURET_OK;
        return ucRet;
    }
    if(AL_PROGRAMMING_SESSION == AlPROx->DsTypes)
    {
        ucRet = MCURET_OK;
        return ucRet;
    }
    if(AL_EXTEND_DIAGNOSTIC_SESSION == AlPROx->DsTypes)
    {
        ucRet = MCURET_OK;
        return ucRet;
    }
#endif

    S_AL_ReportMsg[0u] = pdata[0]+0x40;
    S_AL_ReportMsg[1u] =pdata[1];
    ucRet=CAN_AL_TxDataRequest(NlPROx,&S_AL_ReportMsg[0],2);
    AlPROx->AlTxSid = pdata[0];
    AlPROx->AlTxReason =0;
    return ucRet;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
u8 CAN_Al_SID_ControlDTCSetting_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len)
{
    u8 ucRet;
    u8 ucSubFunction;
    u8 ucSuppressPositiveResponseMsgInd;
    ucRet = MCURET_OK;
    ucSubFunction = pdata[1] & (~Bit_SuppressPositiveResponseMsgInd);
    ucSuppressPositiveResponseMsgInd = pdata[1] & Bit_SuppressPositiveResponseMsgInd;
    //NRC
    //serviceNotSupportedInActiveSession
    if(AL_DEFAULT_SESSION==AlPROx->DsTypes)
    {
        CAN_Al_ErrSendNRC (0x7f);//NRC
        return ucRet;
    }
#if 0
    if(AL_PROGRAMMING_SESSION == AlPROx->DsTypes)
    {
        CAN_Al_ErrSendNRC (0x7f);//NRC
        return ucRet;
    }
#endif
    //subfuntionNotSupported
  /*  if(ucSubFunction > 0x7f)
    {
        CAN_Al_ErrSendNRC (0x12);//NRC
        return ucRet;
    }*/
    //incorrectMessageLengthOrInvalidFormat-- length  less
    if(len<2)
    {
        CAN_Al_ErrSendNRC (0x13);//NRC
        return ucRet;
    }
    //conditionsNotCorrect
    if((ucSubFunction < 1) || (ucSubFunction > 2))
    {
        CAN_Al_ErrSendNRC (0x12);//NRC
        return ucRet;
    }
    //incorrectMessageLengthOrInvalidFormat
    if(((ucSubFunction==1)||(ucSubFunction==2))&&(len != 2))
    {
        CAN_Al_ErrSendNRC (0x13);//NRC
        return ucRet;
    }
	#if 0
//incorrectMessageLengthOrInvalidFormat-- length  less
    if(len<2)
    {
        CAN_Al_ErrSendNRC (0x13);//NRC
        return ucRet;
    }
	#endif
    if(ucSubFunction == 0x01 )
    {
		AlPROx->DTCSetType = AL_ControlDTCSetting_ON;

    }
    else
    {
		AlPROx->DTCSetType = AL_ControlDTCSetting_OFF;
    }
    //check suppressPosRspMsgIndicationBit
    if(ucSuppressPositiveResponseMsgInd !=0)
    {
        ucRet = MCURET_OK;
        return ucRet;
    }
    //msg is right send answer
    S_AL_ReportMsg[0u] = pdata[0]+0x40;
    S_AL_ReportMsg[1u] =pdata[1];
    ucRet=CAN_AL_TxDataRequest(NlPROx,&S_AL_ReportMsg[0],2);
    AlPROx->AlTxSid = pdata[0];
    AlPROx->AlTxReason =0;
    return ucRet;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
u8 CAN_Al_SID_ReadDataByIdentifier_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len)
{
    u16  i,usDidNum;
    u16 rIndex,wIndex;
    u8 ucRet;
    //u8 ucSubFunction;
    //u8 ucSuppressPositiveResponseMsgInd;
    ucRet = MCURET_OK;
    //ucSubFunction = pdata[1] &(~Bit_SuppressPositiveResponseMsgInd);
    //ucSubFunction = pdata[1] ;
    //ucSubFunction=ucSubFunction;
    //ucSuppressPositiveResponseMsgInd=pdata[1] & Bit_SuppressPositiveResponseMsgInd;
    //NRC
    #if 0
	//serviceNotSupportedInActiveSession
    if(AL_PROGRAMMING_SESSION==AlPROx->DsTypes)
    {
        CAN_Al_ErrSendNRC (0x7f);//NRC
        return ucRet;
    }
	#endif

	//incorrectMessageLengthOrInvalidFormat
    if(len < 3)
    {
        CAN_Al_ErrSendNRC (0x13);//NRC
        return ucRet;
    }
    //requestOutOfRange
    usDidNum = (len >> 1);
#if 0
    if(usDidNum > 10)
    {
        CAN_Al_ErrSendNRC (0x31);//NRC
        return ucRet;
    }
#endif
#if 0

    //incorrectMessageLengthOrInvalidFormat
    if((len == 1) || ((len & 0x01) == 0))
    {
        CAN_Al_ErrSendNRC (0x13);//NRC
        return ucRet;
    }
#endif

    //func  process
    S_AL_ReportMsg[0u] = pdata[0]+0x40;
    rIndex=1;
    wIndex=1;
    for(i=0; i<usDidNum; i++)
    {
        //conditionsNotCorrect 0x22 The operating conditions of the server are not met to perform the required action.
        if(MCURET_ERR == CAN_Al_GetReadDataByIdentifier(pdata, rIndex, &S_AL_ReportMsg[wIndex], &wIndex))
        {
            CAN_Al_ErrSendNRC (0x31);//NRC
            return ucRet;
        }
        rIndex +=2;
    }

	//incorrectMessageLengthOrInvalidFormat
    if((len == 1) || ((len & 0x01) == 0))
    {
        CAN_Al_ErrSendNRC (0x13);//NRC
        return ucRet;
    }
    //conditionsNotCorrect 0x22 ?????
    ucRet=CAN_AL_TxDataRequest(NlPROx,&S_AL_ReportMsg[0],wIndex);
    AlPROx->AlTxSid = pdata[0];
    AlPROx->AlTxReason =0;
    return ucRet;

}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
u8 CAN_Al_ReadMemoryByAddress_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len)
{
    u16 wIndex;
    u8 tmpsizelen,tmpstartlen;
    u8 ucRet;
    ucRet = MCURET_OK;
    //NRC 11  13  31  22

    //incorrectMessageLengthOrInvalidFormat
    tmpstartlen =  pdata[1] &0x0f;
    tmpsizelen = (pdata[1]>>4)&0x0f;
    if(len != (u16)(tmpstartlen+tmpsizelen+2))
    {
        CAN_Al_ErrSendNRC (0x13);//NRC
        return ucRet;
    }

    S_AL_ReportMsg[0u] = pdata[0] + 0x40;
    wIndex = 1;
    if(MCURET_ERR == CAN_Sid_ReadMemoryByAddress(pdata, (u16)(tmpstartlen + tmpsizelen), &S_AL_ReportMsg[1], &wIndex))
    {
        CAN_Al_ErrSendNRC (0x31);//NRC
        return ucRet;
    }
    if((AL_DEFAULT_SESSION==AlPROx->DsTypes)||(AL_PROGRAMMING_SESSION==AlPROx->DsTypes))
    {
        CAN_Al_ErrSendNRC (0x7e);//NRC
        return ucRet;
    }
    if(AL_SECURITY_LOCK == AlPROx->Security.Status )//NRC 0x33
    {
        CAN_Al_ErrSendNRC (0x33);//NRC
        return ucRet;
    }
    //conditionsNotCorrect 0x22 ?????

    ucRet=CAN_AL_TxDataRequest(NlPROx,&S_AL_ReportMsg[0],wIndex);
    AlPROx->AlTxSid = pdata[0];
    AlPROx->AlTxReason =0;
    return ucRet;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
u8 CAN_Al_SID_WriteDataByIdentifier_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len)
{
    u8 ucRet;
    u8 ucReuslt;
    ucRet = MCURET_OK;
    //NRC
    //serviceNotSupported
    if(NlPROx->RCtrl.N_PDU.Mtype == NL_FunctionalAddr)
    {
        CAN_Al_ErrSendNRC (0x11);//NRC
        return ucRet;
    }
    //serviceNotSupportedInActiveSession
    if(AL_DEFAULT_SESSION == AlPROx->DsTypes||AL_PROGRAMMING_SESSION == AlPROx->DsTypes)
    {
        CAN_Al_ErrSendNRC (0x7F);//NRC
        return ucRet;
    }
	
#if 1
    //incorrectMessageLengthOrInvalidFormat-- length  less
    if(len<3)
    {
        CAN_Al_ErrSendNRC (0x13);//NRC
        return ucRet;
    }
#endif

    if(AL_SECURITY_LOCK == AlPROx->Security.Status )//NRC 0x33
    {
        CAN_Al_ErrSendNRC (0x33);//NRC
        return ucRet;
    }


    ucReuslt=CAN_Al_WriteDataByIdentifier(AlPROx,NlPROx,pdata,len);
    if(ucReuslt==0)
    {
        S_AL_ReportMsg[0u] = pdata[0]+0x40;
        S_AL_ReportMsg[1u] =pdata[1];
        S_AL_ReportMsg[2u] =pdata[2];
        ucRet=CAN_AL_TxDataRequest(NlPROx,&S_AL_ReportMsg[0],3);
        AlPROx->AlTxSid = pdata[0];
        AlPROx->AlTxReason =0;
        return ucRet;
    }
    else
    {
        CAN_Al_ErrSendNRC (ucReuslt);//NRC
        return ucRet;
    }

}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
u8 CAN_Al_SID_ClearDiagnosticInformation_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len)
{
    u32  DTCGroup;
    u8 ucRet;
    //u8 ucSubFunction;
    //u8 ucSuppressPositiveResponseMsgInd;
    ucRet = MCURET_OK;
    //ucSubFunction = pdata[1] &(~Bit_SuppressPositiveResponseMsgInd);
    //ucSubFunction = pdata[1] ;

    //ucSuppressPositiveResponseMsgInd=pdata[1] & Bit_SuppressPositiveResponseMsgInd;

    //NRC

    //serviceNotSupportedInActiveSession
    if(AL_PROGRAMMING_SESSION == AlPROx->DsTypes)
    {
        CAN_Al_ErrSendNRC (0x7F);//NRC
        return ucRet;
    }

#if 1
    //incorrectMessageLengthOrInvalidFormat-- length  less
    if(len<4)
    {
        CAN_Al_ErrSendNRC (0x13);//NRC
        return ucRet;
    }
#endif
#if 0

    //incorrectMessageLengthOrInvalidFormat
    if(len != 4)
    {
        CAN_Al_ErrSendNRC (0x13);//NRC
        return ucRet;
    }
#endif
//NRX 0x22 ???

    DTCGroup= FourU8ToU32(0,pdata[1],pdata[2],pdata[3]);
    if(DTCGroup<0xd00000)
    {
        CAN_Al_ErrSendNRC (0x31);//NRC
        return ucRet;
    }


    if(MCURET_ERR == CAN_Sid_ClearDtcInfo (DTCGroup))
    {
        CAN_Al_ErrSendNRC (0x31);//NRC
        return ucRet;
    }

	//incorrectMessageLengthOrInvalidFormat
    if(len != 4)
    {
        CAN_Al_ErrSendNRC (0x13);//NRC
        return ucRet;
    }
    //msg is right send answer
    S_AL_ReportMsg[0u] = pdata[0]+0x40;
    ucRet=CAN_AL_TxDataRequest(NlPROx,&S_AL_ReportMsg[0],1);
    AlPROx->AlTxSid = pdata[0];
    AlPROx->AlTxReason =0;
    return ucRet;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
u8 CAN_Al_SID_ReadDTCInformation_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len)
{
    u16 wtIndex;
    u8 ucRet;
    u8 ucSubFunction;
    //u8 ucSuppressPositiveResponseMsgInd;
    ucRet = MCURET_OK;
    ucSubFunction = pdata[1] &(~Bit_SuppressPositiveResponseMsgInd);
    //ucSubFunction = pdata[1] ;
    //ucSuppressPositiveResponseMsgInd=pdata[1] & Bit_SuppressPositiveResponseMsgInd;
    //NRC

    //serviceNotSupportedInActiveSession
    /*if(AL_DEFAULT_SESSION == AlPROx->DsTypes)
	{
	CAN_Al_ErrSendNRC (0x7F);//NRC
	return ucRet;
	}*/

	//incorrectMessageLengthOrInvalidFormat
	if(len < 2)
	{
		CAN_Al_ErrSendNRC (0x13);//NRC
		return ucRet;
	}

    //subfuntionNotSupported
    if(MCURET_ERR == CAN_Sid_ReadDtcInfoChecksubfuncNotSupported(ucSubFunction))
    {
        CAN_Al_ErrSendNRC (0x12);//NRC
        return ucRet;
    }
#if 0
    //incorrectMessageLengthOrInvalidFormat
    if(len < 2)
    {
        CAN_Al_ErrSendNRC (0x13);//NRC
        return ucRet;
    }
#endif
    if(MCURET_ERR == CAN_Sid_ReadDtcInfoCheckInvalidFormat(ucSubFunction, len))
    {
        CAN_Al_ErrSendNRC (0x13);//NRC
        return ucRet;
    }
    //check suppressPosRspMsgIndicationBit
    //if(ucSuppressPositiveResponseMsgInd !=0)
    //{
    //    ucRet=MCURET_OK;
    //    return ucRet;
    //}

    //wtIndex=0;


    S_AL_ReportMsg[0u] = pdata[0]+0x40;
    S_AL_ReportMsg[1u] =pdata[1];
    wtIndex=2;
    CAN_Sid_ReadDtcInfo(ucSubFunction,pdata,&S_AL_ReportMsg[2],&wtIndex);
    ucRet=CAN_AL_TxDataRequest(NlPROx,&S_AL_ReportMsg[0],wtIndex);
    AlPROx->AlTxSid = pdata[0];
    AlPROx->AlTxReason =0;

    return ucRet;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
u8 CAN_Al_InputOutputControlByIdentifier_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len)
{
    u16 wtIndex;
    u8 ucRet;
    u8 ucSubFunction;
	u8 uctemp;
  //  u8 temp;
    //u8 ucSuppressPositiveResponseMsgInd;
    ucRet = MCURET_OK;
    //ucSubFunction = pdata[1] &(~Bit_SuppressPositiveResponseMsgInd);
    ucSubFunction = pdata[1] ;
    ucSubFunction=ucSubFunction;

    //ucSuppressPositiveResponseMsgInd=pdata[1] & Bit_SuppressPositiveResponseMsgInd;

    //NRC  11 7f  33  31  13  12 22
    //serviceNotSupported
    if(NlPROx->RCtrl.N_PDU.Mtype == NL_FunctionalAddr)
    {
        CAN_Al_ErrSendNRC (0x11);//NRC
        return ucRet;
    }
    //serviceNotSupportedInActiveSession
    if((AL_DEFAULT_SESSION==AlPROx->DsTypes)||(AL_PROGRAMMING_SESSION==AlPROx->DsTypes))
    {
        CAN_Al_ErrSendNRC (0x7f);//NRC
        return ucRet;
    }
    //securityAccessDenied
    if(AL_SECURITY_LOCK == AlPROx->Security.Status )//NRC 0x33
    {
        CAN_Al_ErrSendNRC (0x33);//NRC
        return ucRet;
    }
#if 0

    //subfuntionNotSupported
    if((ucSubFunction != 0) && (ucSubFunction != 3))
    {
        CAN_Al_ErrSendNRC (0x12);//NRC
        return ucRet;
    }

    //incorrectMessageLengthOrInvalidFormat-- length  less
    if(len<4)
    {
        CAN_Al_ErrSendNRC (0x13);//NRC
        return ucRet;
    }
    //incorrectMessageLengthOrInvalidFormat
    //if(((len -3) & 0x01 )!=0 )
    //{
    //    CAN_Al_ErrSendNRC (0x13);//NRC
    //    return ucRet;
    // }
#endif

    wtIndex=3;

	uctemp = CAN_Sid_InputOutputControlByIdentifier (pdata,len,&S_AL_ReportMsg[3],&wtIndex);//
    if(uctemp!=0)
    {
        CAN_Al_ErrSendNRC (uctemp);//NRC
        return ucRet;
    }

    //msg is right send answer
    S_AL_ReportMsg[0u] = pdata[0]+0x40;
    S_AL_ReportMsg[1u] =pdata[1];
    S_AL_ReportMsg[2u] =pdata[2];
    ucRet=CAN_AL_TxDataRequest(NlPROx,&S_AL_ReportMsg[0],wtIndex);
    AlPROx->AlTxSid = pdata[0];
    AlPROx->AlTxReason =0;
    return ucRet;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
u8 CAN_Al_SID_RoutineControl_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len)
{
    u16 wtIndex;
    //u8 ucSubFunction;
    //u8 ucSuppressPositiveResponseMsgInd;
    u8 ucRet;
    u8 ucResult;
    ucRet = MCURET_OK;
    //ucSubFunction = pdata[1] &(~Bit_SuppressPositiveResponseMsgInd);
    //ucSuppressPositiveResponseMsgInd=pdata[1] & Bit_SuppressPositiveResponseMsgInd;
    //NRC 11 7f  33 31 13 12 22
    //serviceNotSupported
    if(NlPROx->RCtrl.N_PDU.Mtype == NL_FunctionalAddr)
    {
        CAN_Al_ErrSendNRC (0x11);//NRC
        return ucRet;
    }
    //serviceNotSupportedInActiveSession
    if(AL_DEFAULT_SESSION==AlPROx->DsTypes)
    {
        CAN_Al_ErrSendNRC (0x7f);//NRC
        return ucRet;
    }
    //securityAccessDenied
    if(AL_SECURITY_LOCK == AlPROx->Security.Status )//NRC 0x33
    {
        CAN_Al_ErrSendNRC (0x33);//NRC
        return ucRet;
    }
    //incorrectMessageLengthOrInvalidFormat-- length  less
    if(len<4)
    {
        CAN_Al_ErrSendNRC (0x13);//NRC
        return ucRet;
    }
    wtIndex=4;
    ucResult =CAN_SID_RoutineControl(AlPROx,NlPROx,pdata,len,&S_AL_ReportMsg[wtIndex],&wtIndex);
    if(0!=ucResult)
    {
        CAN_Al_ErrSendNRC (ucResult);//NRC
        return MCURET_ERR;
    }
    S_AL_ReportMsg[0u] = pdata[0]+0x40;
    S_AL_ReportMsg[1u] =pdata[1];
    S_AL_ReportMsg[2u] =pdata[2];
    S_AL_ReportMsg[3u] =pdata[3];
    ucRet=CAN_AL_TxDataRequest(NlPROx,&S_AL_ReportMsg[0],wtIndex);
    AlPROx->AlTxSid = pdata[0];
    AlPROx->AlTxReason = 0;
    return ucRet;
    //return MCURET_OK;
}
/////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
u8 CAN_Al_RequestDownload_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len)
{
    u16  templen;
    u16 wtIndex;
    //u8 ucSubFunction;
    //u8 ucSuppressPositiveResponseMsgInd;
    u8 ucRet;
    ucRet = MCURET_OK;
    //ucSubFunction = pdata[1] &(~Bit_SuppressPositiveResponseMsgInd);
    //ucSuppressPositiveResponseMsgInd=pdata[1] & Bit_SuppressPositiveResponseMsgInd;
    //NRC 11 7f  33 31 13 12 22
    //serviceNotSupportedInActiveSession
    if(AL_DEFAULT_SESSION==AlPROx->DsTypes)
    {
        CAN_Al_ErrSendNRC (0x7f);//NRC
        return ucRet;
    }
    //securityAccessDenied
    if(AL_SECURITY_LOCK == AlPROx->Security.Status )//NRC 0x33
    {
        CAN_Al_ErrSendNRC (0x33);//NRC
        return ucRet;
    }

    templen = 3+  ((pdata[2] >>4)&0x0f)+ (pdata[2]&0x0f);
    if(templen != len)
    {
        CAN_Al_ErrSendNRC (0x13);//NRC
        return ucRet;
    }

    wtIndex = 2;
    CAN_Sid_RequestDownload(pdata, len, &S_AL_ReportMsg[wtIndex], &wtIndex); //Dly  xx ms  reset
    if(NL_FunctionalAddr == NlPROx->RCtrl.N_PDU.N_TAtype)
    {
        CAN_Al_ErrSendNRC (0x22);//NRC
        return ucRet;
    }
    S_AL_ReportMsg[0u] = pdata[0]+0x40;
    S_AL_ReportMsg[1u] =pdata[1];
    ucRet=CAN_AL_TxDataRequest(NlPROx,&S_AL_ReportMsg[0],wtIndex);
    AlPROx->AlTxSid = pdata[0];
    AlPROx->AlTxReason =0;
    return ucRet;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
u8 CAN_Al_TransferData_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len)
{
    u16 wtIndex;
    //u8 ucSubFunction;
    //u8 ucSuppressPositiveResponseMsgInd;
    u8 ucRet;
    ucRet = MCURET_OK;
    //ucSubFunction = pdata[1] &(~Bit_SuppressPositiveResponseMsgInd);
    //ucSuppressPositiveResponseMsgInd=pdata[1] & Bit_SuppressPositiveResponseMsgInd;
    //NRC 11 7f  33 31 13 12 22
    //serviceNotSupportedInActiveSession
    if(AL_DEFAULT_SESSION==AlPROx->DsTypes)
    {
        CAN_Al_ErrSendNRC (0x7f);//NRC
        return ucRet;
    }
    //securityAccessDenied
    if(AL_SECURITY_LOCK == AlPROx->Security.Status )//NRC 0x33
    {
        CAN_Al_ErrSendNRC (0x33);//NRC
        return ucRet;
    }

    //NRC  7f  33  31  13  22 22
    wtIndex = 2;
    CAN_Sid_TransferData(pdata, len, &S_AL_ReportMsg[wtIndex], &wtIndex); //Dly  xx ms  reset
    if(NL_FunctionalAddr == NlPROx->RCtrl.N_PDU.N_TAtype)
    {
        CAN_Al_ErrSendNRC (0x22);//NRC
        return ucRet;
    }

    S_AL_ReportMsg[0u] = pdata[0]+0x40;
    S_AL_ReportMsg[1u] =pdata[1];
    ucRet=CAN_AL_TxDataRequest(NlPROx,&S_AL_ReportMsg[0],wtIndex);
    AlPROx->AlTxSid = pdata[0];
    AlPROx->AlTxReason =0;
    return ucRet;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
u8 CAN_Al_RequestTransferExit_process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len)
{
    u16 wtIndex;
    //u8 ucSubFunction;
    //u8 ucSuppressPositiveResponseMsgInd;
    u8 ucRet;
    ucRet = MCURET_OK;
    //ucSubFunction = pdata[1] &(~Bit_SuppressPositiveResponseMsgInd);
    //ucSuppressPositiveResponseMsgInd=pdata[1] & Bit_SuppressPositiveResponseMsgInd;
    //NRC 11 7f  33 31 13 12 22
    //serviceNotSupportedInActiveSession
    if(AL_DEFAULT_SESSION==AlPROx->DsTypes)
    {
        CAN_Al_ErrSendNRC(0x7f);//NRC
        return ucRet;
    }
    //securityAccessDenied
    if(AL_SECURITY_LOCK == AlPROx->Security.Status )//NRC 0x33
    {
        CAN_Al_ErrSendNRC(0x33);//NRC
        return ucRet;
    }

    //NRC  7f  33  31  13  22 22
    wtIndex=2;
    CAN_Sid_RequestTransferExit(pdata,len,&S_AL_ReportMsg[wtIndex],&wtIndex); //Dly  xx ms  reset

    if(NL_FunctionalAddr == NlPROx->RCtrl.N_PDU.N_TAtype)
    {
        CAN_Al_ErrSendNRC (0x22);//NRC
        return ucRet;
    }
    S_AL_ReportMsg[0u] = pdata[0]+0x40;
    S_AL_ReportMsg[1u] =pdata[1];
    ucRet=CAN_AL_TxDataRequest(NlPROx,&S_AL_ReportMsg[0],wtIndex);
    AlPROx->AlTxSid = pdata[0];
    AlPROx->AlTxReason =0;
    return ucRet;
}
/** 
* @Func CAN_Al_Ack0x11_Process
*
* @brief AL Ack NRC0x11
*
* @note 
*
* @param  Al_Ctrl_STypeDef* AlPROx:	AL struct pointer
*		NL_Process_STypeDef *NlPROx :  struct pointer
*		u8* pdata: pointer of the receive data
*		u16 len:
*
* @return NULL
*/
static u8 CAN_Al_Ack0x11_Process(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len)
{
    u8 ucRet;
    CAN_Al_ErrSendNRC (0x11);
    return ucRet;
}
/** 
* @Func CAN_AL_TxDataRequest
*
* @brief AL Send data
*
* @note 
*
* @param  NL_Process_STypeDef *NlPROx :  struct pointer
*             u8* pData: pointer of the received data
*		u16 DataLen:
*
* @return NULL
*/
u8  CAN_AL_TxDataRequest(NL_Process_STypeDef* NlPROx,u8* pData, u16 DataLen )
{
#ifdef DEBUG_PRINT_DETAIL
    u16 i;
#endif // DEBUG_PRINT_DETAIL

    //access by functional addr
    if(NlPROx->RCtrl.N_PDU.N_TAtype == NL_FunctionalAddr)
    {
        if(pData[0]==0x7f)
        {
            if((pData[2]==0x11)||(pData[2]==0x12)||pData[2]==0x31)
            {
                //not return err
                return MCURET_OK;
            }
        }
    }
#ifdef DEBUG_PRINT_DETAIL
    if(pData[0]!=0x7e)
    {
        DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"CanDiag : TX A_PDU: ");
        for(i=0; i<DataLen; i++)
        {
            DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"%02x  ",pData[i]);
        }
        DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n");
    }
#endif // DEBUG_PRINT_DETAIL

    return CAN_Nl_AlPDUDataRequest(NlPROx, pData,DataLen);
}

/******************************************************************************
**                            End Of File
******************************************************************************/


