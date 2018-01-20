/////////////////////////////////////////////////////////////////////////////////
#ifndef CAN_DIAG_AL_APL_H
#define CAN_DIAG_AL_APL_H
#include "Can_config.h"

#define TRACE_LIUBO	0

/////////////////////////////////////////////////////////////////////////////////
extern NL_Process_STypeDef     NL_Process[CAN_DIAG_CHx_NUM];
extern Al_Ctrl_STypeDef 	AL_Process[CAN_DIAG_CHx_NUM];
///////////////////////////////////////////////////////////////////////////////

typedef struct {
	Message_t   Msg;
	u16         Offset;
	u16         Len;
}SyncEepromDataNf_t;


//DTC list config
typedef enum
{
	DispU001088 = 0,
	DispU117287,
	DispU117387,
	DispU117587,
	DispU117487,
	DispU115687,
	DispU11AD87,
	DispU114787,
	DispU114887,
	DispU119A87,
	DispU115887,
	DispU116287,
	DispU119687,
	DispU119787,
	DispU114587,
	DispU119C87,
	DispU119587,

	DispB2000,
	DispB2001,
	DispB2003,
	DispB2004,
	DispB200B,
	DispB200E,
	DispB200F,
	DispB200C,
	DispB2028,	
	DTCLISTMAXNUM,
} DtcShowlist_ETypeDef;
/***************************** liubo add begin*******************************/
typedef  struct
{
	u8 Product_Serial_Number[4];  // 
	u8 Production_Line_Number[6];  // 
	u8 ProductDate[3]; // ProductDate[0]--Year ProductDate[1]--Month ProductDate[0]--Day 
}STECU_SERIAL_NUMBER;

typedef  struct
{
	u8 Date[3]; // 日期 Date[0]--Year Date[1]--Month Date[0]--Day 	
	u8 DeviceNumber[4];  // 设备号
}STAPP_SOFTWARE_FINGERPRINT_DATAIDENTIFIER;

typedef union
{
	u8  ECallInfoBuff[8];
	struct
	{
	/* Byte 0 */
		u8  OcurrenceCounter;  // 次数	
	/* Byte 1 */
		u8 TriggeringSource; // 触发源
	/* Byte 2 */
		u8 Years; // 年
	/* Byte 3 */
		u8 Months; // 月
	/* Byte 4 */
		u8 Days; // 日
	/* Byte 5 */
		u8 Hour; // 时
	/* Byte 6 */
		u8 Minute; // 分
	/* Byte 7 */
		u8 Second; // 秒
    }bits;
}UNECALL_INFORMATION;

typedef union
{
    u8 Statusbuff;
    struct
    {
        u8    ButtonStatus :1;
        u8    Reserved :7;
    } Bits;
}UNECALL_STATUS_POLL;

typedef  struct
{
	u8    Live_Longitude[4]; // 
	u8    Live_Latitude[4]; // 
	u8    Live_Time[4]; // 
	u8    Live_Num[1]; // 
}STGNSS_BASIC;

typedef  struct
{
	u8    Mobile_Country_Code[3]; // 
	u8    Mobile_Network_Code[3]; // 
}STPLMN_ID_DATA;

typedef union
{
	u8  InternalBatteryBuff[3];
	struct
	{
	/* Byte 0 */
		u8  Inner_Battery_voltage;  // 

	/* Byte 1 */
		u8	Internal_Battery_Temperature;  // 
	
	/* Byte 2 */
		u8	Internal_Battery_Status  :4;  // Bit0~Bit3 
		u8	Charging_Status  :3;  // Bit4~Bit6 
		u8	  :1;  // Bit7 
    }bits;
}UNINTERNAL_BATTERY;


typedef union
{
	u8  LED_Status;
	struct
	{
	/* Byte 0 */
		u8	LED_SOS_Key_Status  :1;  // Bit0 SOS
		u8	LED_WorkStatus_Status  :1;  // Bit1 
		u8	LED_FaultStatus_Status  :1;  // Bit2 
		u8	  :5;  // Bit3~Bit7 预留
    }bits;
}UNLED_STATUS;

