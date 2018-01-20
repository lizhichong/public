/////////////////////////////////////////////////////////////////////////////////
#ifndef CAN_DIAG_AL_FML_H
#define CAN_DIAG_AL_FML_H
/////////////////////////////////////////////////////////////////////////////////
#include "Can_config.h"
#define START_ROUTINE           ((u8)1)
#define STOP_ROUTINE            ((u8)2)
#define RQ_ROUTINE_RESULT       ((u8)3)

#define SA_RQ_SEED              ((u8)0x01u)
#define SA_SEND_KEY             ((u8)0x02u)

//Diagnostic Management
#define  Bit_SuppressPositiveResponseMsgInd      ((u8)0x80u)

#define SID_FunctionRequest                ((u8)0x01u)
#define SID_WriteEEPROM                    ((u8)0x02u)
#define SID_ReadEEPROM                     ((u8)0x03u)
#define SID_TransferFile				((u8)0x04u)

#define SID_DiagnosticSessionControl                 ((u8)0x10u)
#define SID_EcuReset                                 ((u8)0x11u) // 电控单元复位
#define SID_SecurityAccess                           ((u8)0x27u)
#define SID_CommunicationControl                     ((u8)0x28u)
#define SID_TesterPresent                            ((u8)0x3Eu) // 诊断设备在线
#define SID_ControlDTCSetting                        ((u8)0x85u)
//Data Transmission
#define SID_ReadDataByIdentifier                     ((u8)0x22u) // 读取数据（通过标识） DID
#define SID_ReadMemoryByAddress                      ((u8)0x23u)
#define SID_ReadDataByPeriodicIdentifier             ((u8)0x2Au)
#define SID_WriteDataByIdentifier                    ((u8)0x2Eu) // 写入数据（通过标识） DID
#define SID_WriteMemoryByAddress                     ((u8)0x3Du)
#define SID_DynamicallyDefineDataIdentifier          ((u8)0x2Cu)

//Stored data transmission
#define SID_ClearDiagnosticInformation               ((u8)0x14u) // 清除诊断信息
#define SID_ReadDTCInformation                       ((u8)0x19u) // 读取DTC信息
//Input/Output Control
#define SID_InputOutputControlByIdentifier           ((u8)0x2Fu)
//Remote Activation of Routine
#define SID_RoutineControl                           ((u8)0x31u)
//Upload/Download   FBL  just physical mode
#define SID_RequestDownload                          ((u8)0x34u)
#define SID_TransferData                             ((u8)0x36u)
#define SID_RequestTransferExit                      ((u8)0x37u)


/********************************************************************************************/
typedef u32 al_timer_t;
typedef u32 al_bit_t;
typedef MCU_bool al_flag_t;


///////////////////////////////////////////////////////////////////////////////

//AL use define
typedef enum
{
    AL_NULL_SESSION = 0,
    AL_DEFAULT_SESSION,
    AL_PROGRAMMING_SESSION,
    AL_EXTEND_DIAGNOSTIC_SESSION,
} Al_Diagnostic_Session_ETypeDef; //

typedef enum
{
    AL_SECURITY_LOCK    = 0,
    AL_SECURITY_UNLOCK  = 1,
} Al_Security_ST_ETypeDef;

typedef struct
{
    al_flag_t RequestSeed;
    al_flag_t   TimerStart;
    al_timer_t  Timer;
    u8  AttemptsNumber;
    Al_Security_ST_ETypeDef Status;
} Al_Security_Ctrl_STypeDef;

typedef enum
{
    AL_CommunicationControl_enableRxAndTx = 0,
    AL_CommunicationControl_enableRxAndDisableTx,
    AL_CommunicationControl_disableRxAndEnableTx,
    AL_CommunicationControl_disableRxAndTx,
} Al_CCCtrl_ETypeDef;

typedef enum
{
    AL_CommunicationControl_App = 1,
    AL_CommunicationControl_NM,
    AL_CommunicationControl_APPNM,
} Al_CCCom_ETypeDef;

typedef enum
{
    AL_ControlDTCSetting_ON = 1,
    AL_ControlDTCSetting_OFF,
} Al_DTCSet_ETypeDef;

typedef struct
{
    u8   EcuReset_Type;
    u32  EcuReset_Timer;

} Al_EcuReset_Ctrl_STypeDef;

typedef struct
{
    al_flag_t   TimerStart;
    al_timer_t Timer;
    Al_Diagnostic_Session_ETypeDef  DsTypes;
    Al_Diagnostic_Session_ETypeDef  DsTypesNext;
    Al_Diagnostic_Session_ETypeDef  DsTypesTimeOver;
    Al_Security_Ctrl_STypeDef Security;
    Al_CCCtrl_ETypeDef   CCCtrlType;
    Al_CCCom_ETypeDef  CCComType;
    Al_DTCSet_ETypeDef   DTCSetType;
    u8  TPS3OverFlag;
    u8 AlTxSid;
    u8 AlTxReason;
	Al_EcuReset_Ctrl_STypeDef EcuR;
} Al_Ctrl_STypeDef;


///////////////////////////////////////////////////////////////////////////////
void CAN_Al_Ctrl_Init(Al_Ctrl_STypeDef *AlPROx);
void CAN_Al_Timer_Process(Al_Ctrl_STypeDef *AlPROx);
void CAN_Al_NlReciveDataInd(NL_Process_STypeDef *NlPROx, u8 *pdata, u16 len);
void CAN_Al_NlUSData_con(NL_Process_STypeDef *NlPROx, Nl_Com_Al_ETypeDef reason);
///////////////////////////////////////////////////////////////////////////////
#endif
