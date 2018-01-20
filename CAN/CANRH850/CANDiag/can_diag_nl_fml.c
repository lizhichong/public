/** 
* Copyright (C), CHINATSP,www.chinatsp.com.  All rights reserved.
*
* @file CANAppApp.c
* 
* @Description Diagnostics on Controller Area Networks, Network Layer, ISO 15765-2
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
#include "Can_config.h"
#include "Debug.h"

/************************************************
*			constant								*
************************************************/

/************************************************
*			internal macro							*
************************************************/
//Choose one below to debug
//#define  TRACE_NL  TRACE_BOOT  /*Uart debug Print information*/
#define  TRACE_NL     /*Uart debug Not Print information*/

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
//
#define NL_PCI_SF               (0x00)// single frame
#define NL_PCI_FF               (0x10)// first frame
#define NL_PCI_CF               (0x20)// consecutive frame
#define NL_PCI_FC               (0x30)// flow control
//FC define
#define NL_FC_CLR_TO_SEND           (0x00)
#define NL_FC_ST_WAIT               (0X01)
#define NL_FC_ST_OVERFLOW           (0X02)

/************************************************
*			internal data type						*
************************************************/
typedef enum
{
    NL_RESULT_OK = 0,
    NL_RESULT_TIMEOUT_A,
    NL_RESULT_TIMEOUT_Bs,
    NL_RESULT_TIMEOUT_Cr,
    NL_RESULT_WRONG_SN,
    NL_RESULT_INVALID_FS,
    NL_RESULT_UNEXP_PDU,
    NL_RESULT_WFT_OVRN,
    NL_RESULT_BUFFER_OVFLW,
    NL_RESULT_ERROR,
} Nl_Result_ETypeDef;
/************************************************
*			static global variable					*
************************************************/
/************************************************
*			Local function declare					*
************************************************/
static void CAN_Nl_ReceiverInit(NL_Process_STypeDef *NlPROx, Nl_Result_ETypeDef reason);
static void CAN_Nl_SenderInit(NL_Process_STypeDef *NlPROx, Nl_Result_ETypeDef reason);
static u8  CAN_Nl_TxLDataRequest(NL_Process_STypeDef *NlPROx);
static void CAN_Nl_TxMsgPrepare(NL_Process_STypeDef *NlPROx, u8 *ptxbuff, u8 validlen);
static void CAN_Nl_ReceiverMTxFCFrame(NL_Process_STypeDef *NlPROx);
static void CAN_Nl_SenderMTxSFFFFrame(NL_Process_STypeDef *NlPROx);
static void CAN_Nl_SenderMTxCFFrame(NL_Process_STypeDef *NlPROx);
static void CAN_Nl_ReceiverTxOKProcessing( NL_Process_STypeDef *NlPROx);// reciver  txok i
static void CAN_Nl_SenderTxOKProcessing( NL_Process_STypeDef *NlPROx);//sender  txok i
static u8 CAN_Nl_TxSingleFramePrepare(NL_Process_STypeDef *NlPROx, u8 *pNdata, u16 datalen);
static u8 CAN_Nl_TxFirstFramePrepare(NL_Process_STypeDef *NlPROx, u16 datalen);
static u8 CAN_Nl_TxConsecutiveFramePrepare(NL_Process_STypeDef *NlPROx);
static u8 CAN_Nl_TxFlowControlPrepare(NL_Process_STypeDef *NlPROx);
static void CAN_NL_ReceiverTimerManage(NL_Process_STypeDef *NlPROx);
static void  CAN_NL_SenderTimerManage(NL_Process_STypeDef *NlPROx);
static void CAN_Nl_RxPCISFProcess(NL_Process_STypeDef *NlPROx, NL_MSG_SType *rxmsg);
static void CAN_Nl_RxPCIFFProcess(NL_Process_STypeDef *NlPROx, NL_MSG_SType *rxmsg);
static void CAN_Nl_RxPCICFProcess(NL_Process_STypeDef *NlPROx, NL_MSG_SType *rxmsg);
static void CAN_Nl_RxPCIFCProcess(NL_Process_STypeDef *NlPROx, NL_MSG_SType *rxmsg);
/************************************************
*			Global function						*
************************************************/
/** 
* @Func CAN_Nl_Initx
*
* @brief Initializes  the variable of the CAN Network layer
*         according to the specified parameters in the ParaInit.
* @note 
*
* @param  NL_Process_STypeDef *NlPROx :  struct pointer
*             CAN_Nl_Init_STypeDef ParaInit:  struct pointer
*             u8 *pRbuff:  the pointer of  receiver buff
*             u8 *pSbuff:  the pointer of sender buff
* @return NULL
*/
void CAN_Nl_Initx(NL_Process_STypeDef *NlPROx, CAN_Nl_Init_STypeDef *ParaInit, u8 *pRbuff, u8 *pSBuff)
{
    //default para config
    NlPROx->ParaDefault.RxNPDUidPhys = ParaInit->RxNPDUidPhys;
    NlPROx->ParaDefault.RxNPDUidFunc = ParaInit->RxNPDUidFunc;
    NlPROx->ParaDefault.TxNPDUidDiag = ParaInit->TxNPDUidDiag;
    NlPROx->ParaDefault.N_ArTimer = ParaInit->N_ArTimer;
    NlPROx->ParaDefault.N_BrTimer = ParaInit->N_BrTimer;
    NlPROx->ParaDefault.N_CrTimer = ParaInit->N_CrTimer;
    NlPROx->ParaDefault.N_AsTimer = ParaInit->N_AsTimer;
    NlPROx->ParaDefault.N_BsTimer = ParaInit->N_BsTimer;
    NlPROx->ParaDefault.N_CsTimer = ParaInit->N_CsTimer;
    NlPROx->ParaDefault.BsSet = ParaInit->BsSet;
    NlPROx->ParaDefault.STminSet = ParaInit->STminSet;
    //Buff config
    CAN_Nl_ReceiverInit(NlPROx, NL_RESULT_OK);
    CAN_Nl_SenderInit(NlPROx, NL_RESULT_OK);
	NlPROx->RCtrl.N_PDU.pdata = NULL_PTR; // lizhichong for 指针异常
	NlPROx->SCtrl.N_PDU.pdata = NULL_PTR; // lizhichong for 指针异常
	NlPROx->RCtrl.N_PDU.pdata = pRbuff;
    NlPROx->SCtrl.N_PDU.pdata = pSBuff;
}