typedef  struct
{
	u8 ATT_Volume_HP; // 
	u8 ATT_Micro_Gain; // MIC
	u8 ATT_Micro_Echo_Active; // 
	u8 ATT_Micro_Echo_Time[2]; // 
	u8 ATT_Micro_Echo_Single_Len; // 
	u8 ATT_Micro_Echo_Single_Decstr[2]; // 
	u8 ATT_Micro_Echo_Double_Decstr[2]; // 
	u8 ATT_Micro_Noise; // 
	u8 ATT_Micro_Equa[232]; // 
}STAUDIO_TUNING;

typedef union
{
	u8  ECALL_AudioOutControl;
	struct
	{
	/* Byte 0 */
		u8	ECALL_AudioOutControl  :1;  // Bit0 
		u8	  :7;  // Bit1~Bit7 
    }bits;
}UNECALL_AUDIO_OUT_CONTROL;

typedef union
{
	u8  ParkModeBuff;
	struct
	{
	/* Byte 0 */
		u8	ParkModeConfig  :1;  // Bit0 
		u8	  :7;  // Bit1~Bit7
    }bits;
}UNPARK_MODE;

typedef  struct
{
	u8    APN_Name[64]; // APN
	u8    APN_User[32]; // APN
	u8    APN_Password[32]; // APN
}STCHANA_APN;

typedef enum
{
	CAN_BAUDRATE_125K=0,
	CAN_BAUDRATE_500K=1,
	CAN_BAUDRATE_250K=2,
	CAN_BAUDRATE_MAX=3,
}ECAN_BAUDRATE;

/***************************** liubo add end*******************************/

typedef  struct
{
	MCU_bool    DisableAppMsgRx;
	MCU_bool    DisableAppMsgTx;
	MCU_bool    DisableNMMsgRx;
	MCU_bool    DisableNMMsgTx;
	MCU_bool    DisableDiagMsgRx;
	MCU_bool    DisableDiagMsgTx;
}CANCommunicationControl_STypeDef;
extern  CANCommunicationControl_STypeDef  CANDiagCCCtrl;
///////////////////////////////////////////////////////////////////////////////

#define  CHANA_EEPROM_REQ	100

#define mem_offset(type, member)   ((u32) &((type *)0)->member)
//eeprom interface
#pragma pack(1)
typedef  struct
{
//Total Lenth 4 DataFlash Block 256 Byte

/* DTC status */
	u8  S_DTCErr[DTCLISTMAXNUM];  
	
/* DID save */
	u8  ECU_SoftwareVersion[8];             // DID F189 
	u8  ECU_HardwareVersion[8];             // DID F089 
	STECU_SERIAL_NUMBER ECU_SerialNumber;   // DID F18C 
	u8  CAR_VinNumber[17];   				// DID F190 
	u8  VMDate[4];   						// DID F1A2 
	u8  LocalFuncBitConf[6];   				// DID F1FA

	u8  TboxICCID[10];   					// DID F1F3 
	u8  TboxTUID[16];   					// DID F1F6 
	UNECALL_INFORMATION  ECALLInfo;   		// DID F1F9 

	STPLMN_ID_DATA  PLMNID;   				// DID F217
	u8  ParkMode;   						// DID F227 

	u8  TSP_URL[64];   						// DID F210 

    	//u8  Reserver[66];                       // Reserver
    	u8  AppSW_Fingerprint[7];               // DID F184	
    	u8  TboxIMSI[15];//[8];//
	u8  CarPNINumber[21];
   	 u8  Reserver[23];//[37];//
		
	u8  ParaSaveDefFlag1; 					// Position 0
	u8  ParaSaveDefFlag2; 					// Position 1
} EepromPara_STypeDef;
typedef  struct
{
	u8  FILEData[64]; 
}FILEDataPara_STypeDef;
typedef struct
{
	u8 VIN_Match_Status[1];
	u8 EngineSt;
	u8 FuelPercent;
	u8 VinCodeMatchSt;
	u8 awake_src;
}BackUpRAMPara_STypeDef;

typedef struct
{
	STGNSS_BASIC gstGnssBasic;
	u16  BattVoltage[1];
	UNECALL_STATUS_POLL unEcallStatusPoll;
	u8 NetworkStatus[1];
	u8 gSystemTime[4];
}RAMPara_STypeDef;
#pragma pack()

#define  PARASETFLAG1  0xaa
#define  PARASETFLAG2  0x56
#define member_offset(type, member)   ((u32) &((type *)0)->member) //Structure member offsets address //typedef  struct

extern EepromPara_STypeDef eeprom_data;

extern void SystemParaSaveEeprom(u16 targetaddr, u8* sourcedat,u16 len);
extern void SystemParaReadEeprom(u16 sourceAddr,u8* targetaddr,u16 len);
extern void ReadParkModeConfig(u8 *pdata);


///////////////////////////////////////////////////////////////////////////////
typedef union
{
	u8  NodeMissFuncBuff[3];
	struct
	{
	/* Byte 0 */
	
		u8	NodeOf_GW_Missing  :1;	// Bit0 GW
		u8	NodeOf_GW_2B0_Missing  :1;	// Bit1 GW_2B0 
		u8	NodeOf_GW_320_Missing  :1;	// Bit2 GW_320
		u8	NodeOf_GW_326_Missing  :1;	// Bit3 GW_326
		u8	NodeOf_GW_32A_Missing  :1;	// Bit4 GW_32A
		u8	NodeOf_IP_260_Missing  :1;	// Bit5 GW_260
		u8	NodeOf_IP_280_Missing  :1;	// Bit6 IP_280
		u8	NodeOf_IP_380_Missing  :1;	// Bit7 IP_380

	/* Byte 1 */
	
		u8	NodeOf_BCM_288_Missing	:1;  // Bit0 BCM_288
		u8	NodeOf_BCM_347_Missing	:1;  // Bit1 BCM_347	
		u8	NodeOf_BCM_384_Missing	:1;  // Bit2 BCM_384
		u8	NodeOf_PEPS_266_Missing  :1; // Bit3 PEPS_266
		u8	NodeOf_AC_330_Missing  :1;	// Bit4 AC_330
		u8	NodeOf_AC_340_Missing  :1;	// Bit5 AC_340               0
		u8	NodeOf_DVD_2F8_Missing	:1; // Bit6 DVD_2F8
		u8	NodeOf_DVD_500_Missing  :1;	// Bit7 DVD_500		
	/* Byte 2 */
		u8	NodeOf_DVD_501_Missing  :1;	//Bit0 DVD_501
		u8	  :1;  // Bit1 reserved
		u8	  :1;  // Bit2 reserved
		u8	  :1;  // Bit3 reserved
		u8	  :1;  // Bit4 reserved
		u8	  :1;  // Bit5 reserved
		u8	  :1;  // Bit6 reserved
		u8	  :1;  // Bit7 reserved
	}bits;
} NodeMiss_Func_UTypeDef;

extern EepromPara_STypeDef eeprom_data;
///////////////////////////////////////////////////////////////////////////////
extern MCU_bool UdsDtcCtrlIsOffFlag;

///////////////////////////////////////////////////////////////////////////////
typedef enum
{
    RC_STOP=0,
    RC_START,
    //RC_REQRESULT,
} RoutineControl_EType;

typedef  void (*pCallBack_RCdId)(u8*, u16, u8*, u16*);

#define CANALRCDIDNUM  10
typedef struct
{
		u16  Rcdid;
		u16 rxlen;
		pCallBack_RCdId ActOn;
    RoutineControl_EType Mode;
    u8 ActResult;
} RoutineControl_STypeDef;

typedef enum
{
	RCDid_CmdTelephoneCall=0,
	RCDid_CmdActTestSpeaker=1,
	RCDid_CmdActDevicePro=2,
	RCDid_CmdChgDevicePro=3,
	RCDid_RmtUnlockDoor=4,
	RCDid_RmtLockDoor=5,
	RCDid_RmtStartAC=6,
	RCDid_RmtStopAC=7,
	RCDid_CarRemindAct=8,
	RCDid_CallTest=9,
}RCDid_ETypeDef;
extern RoutineControl_STypeDef   CANDiagRCDidTab[CANALRCDIDNUM];
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
u8 CAN_Al_GetReadDataByIdentifier(u8* prxdata,u16 rxIndex,u8* ptxdata,u16 *ptlen);
u8 CAN_Al_WriteDataByIdentifier(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len);
u8 CAN_SID_RoutineControl(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len,u8* pmsg,u16* lenght);