/** 
* @Func CAN_Nl_RxProcess
*
* @brief Rx diag msg process

* @note 
*
* @param   NL_Process_STypeDef *NlPROx :  struct pointer
*             NL_MSG_SType *rxmsg: pointer of the received CAN diag message
* @return NULL
*/
void CAN_Nl_RxProcess(NL_Process_STypeDef *NlPROx, NL_MSG_SType *rxmsg)
{
    /*An ECU shall always set the DLC equal to eight (8) for the CAN-frames
    when transmitting ISO15765-2 transport protocol messages.*/
    if(rxmsg->length != 8)
    {
        CAN_Al_NlUSData_con(NlPROx, NL_IND_AL_RX_UNEXP_PDU) ;
        return;//ignore
    }

    if(rxmsg->id == NlPROx->ParaDefault.RxNPDUidFunc)
    {
        NlPROx->RCtrl.N_PDU.N_TAtype = NL_FunctionalAddr;
    }
    else
    {
        NlPROx->RCtrl.N_PDU.N_TAtype = NL_PhysicalAddr;
    }
    // test
    // TRACE_BOOT("Liubo: Nl Sender Status is %d\r\n ",NlPROx->SCtrl.Status);

    switch( rxmsg->data[0] & 0xf0u )
    {
        case NL_PCI_SF:
            // TRACE_BOOT("Liubo: Nl Rx SF. \r\n");
            CAN_Nl_RxPCISFProcess(NlPROx, rxmsg);
            break;
        case NL_PCI_FF:
            // TRACE_BOOT("Liubo: Nl Rx FF. \r\n");
            CAN_Nl_RxPCIFFProcess(NlPROx, rxmsg);
            break;
        case NL_PCI_CF:
            // TRACE_BOOT("Liubo: Nl Rx CF. \r\n");
            CAN_Nl_RxPCICFProcess(NlPROx, rxmsg);
            break;
        case NL_PCI_FC:
            // TRACE_BOOT("Liubo: Nl Rx FC. \r\n");
            CAN_Nl_RxPCIFCProcess(NlPROx, rxmsg);
            break;
        default:
            CAN_Al_NlUSData_con(NlPROx, NL_IND_AL_RX_UNEXP_PDU) ;
            break;
    }
}
/** 
* @Func CAN_Nl_AlPDUDataRequest
*
* @brief Al request Nl  tx datas

* @note called by AL to send data
*
* @param   NL_Process_STypeDef *NlPROx :  struct pointer
*             u8 *pApdu: pointer of the tx buff
*             u16 DataLen:
* @return NULL
*/
u8 CAN_Nl_AlPDUDataRequest(NL_Process_STypeDef *NlPROx, u8 *pApdu, u16 DataLen)
{
    u16 i;
    u8 ucRet = MCURET_OK;

    if((DataLen >= NL_RXBUFFMAXSIZE) || (0 == DataLen)) { return MCURET_ERR; }

    if(NL_S_ST_Idle != NlPROx->SCtrl.Status) { return MCURET_ERR; }

	//CAN_Nl_SenderInit(NlPROx, NL_RESULT_OK);   // 指针不能为NULL
    if(NlPROx->SCtrl.N_PDU.pdata == NULL_PTR)
    {
  //      TRACE_BOOT("\r\n%s :BUFFER is NULL\r\n", __func__);
		ucRet = MCURET_ERR;
    }
	else
    {
	    if(DataLen < 8) //tx single frame
	    {
	        ucRet = CAN_Nl_TxSingleFramePrepare(NlPROx, pApdu, DataLen);
	    }
	    else//tx  muliti frame
	    {
	        for(i = 0; i < DataLen; i++)
	        {
	            NlPROx->SCtrl.N_PDU.pdata[i] = pApdu[i];
	        }
	        NlPROx->SCtrl.WriteIndex = DataLen;
	        ucRet = CAN_Nl_TxFirstFramePrepare(NlPROx, DataLen);
	    }
    }
    return ucRet;
}
/** 
* @Func CAN_Nl_Timer_Process
*
* @brief CAN NL time manage

* @note call by Diag timer Process
*
* @param   NL_Process_STypeDef *NlPROx :  struct pointer
*
* @return NULL
*/
void CAN_Nl_Timer_Process(NL_Process_STypeDef *NlPROx)
{
    CAN_NL_SenderTimerManage(NlPROx);
    CAN_NL_ReceiverTimerManage(NlPROx);
}
/** 
* @Func CAN_Nl_TxOk
*
* @brief CAN NL Tx finished .

* @note called when Diag frame send OK
*
* @param   NL_Process_STypeDef *NlPROx :  struct pointer
*
* @return NULL
*/
void CAN_Nl_TxOk(NL_Process_STypeDef *NlPROx)//can msg tx finish isr/task
{
    CAN_Nl_SenderTxOKProcessing(NlPROx);
    CAN_Nl_ReceiverTxOKProcessing(NlPROx);
}
void CAN_Nl_TxFailed(NL_Process_STypeDef *NlPROx)//
{

}
void CAN_Nl_CancelTx(NL_Process_STypeDef *NlPROx)//
{

}
/************************************************
*			Local function							*
************************************************/
/** 
* @Func CAN_Nl_ReceiverInit
*
* @brief Initializes  the  Receiver side variable of the CAN Network layer .
*
* @note 
*
* @param  NL_Process_STypeDef *NlPROx :  struct pointer
*             Nl_Result_ETypeDef reason: (reserved)
*
* @return NULL
*/
static void CAN_Nl_ReceiverInit(NL_Process_STypeDef *NlPROx, Nl_Result_ETypeDef reason)
{
    Nl_gIrqDis();
    NlPROx->RCtrl.Status = NL_R_ST_Idle;
    NlPROx->RCtrl.Timer = 0;
    NlPROx->RCtrl.TxFailedTimer = 0;
    NlPROx->RCtrl.TxStatus = NL_Tx_ST_Idle;
    NlPROx->RCtrl.NeedTx = 0;
    NlPROx->RCtrl.Sn = 0;
    NlPROx->RCtrl.Fs = NL_FC_CLR_TO_SEND;
    NlPROx->RCtrl.BsCnt = NlPROx->ParaDefault.BsSet;
    NlPROx->RCtrl.WftCnt = NL_WFT_MAX;
    NlPROx->RCtrl.STmin = NlPROx->ParaDefault.STminSet*NL_BASICTIME;
    NlPROx->RCtrl.ReadIndex = 0;
    NlPROx->RCtrl.WriteIndex = 0;
    NlPROx->RCtrl.N_PDU.Mtype = 0;
    NlPROx->RCtrl.N_PDU.N_SA = 0;
    NlPROx->RCtrl.N_PDU.N_TA = 0;
    NlPROx->RCtrl.N_PDU.N_TAtype = NL_PhysicalAddr;
    NlPROx->RCtrl.N_PDU.N_AE = 0;
    NlPROx->RCtrl.N_PDU.Datalen = 0;
    //NlPROx->RCtrl.N_PDU.pdata = NULL_PTR; // lizhichong for 指针异常
    Nl_gIrqEn();
    DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"CanDiag : CAN Nl Receiver reinit. %d \r\n", reason);

}
/** 
* @Func CAN_Nl_SenderInit
*
* @brief Initializes  the  Sender side variable of the CAN Network layer .
*
* @note 
*
* @param  NL_Process_STypeDef *NlPROx :  struct pointer
*             Nl_Result_ETypeDef reason: (reserved)
*
* @return NULL
*/
static void CAN_Nl_SenderInit(NL_Process_STypeDef *NlPROx, Nl_Result_ETypeDef reason)
{
    Nl_gIrqDis();
    NlPROx->SCtrl.Status = NL_S_ST_Idle;
    NlPROx->SCtrl.Timer = 0u;
    NlPROx->SCtrl.TxFailedTimer = 0u;
    NlPROx->SCtrl.TxStatus = NL_Tx_ST_Idle;
    NlPROx->SCtrl.NeedTx = 0;
    NlPROx->SCtrl.NoBsFlag = 0;
    NlPROx->SCtrl.Sn = 0U;
    NlPROx->SCtrl.Fs = NL_FC_CLR_TO_SEND;
    NlPROx->SCtrl.BsCnt = NlPROx->ParaDefault.BsSet;
    NlPROx->SCtrl.WftCnt = NL_WFT_MAX;
    NlPROx->SCtrl.STmin = NlPROx->ParaDefault.STminSet;
    NlPROx->SCtrl.ReadIndex = 0;
    NlPROx->SCtrl.WriteIndex = 0;
    NlPROx->SCtrl.N_PDU.Mtype = 0;
    NlPROx->SCtrl.N_PDU.N_SA = 0;
    NlPROx->SCtrl.N_PDU.N_TA = 0;
    NlPROx->SCtrl.N_PDU.N_TAtype = NL_PhysicalAddr;
    NlPROx->SCtrl.N_PDU.N_AE = 0;
    NlPROx->SCtrl.N_PDU.Datalen = 0;
    //NlPROx->SCtrl.N_PDU.pdata = NULL_PTR; // lizhichong for 指针异常
    Nl_gIrqEn();
    //TRACE_NL("\r\n##CYZ## : M3 Nl Sender reinit. %d \r\n", reason);
}