///////////////////////////////////////////////////////////////////////////////
void CAN_Sid_ResetEcu(u8 rxcmd);
void CAN_Sid_ResetEcu_Timing(void);

void CAN_Sid_SetIdAppAndNMCtrlType(u8 ctrltype,u8  comtype);

void CAN_Sid_SetDtcCtrlType(u8* prxdata,u16 rxlen);
u8  CAN_Sid_ReadDtcInfoCheckInvalidFormat(u8 sbufunc,u16 rxlen);
u8  CAN_Sid_ReadDtcInfoChecksubfuncNotSupported(u8 sbufunc);
void CAN_Sid_ReadDtcInfo(u8 sbufunc ,u8* prxdata,u8* ptxdata,u16* ptxlen);
u8 CAN_Sid_ClearDtcInfo(u32 DTCIndex);

u8 CAN_Sid_ReadMemoryByAddress(u8* prxdata,u16 rxlen,u8* ptxdata,u16* plen);
u8 CAN_Sid_InputOutputControlByIdentifier(u8* prxdata,u16 rxlen,u8* ptxdata,u16* plen);

void CAN_Sid_RequestDownload(u8* prxdata,u16 rxlen,u8* ptxdata,u16* plen);
void CAN_Sid_TransferData(u8* prxdata,u16 rxlen,u8* ptxdata,u16* plen);
void CAN_Sid_RequestTransferExit(u8* prxdata,u16 rxlen,u8* ptxdata,u16* plen);
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void Tbox_SaveTuid(void);
///////////////////////////////////////////////////////////////////////////////
void  CAN_DIAG_Init(void);
void CAN_DIAG_Timer_Process(void);// 2ms

void CAN_DIAG_TxConf(u8 ucPara);
u8 Can_Dll_Process( CAN_MSG_Type  *rxmsg ,u8 nlChn);
///////////////////////////////////////////////////////////////////////////////

/*********************************** DID Begin ************************************************/


/*********************************** DID End ************************************************/
/*
* DTC
*/
void  DTCCanBusOffProcess(u8 type);
void  DTCRxCANID2B0Process(void);
void  DTCRxCANID320Process(void);
void  DTCRxCANID32AProcess(void);
void  DTCRxCANID326Process(void);
void  DTCRxCANID280Process(void);
void  DTCRxCANID380Process(void);
void  DTCRxCANID288Process(void);
void  DTCRxCANID347Process(void);
void  DTCRxCANID384Process(void);
void  DTCRxCANID266Process(void);
void  DTCRxCANID512Process(void);
void  DTCRxCANID500Process(void);
void  DTCRxCANID501Process(void);
void  DTCRxCANID330Process(void);
void  DTCRxCANID260Process(void);
void  DTCRxCANID2F8Process(void);
void  DTCGPSOpenProcess(u8 type);
void  DTCGPSShortProcess(u8 type);
void  DTCHvoltageProcess(u8 type);
void  DTCLvoltageProcess(u8 type);
void  DTCECALLPressProcess(u8 type);
void  DTCMicOpenProcess(u8 type);
void  DTCMicShortToPowerProcess(u8 type);
void  DTCEepromFailureProcess(u8 type);
void  DTCPartFaultProcess(u8 type);

/**************************Function Declarate********************/
extern void PwrOnScanSystermPara(void );
extern void CanDiagRec4GDiagDataHandle(const Message_t *pMsg);
extern void CanDiagRecKeyEvtHandle(const Message_t *pMsg);
extern void CanDiagRecMainVoltErrStateHandle(const Message_t *pMsg);
extern void CanDiagRecGpsErrStateHandle(const Message_t *pMsg);
extern void CanDiagRecMicErrStateHandle(const Message_t *pMsg);
extern void CanDiagRecEcallErrStateHandle(const Message_t *pMsg);
extern void CanDiagSendParkModeTo4G(void);
extern void CanDiagSendTUIDTo4G(void);
extern void CanDiagSendHardVersionTo4G(void);
extern void CanDiagSendVinTo4G(void);
extern void CanDiagSendMcuSoftWareTo4G(void);

#endif