/** 
* @Func CAN_Nl_TxLDataRequest
*
* @brief Send CAN frame data to queue
*
* @note 
*
* @param  NL_Process_STypeDef *NlPROx :  struct pointer
*
* @return NULL
*/
static u8  CAN_Nl_TxLDataRequest(NL_Process_STypeDef *NlPROx)
{
    Can_ReturnType ucRet;
    //ucRet = can_send_frame(&NlPROx->LData_txmsg);
	ucRet = CAN_TxMsgQueue(&NlPROx->LData_txmsg);
    if( CAN_BUSY == ucRet )
    {
        return MCURET_ERR;
    }
    return MCURET_OK;
}
/** 
* @Func CAN_Nl_TxMsgPrepare
*
* @brief Prepare CAN Msg Data
*
* @note 
*
* @param  NL_Process_STypeDef *NlPROx :  struct pointer
*             u8 *ptxbuff: pointer of the tx buff
*             u8 validlen:must <= 8
*
* @return NULL
*/
static void CAN_Nl_TxMsgPrepare(NL_Process_STypeDef *NlPROx, u8 *ptxbuff, u8 validlen)
{
    u8 i;
    //correct
    if(validlen > 8)
    {
        validlen = 8;
        //TRACE_NL("\r\n##CYZ## : CAN Nl tx err. %d \r\n");
    }
    NlPROx->LData_txmsg.id = NlPROx->ParaDefault.TxNPDUidDiag;
    NlPROx->LData_txmsg.length = 8;
    for(i = 0; i < 8; i++)
    {
        NlPROx->LData_txmsg.data[i] = NL_UNUSEDBYTE;//
    }
    for(i = 0; i < validlen; i++)
    {
        NlPROx->LData_txmsg.data[i] = *ptxbuff;
        ptxbuff++;
    }
}
/** 
* @Func CAN_Nl_ReceiverMTxFCFrame
*
* @brief change  the  transmit state of FC Frame
*
* @note 
*
* @param  NL_Process_STypeDef *NlPROx :  struct pointer
*
* @return NULL
*/
static void CAN_Nl_ReceiverMTxFCFrame(NL_Process_STypeDef *NlPROx)
{
    u8 ucRet;
    if(1 == NlPROx->RCtrl.NeedTx)
    {
        ucRet = CAN_Nl_TxLDataRequest(NlPROx);
        if(0 == ucRet)
        {
            NlPROx->RCtrl.Timer = NlPROx->ParaDefault.N_ArTimer;
            NlPROx->RCtrl.TxStatus = NL_Tx_ST_TxingWaitIsr;
            NlPROx->RCtrl.NeedTx = 0;
        }
        else
        {
            NlPROx->RCtrl.NeedTx = 1;
            NlPROx->RCtrl.TxStatus = NL_Tx_ST_Txing;
        }
        //change state
        NlPROx->RCtrl.Status = NL_R_ST_TxFC;
    }
}
/** 
* @Func CAN_Nl_SenderMTxSFFFFrame
*
* @briefchange  the  transmit state of SF/FF Frame
*
* @note 
*
* @param  NL_Process_STypeDef *NlPROx :  struct pointer
*
* @return NULL
*/
static void CAN_Nl_SenderMTxSFFFFrame(NL_Process_STypeDef *NlPROx)
{
    u8 ucRet;
    if(1 == NlPROx->SCtrl.NeedTx)
    {
        ucRet = CAN_Nl_TxLDataRequest(NlPROx);
        if(MCURET_OK == ucRet)
        {
            NlPROx->SCtrl.Timer = NlPROx->ParaDefault.N_AsTimer;
            NlPROx->SCtrl.TxStatus = NL_Tx_ST_TxingWaitIsr;
            NlPROx->SCtrl.NeedTx = 0;
        }
        else
        {
            NlPROx->SCtrl.NeedTx = 1;
            NlPROx->SCtrl.TxStatus = NL_Tx_ST_Txing;
        }
        //change state
        if(NL_S_ST_WaitTxFF == NlPROx->SCtrl.Status)  { NlPROx->SCtrl.Status = NL_S_ST_TxFF; }
        else  { NlPROx->SCtrl.Status = NL_S_ST_TxSF; }
    }
}
/** 
* @Func CAN_Nl_SenderMTxCFFrame
*
* @brief change  the  transmit state of CF Frame
*
* @note 
*
* @param  NL_Process_STypeDef *NlPROx :  struct pointer
*
* @return NULL
*/
static void CAN_Nl_SenderMTxCFFrame(NL_Process_STypeDef *NlPROx)
{
    u8 ucRet;
    ucRet = CAN_Nl_TxLDataRequest(NlPROx);
    if(0 == ucRet)
    {
        NlPROx->SCtrl.Timer = NlPROx->ParaDefault.N_AsTimer;
        NlPROx->SCtrl.TxStatus = NL_Tx_ST_TxingWaitIsr;
        NlPROx->SCtrl.NeedTx = 0;
    }
    else
    {
        NlPROx->SCtrl.NeedTx = 1;
        NlPROx->SCtrl.TxStatus = NL_Tx_ST_Txing;
    }
    //change state
    if(NL_S_ST_WaitTxCFOver == NlPROx->SCtrl.Status)     { NlPROx->SCtrl.Status = NL_S_ST_TxCFOver; }
    else   { NlPROx->SCtrl.Status = NL_S_ST_TxCF; }
}
/** 
* @Func CAN_Nl_ReceiverTxOKProcessing
*
* @brief Receiver transmit msg ok ISR or task interface
*
* @note 
*
* @param  NL_Process_STypeDef *NlPROx :  struct pointer
*
* @return NULL
*/
static void CAN_Nl_ReceiverTxOKProcessing( NL_Process_STypeDef *NlPROx)// reciver  txok i
{
    if(NL_Tx_ST_TxingWaitIsr == NlPROx->RCtrl.TxStatus)
    {
        NlPROx->RCtrl.TxStatus = NL_Tx_ST_Idle;
    }
    else
    {
        if(NL_Tx_ST_Idle != NlPROx->RCtrl.TxStatus)
        {
            //TRACE_NL("\r\n##CYZ## : Nl Receiver Tx Ok Processing  Error %d. \r\n ", NlPROx->RCtrl.TxStatus);
        }
        return ;
    }

    switch( NlPROx->RCtrl.Status )
    {
        case NL_R_ST_TxFC://wait  FC  TX OVER  isr
            if( NL_FC_ST_WAIT == NlPROx->RCtrl.Fs )
            {
                if(NlPROx->RCtrl.WftCnt > 0)
                {
                    NlPROx->RCtrl.WftCnt--;
                    NlPROx->RCtrl.Timer = NlPROx->ParaDefault.N_BrTimer;
                    NlPROx->RCtrl.Status = NL_R_ST_WaitTxFC;
                }
                else
                {
                    CAN_Nl_ReceiverInit(NlPROx, NL_RESULT_WFT_OVRN);
                }
            }
            else
            {
                NlPROx->RCtrl.WftCnt = NL_WFT_MAX;
                NlPROx->RCtrl.Timer = NlPROx->ParaDefault.N_CrTimer;
                NlPROx->RCtrl.Status = NL_R_ST_WaitRxCF;
            }
            break;
        default:
            return;
    }
}
/** 
* @Func CAN_Nl_SenderTxOKProcessing
*
* @brief Sender transmit msg ok ISR or task interface
*
* @note 
*
* @param  NL_Process_STypeDef *NlPROx :  struct pointer
*
* @return NULL
*/
static void CAN_Nl_SenderTxOKProcessing( NL_Process_STypeDef *NlPROx)//sender  txok i
{
    if(NL_Tx_ST_TxingWaitIsr == NlPROx->SCtrl.TxStatus)
    {
        NlPROx->SCtrl.TxStatus = NL_Tx_ST_Idle;
    }
    else
    {
        if(NL_Tx_ST_Idle != NlPROx->RCtrl.TxStatus)
        {
            //TRACE_NL("\r\n##CYZ## : Nl Sender Tx Ok Processing  Error. %d\r\n ", NlPROx->SCtrl.TxStatus);
        }
        return ;
    }

    switch( NlPROx->SCtrl.Status )
    {
        case NL_S_ST_TxSF://wait  SF  TX OVER  isr
            CAN_Al_NlUSData_con( NlPROx, NL_ACK_AL_SENDSUCESS );
            NlPROx->SCtrl.Timer = 0;
            NlPROx->SCtrl.Status = NL_S_ST_Idle;
            //NlPROx->SCtrl.TxFailedTimer = NL_TX_MIN_TIMER;
            break;
        case NL_S_ST_TxFF:
            NlPROx->SCtrl.Timer = NlPROx->ParaDefault.N_BsTimer;
            NlPROx->SCtrl.Status = NL_S_ST_WaitRxFC;
            break;
        case NL_S_ST_TxCF:
            if(NlPROx->SCtrl.NoBsFlag == 0)
            {
                if( NlPROx->SCtrl.BsCnt > 0) { NlPROx->SCtrl.BsCnt--; }
                if( 0u == NlPROx->SCtrl.BsCnt )
                {
                    NlPROx->SCtrl.Timer = NlPROx->ParaDefault.N_BsTimer;    //N_Bs  L_Data.confirm (FF/CF)-->L_Data.indication (FC)
                    NlPROx->SCtrl.Status = NL_S_ST_WaitRxFC;
                }
                else
                {
                    NlPROx->SCtrl.Timer = NlPROx->ParaDefault.N_CsTimer;
                    NlPROx->SCtrl.Status = NL_S_ST_WaitTxCF;
                }
            }
            else
            {
                NlPROx->SCtrl.Timer = NlPROx->SCtrl.STmin;
                NlPROx->SCtrl.Status = NL_S_ST_WaitTxCF;
            }
            break;
        case  NL_S_ST_TxCFOver:
            CAN_Al_NlUSData_con( NlPROx, NL_ACK_AL_SENDSUCESS );
            NlPROx->SCtrl.Timer = 0;
            NlPROx->SCtrl.Status = NL_S_ST_Idle;
            //NlPROx->SCtrl.TxFailedTimer = NL_TX_MIN_TIMER;
            break;
        default:
            return;
    }
}
/** 
* @Func CAN_Nl_TxSingleFramePrepare
*
* @brief prepare transmit msg SF
*
* @note 
*
* @param  NL_Process_STypeDef *NlPROx :  struct pointer
*             u8 *pNdata: pointer of the tx buff
*             u16 datalen:must < 8
*
* @return NULL
*/
static u8 CAN_Nl_TxSingleFramePrepare(NL_Process_STypeDef *NlPROx, u8 *pNdata, u16 datalen)
{
    u16 i;

    if(datalen > 7)  { return MCURET_ERR; }

    NlPROx->TxLData[0] = NL_PCI_SF | datalen;
    for(i = 0; i < datalen; i++)
    {
        NlPROx->TxLData[1 + i] = *pNdata;
        pNdata++;
    }
    CAN_Nl_TxMsgPrepare(NlPROx, &NlPROx->TxLData[0] , datalen + 1);
    NlPROx->SCtrl.Timer = NlPROx->ParaDefault.N_CsTimer;
    NlPROx->SCtrl.Status = NL_S_ST_WaitTxSF;
    NlPROx->SCtrl.NeedTx = 1;
    CAN_Nl_SenderMTxSFFFFrame(NlPROx);//tx msg now
    //TRACE_NL("\r\n##CYZ## : Nl Tx SF. \r\n");
    return MCURET_OK;
}
/** 
* @Func CAN_Nl_TxFirstFramePrepare
*
* @brief prepare transmit msg FF
*
* @note 
*
* @param  NL_Process_STypeDef *NlPROx :  struct pointer
*             u16 datalen:must <  4096
*
* @return NULL
*/
static u8 CAN_Nl_TxFirstFramePrepare(NL_Process_STypeDef *NlPROx, u16 datalen)
{
    u8 i;

    if(datalen > 0x0fff)  { return MCURET_ERR; }

    NlPROx->SCtrl.ReadIndex = 0;
    NlPROx->TxLData[0] = NL_PCI_FF | (GetU16HighU8(datalen) & 0x0f);
    NlPROx->TxLData[1] = GetU16LowU8(datalen);
    for(i = 2; i < 8; i++)
    {
        NlPROx->TxLData[i] = NlPROx->SCtrl.N_PDU.pdata[NlPROx->SCtrl.ReadIndex];
        NlPROx->SCtrl.ReadIndex++;
    }
    CAN_Nl_TxMsgPrepare(NlPROx, &NlPROx->TxLData[0] , 8);
    NlPROx->SCtrl.Timer = NlPROx->ParaDefault.N_CsTimer;
    NlPROx->SCtrl.Status = NL_S_ST_WaitTxFF;
    NlPROx->SCtrl.NeedTx = 1;
    NlPROx->SCtrl.Sn = 1;
    CAN_Nl_SenderMTxSFFFFrame(NlPROx);
    //TRACE_NL("\r\n##CYZ## : Nl Tx FF. \r\n");
    return MCURET_OK;
}
/** 
* @Func CAN_Nl_TxConsecutiveFramePrepare
*
* @brief prepare transmit msg FC
*
* @note 
*
* @param  NL_Process_STypeDef *NlPROx :  struct pointer
*
* @return NULL
*/
static u8 CAN_Nl_TxConsecutiveFramePrepare(NL_Process_STypeDef *NlPROx)
{
    u8 i, j;
    u8 uctxover;

    //AutoCorrect Sn
    if(NlPROx->SCtrl.Sn > 15) { NlPROx->SCtrl.Sn = 0; }

    uctxover = 0;
    NlPROx->TxLData[0] = NL_PCI_CF | NlPROx->SCtrl.Sn;
    for(i = 1, j = 1; i < 8; i++, j++)
    {
        NlPROx->TxLData[i] = NlPROx->SCtrl.N_PDU.pdata[NlPROx->SCtrl.ReadIndex];
        NlPROx->SCtrl.ReadIndex++;
        if(NlPROx->SCtrl.ReadIndex >= NlPROx->SCtrl.WriteIndex)
        {
            //send over
            i = 8;
            uctxover = 1;
        }
    }
    CAN_Nl_TxMsgPrepare(NlPROx, &NlPROx->TxLData[0] , j);
    NlPROx->SCtrl.Timer = NlPROx->ParaDefault.N_CsTimer;
    NlPROx->SCtrl.Status = NL_S_ST_WaitTxCF;
    NlPROx->SCtrl.NeedTx = 1;
    //AutoCorrect Sn
    NlPROx->SCtrl.Sn++;
    if(NlPROx->SCtrl.Sn > 15) { NlPROx->SCtrl.Sn = 0; }
    if(1 == uctxover)
    {
        NlPROx->SCtrl.Status = NL_S_ST_WaitTxCFOver;
    }
    CAN_Nl_SenderMTxCFFrame(NlPROx);
    //TRACE_NL("\r\n##CYZ## : Nl Tx CF. \r\n");
    return MCURET_OK;
}
/** 
* @Func CAN_Nl_TxFlowControlPrepare
*
* @brief prepare transmit msg CF
*
* @note 
*
* @param  NL_Process_STypeDef *NlPROx :  struct pointer
*
* @return NULL
*/
static u8 CAN_Nl_TxFlowControlPrepare(NL_Process_STypeDef *NlPROx)
{
    if(NlPROx->RCtrl.Fs > 2) { return MCURET_ERR; }

    NlPROx->TxLData[0] = NL_PCI_FC | NlPROx->RCtrl.Fs;
    NlPROx->TxLData[1] = NlPROx->RCtrl.BsCnt;
    NlPROx->TxLData[2] = NlPROx->RCtrl.STmin;
    CAN_Nl_TxMsgPrepare(NlPROx, &NlPROx->TxLData[0] , 3);
    NlPROx->RCtrl.Timer = NlPROx->ParaDefault.N_BrTimer;
    NlPROx->RCtrl.Status = NL_R_ST_WaitTxFC;
    NlPROx->RCtrl.NeedTx = 1;
    CAN_Nl_ReceiverMTxFCFrame(NlPROx);
    //TRACE_NL("\r\n##CYZ## : Nl Tx FC. \r\n");
    return MCURET_OK;
}
/** 
* @Func CAN_NL_ReceiverTimerManage
*
* @brief Receiver time manage
*
* @note 
*
* @param  NL_Process_STypeDef *NlPROx :  struct pointer
*
* @return NULL
*/
static void CAN_NL_ReceiverTimerManage(NL_Process_STypeDef *NlPROx)
{
    u8 ucRet;

    Nl_gIrqDis();
    //    if(NlPROx->RCtrl.TxFailedTimer > 0) NlPROx->RCtrl.TxFailedTimer--;

    if(NlPROx->RCtrl.Timer > 1)
    {
        NlPROx->RCtrl.Timer--;
        if(NlPROx->RCtrl.TxStatus == NL_Tx_ST_Txing)
        {
            //         if((0 !=  NlPROx->RCtrl.NeedTx)&&(0 == NlPROx->RCtrl.TxFailedTimer))
            if(0 !=  NlPROx->RCtrl.NeedTx)
            {
                ucRet = CAN_Nl_TxLDataRequest(NlPROx);
                if(0 == ucRet)
                {
                    NlPROx->RCtrl.Timer = NlPROx->ParaDefault.N_ArTimer;
                    NlPROx->RCtrl.TxStatus = NL_Tx_ST_TxingWaitIsr;
                    NlPROx->RCtrl.NeedTx = 0; //Tx failed retx
                }
                else  { NlPROx->RCtrl.NeedTx = 1; } //Tx failed retx
            }
        }
    }
    else if(NlPROx->RCtrl.Timer == 1)//time over
    {
        NlPROx->RCtrl.Timer = 0;
        switch(NlPROx->RCtrl.Status )
        {
            case NL_R_ST_TxFC:
                CAN_Nl_ReceiverInit(NlPROx, NL_RESULT_TIMEOUT_A);
                break;
            case NL_R_ST_WaitRxCF:
                CAN_Nl_ReceiverInit(NlPROx, NL_RESULT_TIMEOUT_Cr);
                break;
            default:
                break;
        }
    }
    else;
    Nl_gIrqEn();
}
/** 
* @Func CAN_NL_SenderTimerManage
*
* @brief Sender time manage
*
* @note 
*
* @param  NL_Process_STypeDef *NlPROx :  struct pointer
*
* @return NULL
*/
static void  CAN_NL_SenderTimerManage(NL_Process_STypeDef *NlPROx)
{
    u8  ucRet;

    Nl_gIrqDis();
    //  if(NlPROx->SCtrl.TxFailedTimer > 0) NlPROx->SCtrl.TxFailedTimer--;

    if(NlPROx->SCtrl.Timer > 1)
    {
        NlPROx->SCtrl.Timer--;
        if(NlPROx->SCtrl.TxStatus == NL_Tx_ST_Txing)//if  need retx
        {
            //            if((0 !=  NlPROx->SCtrl.NeedTx)&&(0 == NlPROx->SCtrl.TxFailedTimer))
            if(0 !=  NlPROx->SCtrl.NeedTx)
            {
                ucRet = CAN_Nl_TxLDataRequest(NlPROx);
                if(0 == ucRet)
                {
                    NlPROx->SCtrl.Timer = NlPROx->ParaDefault.N_AsTimer;
                    NlPROx->SCtrl.TxStatus = NL_Tx_ST_TxingWaitIsr;
                    NlPROx->SCtrl.NeedTx = 0;
                }
                else { NlPROx->SCtrl.NeedTx = 1; }
            }
            else;
        }
    }
    else if(NlPROx->SCtrl.Timer == 1)
    {
        NlPROx->SCtrl.Timer = 0;
        switch(NlPROx->SCtrl.Status)
        {
        	case NL_S_ST_WaitRxFC:
				CAN_Nl_SenderInit( NlPROx,  NL_RESULT_TIMEOUT_Bs );
				break;
            case NL_S_ST_WaitTxCF:
            case NL_S_ST_WaitTxCFOver:
                CAN_Nl_TxConsecutiveFramePrepare(NlPROx);
                break;

            case NL_S_ST_TxSF:
            case NL_S_ST_TxFF:
            case NL_S_ST_TxCF:
            case NL_S_ST_TxCFOver:
                CAN_Nl_SenderInit( NlPROx,  NL_RESULT_TIMEOUT_A );
                CAN_Al_NlUSData_con( NlPROx, NL_ACK_AL_SENDFAILED);
                break;
            default:
                break;
        }
    }
    else;
    Nl_gIrqEn();
}
/** 
* @Func CAN_Nl_RxPCISFProcess
*
* @brief Receiver Rx SF process
*
* @note 
*
* @param  NL_Process_STypeDef *NlPROx :  struct pointer
*             NL_MSG_SType *rxmsg: pointer of the received CAN diag message
*
* @return NULL
*/
static void CAN_Nl_RxPCISFProcess(NL_Process_STypeDef *NlPROx, NL_MSG_SType *rxmsg)
{
    u8 temp_SF_DL, i;

    //Check SF_DL
    temp_SF_DL = rxmsg->data[0] & 0x0fu;
    /*sf DataLen err!!!!!
      If the network layer receives an SF with an SF_DL equal to zero (0), then the network layer shall ignore the received SF N_PDU.
      If the network layer receives an SF with an SF_DL greater than 7 when using normal addressing,
      or greater than 6 for extended or mixed addressing, then the network layer shall ignore the received SF N_PDU.*/
    if((0 == temp_SF_DL) || (temp_SF_DL > 7 )) { return; } //ignore rx N_PDU

#if NL_FULL_DUPLEX > 0
    if(NlPROx->SCtrl.Status != NL_S_ST_Idle)
    {
    }
    if(NlPROx->RCtrl.Status != NL_R_ST_Idle)
    {
        CAN_Nl_ReceiverInit(NlPROx, NL_RESULT_UNEXP_PDU); //not idle reset var
        CAN_Al_NlUSData_con(NlPROx, NL_IND_AL_RX_UNEXP_PDU) ;
    }
#else
    if(NlPROx->SCtrl.Status != NL_S_ST_Idle)
    {
        return;
    }
    else if(NlPROx->RCtrl.Status != NL_R_ST_Idle)
    {
        CAN_Nl_ReceiverInit(NlPROx, NL_RESULT_UNEXP_PDU); //not idle reset var
        CAN_Al_NlUSData_con(NlPROx, NL_IND_AL_RX_UNEXP_PDU);
    }
    else ;
#endif

    for(i = 0; i < temp_SF_DL; i++) //SAVE
    {
        NlPROx->RCtrl.N_PDU.pdata[i] = rxmsg->data[1 + i];
    }
    CAN_Al_NlReciveDataInd(NlPROx, NlPROx->RCtrl.N_PDU.pdata, (u16)temp_SF_DL); //N_USData.indication，网络层向应用层? ? ?的指示原?

    NlPROx->RCtrl.Status = NL_R_ST_Idle;
}
/** 
* @Func CAN_Nl_RxPCIFFProcess
*
* @brief Receiver Rx FF process
*
* @note 
*
* @param  NL_Process_STypeDef *NlPROx :  struct pointer
*             NL_MSG_SType *rxmsg: pointer of the received CAN diag message
*
* @return NULL
*/
static void CAN_Nl_RxPCIFFProcess(NL_Process_STypeDef *NlPROx, NL_MSG_SType *rxmsg)
{
    u16 temp_FF_DL, i;

	//Check Functinal address ,no accept Functinal address  ///add by nongyubao 2016.12.4
	if(NlPROx->RCtrl.N_PDU.N_TAtype == NL_FunctionalAddr){ return; } //ignore rx Functinal address msg
    //Check FF_DL
    temp_FF_DL = TwoU8ToU16(rxmsg->data[0] & 0x0fu, rxmsg->data[1]);
    if(temp_FF_DL < 7) { return; } //ignore rx N_PDU

#if NL_FULL_DUPLEX > 0
    if(NlPROx->SCtrl.Status != NL_S_ST_Idle)
    {
    }
    if(NlPROx->RCtrl.Status != NL_R_ST_Idle)
    {
        CAN_Nl_ReceiverInit(NlPROx, NL_RESULT_UNEXP_PDU); //not idle reset var
        CAN_Al_NlUSData_con(NlPROx, NL_IND_AL_RX_UNEXP_PDU) ;
    }
#else
    if(NlPROx->SCtrl.Status != NL_S_ST_Idle)
    {
        return;
    }
    else if(NlPROx->RCtrl.Status != NL_R_ST_Idle)
    {
        CAN_Nl_ReceiverInit(NlPROx, NL_RESULT_UNEXP_PDU); //not idle reset var
        CAN_Al_NlUSData_con(NlPROx, NL_IND_AL_RX_UNEXP_PDU) ;
    }
    else ;
#endif

    if(temp_FF_DL >= NL_RXBUFFMAXSIZE)
    {
        //FC = Overflow
        NlPROx->RCtrl.Fs = NL_FC_ST_OVERFLOW;
        NlPROx->RCtrl.BsCnt = NlPROx->ParaDefault.BsSet;
        NlPROx->RCtrl.STmin = NL_STMIN_DEFAULT_TIME;//NlPROx->ParaDefault.STminSet;

        CAN_Nl_TxFlowControlPrepare(NlPROx);
        CAN_Al_NlUSData_con(NlPROx, NL_IND_AL_RX_UNEXP_PDU) ;
        return;
    }

    NlPROx->RCtrl.WriteIndex = 0;
    if((void *)0  !=  NlPROx->RCtrl.N_PDU.pdata)
    {
        NlPROx->RCtrl.N_PDU.Datalen = (u16)temp_FF_DL;
        for(i = 0; i < 6; i++) //SAVE
        {
            NlPROx->RCtrl.N_PDU.pdata[NlPROx->RCtrl.WriteIndex] = rxmsg->data[2 + i];
            NlPROx->RCtrl.WriteIndex++;
        }
        //TX FC
        NlPROx->RCtrl.Fs = NL_FC_CLR_TO_SEND;
        NlPROx->RCtrl.BsCnt = NlPROx->ParaDefault.BsSet;
        NlPROx->RCtrl.STmin = NL_STMIN_DEFAULT_TIME;//NlPROx->ParaDefault.STminSet;

        NlPROx->RCtrl.Sn = 1;
        NlPROx->RCtrl.NeedTx = 1;
        CAN_Nl_TxFlowControlPrepare(NlPROx);
        CAN_Al_NlUSData_con(NlPROx, NL_IND_AL_RX_FF);
        return;
    }
    NlPROx->RCtrl.Status = NL_R_ST_Idle;
}
/** 
* @Func CAN_Nl_RxPCICFProcess
*
* @brief Receiver Rx CF process
*
* @note 
*
* @param  NL_Process_STypeDef *NlPROx :  struct pointer
*             NL_MSG_SType *rxmsg: pointer of the received CAN diag message
*
* @return NULL
*/
static void CAN_Nl_RxPCICFProcess(NL_Process_STypeDef *NlPROx, NL_MSG_SType *rxmsg)
{
    u8 i;

#ifdef DEBUG_PRINT_DETAIL
	DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"CanDiag : CAN_Nl_RxPCICFProcess NlPROx->RCtrl.Status=%d \r\n",NlPROx->RCtrl.Status);
#endif // DEBUG_PRINT_DETAIL

    if(NL_R_ST_WaitRxCF != NlPROx->RCtrl.Status)
    {
        return;
    }
#ifdef DEBUG_PRINT_DETAIL	
	DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"CanDiag :CAN_Nl_RxPCICFProcess  %d %d \r\n",(rxmsg->data[0] & 0x0f),NlPROx->RCtrl.Sn);
#endif // DEBUG_PRINT_DETAIL
	
    if((rxmsg->data[0] & 0x0f) == ( NlPROx->RCtrl.Sn)) //check SN
    {
        for(i = 0; i < 7; i++)
        {
            NlPROx->RCtrl.N_PDU.pdata[NlPROx->RCtrl.WriteIndex] = rxmsg->data[1 + i];
            NlPROx->RCtrl.WriteIndex++;
            if(NlPROx->RCtrl.WriteIndex >= NlPROx->RCtrl.N_PDU.Datalen) //data Rx Finished
            {
            #ifdef DEBUG_PRINT_DETAIL
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"CanDiag : RxPCICF OK %d %d \r\n",NlPROx->RCtrl.WriteIndex,NlPROx->RCtrl.N_PDU.Datalen);
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"CanDiag : RxPCICF Last Data=%d \r\n",NlPROx->RCtrl.N_PDU.pdata[NlPROx->RCtrl.N_PDU.Datalen-1]);
			#endif // DEBUG_PRINT_DETAIL
			#if 0
                NlPROx->RCtrl.Status = NL_R_ST_Idle;
                CAN_Al_NlReciveDataInd(NlPROx, NlPROx->RCtrl.N_PDU.pdata, NlPROx->RCtrl.N_PDU.Datalen); //N_USData.indication，网络层向应用层? ? ?的指示原?
                NlPROx->RCtrl.Sn = 0;
			#else
                CAN_Al_NlReciveDataInd(NlPROx, NlPROx->RCtrl.N_PDU.pdata, NlPROx->RCtrl.N_PDU.Datalen); //N_USData.indication，网络层向应用层? ? ?的指示原?
                CAN_Nl_ReceiverInit(NlPROx, NL_RESULT_OK);
			#endif
                return;
            }
        }

        // correct Sn
        NlPROx->RCtrl.Sn++;
        if(NlPROx->RCtrl.Sn > 15) { NlPROx->RCtrl.Sn = 0; } // 0--15

        if(NlPROx->RCtrl.BsCnt > 0)  { NlPROx->RCtrl.BsCnt--; }
	
#ifdef DEBUG_PRINT_DETAIL	
		DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"CanDiag : NlPROx->RCtrl.BsCnt=%d \r\n",NlPROx->RCtrl.BsCnt);
#endif // DEBUG_PRINT_DETAIL

        if((0 == NlPROx->RCtrl.BsCnt) && (0 != NlPROx->ParaDefault.BsSet))
        {
            // tx FC frame
            NlPROx->RCtrl.Fs = NL_FC_CLR_TO_SEND;
            NlPROx->RCtrl.BsCnt = NlPROx->ParaDefault.BsSet;
            NlPROx->RCtrl.STmin = NL_STMIN_DEFAULT_TIME; // NlPROx->ParaDefault.STminSet;

            CAN_Nl_TxFlowControlPrepare(NlPROx);
            return;
        }
		else
		{
			NlPROx->RCtrl.WftCnt = NL_WFT_MAX;
			NlPROx->RCtrl.Timer = NlPROx->ParaDefault.N_CrTimer;
			NlPROx->RCtrl.Status = NL_R_ST_WaitRxCF;
		}
    }
    else
    {
        CAN_Nl_ReceiverInit(NlPROx, NL_RESULT_WRONG_SN);
        CAN_Al_NlUSData_con(NlPROx, NL_IND_AL_RX_UNEXP_PDU) ;
    }
    //NlPROx->RCtrl.Status = NL_R_ST_Idle;
}
/** 
* @Func CAN_Nl_RxPCIFCProcess
*
* @brief Sender Rx CF process
*
* @note 
*
* @param  NL_Process_STypeDef *NlPROx :  struct pointer
*             NL_MSG_SType *rxmsg: pointer of the received CAN diag message
*
* @return NULL
*/
static void CAN_Nl_RxPCIFCProcess(NL_Process_STypeDef *NlPROx, NL_MSG_SType *rxmsg)
{
    u8 temp_FC_FS;
    u8 tempdat;
    if(NL_S_ST_WaitRxFC != NlPROx->SCtrl.Status)
    {
        return;
    }
	
	if(NlPROx->RCtrl.N_PDU.N_TAtype == NL_FunctionalAddr)
	{ 
		CAN_Nl_SenderInit( NlPROx,  NL_RESULT_UNEXP_PDU );
		return; ///add by nongyubao2016.12.04

	} //ignore rx Functinal address msg
	
    temp_FC_FS = rxmsg->data[0] & 0x0f;
    if(0 == temp_FC_FS)//ContinueToSend
    {
        NlPROx->SCtrl.BsCnt = rxmsg->data[1];
        tempdat = rxmsg->data[2] / NL_BASICTIME;
        if(tempdat == 0) { tempdat = NL_N_WAITLITTLETIME_MAX; }
        NlPROx->SCtrl.STmin = tempdat+1;///add 1  corret 10ms
        if(0 == NlPROx->SCtrl.BsCnt)
        {
            /*The sending network layer entity shall send all remaining consecutive frames
            without any stop for further FC frames from the receiving network layer entity.*/
            NlPROx->SCtrl.NoBsFlag = 1;
        }
        else
        {
            NlPROx->SCtrl.NoBsFlag = 0;
        }
        //TX msg
        NlPROx->SCtrl.Timer = NlPROx->SCtrl.STmin;
        NlPROx->SCtrl.Status = NL_S_ST_WaitTxCF;
        //CAN_Nl_TxConsecutiveFramePrepare(NlPROx);///deleted by nongyubao wait stmin to send
    }
    else if(1 == temp_FC_FS)//Wait next FC Frame
    {
        NlPROx->SCtrl.Timer = NL_N_Bs_MAX;
        if(NlPROx->SCtrl.WftCnt == 0)
        {
            CAN_Nl_SenderInit(NlPROx, NL_RESULT_WFT_OVRN);
        }
        else
        {
            NlPROx->SCtrl.WftCnt--;
            NlPROx->SCtrl.Status = NL_S_ST_WaitRxFC;
        }
    }
    else if(2 == temp_FC_FS)//Reciver Rx Buff Overflow
    {
        CAN_Nl_SenderInit(NlPROx, NL_RESULT_BUFFER_OVFLW);
    }
    else//Reserved
    {
        CAN_Nl_SenderInit(NlPROx, NL_RESULT_INVALID_FS);
        CAN_Al_NlUSData_con( NlPROx, NL_IND_AL_N_INVALID_FS );
    }
}
/******************************************************************************
**                            End Of File
******************************************************************************/

