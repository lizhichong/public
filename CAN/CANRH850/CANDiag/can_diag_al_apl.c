/** 
* Copyright (C), CHINATSP,www.chinatsp.com.  All rights reserved.
*
* @file can_diag_al_apl.c
* 
* @Description Diagnostics on Controller Area Networks,  Application layer, ISO 15765-3
*
* @author jason
* 
* @version v1.0.0 
* 
* @date 2017/7/20
* 
* History£∫
*
*/  
/************************************************
*			Include file							*
************************************************/
#include "Can_config.h"
#include "PtlTboxInfo.h"
#include "Time.h"
#include "Debug.h"
#include "DiagApp.h"

#include "NVM_mgr.h"
#include "NVM_if.h"


#include "rtc.h"//zhouxb add
sTime time_t;
u32 UTCTime = 0;

//global variable
CANCommunicationControl_STypeDef  CANDiagCCCtrl;
/************************************************
*			constant								*
************************************************/

/************************************************
*			internal macro							*
************************************************/
#define DIDLISTMAXNUM	(sizeof(DIDInfoTab)/sizeof(DIDInfo_TypeDef))
#define ADDR_MAX	(0xffffffff)

#define  CAN_DIAG_CHANAWDBI
#define  CAN_DIAG_WRITE_DATA_HEAD_LENGTH  3U
#define  CAN_YUANTEL_HOST_HEAD_LENGTH	  2U

/************************************************
*			internal data type						*
************************************************/
typedef union
{
    u8 data;
    struct
    {
        u8 TestFailed   : 1; // ÊµãËØïÂ§±Êïà
        u8 TestFailedMonitorCycle   : 1;
        u8 PendingDTC   : 1;
        u8 ConfirmedDTC : 1; // Á°ÆËÆ§DTC
        u8 TestNotCompletedSinceLastClear   : 1;
        u8 TestFailedSinceLastClear   : 1;
        u8 TestNotCompletedMonitoringCycle   : 1;
        u8 WarningIndicatorRequested   : 1;
    } bits;
} Dtc_BitStatusSTypeDef;

typedef void (*pfDTCCheckEvent) (u8*);

typedef struct
{
    MCU_bool EnableFlag;
	MCU_bool ConfEnableOrNotFlag;
    MCU_bool NoErrFlag;
    MCU_bool NowInfoRecordFlag;
    MCU_bool HistoryInfoRecordFlag;
    //MCU_bool NoErrFlag;
    //MCU_bool CancelFlag;
    u8  ModeType;       //cycle or event msg
    //u8 FaultCounter;  //
    //u8 DtcSavedData;  //save eeprom
    u16  BasicTimer;    //Ê£ÄÊµãÊïÖÈöúÂë®ÊúüÊó∂Èó¥
    u16  WarnTimer;     //Á°ÆÂÆöÊïÖÈöúÊ¨°Êï∞
    u16  CancelWarnTimer; //Ëß£Èô§ÊïÖÈöúÊ¨°Êï∞
    Dtc_BitStatusSTypeDef Status; //ÂØπÂ∫îËØäÊñ≠ÈóÆÂç∑DTCÁä∂ÊÄÅ
}Dtc_STypeDef;

typedef enum
{
    DTC_PERIOD_MODE=0,
    DTC_EVENT_MODE,
    DTC_EVENTPERIOD_MODE,
}DtcListMode_ETypeDef;

//typedef void (*pfDTCCheckEvent) (u8*);
typedef struct
{
	Dtc_STypeDef* pDtcctrlx; // DTC status type
	u32  number; // DTC display number
	u16  BasicTimeDef; //test cycle basictime
	u16  WarnTimeDef; // confirm DTC cycle
	u16  CancelWarnTimeDef; // cancel DTC cycle
	u8   ModeTypeDef; // period or event
	u8   SetAllow; // enable config
	//pfDTCCheckEvent  DTCCheckEventCall;//
} DtcInfoConf_STypeDef;

typedef unsigned int	saveaddr;
typedef enum
{
	SaveIn_RAM=0x0,
	SaveIn_ROM,
	SaveIn_EEPROM,
	SaveIn_BackupRAM,
}Save_TypeDef;
typedef struct
{
	u16 DID_num;
	u16 DataLength;
	Save_TypeDef saveType;
	bool EnWrite;
	saveaddr address;	//indicate postion to eeprom, write max value when do not save in eeprom
}DIDInfo_TypeDef; 


/************************************************
*			constant							*
************************************************/
static const u8 def_ECU_SoftwareVersion[8] = "SW:A.1.0";
static const u8 def_ECU_HardwareVersion[8] = "HW:A.1.0";
static const STECU_SERIAL_NUMBER def_ECU_SerialNumber = {{0}, {"123456"}, {0}};
static const u8 def_VMDate[4] = {20,17,4,10};
static const u8 def_LocalFuncBitConf[6] = {0xFF,0xDF,0x01,0x70,0x00,0x00};
static const u8 def_TSP_URL[64] = "https://tsp.changan.com.cn";
static const u8 def_TBOXTerminalICCIDNumber[20] = "00000000000000000000";
static const u8 def_TBOXTerminalTUID[32] = "00000000000000000000000000000000";

static const u8 NetworkInfo[4] = " 0.9";	//DIDF120
static const u8 FuncSpecificaion[4]=" 2.2";	//DIDF121
static const u8 DiagDefiniion[4]=" 1.3";	//DIDF122
static const u8 ParNum[21]= "7900020-BM50		   ";	//DIDF187
static const u8 SystemSupplier[7]="S11529A";	//DIDF18A
static const STAPP_SOFTWARE_FINGERPRINT_DATAIDENTIFIER def_AppSW_Fingerprint ={{16, 3, 17}, {"1234"}}; 	//DIDF184
static const u8 SWDownloadVersion[1]={0x11};	//DIDF170 

/************************************************
*			static global variable				*
************************************************/
NL_Process_STypeDef NL_Process[CAN_DIAG_CHx_NUM];
Al_Ctrl_STypeDef    AL_Process[CAN_DIAG_CHx_NUM];
u8 NL_RxBuff[NL_RXBUFFMAXSIZE];
u8 NL_TxBuff[NL_TXBUFFMAXSIZE];

EepromPara_STypeDef eeprom_data;
FILEDataPara_STypeDef  FILE_Data;

//zhouxb add  20171019
#if  0   //zhouxb …œŒªª˙√ª”–’‚–©…Ë÷√

static  const u8 DEF_VINDataIdentifier[17]={0};
static  const u8 DEF_CarModelTypeConfiguration[1] = {0};
static  const u8 DEF_LocalFuncConf[11] = {0xFF,0xDF,0x01,0x70,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

static  const u8 DEF_TSP_URL[64] = "https://tsp.changan.com.cn";
static  const u8 DEF_AudioOutControl = 0x01;
static  const u8 DEF_APN_Name[64] = "cqcaqc01.c1fu.njm2mapn";
static  const u8 DEF_APN_User[32] = {0};
static  const u8 DEF_APN_Password[32] = {0};
#endif
//static STECU_SERIAL_NUMBER gstECU_SerialNumber; // EEPROM

//static const u8 DEF_CHANA_ECUSoftwareVersionNumberDataIdentifier[8]="SW:A.3.4";
//static  u8 CHANA_ECUSoftwareVersionNumberDataIdentifier[8];// EEPROM
//static const u8 DEF_CHANA_ECUHardwareVersionNumberDataIdentifier[8]="HW:A.1.6";
//static  u8 CHANA_ECUHardwareVersionNumberDataIdentifier[8];// EEPROM
//static  const u8 DEF_TBOXTerminalICCIDNumber[20] = "00000000000000000000";
//static  u8 TBOXTerminalICCIDNumber[20] = "00000000000000000000"; // EEPROM
//u8 VINDataIdentifier[17]={0}; // EEPROM
//static  const u8 DEF_TBOXTerminalIMSINumber[16] = "000000000000000";
//static  u8 TBOXTerminalIMSINumber[16] = "000000000000000"; // EEPROM
//static  const u8 DEF_TBOXTerminalTUID[32] = "00000000000000000000000000000000";
static  u8 TBOXTerminalTUID[32] = "00000000000000000000000000000000"; // EEPROM
//static UNPARK_MODE gunParkMode; // EEPROM

//static u8 gNetwork_Detect_Status = 0; 
//static STGNSS_BASIC gstGnssBasic; // RAM
//static STPLMN_ID_DATA gstPlmnID_Data; // EEPROM
//static u8 gSystemTime[4]; // RAM
//static UNINTERNAL_BATTERY gunInternalBattery; // RAM
//static UNLED_STATUS gunLedStatus; // RAM
u8 GSM_Signal; //RAM
u8 filetransfer_OK = 0;
u8 CANetWork_status = 0;




//#define  ECU_SW_VER_LENGTH		(sizeof(DEF_CHANA_ECUSoftwareVersionNumberDataIdentifier) / sizeof(DEF_CHANA_ECUSoftwareVersionNumberDataIdentifier[0]))
#define  ECU_SW_VER_LENGTH		(sizeof(eeprom_data.ECU_SoftwareVersion) / sizeof(eeprom_data.ECU_SoftwareVersion[0]))

//#define  ECU_HW_VER_LENGTH		(sizeof(DEF_CHANA_ECUHardwareVersionNumberDataIdentifier) / sizeof(DEF_CHANA_ECUHardwareVersionNumberDataIdentifier[0]))
//#define  ECU_SERIAL_NUMBER_LENGTH		sizeof(gstECU_SerialNumber)
#define  ECU_HW_VER_LENGTH		(sizeof(eeprom_data.ECU_HardwareVersion) / sizeof(eeprom_data.ECU_HardwareVersion[0]))
#define  ECU_SERIAL_NUMBER_LENGTH		sizeof(eeprom_data.ECU_SerialNumber)


//#define  VIN_DATA_LENGTH		(sizeof(VINDataIdentifier) / sizeof(VINDataIdentifier[0]))
#define  VIN_DATA_LENGTH		(sizeof(eeprom_data.CAR_VinNumber) / sizeof(eeprom_data.CAR_VinNumber[0]))
#define   CarPNI_DATA_LENGTH   (sizeof(eeprom_data.CarPNINumber) / sizeof(eeprom_data.CarPNINumber[0]))
//#define  ICCID_LENGTH		(sizeof(TBOXTerminalICCIDNumber) / sizeof(TBOXTerminalICCIDNumber[0]))
#define  ICCID_LENGTH		(sizeof(eeprom_data.TboxICCID) / sizeof(eeprom_data.TboxICCID[0]))

//#define  VMDATE_DATA_LENGTH		(sizeof(Vehicle_Manufacturing_Date) / sizeof(Vehicle_Manufacturing_Date[0]))
//#define  APPSFFP_DATA_LENGTH		(sizeof(gstAppSW_FingerprintDataIdentifier) / sizeof(gstAppSW_FingerprintDataIdentifier.Date[0]))
//#define  CAR_TYPE_LENGTH		(sizeof(CarModelTypeConfiguration) / sizeof(CarModelTypeConfiguration[0]))
//#define  LOCAL_FUNC_CONF_LENGTH		sizeof(LocalFuncConf)

//#define  IMSI_LENGTH		(sizeof(TBOXTerminalIMSINumber) / sizeof(TBOXTerminalIMSINumber[0]))
#define  IMSI_LENGTH		(sizeof(eeprom_data.TboxIMSI) / sizeof(eeprom_data.TboxIMSI[0]))

#define  TUID_LENGTH		(sizeof(TBOXTerminalTUID) / sizeof(TBOXTerminalTUID[0]))
//#define  TUID_LENGTH		(sizeof(eeprom_data.TboxTUID) / sizeof(eeprom_data[0]))

//#define  BATT_VOLTAGE_LENGTH		sizeof(BattVoltage_Current)
//#define  TEMPERATURE_LENGTH		sizeof(Temperature_Current)
//#define  ECALL_INFO_LENGTH		(sizeof(gunECALLInformation.ECallInfoBuff) / sizeof(gunECALLInformation.ECallInfoBuff[0]))
//#define  ECALL_STATUS_POLL_LENGTH		sizeof(unEcallStatusPoll.Statusbuff)
//#define  NETWORK_DETECT_LENGTH		sizeof(gNetwork_Detect_Status)
//#define  TSP_URL_LENGTH		(sizeof(gTSP_URL) / sizeof(gTSP_URL[0]))
//#define  GNSS_BASIC_LENGTH		sizeof(gstGnssBasic)
//#define  PLMN_ID_LENGTH		sizeof(gstPlmnID_Data)
//#define  SYS_TIME_LENGTH		(sizeof(gSystemTime) / sizeof(gSystemTime[0]))
//#define  INTER_BATTERY_LENGTH		(sizeof(gunInternalBattery.InternalBatteryBuff) / sizeof(gunInternalBattery.InternalBatteryBuff[0]))
//#define  LED_STATUS_LENGTH		sizeof(gunLedStatus.LED_Status)
//#define  AUDIO_TUNING_LENGTH		sizeof(gstAudioTuning)
//#define  ECALL_AUDIO_OUT_LENGTH		sizeof(gunECallAudioOutControl.ECALL_AudioOutControl)
#define  PARK_MODE_LENGTH		sizeof(eeprom_data.ParkMode)
//#define  CHANA_APN_LENGTH		sizeof(gstChanaAPN)
//#define  CAN_BAUDRATE_LENGTH		1
//#define  VIN_MATCH_LENGTH		sizeof(gVIN_Match_Status)

static void  SendFileMsgToOther(u8* pdata, u16 len);
static void  SendCERIDMsgToOther(u8* pdata, u16 len);
static void  SendRequestCallToOther(u8* pdata, u16 len);
static void  SendRequestNetTestToOther();


void CAN_RCDid_DialPhoneNumber(u8* prxdata,u16 rxlen, u8 *ptxdata,u16* txlen);
void  CAN_RCDid_TestSpeaker(u8* prxdata,u16 rxlen, u8 *ptxdata,u16* txlen);
void CAN_RCDid_ActDevicePro(u8* prxdata,u16 rxlen, u8 *ptxdata,u16* txlen);
void CAN_RCDid_ChgDevicePro(u8* prxdata,u16 rxlen, u8 *ptxdata,u16* txlen);
void CAN_RCDid_UnlockDoor(u8* prxdata,u16 rxlen, u8 *ptxdata,u16* txlen);
void CAN_RCDid_LockDoor(u8* prxdata,u16 rxlen, u8 *ptxdata,u16* txlen);
void CAN_RCDid_StartAC(u8* prxdata,u16 rxlen, u8 *ptxdata,u16* txlen);
void CAN_RCDid_StopAC(u8* prxdata,u16 rxlen, u8 *ptxdata,u16* txlen);
void CAN_RCDid_CarRemind(u8* prxdata,u16 rxlen, u8 *ptxdata,u16* txlen);
void CAN_RCDid_CallTest(u8* prxdata,u16 rxlen, u8 *ptxdata,u16* txlen);
RoutineControl_STypeDef   CANDiagRCDidTab[CANALRCDIDNUM]=
{
    {0x0101,7,CAN_RCDid_DialPhoneNumber,RC_STOP,FALSE},
    {0x0102,4,CAN_RCDid_TestSpeaker,RC_STOP,FALSE},
    {0x0103,4,CAN_RCDid_ActDevicePro,RC_STOP,FALSE},
    {0x0104,4,CAN_RCDid_ChgDevicePro,RC_STOP,FALSE},
    {0x0105,4,CAN_RCDid_UnlockDoor,RC_STOP,FALSE},
    {0x0106,4,CAN_RCDid_LockDoor,RC_STOP,FALSE},
    {0x0107,5,CAN_RCDid_StartAC,RC_STOP,FALSE},
    {0x0108,4,CAN_RCDid_StopAC,RC_STOP,FALSE},
    {0x0109,4,CAN_RCDid_CarRemind,RC_STOP,FALSE},
    {0x010a,4,CAN_RCDid_CallTest,RC_STOP,FALSE},
};

MCU_bool UdsDtcCtrlIsOffFlag = false;
extern bool gblCanSleepFlag;

Dtc_STypeDef    DtcCtrl[DTCLISTMAXNUM];

//static const u8 E_DtcErrSaved[DTCLISTMAXNUM] = {0};
u8 DtcErrSaved[DTCLISTMAXNUM] = {0};

DtcInfoConf_STypeDef    DtcInfoConfTab[DTCLISTMAXNUM]=
{
	{&DtcCtrl[DispU001088],0x00C01088,(50/AL_BASICTIME),5,5,DTC_EVENT_MODE,0},//busoff
	//{&DtcCtrl[reserve1],0,0,0,0,0,1},		//GW node ,we have F1FA config ,but do not have it's DTC
	{&DtcCtrl[DispU117287],0x00D17287,(50/AL_BASICTIME),5,1,DTC_PERIOD_MODE,1},///2B0
	{&DtcCtrl[DispU117387],0x00D17387,(100/AL_BASICTIME),5,1,DTC_PERIOD_MODE,1},///320
	{&DtcCtrl[DispU117587],0x00D17587,(100/AL_BASICTIME),5,1,DTC_PERIOD_MODE,1},///32A
	{&DtcCtrl[DispU117487],0x00D17487,(100/AL_BASICTIME),5,1,DTC_PERIOD_MODE,1},///326
	{&DtcCtrl[DispU115687],0x00D15687,(50/AL_BASICTIME),5,1,DTC_PERIOD_MODE,1},///280
	{&DtcCtrl[DispU11AD87],0x00D1AD87,(500/AL_BASICTIME),5,1,DTC_PERIOD_MODE,1},///380
	{&DtcCtrl[DispU114787],0x00D14787,(40/AL_BASICTIME),5,1,DTC_PERIOD_MODE,1},///288		
	{&DtcCtrl[DispU114887],0x00D14887,(500/AL_BASICTIME),5,1,DTC_PERIOD_MODE,1},///347
	{&DtcCtrl[DispU119A87],0x00D19A87,(200/AL_BASICTIME),5,1,DTC_PERIOD_MODE,1},///384
	{&DtcCtrl[DispU115887],0x00D15887,(40/AL_BASICTIME),5,1,DTC_PERIOD_MODE,1},///266
	{&DtcCtrl[DispU116287],0x00D16287,(40/AL_BASICTIME),5,1,DTC_EVENT_MODE,0},///512////EVENT
	{&DtcCtrl[DispU119687],0x00D19687,(1000/AL_BASICTIME),5,1,DTC_PERIOD_MODE,1},///500	
	{&DtcCtrl[DispU119787],0x00D19787,(1000/AL_BASICTIME),5,1,DTC_PERIOD_MODE,1},///501
	{&DtcCtrl[DispU114587],0x00D14587,(100/AL_BASICTIME),5,1,DTC_PERIOD_MODE,1},///330
	{&DtcCtrl[DispU119C87],0x00D19C87,(40/AL_BASICTIME),5,1,DTC_PERIOD_MODE,1},///260
	{&DtcCtrl[DispU119587],0x00D19587,(100/AL_BASICTIME),5,1,DTC_PERIOD_MODE,1},///2F8

	{&DtcCtrl[DispB2000],0x00A00000,(100/AL_BASICTIME),5,1,DTC_EVENT_MODE,0},	//The GPS circuit of TBOX failure.in open state.
	{&DtcCtrl[DispB2001],0x00A00100,(100/AL_BASICTIME),5,1,DTC_EVENT_MODE,0},	//The GPS circuit of TBOX failure.in short state.
	{&DtcCtrl[DispB2003],0x00A00300,(100/AL_BASICTIME),5,1,DTC_EVENT_MODE,0},	//Vehicle battery voltage become too high.
	{&DtcCtrl[DispB2004],0x00A00400,(100/AL_BASICTIME),5,1,DTC_EVENT_MODE,0},	 //Vehicle battery voltage become too low.	
	{&DtcCtrl[DispB200B],0x00A00b00,(100/AL_BASICTIME),5,1,DTC_EVENT_MODE,0},	////E-CALL Button fault E-CALl Button always in pressing status	
	{&DtcCtrl[DispB200E],0x00A00e00,(100/AL_BASICTIME),5,1,DTC_EVENT_MODE,0},	//Mic Open Circuit
	{&DtcCtrl[DispB200F],0x00A00f00,(100/AL_BASICTIME),5,1,DTC_EVENT_MODE,0},//Mic Short to B+
	{&DtcCtrl[DispB200C],0x00A00c00,(100/AL_BASICTIME),5,1,DTC_EVENT_MODE,0},//EEPROM read/rwrite operation failure
	{&DtcCtrl[DispB2028],0x00A02800,(200/AL_BASICTIME),1,1,DTC_EVENT_MODE,0},//part fault
};

BackUpRAMPara_STypeDef backupRam_data;
RAMPara_STypeDef ram_data;

static const DIDInfo_TypeDef	DIDInfoTab[]=
{
	{0xF120, 4, SaveIn_ROM, false, (saveaddr)&NetworkInfo[0]},
	{0xF121, 4, SaveIn_ROM, false, (saveaddr)&FuncSpecificaion[0]},
	{0xF122, 4, SaveIn_ROM, false, (saveaddr)&DiagDefiniion[0]},
	{0xF187, 21, SaveIn_ROM, false, (saveaddr)&ParNum[0]},
	{0xF18A, 7, SaveIn_ROM, false, (saveaddr)&SystemSupplier[0]},
	{0xF170, 1, SaveIn_ROM, false, (saveaddr)&SWDownloadVersion[0]},

	{0xF213, 12, SaveIn_RAM, true, mem_offset(RAMPara_STypeDef, gstGnssBasic)},
	{0xF1F7, 2, SaveIn_RAM, true, mem_offset(RAMPara_STypeDef, BattVoltage)},
	{0xF1FC, 1, SaveIn_RAM, true, mem_offset(RAMPara_STypeDef, unEcallStatusPoll)},
	{0xF1FD, 1, SaveIn_RAM, true, mem_offset(RAMPara_STypeDef, NetworkStatus)},
	{0xF220, 4, SaveIn_RAM, true, mem_offset(RAMPara_STypeDef, gSystemTime)},
		
	{0xF22A, 1, SaveIn_BackupRAM, true, mem_offset(BackUpRAMPara_STypeDef, VIN_Match_Status)},
		
	{0xF189, 8, SaveIn_EEPROM, true, mem_offset(EepromPara_STypeDef,ECU_SoftwareVersion)},
	{0xF089, 8, SaveIn_EEPROM, true, mem_offset(EepromPara_STypeDef,ECU_HardwareVersion)},
	{0xF18C, 13, SaveIn_EEPROM, true, mem_offset(EepromPara_STypeDef,ECU_SerialNumber)},
	{0xF190, 17, SaveIn_EEPROM, true, mem_offset(EepromPara_STypeDef,CAR_VinNumber)},
	{0xF1A2, 4, SaveIn_EEPROM, true, mem_offset(EepromPara_STypeDef,VMDate)},
	{0xF1FA, 6, SaveIn_EEPROM, true, mem_offset(EepromPara_STypeDef,LocalFuncBitConf)},
	{0xF1F3, 10, SaveIn_EEPROM, true, mem_offset(EepromPara_STypeDef,TboxICCID)},
	{0xF1F6, 16, SaveIn_EEPROM, true, mem_offset(EepromPara_STypeDef,TboxTUID)},
	{0xF1F9, 8, SaveIn_EEPROM, true, mem_offset(EepromPara_STypeDef,ECALLInfo)},
	{0xF210, 64, SaveIn_EEPROM, true, mem_offset(EepromPara_STypeDef,TSP_URL)},
	{0xF217, 6, SaveIn_EEPROM, true, mem_offset(EepromPara_STypeDef,PLMNID)},
	{0xF227, 1, SaveIn_EEPROM, true, mem_offset(EepromPara_STypeDef,ParkMode)},
	{0xF184, 7, SaveIn_EEPROM, true, mem_offset(EepromPara_STypeDef,AppSW_Fingerprint)},
};

NodeMiss_Func_UTypeDef NodeMissConfig;


static u8 BattVoltage_HighFlg = FALSE; //RAM
static u8 BattVoltage_LowFlg = FALSE;//RAM
static u16 BattVoltage_HighDelayTimer = 0;
static u16 BattVoltage_HighRecoveryTimer = 0;
static u16 BattVoltage_LowDelayTimer = 0;
static u16 BattVoltage_LowRecoveryTimer = 0;

/************************************************
*			Local function declare					*
************************************************/
void CAN_Nl_InitPowrOn(void);
void CAN_Al_InitPowrOn(void);

u8 ChangeString09ToBCD(u8 dath, u8 datl);
void DTCListAllInit(void);
void DTCxInit(DtcInfoConf_STypeDef* pDtcInfoConf);
void CAN_Uds_DtcxTimer(void);
void DTCConfirmThingEventIsNoErr(DtcInfoConf_STypeDef *pDtcInfoConf);
void DTCConfirmThingEventIsErr(DtcInfoConf_STypeDef *pDtcInfoConf);
void DTCConfirmThingPeriodIsErr(DtcInfoConf_STypeDef *pDtcInfoConf);
void DTCConfirmThingPeriodIsNoErr(DtcInfoConf_STypeDef *pDtcInfoConf);

void CAN_DiagGlobalVariableInit(void);
/************************************************
*			Global function						*
************************************************/
void Node_Miss_FunctionConfig(void)
{
	memcpy(&NodeMissConfig.NodeMissFuncBuff[0],&eeprom_data.LocalFuncBitConf[0],3);
}
void CanDiagSendParkModeTo4G(void)
{
	TboxSendParkModeEvt_t st_ParkMode;

	st_ParkMode.Msg.Type = EVENT_TBOX_SEND_DIAG_DATA;
	
	st_ParkMode.CID = PARK_MODE_CID;
	st_ParkMode.ParkMode = eeprom_data.ParkMode;	
	st_ParkMode.Length = 2;
	
	SendMsgToComTbox((u8 *)&st_ParkMode, sizeof(st_ParkMode));	
}

void CanDiagSendTUIDTo4G(void)
{
	u8 i,t_u8TuidAscII[32];

	TboxSendTUIDEvt_t st_SendTuid;

	for(i=0;i<16;i++)
	{
        t_u8TuidAscII[(i<<1)+0] = '0'+ ((eeprom_data.TboxTUID[i]&0xf0)>>4);
    	t_u8TuidAscII[(i<<1)+1] = '0'+ (eeprom_data.TboxTUID[i]&0x0f);
	}

	st_SendTuid.Msg.Type = EVENT_TBOX_SEND_DIAG_DATA;

	st_SendTuid.CID = TUID_CID;
	memcpy(&st_SendTuid.TUID[0],t_u8TuidAscII,32);
	st_SendTuid.Length = 33;

	SendMsgToComTbox((u8 *)&st_SendTuid, sizeof(TboxSendTUIDEvt_t));
}

void CanDiagSendHardVersionTo4G(void)
{
	TboxSendHwVerEvt_t st_SendHwVer;

	st_SendHwVer.Msg.Type = EVENT_TBOX_SEND_DIAG_DATA;

	st_SendHwVer.CID = HW_VER_CID;
	memcpy(&st_SendHwVer.HwVer[0],&eeprom_data.ECU_HardwareVersion[0],8);
	st_SendHwVer.Length = 9;

	SendMsgToComTbox((u8 *)&st_SendHwVer, sizeof(TboxSendHwVerEvt_t));
}

void CanDiagSendVinTo4G(void)
{
	TboxSendVINEvt_t st_SendVin;

	st_SendVin.Msg.Type = EVENT_TBOX_SEND_DIAG_DATA;

	st_SendVin.CID = VINNUM_CID;
	memcpy(&st_SendVin.VINNUM[0],&eeprom_data.CAR_VinNumber[0],17);
	st_SendVin.Length = 18;

	SendMsgToComTbox((u8 *)&st_SendVin, sizeof(TboxSendVINEvt_t));
}

void CanDiagSendMcuSoftWareTo4G(void)
{
	TboxSendMcuSWEvt_t st_SendMcuSW;
	char SysVer[] = {SYSTEM_VER};
	
	st_SendMcuSW.Msg.Type = EVENT_TBOX_SEND_DIAG_DATA;

	st_SendMcuSW.CID = SW_VER_CID;
	memcpy(&st_SendMcuSW.MCUSW[0],(u8*)&SysVer,8);
	st_SendMcuSW.Length = 9;

	SendMsgToComTbox((u8 *)&st_SendMcuSW, sizeof(TboxSendMcuSWEvt_t));	
}

static void CanDiagSendTUIDToBB(void)
{
	u8 i,t_u8TuidAscII[32],t_u8RespFrame[40] = {0};

	for(i=0;i<16;i++)
	{
        t_u8TuidAscII[(i<<1)+0] = '0'+ ((eeprom_data.TboxTUID[i]&0xf0)>>4);
    	t_u8TuidAscII[(i<<1)+1] = '0'+ (eeprom_data.TboxTUID[i]&0x0f);
	}

	t_u8RespFrame[0] = 37;
	t_u8RespFrame[1] = 0x11;
	t_u8RespFrame[2] = 0x12;
	t_u8RespFrame[3] = 2;
	t_u8RespFrame[4] = 4;
	t_u8RespFrame[5] = 0;

	memcpy(&t_u8RespFrame[6], t_u8TuidAscII, 32);
	
	Rte_IWrite_Fun_TboxLogic_Call_UARTTX_Buff_UARTTX_Buff(t_u8RespFrame);
}

void SystemParaReadEeprom(u16 sourceAddr,u8* targetaddr,u16 len)
{
	u8 * pEepromData;
    u8 i;
	
    pEepromData = (u8*)&eeprom_data;
	
	for(i=0; i<len; i++)
	{
		*targetaddr = pEepromData[sourceAddr+i];
		targetaddr++;
	}
}

void SystemParaSaveEeprom(u16 targetaddr, u8* sourcedat,u16 len)
{
   SyncEepromDataNf_t st_SyncEepromDataNf;

   if((targetaddr + len <= sizeof(eeprom_data))
   	  &&(len > 0))
   {
		memcpy((u8*)&eeprom_data+targetaddr,sourcedat,len);

		st_SyncEepromDataNf.Msg.Type = EVENT_NVM_RECV_UPDATAEEPROM_NF;
		st_SyncEepromDataNf.Offset = targetaddr,
		st_SyncEepromDataNf.Len = len;	

		SendMsgToNVM((u8 *) &st_SyncEepromDataNf, sizeof(st_SyncEepromDataNf));
   }
}
static void SaveParkModeConfigToEeprom(u8* pdata,u8 len)
{
	if(len < 1)
	{
		DEBUG_MCU(DBG_ERROR, CAN_MODULE_ID, "CanDiag Recv 4G ParkMode Len too Short,Len = %d;", len);
		return;
	}

	DEBUG_MCU(DBG_INFO, CAN_MODULE_ID, "CanDiag Recv 4G ParkMode = %d;", pdata[0]);
	
	if(eeprom_data.ParkMode != pdata[0])
	{
		SystemParaSaveEeprom(member_offset(EepromPara_STypeDef,ParkMode),pdata,1);
	}
	
	CanDiagSendParkModeTo4G();
}

void ReadParkModeConfig(u8 *pdata)
{
	SystemParaReadEeprom(member_offset(EepromPara_STypeDef,ParkMode),pdata,1);
}

static void SaveTUIDToEeprom(u8* pdata,u8 len)
{
	u8 i,t_u8buff[16];
	bool Check;
	
	if(len < 32)
	{
		DEBUG_MCU(DBG_ERROR, CAN_MODULE_ID, "CanDiag Recv 4G Tuid Len too Short,Len = %d \r\n", len);
		return;
	}

	Check = false;
	
	for(i=0;i<16;i++)
	{
		t_u8buff[i] = ChangeString09ToBCD(pdata[(i<<1)+0],pdata[(i<<1)+1]);
		if(t_u8buff[i] != eeprom_data.TboxTUID[i])
		{
			Check = true;
		}
	}

	if(Check)
	{
		SystemParaSaveEeprom(member_offset(EepromPara_STypeDef,TboxTUID),t_u8buff,16);
	}

	DEBUG_MCU(DBG_INFO, CAN_MODULE_ID, "CanDiag Recv 4G Tuid;");
    DEBUG_MCU_ARRAY(DBG_INFO, CAN_MODULE_ID, t_u8buff, 1, 16, 1);
    DEBUG_MCU_STR(DBG_INFO, CAN_MODULE_ID,"\r\n");

	CanDiagSendTUIDTo4G();
	CanDiagSendTUIDToBB();
}

static void SaveHardVersionToEeprom(u8* pdata,u8 len)
{
	u8 i;
	bool Check;
	
	if(len < 8)
	{
		DEBUG_MCU(DBG_ERROR, CAN_MODULE_ID, "CanDiag Recv 4G HW Len too Short,Len = %d\r\n", len);
		return;
	}

	Check = false;

	for(i=0;i<8;i++)
	{
		if(eeprom_data.ECU_HardwareVersion[i] != pdata[i])
		{
			Check = true;
			break;
		}
	}

	if(Check)
	{
		SystemParaSaveEeprom(member_offset(EepromPara_STypeDef,ECU_HardwareVersion),pdata,8);
	}

	DEBUG_MCU(DBG_INFO, CAN_MODULE_ID, "CanDiag Recv 4G HW;");
    DEBUG_MCU_ARRAY(DBG_INFO, CAN_MODULE_ID, pdata, 1, 8, 1);
    DEBUG_MCU_STR(DBG_INFO, CAN_MODULE_ID,"\r\n");
	
	CanDiagSendHardVersionTo4G();
}

static void SaveSoftVersionToEeprom(u8* pdata,u8 len)
{
	u8 i;
	bool Check;
	
	if(len < 8)
	{
		DEBUG_MCU(DBG_ERROR, CAN_MODULE_ID, "CanDiag Recv 4G SW Len too Short,Len = %d \r\n", len);
		return;
	}

	Check = false;

	for(i=0;i<8;i++)
	{
		if(eeprom_data.ECU_SoftwareVersion[i] != pdata[i])
		{
			Check = true;
			break;
		}
	}

	if(Check)
	{
		SystemParaSaveEeprom(member_offset(EepromPara_STypeDef,ECU_SoftwareVersion),pdata,8);
	}

	DEBUG_MCU(DBG_INFO, CAN_MODULE_ID, "CanDiag Recv 4G SW;");
    DEBUG_MCU_ARRAY(DBG_INFO, CAN_MODULE_ID, pdata, 1, 8, 1);
    DEBUG_MCU_STR(DBG_INFO, CAN_MODULE_ID,"\r\n");
}


static void SavePLMNIDToEerprom(u8* pdata,u8 len)
{
	u8 i;
    u8* pPLMNID = NULL;
	bool Check;
	
	if(len < 6)
	{
		DEBUG_MCU(DBG_ERROR, CAN_MODULE_ID, "CanDiag Recv 4G PLMNID Len too Short,Len = %d \r\n", len);
		return;
	}

	Check = false;
	pPLMNID = (u8 *)&eeprom_data.PLMNID;
	
	for(i=0;i<6;i++)
	{
		if(pPLMNID[i] != pdata[i])
		{
			Check = true;
			break;
		}
	}

	if(Check)
	{
		SystemParaSaveEeprom(member_offset(EepromPara_STypeDef,PLMNID),pdata,6);
	}

	DEBUG_MCU(DBG_INFO, CAN_MODULE_ID, "CanDiag Recv 4G PLMNID;");
    DEBUG_MCU_ARRAY(DBG_INFO, CAN_MODULE_ID, pdata, 1, 6, 1);
    DEBUG_MCU_STR(DBG_INFO, CAN_MODULE_ID,"\r\n");
}


static void SaveICCIDToEerprom(u8* pdata,u8 len)
{
	u8 i,t_u8buff[10];
    bool Check;

	if(len < 20)
	{
		DEBUG_MCU(DBG_ERROR, CAN_MODULE_ID, "CanDiag Recv 4G ICCID Len too Short,Len = %d \r\n", len);
		return;
	}

	Check = false;
	
	for(i=0;i<10;i++)
	{
		t_u8buff[i] = ChangeString09ToBCD(pdata[(i<<1)+0],pdata[(i<<1)+1]);
		if(t_u8buff[i] != eeprom_data.TboxICCID[i])
		{
			Check = true;
		}
	}

    if(Check)
	{
		SystemParaSaveEeprom(member_offset(EepromPara_STypeDef,TboxICCID),t_u8buff,10);
	}

	DEBUG_MCU(DBG_INFO, CAN_MODULE_ID, "CanDiag Recv 4G ICCID;");
    DEBUG_MCU_ARRAY(DBG_INFO, CAN_MODULE_ID, t_u8buff, 1, 10, 1);
    DEBUG_MCU_STR(DBG_INFO, CAN_MODULE_ID,"\r\n");
}


static void SaveNetTestResultToRam(u8* pdata,u8 len)
{
	if(len < 1)	
	{
		DEBUG_MCU(DBG_ERROR, CAN_MODULE_ID, "CanDiag Recv 4G NetTestResult Len too Short,Len = %d \r\n", len);
		return;
	}

	ram_data.NetworkStatus[0] = pdata[0];

	DEBUG_MCU(DBG_INFO, CAN_MODULE_ID, "CanDiag Recv 4G NetWordStatus = %d;", pdata[0]);
}


static void SaveGpsInforToRam(u8* pdata,u8 len)
{
	if(len < 9)
	{
		DEBUG_MCU(DBG_ERROR, CAN_MODULE_ID, "CanDiag Recv 4G GpsInformation Len too Short,Len = %d \r\n", len);
		return;
	}
	memcpy(&(ram_data.gstGnssBasic.Live_Longitude[0]),pdata,4);
	memcpy(&(ram_data.gstGnssBasic.Live_Latitude[0]),pdata+4,4);
	//pdata+8   speed
	memcpy(&(ram_data.gstGnssBasic.Live_Time[0]),pdata+9,4);
	memcpy(&(ram_data.gstGnssBasic.Live_Num[0]),pdata+13,1);
}
static void   Get4gSingle(u8* pdata,u8 len)//zhouxb
{
	if(len < 1)
	{
		return;
	}
	GSM_Signal = pdata[0];
}
static void   GetIMEI(u8* pdata,u8 len)//zhouxb
{
	if(len < 1)
	{
		return;
	}
	//memcpy(&(eeprom_data.TboxIMSI[0]),pdata,8);
	memcpy(&eeprom_data.TboxIMSI[0],pdata,15);
}

void CanDiagRec4GDiagDataHandle(const Message_t *pMsg)
{
	PtlDataEvt_t *pEvt =NULL;
    u8 Len;
	

	if(NULL != pMsg)
	{
		pEvt = (PtlDataEvt_t *)pMsg;

		Len = pEvt->Length;

		switch(pEvt->Data[0])
		{
			case PARK_MODE_CID:
			{
				SaveParkModeConfigToEeprom(&pEvt->Data[1],Len-1);
			}break;

			case TUID_CID:
			{
				SaveTUIDToEeprom(&pEvt->Data[1],Len-1);
			}break;
			
            case NET_TEST_CID:
        	{
        		SaveNetTestResultToRam(&pEvt->Data[1],Len-1);
        	}break;

			case NET_PARA_CID:
			{
				SavePLMNIDToEerprom(&pEvt->Data[1],Len-1);
			}break;

			case HW_VER_CID:
			{
				SaveHardVersionToEeprom(&pEvt->Data[1],Len-1);
			}break;
			
			case ICCID_CID:
			{
				SaveICCIDToEerprom(&pEvt->Data[1],Len-1);
			}break;
			
           		 case SW_VER_CID:
        		{
        			SaveSoftVersionToEeprom(&pEvt->Data[1],Len-1);
        		}break;
			
			case GPS_INF_CID:
			{
				SaveGpsInforToRam(&pEvt->Data[1],Len-1);
			}break;	
			case  FOURG_SIGNAL_CID:
			{
				Get4gSingle(&pEvt->Data[1],Len-1);
			}
				break;
			case   IMEI_CID:
			{
				GetIMEI(&pEvt->Data[1],Len-1);
			}
				break;
			default:
				break;
		}
	}
}

void CanDiagRecKeyEvtHandle(const Message_t *pMsg)
{
	TboxMsgTypeEvt_t * pEvt = NULL;
	UNECALL_INFORMATION unEcallInfor;
    sTime st_CurTime;
    u16 t_u16Years;
	u8 t_u8Hour;

	if(NULL != pMsg)
	{
		pEvt = (TboxMsgTypeEvt_t *)pMsg;

		if(ECALL_RESCUE_CID == pEvt->CID)
		{
			unEcallInfor.bits.TriggeringSource = 0x03;
		}
		else if(SRS_CID == pEvt->CID)
		{
			unEcallInfor.bits.TriggeringSource = 0x01;
		}
		else
		{
			return;
		}
		
		unEcallInfor.bits.OcurrenceCounter += 1;
		
		GetRTCTime(&st_CurTime);
        t_u16Years = ((u16)st_CurTime.YearH<<8)|st_CurTime.YearL;

		if(t_u16Years > 2000)
		{
			unEcallInfor.bits.Years = t_u16Years - 2000;
		}
		else
		{
			unEcallInfor.bits.Years = 0;
		}

		unEcallInfor.bits.Months = st_CurTime.Month;
		unEcallInfor.bits.Days = st_CurTime.Day;

        t_u8Hour = st_CurTime.Hour;
		
		if((3 == st_CurTime.HourFormat)
		  ||(1 == st_CurTime.HourFormat))
		{
			unEcallInfor.bits.Hour = t_u8Hour;
		}
		else if(2 == st_CurTime.HourFormat)
		{
			unEcallInfor.bits.Hour = t_u8Hour+12;
		}

		unEcallInfor.bits.Minute = st_CurTime.Min;
		unEcallInfor.bits.Second = st_CurTime.Sec;

		SystemParaSaveEeprom(member_offset(EepromPara_STypeDef,ECALLInfo),(u8 *)&unEcallInfor.ECallInfoBuff[0],sizeof(UNECALL_INFORMATION));
	}
}

void CanDiagRecMainVoltErrStateHandle(const Message_t *pMsg)
{
	ModuleDiagState_t *pEvt = NULL;
 
	if(NULL != pMsg)
	{
		pEvt = (ModuleDiagState_t *)pMsg;

		switch(pEvt->MsState)
		{
			case MAIN12V_UVP:       //No break here
			case MAIN12V_UVP_TOO_LOW:
			{
				DTCLvoltageProcess(TRUE);
			}break;
			
			case MAIN12V_OVP:		//No break here
			case MAIN12V_OVP_TOO_HIGH:
			{
				DTCHvoltageProcess(TRUE);
			}break;
			
			case MAIN12V_NORMAL:	
			{
				DTCLvoltageProcess(FALSE);
				DTCHvoltageProcess(FALSE);
			}break;
			
			default:
				break;
		}
	}
}

void CanDiagRecGpsErrStateHandle(const Message_t *pMsg)
{
	ModuleDiagState_t *pEvt = NULL;
 
	if(NULL != pMsg)
	{	
		pEvt = (ModuleDiagState_t *)pMsg;

		switch(pEvt->MsState)
		{
			case GPSANTNORMAL:
			{
				DTCGPSOpenProcess(FALSE);
				DTCGPSShortProcess(FALSE);
			}break;
			
			case GPSANTOPEN:
			{
				DTCGPSOpenProcess(TRUE);
			}break;
			
			case GPSANTSHORT:
			{
				DTCGPSShortProcess(TRUE);
			}break;

			default:
				break;
		}
	}
}

void CanDiagRecMicErrStateHandle(const Message_t *pMsg)
{
	ModuleDiagState_t *pEvt = NULL;
 
	if(NULL != pMsg)
	{	
		pEvt = (ModuleDiagState_t *)pMsg;

		switch(pEvt->MsState)
		{
			case MICNORMAL:
			{
				DTCMicOpenProcess(FALSE);
				DTCMicShortToPowerProcess(FALSE);
			}break;
			
			case MICOPEN:
			{
				DTCMicOpenProcess(TRUE);
			}break;
			
			case MICSHORT:
			{
				DTCMicShortToPowerProcess(TRUE);
			}break;

			default:
				break;
		}
	}	
}

void CanDiagRecEcallErrStateHandle(const Message_t *pMsg)
{
	ModuleDiagState_t *pEvt = NULL;
 
	if(NULL != pMsg)
	{	
		pEvt = (ModuleDiagState_t *)pMsg;

		switch(pEvt->MsState)
		{
			case ECALL_KEY_NORMAL:
			{
				DTCECALLPressProcess(FALSE);
			}break;
			
			case ECALL_KEY_STUCK:
			{
				DTCECALLPressProcess(TRUE);
			}break;

			default:
				break;
		}
	}		
}

u8 ChangeString09ToBCD(u8 dath, u8 datl)
{
    u8  ucdat;
    if((dath<'0') ||(dath>'9'))
    {
        dath='0';
    }
    if((datl<'0') ||(datl>'9'))
    {
        datl='0';
    }
#if 1
    ucdat = (dath - '0') <<4;
    ucdat = ucdat +  (datl - '0');
#else
	ucdat = (((dath - '0')<<4)|(datl - '0'));  //liubo change 20160925
#endif
    return ucdat;
}


static bool CheckParaDefaultIsSavedToEeprom(void)
{
	bool ret = false;
	
    if((PARASETFLAG1==eeprom_data.ParaSaveDefFlag1) 
	  &&(PARASETFLAG2==eeprom_data.ParaSaveDefFlag2))
    {
		ret = true;
    }
   
    return ret;

}

static void SetEepromDefaultPara(void)
{
    u8 t_u8buff[40];
    u16 i=0;

    u8 *paddr = (u8 *)&eeprom_data;
   

	paddr[member_offset(EepromPara_STypeDef,ParaSaveDefFlag1)] = PARASETFLAG1;
    paddr[member_offset(EepromPara_STypeDef,ParaSaveDefFlag2)] = PARASETFLAG2;

    memset((u8*)(paddr+member_offset(EepromPara_STypeDef,S_DTCErr)), 0, DTCLISTMAXNUM);

	memcpy((u8*)(paddr+member_offset(EepromPara_STypeDef,ECU_SoftwareVersion)),&def_ECU_SoftwareVersion[0], sizeof(def_ECU_SoftwareVersion));
    
    memcpy((u8*)(paddr+member_offset(EepromPara_STypeDef,ECU_HardwareVersion)),&def_ECU_HardwareVersion[0], sizeof(def_ECU_HardwareVersion));
    
    memcpy((u8*)(paddr+member_offset(EepromPara_STypeDef,ECU_SerialNumber)),&def_ECU_SerialNumber, sizeof(def_ECU_SerialNumber));
     
    memset((u8*)(paddr+member_offset(EepromPara_STypeDef,CAR_VinNumber)),0, 17);

	memcpy((u8*)(paddr+member_offset(EepromPara_STypeDef,VMDate)), &def_VMDate[0], 4);   
    
    memcpy((u8*)(paddr+member_offset(EepromPara_STypeDef,LocalFuncBitConf)),&def_LocalFuncBitConf[0], sizeof(def_LocalFuncBitConf));

	for(i=0; i<10; i++)
    {
        t_u8buff[i] = ChangeString09ToBCD(def_TBOXTerminalICCIDNumber[(i<<1)+0],def_TBOXTerminalICCIDNumber[(i<<1)+1]);
    }
    memcpy((u8*)(paddr+member_offset(EepromPara_STypeDef,TboxICCID)),&t_u8buff[0], 10);

	for(i=0; i<16; i++)
    {
        t_u8buff[i] = ChangeString09ToBCD(def_TBOXTerminalTUID[(i<<1)+0],def_TBOXTerminalTUID[(i<<1)+1]);
    }
	memcpy((u8*)(paddr+member_offset(EepromPara_STypeDef,TboxTUID)),&t_u8buff[0], 16);

	memset((u8*)(paddr+member_offset(EepromPara_STypeDef,ECALLInfo)), 0, sizeof(UNECALL_INFORMATION));

	memset((u8*)(paddr+member_offset(EepromPara_STypeDef,PLMNID)), 0, sizeof(STPLMN_ID_DATA));
    
	memset((u8*)(paddr+member_offset(EepromPara_STypeDef,ParkMode)), 0, 1);

	memcpy((u8*)(paddr+member_offset(EepromPara_STypeDef,TSP_URL)), &def_TSP_URL[0], sizeof(def_TSP_URL));
	
	memcpy((u8*)(paddr+member_offset(EepromPara_STypeDef,AppSW_Fingerprint)), (u8*)&def_AppSW_Fingerprint, 7);
	 
	SystemParaSaveEeprom(0,(u8*)&eeprom_data,sizeof(eeprom_data));
}

void PwrOnScanSystermPara(void)
{
	u8 tempbuff[40];
	u16 i;
	u16 templen=0;
	if(FALSE == CheckParaDefaultIsSavedToEeprom())
	{
		SetEepromDefaultPara();
	}
	
	SystemParaReadEeprom(member_offset(EepromPara_STypeDef,S_DTCErr),&DtcErrSaved[0], DTCLISTMAXNUM);

	Car_Control_FunctionConfig();
	Node_Miss_FunctionConfig();
}

static void CanDiagSendDIDdataTo4G(u16 u16DID)
{
	switch(u16DID)
	{
		case 0xF227:
		{
			CanDiagSendParkModeTo4G();
		}break;

		case 0xF1F6:
		{
			CanDiagSendTUIDToBB();
			CanDiagSendTUIDTo4G();
		}break;

		case 0xF089:
		{
			CanDiagSendHardVersionTo4G();
		}break;
		
		case 0xF190:
		{
			CanDiagSendVinTo4G();
		}break;
		
		default:
		 break;
	}
}


void  CAN_DIAG_Init(void)
{
	CAN_Nl_InitPowrOn();
	CAN_Al_InitPowrOn();
	DTCListAllInit();
	CAN_DiagGlobalVariableInit();
}
//CAN  Data Link Layer(DLL)<=====>Network Layer
u8 Can_Dll_Process( CAN_MSG_Type  *rxmsg ,u8 nlChn)
{
    CAN_Nl_RxProcess(&NL_Process[nlChn], rxmsg);
    return MCURET_OK;
}
//TX OK
void CAN_DIAG_TxConf(u8 ucPara)
{
    if( ucPara == 6u )
    {
        CAN_Nl_TxOk(&NL_Process[0]);
        //Al_DscConf();
        //CAN_Al_TxOk(&AL_Process[0]);
    }
}
//   10ms
void CAN_DIAG_Timer_Process(void)
{
	CAN_Nl_Timer_Process(&NL_Process[0]) ;//CAN  NL<======>Timer
	CAN_Al_Timer_Process(&AL_Process[0]);//CAN  AL<======>Timer
	
	CAN_Sid_ResetEcu_Timing();
	
	CAN_Uds_DtcxTimer();
}
//eeprom_data
void CAN_Did_W_CHANA_ECUSoftwareVersionNumberDataIdentifier(u8* prxdata,u16 rxlen)
{
	#if  1
	if(rxlen == 8)
	{
	  	 SystemParaSaveEeprom(member_offset(EepromPara_STypeDef,ECU_SoftwareVersion), prxdata, rxlen);
	}
	else
	{

	}
	#endif
}
void CAN_Did_W_CHANA_ECUHardwareVersionNumberDataIdentifier(u8* prxdata,u16 rxlen)
{
	#if  1
	if(rxlen == 8)
	{
	    SystemParaSaveEeprom(member_offset(EepromPara_STypeDef,ECU_HardwareVersion), prxdata, rxlen);
	}
	else
	{
		
	}
	#endif
}
void CAN_Did_W_ECUSerialNumberDataIdentifier(u8* prxdata,u16 rxlen)
{
	#if  1
	if(rxlen == 13)
	{
	   SystemParaSaveEeprom(member_offset(EepromPara_STypeDef,ECU_SerialNumber),prxdata, rxlen);
	}
	else
	{
		
	}
	#endif
}
void CAN_Did_W_TBOXTerminalICCIDNumber(u8* prxdata,u16 rxlen)
{
	#if  1
    u16 i;

	if(rxlen == 10)
	{
	    #if  0
	    for(i=0; i<rxlen; i++)
	    {
	        TBOXTerminalICCIDNumber[(i<<1)+0] = '0'+ ((prxdata[i]&0xf0)>>4);
	        TBOXTerminalICCIDNumber[(i<<1)+1] = '0'+ (prxdata[i]&0x0f);
	    }
		#endif  
		SystemParaSaveEeprom(member_offset(EepromPara_STypeDef,TboxICCID), prxdata, rxlen);
	}
	else
	{
		
	}
	#endif
}
void CAN_Did_W_TBOXTerminalIMSINumber(u8* prxdata,u16 rxlen)
{
	#if  1
    u16 i;
	u8 TempIMSI[8];
	
	if(rxlen == 8)
	{
		#if  0
	    for(i=0; i<rxlen; i++)
	    {
	        TBOXTerminalIMSINumber[(i<<1)+0] = '0'+ ((prxdata[i]&0xf0)>>4);
	        TBOXTerminalIMSINumber[(i<<1)+1] = '0'+ (prxdata[i]&0x0f);
	    }
		#endif
		SystemParaSaveEeprom(member_offset(EepromPara_STypeDef,TboxIMSI), prxdata, rxlen);
	}
	else if(rxlen == 15)
	{
		#if  0
		for(i=0; i<rxlen; i++)
	    {
			TBOXTerminalIMSINumber[i] = prxdata[i];
	    }
		TBOXTerminalIMSINumber[15] = '\0';
		
		for(i=0; i<8; i++)
		{
			TempIMSI[i] = ChangeString09ToBCD(TBOXTerminalIMSINumber[(i<<1)+0], TBOXTerminalIMSINumber[(i<<1)+1]);
		}
		#endif
		SystemParaSaveEeprom(member_offset(EepromPara_STypeDef,TboxIMSI), prxdata, rxlen);
	}
	else
	{
		
	}
	#endif
}
void CAN_Did_W_TBOXTerminalTUID(u8* prxdata,u16 rxlen)
{
	#if  1
    u16 i;
	u8 respframe[40] = {0};
	u16 length;
	if(rxlen == 16)
	{
	    #if  0
	    for(i=0; i<rxlen; i++)
	    {
	        TBOXTerminalTUID[(i<<1)+0] = '0'+ ((prxdata[i]&0xf0)>>4);
	        TBOXTerminalTUID[(i<<1)+1] = '0'+ (prxdata[i]&0x0f);
	    }
		#endif
	    SystemParaSaveEeprom(member_offset(EepromPara_STypeDef,TboxTUID), &prxdata[0] , rxlen);
	}
	else
	{
		
	}
	#endif
}
void CAN_Did_W_ParkModeConfig(u8* prxdata,u16 rxlen)
{
	#if  1
	if(rxlen == 1)
	{
		eeprom_data.ParkMode= prxdata[0];
		SystemParaSaveEeprom(member_offset(EepromPara_STypeDef,ParkMode), &(eeprom_data.ParkMode), rxlen);

		CanDiagSendParkModeTo4G();
	}
	else
	{
	
	}
	#endif
}
void CAN_Did_W_VINDataIdentifier(u8* prxdata,u16 rxlen)
{
	#if  1
	if(rxlen == 17)
	{
	    SystemParaSaveEeprom(member_offset(EepromPara_STypeDef,CAR_VinNumber), prxdata, rxlen);
	}
	else
	{
		
	}
	#endif
}
void CAN_Did_W_CarPNIdentifier(u8* prxdata,u16 rxlen)
{
	#if  1
	if(rxlen == 21)
	{
	    SystemParaSaveEeprom(member_offset(EepromPara_STypeDef,CarPNINumber), prxdata, rxlen);
	}
	else
	{
		
	}
	#endif
}
#if   1
static u8* pSaveFileData[1] = {NULL};
#define   FileStartFlg    0xaa55
#define   FileEndFlg      0x55aa
static u8   filetype=0;
/** CRC table for the CRC-16. The poly is 0x8005 (x^16 + x^15 + x^2 + 1) */
u16 const Upcomputer_crc16_table[256] = {
	0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
	0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
	0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
	0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
	0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
	0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
	0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
	0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
	0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
	0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
	0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
	0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
	0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
	0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
	0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
	0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
	0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
	0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
	0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
	0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
	0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
	0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
	0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
	0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
	0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
	0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
	0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
	0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
	0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
	0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
	0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
	0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

u16 Upcomputer_crc16_byte(u16 crc, const u8 data)
{
	return (crc >> 8) ^ Upcomputer_crc16_table[(crc ^ data) & 0xff];
}

/**
 * crc16 - compute the CRC-16 for the data buffer
 * @crc:	previous CRC value
 * @buffer:	data pointer
 * @len:	number of bytes in the buffer
 *
 * Returns the updated CRC value.
 */
u16 Upcomputer_crc16(u16 crc, const u8 *buffer, u32 len)
{
	while (len--)
		crc = Upcomputer_crc16_byte(crc, *buffer++);
	return crc;
}

void CAN_Did_W_CHANA_FILEDataIdentifier(u8* prxdata,u16 rxlen,u16 num)//Œƒº˛ ˝æ›∑÷∞¸¥´ ‰±£¥Ê
	{
		//u32	FILEDataAddr=0;
		u16   CRC16Check=0;
	
		static u8 file_packet=0;
		static u8 write_statu=0;
		u8 i=0;
	#if  0
		// ∂±.cer	.key
		//u32  Num=0;	 .cer
		//œ»∞¥’’…œ≤„»Ìº˛–≠“È‘≠±£¥Ê
		pSaveFileData[0]=NVM_GetStartAddress(NVM_EELNAME(CANDiagstore_Infor_4));
		memcpy( &(FILE_Data.FILEData[0u]), prxdata, rxlen);
		//FILEDataAddr=member_offset(FILEDataPara_STypeDef,FILEData)+64*num;
		FILEDataAddr=pSaveFileData[0]+64*num;
		SystemParaSaveEeprom(FILEDataAddr, &FILE_Data.FILEData[0], rxlen);
		
		//u32  Num=0;	 .key
	#else
		memcpy( &(FILE_Data.FILEData[0u]), prxdata, rxlen);
	
		//if(FILE_Data.FILEData[0]==0x04)
		if(FILE_Data.FILEData[0]==0x24)
		{
				if(FILE_Data.FILEData[1]==0x01)//Œƒº˛√˚°¢ø™ º
				{
					CRC16Check = Upcomputer_crc16(CRC16Check,&FILE_Data.FILEData[6], (FILE_Data.FILEData[5]<<8|FILE_Data.FILEData[4]));//–£—È
					if(CRC16Check==(FILE_Data.FILEData[rxlen-2]<<8|FILE_Data.FILEData[rxlen-3]))
					{
						write_statu = 0;
						filetype = 0;
	
						for(i=6;i<50;i++)
						{
							if(FILE_Data.FILEData[i]==0x2e)
							{
								if(FILE_Data.FILEData[i+1]==0x63)//c
								{
									file_packet = FILE_Data.FILEData[6];
									filetype=2;
									break;
								}
								else if(FILE_Data.FILEData[i+1]==0x70)//p
								{
									file_packet = FILE_Data.FILEData[6];
									filetype=1;
									break;
								}
							}
						}
						
						if(filetype==1)
						{
							filetype=1;
							filetransfer_OK &= (~0x01);
							if(NVM_SwdlErase(NVM_ROMNAME(secretkey)) == NVM_SWDL_OPERATER_SUCCESS)
							{
									if(NVM_SwdlWrite_CAN(NVM_ROMNAME(secretkey),(u32 *)&FILE_Data.FILEData[6],\
									16,num)!= NVM_SWDL_OPERATER_SUCCESS)
									{
									write_statu |= 0x01;
									}
								if(NVM_SwdlWrite_CAN(NVM_ROMNAME(secretkey),(u32 *)&FILE_Data.FILEData[6+64],\
									2,num+1)!= NVM_SWDL_OPERATER_SUCCESS)
									{
									write_statu |= 0x01;
									}
							}
							else
							{
								write_statu |= 0x01;
							}
						}
						else if(filetype==2)
						{
							filetype=2;
							filetransfer_OK &= (~0x02);
							
							if(NVM_SwdlErase(NVM_ROMNAME(certificate)) == NVM_SWDL_OPERATER_SUCCESS)
							{
								if(NVM_SwdlWrite_CAN(NVM_ROMNAME(certificate),(u32 *)&FILE_Data.FILEData[6],\
									16,num)!= NVM_SWDL_OPERATER_SUCCESS)
									{
									write_statu |= 0x02;
									}
								if(NVM_SwdlWrite_CAN(NVM_ROMNAME(certificate),(u32 *)&FILE_Data.FILEData[6+64],\
									2,num+1)!= NVM_SWDL_OPERATER_SUCCESS)
									{
									write_statu |= 0x02;
									}
							}
							else
							{
								write_statu |= 0x02;
							}						
						}	
					}
					else
					{
					}
				}
				else  if(FILE_Data.FILEData[1]==0x02)//Œƒº˛ ˝æ›   CANDiagstore_Infor_5-37
				{
					CRC16Check = Upcomputer_crc16(CRC16Check,&FILE_Data.FILEData[6], (FILE_Data.FILEData[5]<<8|FILE_Data.FILEData[4]));//–£—È
					if(CRC16Check==(FILE_Data.FILEData[rxlen-2]<<8|FILE_Data.FILEData[rxlen-3]))
					{
						if(filetype==1)
						{
						
							if(NVM_SwdlWrite_CAN(NVM_ROMNAME(secretkey),(u32 *)&FILE_Data.FILEData[6],16,num+1) == NVM_SWDL_OPERATER_SUCCESS)
							{
	
								if((file_packet == num) && (!(write_statu&0x01)))
								{
									filetransfer_OK |= 0x01;
									//return 0;
								}
							}
							else
							{
								write_statu |= 0x01;
							}
								
							//NVM_SwdlWrite_CAN(NVM_ROMNAME(secretkey),&FILE_Data.FILEData[6+64],16,(num*4)+1);
							
							//NVM_SwdlWrite_CAN(NVM_ROMNAME(secretkey),&FILE_Data.FILEData[6+(64*2)],16,(num*4)+2);
	
							//NVM_SwdlWrite_CAN(NVM_ROMNAME(secretkey),&FILE_Data.FILEData[6+(64*3)],16,(num*4)+3);
						}
						else if(filetype==2)
						{
							
							if(NVM_SwdlWrite_CAN(NVM_ROMNAME(certificate),(u32 *)&FILE_Data.FILEData[6],16,num+1) == NVM_SWDL_OPERATER_SUCCESS)
							{
	
								if((file_packet == num) && (!(write_statu&0x02)))
								{
									filetransfer_OK |= 0x02;
									//return 0;
								}
							}
							else
							{
								write_statu |= 0x02;
							}
							//NVM_SwdlWrite_CAN(NVM_ROMNAME(certificate),&FILE_Data.FILEData[6+64],16,(num*4)+1);
							
							//NVM_SwdlWrite_CAN(NVM_ROMNAME(certificate),&FILE_Data.FILEData[6+(64*2)],16,(num*4)+2);
	
							//NVM_SwdlWrite_CAN(NVM_ROMNAME(certificate),&FILE_Data.FILEData[6+(64*3)],16,(num*4)+3);
						}
							
					}
					else
					{
					}
				}
				else  if(FILE_Data.FILEData[1]==0x03)//Œƒº˛–£—È¬Î
				{
					CRC16Check = Upcomputer_crc16(CRC16Check,&FILE_Data.FILEData[6], (FILE_Data.FILEData[5]<<8|FILE_Data.FILEData[4]));//–£—È
					if(CRC16Check==(FILE_Data.FILEData[rxlen-2]<<8|FILE_Data.FILEData[rxlen-3]))
					{
						//filetype=0;
					}
					else
					{
					}
				}
				else  if(FILE_Data.FILEData[1]==0x04)//Ω· ¯
				{
					CRC16Check = Upcomputer_crc16(CRC16Check,&FILE_Data.FILEData[6], (FILE_Data.FILEData[5]<<8|FILE_Data.FILEData[4]));//–£—È
					if(CRC16Check==(FILE_Data.FILEData[rxlen-2]<<8|FILE_Data.FILEData[rxlen-3]))
					{
						//filetype=0;
					}
					else
					{
					}
				}
		}
	#endif
	
	
		
	}

#endif
void CAN_Did_R_CHANA_ECUSoftwareVersionNumberDataIdentifier(u8* ptxdata,u16 *ptxlen)
{
    #if  1
    u16 i;
    for(i=0; i<8; i++)
    {
        ptxdata[i]=eeprom_data.ECU_SoftwareVersion[i];
    }
    *ptxlen +=8;
	#endif
}
void CAN_Did_R_CHANA_ECUHardwareVersionNumberDataIdentifier(u8* ptxdata,u16 *ptxlen)
{
	#if  1
    u16 i;
    for(i=0; i<8; i++)
    {
        ptxdata[i]=eeprom_data.ECU_HardwareVersion[i];
    }
    *ptxlen +=8;
	#endif
}
void CAN_Did_R_ECUSerialNumberDataIdentifier(u8* ptxdata,u16 *ptxlen)
{
	#if  1 //test  zhoxb
	memcpy(ptxdata, eeprom_data.ECU_SerialNumber.Product_Serial_Number, 4u);
	memcpy(ptxdata+4, eeprom_data.ECU_SerialNumber.Production_Line_Number, 6u);
	memcpy(ptxdata+10, eeprom_data.ECU_SerialNumber.ProductDate, 3u);
    *ptxlen+=13;
	#endif
}
void CAN_Did_R_TBOXTerminalICCIDNumber(u8* ptxdata,u16*ptxlen)
{
    u16 i;
#if  0
    for(i=0; i<10; i++)
    {
        //ptxdata[i] = ChangeString09ToBCD(TBOXTerminalICCIDNumber[(i<<1)+0],TBOXTerminalICCIDNumber[(i<<1)+1]);
        ptxdata[i]=eeprom_data.TboxICCID[i];
    }
	#else
	u8  ptxdata_Temp[10]={0x89,0x86,0x01,0x17,0x75,0x00,0x09,0x07,0x87,0x35};
 	for(i=0; i<10; i++)
    	{
        	ptxdata[i]=ptxdata_Temp[i];
   	 }
	#endif
  	  *ptxlen += 10;
}
void CAN_Did_R_TBOXTerminalIMSINumber(u8* ptxdata,u16*ptxlen)
{
	#if  1
    u16 i;
    //for(i=0; i<8; i++)
    for(i=0; i<15; i++)
    {
       // ptxdata[i] = ChangeString09ToBCD(eeprom_data.TboxIMSI[(i<<1)+0],eeprom_data.TboxIMSI[(i<<1)+1]);
       
      ptxdata[i]=eeprom_data.TboxIMSI[i];
    }
    // *ptxlen +=8;
    *ptxlen +=15;
	#endif
}
void CAN_Did_R_TBOXTerminalTUID(u8* ptxdata,u16*ptxlen)
{
	#if  1
    u16 i;
    for(i=0; i<16; i++)
    {
       // ptxdata[i] = ChangeString09ToBCD(TBOXTerminalTUID[(i<<1)+0],TBOXTerminalTUID[(i<<1)+1]);
        ptxdata[i]=eeprom_data.TboxTUID[i];
    }
    *ptxlen +=16;
	#endif
}
void CAN_Did_R_ParkModeConfig(u8* ptxdata,u16* ptxlen)
{
	#if  1
	ptxdata[0] =eeprom_data.ParkMode;
	*ptxlen += 1;
	#endif
}
void CAN_Did_R_NetworkDetectStatus(u8* ptxdata,u16* ptxlen)
{
	#if  1
    ptxdata[0] =ram_data.NetworkStatus[0];// gNetwork_Detect_Status;
    *ptxlen += 1;
	#endif
}
void CAN_Did_R_Gnss_Basic(u8* ptxdata,u16* ptxlen)//zhouxb
{
	u8   Longitude_temp0=0,Longitude_temp1=0,Longitude_temp2=0,Longitude_temp3=0;
	u8   Latitude_temp0=0,Latitude_temp1=0,Latitude_temp2=0,Latitude_temp3=0;

	ptxdata[0]=ram_data.gstGnssBasic.Live_Longitude[3];
	ptxdata[1]=ram_data.gstGnssBasic.Live_Longitude[2];
	ptxdata[2]=ram_data.gstGnssBasic.Live_Longitude[1];
	ptxdata[3]=ram_data.gstGnssBasic.Live_Longitude[0];

	ptxdata[4]=ram_data.gstGnssBasic.Live_Latitude[3];
	ptxdata[5]=ram_data.gstGnssBasic.Live_Latitude[2];
	ptxdata[6]=ram_data.gstGnssBasic.Live_Latitude[1];
	ptxdata[7]=ram_data.gstGnssBasic.Live_Latitude[0];

	ptxdata[8]=ram_data.gstGnssBasic.Live_Time[0];
	ptxdata[9]=ram_data.gstGnssBasic.Live_Time[1];
	ptxdata[10]=ram_data.gstGnssBasic.Live_Time[2];
	ptxdata[11]=ram_data.gstGnssBasic.Live_Time[3];

	ptxdata[12]=  ram_data.gstGnssBasic.Live_Num[0];

    *ptxlen += 13;
}
void CAN_Did_R_GSM(u8* ptxdata,u16* ptxlen)
{
	#if   1
	ptxdata[0] = GSM_Signal;
	
	*ptxlen += 1;
	#endif
}
void CAN_Did_R_CLASSD(u8* ptxdata,u16* ptxlen)//zhouxb  add
{
	#if   1
	ptxdata[0] = GetTAS5411Status();
	
	*ptxlen += 1;
	#endif
}

void CAN_Did_R_VINDataIdentifier(u8* ptxdata,u16*ptxlen)
{
	#if  1
    //memcpy(ptxdata,&(VINDataIdentifier[0u]), 17u);
    memcpy(ptxdata,&(eeprom_data.CAR_VinNumber[0u]), 17u);
    *ptxlen+=17;
	#endif
}
void CAN_Did_R_CarPNIDataIdentifier(u8* ptxdata,u16*ptxlen)
{
	#if   1
   	memcpy(ptxdata,&(eeprom_data.CarPNINumber[0u]), 21u);
   	 *ptxlen+=21;
	#endif
}

void CAN_Did_R_FileTransResult(u8* ptxdata,u16* ptxlen)
{
	#if   1
	ptxdata[0] = filetransfer_OK;
	
	*ptxlen += 1;
	#endif
}
void CAN_Did_W_FileTransResult(u8* prxdata,u16 rxlen)
{
	#if  1
	if(rxlen == 1)
	{
		filetransfer_OK = prxdata[0];
	}
	else
	{
		TRACE_APL("[Error] Liubo: FileTransResult Lenth==%d \r\n",rxlen);
	}
	#endif
}
void CAN_Did_R_CANetWorkStatus(u8* ptxdata,u16* ptxlen)
{
	#if   1
	ptxdata[0] = CANetWork_status;
	
	*ptxlen += 1;
	#endif
}

void CAN_Did_W_CANetWorkStatus(u8* ptxdata,u16 rxlen)
{
	#if   1

	if(rxlen == 1)
	{
		CANetWork_status = ptxdata[0];
	}

	#endif
}



#if CAN_YUANTEL_HOST_ENABLE
//extern xQueueHandle keyQueuePlus;
void SetFactoryMode(void)
{
	#if   1
	u8 *paddr;
	paddr = (u8 *)&eeprom_data;
	
	//memcpy((u8*)(paddr+member_offset(EepromPara_STypeDef,CAR_VinNumber)),&DEF_VINDataIdentifier[0], 17);
	//memcpy((u8*)(paddr+member_offset(EepromPara_STypeDef,CarModelType)),&DEF_CarModelTypeConfiguration[0], 1);
	//memcpy((u8*)(paddr+member_offset(EepromPara_STypeDef,LocalFuncBitConf)),&DEF_LocalFuncConf[0], 6);
	
	//memcpy((u8*)(paddr+member_offset(EepromPara_STypeDef,TSP_URL)), &DEF_TSP_URL[0], 64);
	//memset((u8*)(paddr+member_offset(EepromPara_STypeDef,ECallAudioOutControl)), DEF_AudioOutControl, 1);
	//memcpy((u8*)(paddr+member_offset(EepromPara_STypeDef,APN_Name)), &DEF_APN_Name[0], 64);
	//memcpy((u8*)(paddr+member_offset(EepromPara_STypeDef,APN_User)), &DEF_APN_User[0], 32);
	//memcpy((u8*)(paddr+member_offset(EepromPara_STypeDef,APN_Password)), &DEF_APN_Password[0], 32);
	//memset((u8*)(paddr+member_offset(EepromPara_STypeDef,CAN_BaudRate)), 0, 1);
	
	//SystemParaSaveEeprom(0,(u8*)&eeprom_data,sizeof(eeprom_data));
	#endif
}

//’Ô∂œ∑˛ŒÒπ¶ƒ‹«Î«ÛSID     ≤˙œﬂºÏ≤‚
u8 CAN_Al_DiagnosticFunctionRequest(u8 *ucData)
{
	#if   1
	u8 ucRet = 0;
	
	switch(ucData[0])
	{
		case 0x01:
		{
			SetFactoryMode();//zhouxb ≤Œ ˝…Ë÷√µΩEEP
		}
		break;
		case 0x02:
		{
		}
		break;
		case 0x03:
		{
		}
		break;
		case 0x04://≤‚ ‘≤¶¥ÚµÁª∞
		{
			SendRequestCallToOther(ucData,12);//0x04+telephone  num
		}
		break;
		case 0x05:
		{
			SendRequestNetTestToOther();
		}
		break;
		case 0x06://«Î«ÛºÏ≤‚Õ¯¬Á-ÀΩÕ¯
		{
			u8 senddata[10] = {0};
			senddata[0] = 5;///lenght
			senddata[1] = 0x11;
			senddata[2] = 0x12;
			senddata[3] = 0xff;
			senddata[4] = 0x02;
			senddata[5] = 0;/// match state
			DEBUG_MCU(DBG_INFO, CAN_MODULE_ID,"Request CPU test CA NETwork\r\n");
			Rte_IWrite_Fun_TboxLogic_Call_UARTTX_Buff_UARTTX_Buff(senddata);	
		}
		break;
		case 0x07:
		{
		}
		break;
		case 0x08:
		{
			CAN_Sid_ResetEcu(1);
		}
		break;
		default:
		{
			ucRet = 0x31;
		}
		break;
	}

	return ucRet;
	#endif
}

u8 CAN_Al_DiagnosticWriteEEPROM(u8* pdata, u16 len)
{
	#if   1
	u8 ucRet = 0;
	
	switch(pdata[1])
	{
		case 0x01:
		{
	        	CAN_Did_W_CHANA_ECUSoftwareVersionNumberDataIdentifier((u8*)&(pdata[CAN_YUANTEL_HOST_HEAD_LENGTH]), ECU_SW_VER_LENGTH);
		}
        	break;
		case 0x02:
		{
			CAN_Did_W_CHANA_ECUHardwareVersionNumberDataIdentifier((u8*)&(pdata[CAN_YUANTEL_HOST_HEAD_LENGTH]), ECU_HW_VER_LENGTH);
		}
		break;
		case 0x03:
		{
			CAN_Did_W_ECUSerialNumberDataIdentifier((u8*)&(pdata[CAN_YUANTEL_HOST_HEAD_LENGTH]), ECU_SERIAL_NUMBER_LENGTH);
		}
		break;
		case 0x04:
		{
			CAN_Did_W_TBOXTerminalICCIDNumber((u8*)&(pdata[CAN_YUANTEL_HOST_HEAD_LENGTH]), (ICCID_LENGTH>>1));
		}
		break;
		case 0x05:
		{
			CAN_Did_W_TBOXTerminalIMSINumber((u8*)&(pdata[CAN_YUANTEL_HOST_HEAD_LENGTH]), (IMSI_LENGTH>>1));
		}
		break;
		case 0x06:
		{
			CAN_Did_W_TBOXTerminalTUID((u8*)&(pdata[CAN_YUANTEL_HOST_HEAD_LENGTH]), (TUID_LENGTH>>1));
		}
		break;
		case 0x07:
		{
			CAN_Did_W_ParkModeConfig((u8*)&(pdata[CAN_YUANTEL_HOST_HEAD_LENGTH]), PARK_MODE_LENGTH);
		}
		break;
		case 0x0b:
		{
			CAN_Did_W_VINDataIdentifier((u8*)&(pdata[CAN_YUANTEL_HOST_HEAD_LENGTH]), VIN_DATA_LENGTH);
		}
		break;
		case 0x0e:
		{
			CAN_Did_W_CarPNIdentifier((u8*)&(pdata[CAN_YUANTEL_HOST_HEAD_LENGTH]), CarPNI_DATA_LENGTH);
		}
		break;
		default:
		{
			ucRet = 0x31;
		}
		break;
	}

	return ucRet;
	#endif
}


u8 CAN_Al_DiagnosticReadEEPROM(u8* prxdata,u16 rxIndex,u8* ptxdata,u16 *ptxlen)
{
	#if  1
    u8 temp;
    u16  usHostDid;
	
    temp = MCURET_OK;	
    usHostDid = prxdata[rxIndex];
   //u8 Cleardata = 0;
    switch(usHostDid)
    {
    case 0x01:
	        ptxdata[0u] =prxdata[rxIndex];
	        *ptxlen +=1;
	        CAN_Did_R_CHANA_ECUSoftwareVersionNumberDataIdentifier(&ptxdata[1],ptxlen);
        break;
	case 0x02:
	        ptxdata[0u] =prxdata[rxIndex];
	        *ptxlen +=1;
	        CAN_Did_R_CHANA_ECUHardwareVersionNumberDataIdentifier(&ptxdata[1],ptxlen);
        break;
    case 0x03:
	        ptxdata[0u] =prxdata[rxIndex];
	        *ptxlen +=1;
	       CAN_Did_R_ECUSerialNumberDataIdentifier(&ptxdata[1],ptxlen);
        break;   
    case 0x04:
	        ptxdata[0u] =prxdata[rxIndex];
	        *ptxlen +=1;
	       CAN_Did_R_TBOXTerminalICCIDNumber(&ptxdata[1],ptxlen);
        break;
    case 0x05:
	        ptxdata[0u] =prxdata[rxIndex];
	        *ptxlen +=1;
	       CAN_Did_R_TBOXTerminalIMSINumber(&ptxdata[1],ptxlen);
        break;
    case 0x06:
	        ptxdata[0u] =prxdata[rxIndex];
	        *ptxlen +=1;
	       CAN_Did_R_TBOXTerminalTUID(&ptxdata[1],ptxlen);
        break;
	case 0x07:
		ptxdata[0u] =prxdata[rxIndex];
		*ptxlen +=1;
		CAN_Did_R_ParkModeConfig(&ptxdata[1],ptxlen);
	break;
	case 0x08:
		ptxdata[0u] =prxdata[rxIndex];
		*ptxlen +=1;
		CAN_Did_R_NetworkDetectStatus(&ptxdata[1],ptxlen);
	break;
	case 0x09:
		ptxdata[0u] =prxdata[rxIndex];
		*ptxlen +=1;
		CAN_Did_R_Gnss_Basic(&ptxdata[1],ptxlen);
		break;
	case 0x0a:
		ptxdata[0u] =prxdata[rxIndex];
		*ptxlen +=1;
		CAN_Did_R_GSM(&ptxdata[1],ptxlen);
		break;
	case 0x0b:
		ptxdata[0u] =prxdata[rxIndex];
		*ptxlen +=1;
		CAN_Did_R_VINDataIdentifier(&ptxdata[1],ptxlen);
		break;
	case 0x0c:
		ptxdata[0u] =prxdata[rxIndex];
		*ptxlen +=1;
		CAN_Did_R_FileTransResult(&ptxdata[1],ptxlen);
		//CAN_Did_W_FileTransResult(&Cleardata,1);
		break;
	case 0x0d:
		ptxdata[0u] =prxdata[rxIndex];
		*ptxlen +=1;
		CAN_Did_R_CANetWorkStatus(&ptxdata[1],ptxlen);
		break;
	case  0x0e://zhouxb 20171101  7A6  ¡„≤øº˛∫≈   
		ptxdata[0u] =prxdata[rxIndex];
		*ptxlen +=1;
		CAN_Did_R_CarPNIDataIdentifier(&ptxdata[1],ptxlen);
		break;
	case  0x0f://zhouxb 20171025 «Î«ÛºÏ≤‚π¶∑≈£¨ªÒ»°π¶∑≈ «∑Ò”–π ’œ
	{
		ptxdata[0u] =prxdata[rxIndex];
		*ptxlen +=1;
		CAN_Did_R_CLASSD(&ptxdata[1],ptxlen);
	}
		break;
    default:
        temp=MCURET_ERR;
        break;
    }
    return temp;
	#endif
}
//extern xQueueHandle keyQueueFile;
#define  CMD_TransFer_File             0x24               //zhouxb  add
#define  CMD_TransFer_File_Resp   0xa4  

u8 CAN_Al_DiagnosticTransferFile(u8* pdata, u16 len)
{
	#if   1
	u8 ucRet = 0;
	static u16 lastsn = 0;
	switch(pdata[1])
	{
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		{
		       	//send client.cer TODO
		       	u16 sn = *(u16 *)&pdata[2];
		    //Ω´Œƒº˛–≈œ¢Õ∏¥´◊™∑¢∏¯4Gƒ£øÈ
		    	pdata[0] = CMD_TransFer_File;//zhouxb  CMD_TransFer_File
			
			if((sn!=0)&&(sn==lastsn))
			{
					
			}
			else
			{
				#if   1
				SendFileMsgToOther(pdata,len);
				#else
				/*…œŒªª˙∑÷∞¸œ¬∑¢Œƒº˛ ˝æ›£¨√ø∞¸64BYTE£¨MCU±æµÿ±£¥Ê*/
				CAN_Did_W_CHANA_FILEDataIdentifier(pdata,len,sn);
				#endif
			}
			lastsn = sn;
		}
        	break;
		default:
		{
			ucRet = 0x31;
		}
		break;
	}

	return ucRet;
	#endif
}


#endif
//
static void  SendFileMsgToOther(u8* pdata, u16 len)//zhouxb  20171107  add
{
	TboxFileTypeEvt_t  FileMsg;

    	FileMsg.Msg.Type = EVENT_TBOX_SEND_FILE_CMD;
	FileMsg.Length = len+1;
	FileMsg.CID = 0x10;//CAN…˝º∂
	
	memcpy(&FileMsg.File[0],pdata,len);
	if(1 == SendMsgToComTbox((u8 *)(&FileMsg), sizeof(TboxFileTypeEvt_t)))
	{

	}
	else
	{

	}
}
static void  SendRequestCallToOther(u8* pdata, u16 len)//zhouxb  20171113  add
{
	TboxSendRequestCallEvt_t  RequestCallMsg;

    	RequestCallMsg.Msg.Type = EVENT_TBOX_SEND_DIAG_DATA;
	RequestCallMsg.Length = len+1;//
    	RequestCallMsg.CID = MK_TESTCALL_CID;

	memcpy(&RequestCallMsg.Call[0],pdata,len);
	if(1 == SendMsgToComTbox((u8 *)(&RequestCallMsg), sizeof(TboxSendRequestCallEvt_t)))
	{
		// DEBUG_MCU(DBG_INFO, VEH_MODULE_ID, "File Send To Com OK!!!\r\n");		
	}
	else
	{
		//DEBUG_MCU(DBG_ERROR, VEH_MODULE_ID, "File Send To Com Fail!!!\r\n");
	}
}
static void  SendRequestNetTestToOther()//zhouxb  20171113  add
{
	TboxSendRequestNetTestEvt_t  NetTestMsg;

    	NetTestMsg.Msg.Type = EVENT_TBOX_SEND_DIAG_DATA;
	NetTestMsg.Length = 2;//
    	NetTestMsg.CID = NET_TEST_CID;

	if(1 == SendMsgToComTbox((u8 *)(&NetTestMsg), sizeof(TboxSendRequestNetTestEvt_t)))
	{
		// DEBUG_MCU(DBG_INFO, VEH_MODULE_ID, "File Send To Com OK!!!\r\n");		
	}
	else
	{
		//DEBUG_MCU(DBG_ERROR, VEH_MODULE_ID, "File Send To Com Fail!!!\r\n");
	}
}

/************************************************
*			Local function							*
************************************************/
//ALL init function interface
//init NL
void CAN_Nl_InitPowrOn(void)
{
    CAN_Nl_Init_STypeDef  NlPwrOnInit;

    NlPwrOnInit.RxNPDUidPhys =CAN_Diag_PhysicalRequests_ID;
    NlPwrOnInit.RxNPDUidFunc =CAN_Diag_FunctionalRequests_ID;
    NlPwrOnInit.TxNPDUidDiag = CAN_Diag_PhysicalResponses_ID;
    NlPwrOnInit.N_ArTimer = NL_N_Ar_MAX;
    NlPwrOnInit.N_BrTimer = NL_N_Br_MAX;
    NlPwrOnInit.N_CrTimer = NL_N_Cr_MAX;
    NlPwrOnInit.N_AsTimer = NL_N_As_MAX;
    NlPwrOnInit.N_BsTimer = NL_N_Bs_MAX;
    NlPwrOnInit.N_CsTimer = NL_N_Cs_MAX;
    NlPwrOnInit.BsSet = NL_BS_DEFAULT;
    NlPwrOnInit.STminSet = NL_STMIN_DEFAULT;//NL_STMIN_DEFAULT_TIME;

    CAN_Nl_Initx(&NL_Process[0],&NlPwrOnInit,&NL_RxBuff[0],&NL_TxBuff[0]);
}
//init AL
void CAN_Al_InitPowrOn(void)
{
    CAN_Al_Ctrl_Init(&AL_Process[0]);
}

/*
* DID function
*/
void CAN_R_DID(u16 DIDnum, u8* ptxdata, u16 *ptxlen)
{
	u8 i;
	u32 UtcSec = 0;

    if((DIDnum == 0xF213)||(DIDnum == 0xF220))
	{
		sTime stRtc;
		GetRTCTime(&stRtc);
		UtcSec = ConvertRtcToSecCnt(stRtc);

	    memcpy(&ram_data.gSystemTime[0],(u8*)&UtcSec,4);
	}
		
	for(i=0;i<DIDLISTMAXNUM;i++)
	{
		if(DIDnum == DIDInfoTab[i].DID_num)
		{
			if(DIDInfoTab[i].saveType == SaveIn_ROM)
			{
				memcpy(ptxdata, (u8 *)DIDInfoTab[i].address, DIDInfoTab[i].DataLength);
			}else if(DIDInfoTab[i].saveType == SaveIn_RAM)
			{
				memcpy(ptxdata, (u8 *)((u8 *)&ram_data+DIDInfoTab[i].address), DIDInfoTab[i].DataLength);
			}else if(DIDInfoTab[i].saveType == SaveIn_BackupRAM)
			{
				memcpy(ptxdata, (u8 *)((u8 *)&backupRam_data+DIDInfoTab[i].address), DIDInfoTab[i].DataLength);
			}else if(DIDInfoTab[i].saveType == SaveIn_EEPROM)
			{
				memcpy(ptxdata, (u8 *)((u8 *)&eeprom_data+DIDInfoTab[i].address), DIDInfoTab[i].DataLength);
			}
			*ptxlen += DIDInfoTab[i].DataLength;
			break;
		}
	}
}
u8 CAN_W_DID(u16 DIDnum, u8* prxdata,u16 rxlen)
{
	u8 i;
	u8 status = MCURET_OK;
	for(i=0;i<DIDLISTMAXNUM;i++)
	{
		if(DIDnum == DIDInfoTab[i].DID_num)
		{
			if(true == DIDInfoTab[i].EnWrite)
			{
				if(rxlen == DIDInfoTab[i].DataLength)
				{
					if(SaveIn_EEPROM == DIDInfoTab[i].saveType)
					{
						SystemParaSaveEeprom((u16)(DIDInfoTab[i].address), prxdata, DIDInfoTab[i].DataLength);
					}else if(SaveIn_RAM== DIDInfoTab[i].saveType)
					{
						memcpy((u8 *)((u8 *)&ram_data+DIDInfoTab[i].address), prxdata, DIDInfoTab[i].DataLength);
					}else if(SaveIn_BackupRAM== DIDInfoTab[i].saveType)
					{
						memcpy((u8 *)((u8 *)&backupRam_data+DIDInfoTab[i].address), prxdata, DIDInfoTab[i].DataLength);
					}else 
					{
						
					}
				}else
				{
					//write length error
					status = MCURET_ERR;
				}
			}else
			{
				//disable write
			}

			break;
		}
	}

	return status;
}
/* end DID */
/*
* Routine control
*/
#if  1
void CAN_RCDid_DialPhoneNumber(u8* prxdata,u16 rxlen, u8 *ptxdata,u16* txlen)
{

}

void  CAN_RCDid_TestSpeaker(u8* prxdata,u16 rxlen, u8 *ptxdata,u16* txlen)
{

}

void CAN_RCDid_ActDevicePro(u8* prxdata,u16 rxlen, u8 *ptxdata,u16* txlen)
{
    
}

void CAN_RCDid_ChgDevicePro(u8* prxdata,u16 rxlen, u8 *ptxdata,u16* txlen)
{
    
}

void CAN_RCDid_UnlockDoor(u8* prxdata,u16 rxlen, u8 *ptxdata,u16* txlen)
{
    
}

void CAN_RCDid_LockDoor(u8* prxdata,u16 rxlen, u8 *ptxdata,u16* txlen)
{
    
}

void CAN_RCDid_StartAC(u8* prxdata,u16 rxlen, u8 *ptxdata,u16* txlen)
{
    
}

void CAN_RCDid_StopAC(u8* prxdata,u16 rxlen, u8 *ptxdata,u16* txlen)
{
    
}

void CAN_RCDid_CarRemind(u8* prxdata,u16 rxlen, u8 *ptxdata,u16* txlen)
{

}

void CAN_RCDid_CallTest(u8* prxdata,u16 rxlen, u8 *ptxdata,u16* txlen)
{

}
#endif
/****************************** DTC function*********************************/
//local function
//init DTC
void DTCListAllInit(void)
{
    u16 i;
    Dtc_STypeDef* ptrDtcCtrlx;

    for(i=0; i<DTCLISTMAXNUM; i++)
    {
        if((void*)0 !=DtcInfoConfTab[i].pDtcctrlx)
        {
            ptrDtcCtrlx=DtcInfoConfTab[i].pDtcctrlx;
	     	ptrDtcCtrlx->EnableFlag=true;
			
            DTCxInit(&DtcInfoConfTab[i]);
        }
		else
		{
            ptrDtcCtrlx=DtcInfoConfTab[i].pDtcctrlx;
            ptrDtcCtrlx->EnableFlag=false;		
		}
    }
}
void DTCxInit(DtcInfoConf_STypeDef* pDtcInfoConf)
{
    Dtc_STypeDef* ptrDtcCtrlx;
    ptrDtcCtrlx=pDtcInfoConf->pDtcctrlx;

    //ptrDtcCtrlx->EnableFlag=true;
    ptrDtcCtrlx->NowInfoRecordFlag = false;
    ptrDtcCtrlx->HistoryInfoRecordFlag = false;
    ptrDtcCtrlx->NoErrFlag =  false;
    ptrDtcCtrlx->ConfEnableOrNotFlag =  false;
    //ptrDtcCtrlx->TimeStartFlag=false;
    ptrDtcCtrlx->BasicTimer = pDtcInfoConf->BasicTimeDef;
    ptrDtcCtrlx->WarnTimer = pDtcInfoConf->WarnTimeDef;
    ptrDtcCtrlx->CancelWarnTimer = pDtcInfoConf->CancelWarnTimeDef;
    ptrDtcCtrlx->ModeType = pDtcInfoConf->ModeTypeDef;
    ptrDtcCtrlx->Status.data = 0x00;
}
void DTCListAllResetTimer(void)
{
    u16 i;
    Dtc_STypeDef* ptrDtcCtrlx;

    for(i=0; i<DTCLISTMAXNUM; i++)
    {
        if((void*)0 !=DtcInfoConfTab[i].pDtcctrlx)
        {
            ptrDtcCtrlx = DtcInfoConfTab[i].pDtcctrlx;
			ptrDtcCtrlx->BasicTimer = DtcInfoConfTab[i].BasicTimeDef;
			ptrDtcCtrlx->WarnTimer = DtcInfoConfTab[i].WarnTimeDef;
			ptrDtcCtrlx->CancelWarnTimer = DtcInfoConfTab[i].CancelWarnTimeDef;
			ptrDtcCtrlx->NoErrFlag = true;
        }
    }
}

bool getDTCConfig(u32 DTCNum)
{
	u8 ret;
	
	switch(DTCNum)
	{
		case 0x00D17287: //2b0
			ret = NodeMissConfig.bits.NodeOf_GW_2B0_Missing;
			break;
		case 0x00D17387: //320
			ret = NodeMissConfig.bits.NodeOf_GW_320_Missing;
			break;
		case 0x00D17587: //32A
			ret = NodeMissConfig.bits.NodeOf_GW_32A_Missing;
		    break;
		case 0x00D17487: //326
			ret = NodeMissConfig.bits.NodeOf_GW_326_Missing;
			break;
		case 0x00D15687: //280
			ret = NodeMissConfig.bits.NodeOf_IP_280_Missing;
			break;
		case 0x00D1AD87: //380
			ret = NodeMissConfig.bits.NodeOf_IP_380_Missing;
			break;
		case 0x00D14787: //288
			ret = NodeMissConfig.bits.NodeOf_BCM_288_Missing;
			break;
		case 0x00D14887: //347
			ret = NodeMissConfig.bits.NodeOf_BCM_347_Missing;
			break;
		case 0x00D19A87: //384
			ret = NodeMissConfig.bits.NodeOf_BCM_384_Missing;
			break;
		case 0x00D15887: //266
			ret = NodeMissConfig.bits.NodeOf_PEPS_266_Missing;
			break;
		case 0x00D19687: //500
			ret = NodeMissConfig.bits.NodeOf_DVD_500_Missing;
			break;
		case 0x00D19787: //501
			ret = NodeMissConfig.bits.NodeOf_DVD_501_Missing;
			break;
		case 0x00D14587: //330
			ret = NodeMissConfig.bits.NodeOf_AC_330_Missing;
			break;
		case 0x00D19C87: //260
			ret = NodeMissConfig.bits.NodeOf_IP_260_Missing;
			break;
		case 0x00D19587: //2F8
			ret = NodeMissConfig.bits.NodeOf_DVD_2F8_Missing;
			break;
		default:
			ret = TRUE;
		    break;
	}

	return ret;
	
}



void ScanDtcEnable(void)
{
	Dtc_STypeDef* ptrDtcCtrlx;
	DtcInfoConf_STypeDef*  ptrDtcInfoConf;
	u8 i;

	for(i = 0; i < DTCLISTMAXNUM; i++)
	{
		ptrDtcCtrlx = DtcInfoConfTab[i].pDtcctrlx;
		ptrDtcInfoConf = & DtcInfoConfTab[i];


		if(ptrDtcInfoConf->SetAllow == 1)//if enable config
		{
			if((void*)0 !=ptrDtcInfoConf->pDtcctrlx)
			{
				ptrDtcCtrlx->ConfEnableOrNotFlag = getDTCConfig(ptrDtcInfoConf->number);
				if(0 == ptrDtcCtrlx->ConfEnableOrNotFlag)//disable
				{
					ptrDtcCtrlx->EnableFlag=false;
				}else
				{
					ptrDtcCtrlx->EnableFlag=true;
				}
			}
		}
	}	
}

u8 AccJump = 0;//ACC OFF IS 0;Acc Off to On jump is 1;Acc On alway is 2
u16 AccJump10msCnt = 0;

u8 Can_Uds_AccJumpGetStatus(void)
{
	return (AccJump);
}
void Can_Uds_AccJump10msCnt(void)
{
	if (AccJump == 1)
	{
		AccJump10msCnt++;
		if (AccJump10msCnt >= 300)
		{
			AccJump = 2;
			AccJump10msCnt = 0;
		}
	}
}
void  Can_Uds_AccOffToOnProcess(void)
{	
	if (CAN_MCU_Interface.gblIndAccOn)
	{
		if (AccJump == 0)
		{
			AccJump = 1;
			AccJump10msCnt = 0;
		}
	}
	else
	{
		AccJump = 0;
		AccJump10msCnt = 0;
	}
}
u8 Get_BattVoltageAbortSt(void)
{
	if((BattVoltage_LowFlg == 1) ||(BattVoltage_HighFlg == 1))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
void DTCVoltageAbortTimerProcess(void)
{

	if(BattVoltage_HighDelayTimer)
	{
		BattVoltage_HighDelayTimer--;
		if(BattVoltage_HighDelayTimer == 0)
		{
			BattVoltage_HighDelayTimer = 0;
			if(BattVoltage_HighFlg == FALSE)
			{
				BattVoltage_HighFlg = TRUE;
				DTCListAllResetTimer();
				if(DtcErrSaved[DispB2003] != 1)
				{
					DtcErrSaved[DispB2003]=1;
					SystemParaSaveEeprom(member_offset(EepromPara_STypeDef, S_DTCErr)+DispB2003, &DtcErrSaved[DispB2003], 1);
				}
				DTCConfirmThingEventIsErr(&DtcInfoConfTab[DispB2003]);
			}
		}
	}

	if(BattVoltage_HighRecoveryTimer)
	{
		BattVoltage_HighRecoveryTimer--;
		if(BattVoltage_HighRecoveryTimer == 0)
		{
			BattVoltage_HighRecoveryTimer = 0;
			if(BattVoltage_HighFlg == TRUE)
			{
				BattVoltage_HighFlg = FALSE;
			}
			DTCConfirmThingEventIsNoErr(&DtcInfoConfTab[DispB2003]);
		}
	}

	if(BattVoltage_LowDelayTimer)
	{
		BattVoltage_LowDelayTimer--;
		if(BattVoltage_LowDelayTimer == 0)
		{
			BattVoltage_LowDelayTimer = 0;
			if(BattVoltage_LowFlg == FALSE)
			{
				BattVoltage_LowFlg = TRUE;
				DTCListAllResetTimer();
				if(DtcErrSaved[DispB2004] != 1)
				{
					DtcErrSaved[DispB2004]=1;
					SystemParaSaveEeprom(member_offset(EepromPara_STypeDef, S_DTCErr)+DispB2004, &DtcErrSaved[DispB2004], 1);
				}
				DTCConfirmThingEventIsErr(&DtcInfoConfTab[DispB2004]);
			}
		}
	}

	if(BattVoltage_LowRecoveryTimer)
	{
		BattVoltage_LowRecoveryTimer--;
		if(BattVoltage_LowRecoveryTimer == 0)
		{
			BattVoltage_LowRecoveryTimer = 0;
			if(BattVoltage_LowFlg == TRUE)
			{
				BattVoltage_LowFlg = FALSE;
			}
			DTCConfirmThingEventIsNoErr(&DtcInfoConfTab[DispB2004]);

		}
	}

}
/*
* DTC timer process 10ms
*/
void CAN_Uds_DtcxTimer(void)
{
	Dtc_STypeDef* ptrDtcCtrlx;
	DtcInfoConf_STypeDef*  ptrDtcInfoConf;
	u16 i;

	if(AL_ControlDTCSetting_OFF == AL_Process[0].DTCSetType)
	{
		return;
	}
	
	DTCVoltageAbortTimerProcess();
	ScanDtcEnable();
	Can_Uds_AccOffToOnProcess( );	
	Can_Uds_AccJump10msCnt( );
	if(Get_BattVoltageAbortSt()||(gblCanSleepFlag == true) || (Can_Uds_AccJumpGetStatus() != 2))return;

	for(i = 0; i < DTCLISTMAXNUM; i++)
	{
		ptrDtcCtrlx = DtcInfoConfTab[i].pDtcctrlx;
		ptrDtcInfoConf = & DtcInfoConfTab[i];

		if((void*)0 !=ptrDtcInfoConf->pDtcctrlx)
		{
			if(true == ptrDtcCtrlx->EnableFlag)
			{
				if(ptrDtcCtrlx->BasicTimer > 0 ) ptrDtcCtrlx->BasicTimer --;
				if(ptrDtcCtrlx->BasicTimer==0)
				{
					ptrDtcCtrlx->BasicTimer = ptrDtcInfoConf->BasicTimeDef;
					if(DTC_PERIOD_MODE == ptrDtcCtrlx->ModeType) //period
					{
						if(true == ptrDtcCtrlx->NoErrFlag)
						{
							ptrDtcCtrlx->WarnTimer = ptrDtcInfoConf->WarnTimeDef;
							ptrDtcCtrlx->NoErrFlag = false;
							ptrDtcCtrlx->Status.bits.TestFailed = 0;
							if(true == ptrDtcCtrlx->NowInfoRecordFlag)
							{
								if(ptrDtcCtrlx->CancelWarnTimer > 0 )  ptrDtcCtrlx->CancelWarnTimer--;
								if(ptrDtcCtrlx->CancelWarnTimer == 0)
								{
									ptrDtcCtrlx->HistoryInfoRecordFlag = true;
									ptrDtcCtrlx->NowInfoRecordFlag = false;
									ptrDtcCtrlx->WarnTimer = ptrDtcInfoConf->WarnTimeDef;
									ptrDtcCtrlx->CancelWarnTimer = ptrDtcInfoConf->CancelWarnTimeDef;
								}
							}
						}else//error happen
						{
							if(ptrDtcCtrlx->WarnTimer > 0 )  ptrDtcCtrlx->WarnTimer--;
							if( ptrDtcCtrlx->WarnTimer == 0)
							{
								if(false == ptrDtcCtrlx->NowInfoRecordFlag)
								{
								//ptrDtcCtrlx->Status.bits.TestFailed = 1;
								// ptrDtcCtrlx->Status.bits.ConfirmedDTC = 1;
									ptrDtcCtrlx->Status.data = 0x09;	//reduce instruct time
								//save eeprom
									if(DtcErrSaved[i] != 1)
									{
										DtcErrSaved[i]=1;
										SystemParaSaveEeprom(member_offset(EepromPara_STypeDef, S_DTCErr)+i, &DtcErrSaved[i], 1);
									}
								}
								ptrDtcCtrlx->NowInfoRecordFlag = true;
								ptrDtcCtrlx->WarnTimer = 0;
								ptrDtcCtrlx->CancelWarnTimer = ptrDtcInfoConf->CancelWarnTimeDef;
							}
						}
					}
				}
			}
		}
	}
}
/*
* DTC confirm
*/
//Period
void DTCConfirmThingPeriodIsNoErr(DtcInfoConf_STypeDef *pDtcInfoConf)
{
    Dtc_STypeDef *ptrDtcCtrlx;
	if((void*)0 !=pDtcInfoConf->pDtcctrlx)
	{
	    ptrDtcCtrlx =  pDtcInfoConf->pDtcctrlx;
	    if(ptrDtcCtrlx->EnableFlag == true)
	    {
	        ptrDtcCtrlx->NoErrFlag =  true;
	    }
	}
}
void DTCConfirmThingPeriodIsErr(DtcInfoConf_STypeDef *pDtcInfoConf)
{
    Dtc_STypeDef *ptrDtcCtrlx;
	if((void*)0 !=pDtcInfoConf->pDtcctrlx)
	{
	    ptrDtcCtrlx =  pDtcInfoConf->pDtcctrlx;
	    if(ptrDtcCtrlx->EnableFlag == true)
	    {
	        ptrDtcCtrlx->NoErrFlag =  false;
	    }
	}
}
//event
void DTCConfirmThingEventIsErr(DtcInfoConf_STypeDef *pDtcInfoConf)
{
    Dtc_STypeDef *ptrDtcCtrlx;
	if((void*)0 !=pDtcInfoConf->pDtcctrlx)
	{
	    ptrDtcCtrlx =  pDtcInfoConf->pDtcctrlx;
	    //ptrDtcCtrlx->NoErrFlag =  TRUE;
	    if(ptrDtcCtrlx->EnableFlag == true)
	    {
	        ptrDtcCtrlx->NowInfoRecordFlag = true;
	        ptrDtcCtrlx->Status.bits.TestFailed= 1;
			ptrDtcCtrlx->Status.bits.ConfirmedDTC= 1;
	    }
	}
}
void DTCConfirmThingEventIsNoErr(DtcInfoConf_STypeDef *pDtcInfoConf)
{
    Dtc_STypeDef *ptrDtcCtrlx;
	if((void*)0 !=pDtcInfoConf->pDtcctrlx)
	{
	    ptrDtcCtrlx =  pDtcInfoConf->pDtcctrlx;
	    //ptrDtcCtrlx->NoErrFlag =  FALSE;
	    if(ptrDtcCtrlx->EnableFlag == true)
	    {
	        if(ptrDtcCtrlx->NowInfoRecordFlag == true)
	        {
	            ptrDtcCtrlx->HistoryInfoRecordFlag = true;
				//ptrDtcCtrlx->Status.bits.ConfirmedDTC= 1;
	        }
	        ptrDtcCtrlx->NowInfoRecordFlag = false;
			ptrDtcCtrlx->Status.bits.TestFailed = 0;
	    }
	}
}
//global function
void  DTCCanBusOffProcess(u8 type)
{
	if(Can_Uds_AccJumpGetStatus() != 2)return;
	if(AL_ControlDTCSetting_OFF == AL_Process[0].DTCSetType)
	{
		return;
	}
	
	if(type) 
	{
		DTCConfirmThingEventIsErr(&DtcInfoConfTab[DispU001088]);
		if(DtcErrSaved[DispU001088] != 1)
		{
			DtcErrSaved[DispU001088]=1;
			SystemParaSaveEeprom(member_offset(EepromPara_STypeDef, S_DTCErr)+DispU001088, &DtcErrSaved[DispU001088], 1);
		}
	}
	else 
	{
		DTCConfirmThingEventIsNoErr(&DtcInfoConfTab[DispU001088]);
	}
}
//period
void  DTCRxCANID2B0Process(void)
{
    DTCConfirmThingPeriodIsNoErr(&DtcInfoConfTab[DispU117287]);
}
void  DTCRxCANID320Process(void)
{
    DTCConfirmThingPeriodIsNoErr(&DtcInfoConfTab[DispU117387]);
}
void  DTCRxCANID32AProcess(void)
{
    DTCConfirmThingPeriodIsNoErr(&DtcInfoConfTab[DispU117587]);
}

void  DTCRxCANID326Process(void)
{
    DTCConfirmThingPeriodIsNoErr(&DtcInfoConfTab[DispU117487]);
}
void  DTCRxCANID280Process(void)
{
    DTCConfirmThingPeriodIsNoErr(&DtcInfoConfTab[DispU115687]);
}
void  DTCRxCANID380Process(void)
{
    DTCConfirmThingPeriodIsNoErr(&DtcInfoConfTab[DispU11AD87]);
}
void  DTCRxCANID288Process(void)
{
    DTCConfirmThingPeriodIsNoErr(&DtcInfoConfTab[DispU114787]);
}
void  DTCRxCANID347Process(void)
{
    DTCConfirmThingPeriodIsNoErr(&DtcInfoConfTab[DispU114887]);
}
void  DTCRxCANID384Process(void)
{
    DTCConfirmThingPeriodIsNoErr(&DtcInfoConfTab[DispU119A87]);
}
/*
* add by jason
*/
void  DTCRxCANID266Process(void)
{
    DTCConfirmThingPeriodIsNoErr(&DtcInfoConfTab[DispU115887]);
}

void  DTCRxCANID512Process(void)
{
    DTCConfirmThingPeriodIsNoErr(&DtcInfoConfTab[DispU116287]);
}

void  DTCRxCANID500Process(void)
{
    DTCConfirmThingPeriodIsNoErr(&DtcInfoConfTab[DispU119687]);
}

void  DTCRxCANID501Process(void)
{
    DTCConfirmThingPeriodIsNoErr(&DtcInfoConfTab[DispU119787]);
}
void  DTCRxCANID330Process(void)
{
    DTCConfirmThingPeriodIsNoErr(&DtcInfoConfTab[DispU114587]);
}

void  DTCRxCANID260Process(void)
{
    DTCConfirmThingPeriodIsNoErr(&DtcInfoConfTab[DispU119C87]);
}
void  DTCRxCANID2F8Process(void)
{
    DTCConfirmThingPeriodIsNoErr(&DtcInfoConfTab[DispU119587]);
	
}
//event
void  DTCGPSOpenProcess(u8 type)
{
	if(AL_ControlDTCSetting_OFF == AL_Process[0].DTCSetType)
	{
		return;
	}

    if(type) 
 	{
		if(DtcErrSaved[DispB2000] != 1)
		{
			DtcErrSaved[DispB2000]=1;
			SystemParaSaveEeprom(member_offset(EepromPara_STypeDef, S_DTCErr)+DispB2000, &DtcErrSaved[DispB2000], 1);
		}
	
 		DTCConfirmThingEventIsErr(&DtcInfoConfTab[DispB2000]);
 	}
    else DTCConfirmThingEventIsNoErr(&DtcInfoConfTab[DispB2000]);
}
void  DTCGPSShortProcess(u8 type)
{
	if(AL_ControlDTCSetting_OFF == AL_Process[0].DTCSetType)
	{
		return;
	}

     if(type) 
 	{
		if(DtcErrSaved[DispB2001] != 1)
		{
			DtcErrSaved[DispB2001]=1;
			SystemParaSaveEeprom(member_offset(EepromPara_STypeDef, S_DTCErr)+DispB2001, &DtcErrSaved[DispB2001], 1);
		}
		DTCConfirmThingEventIsErr(&DtcInfoConfTab[DispB2001]);
 	}
    else DTCConfirmThingEventIsNoErr(&DtcInfoConfTab[DispB2001]);
}
void  DTCHvoltageProcess(u8 type)
{
	static u8 Hvolt = FALSE;
	
	if(type == TRUE)
	{
		if(Hvolt == FALSE)
		{
			Hvolt = TRUE;
			BattVoltage_HighRecoveryTimer = 0;
			BattVoltage_HighDelayTimer = (2000u/NL_BASICTIME);
		}

	}
	else if(type == FALSE)
	{
		if(Hvolt == TRUE)
		{
			Hvolt = FALSE;
			BattVoltage_HighDelayTimer = 0;
			BattVoltage_HighRecoveryTimer = (1500u/NL_BASICTIME);;
		}

	}
	
}
void  DTCLvoltageProcess(u8 type)
{
	static u8 Lvolt = FALSE;

	if(AL_ControlDTCSetting_OFF == AL_Process[0].DTCSetType)
	{
		return;
	}
	
	if(type == TRUE)
	{
		if(Lvolt == FALSE)
		{
			Lvolt = TRUE;
			BattVoltage_LowRecoveryTimer = 0;
			BattVoltage_LowDelayTimer = (2000u/NL_BASICTIME);
		}

	}
	else if(type == FALSE)
	{
		if(Lvolt == TRUE)
		{
			Lvolt = FALSE;
			BattVoltage_LowDelayTimer = 0;
			BattVoltage_LowRecoveryTimer = (1500u/NL_BASICTIME);;
		}

	}
}
void  DTCECALLPressProcess(u8 type)
{
	if(AL_ControlDTCSetting_OFF == AL_Process[0].DTCSetType)
	{
		return;
	}

     if(type)
	{
		if(DtcErrSaved[DispB200B] != 1)
		{
			DtcErrSaved[DispB200B]=1;
			SystemParaSaveEeprom(member_offset(EepromPara_STypeDef, S_DTCErr)+DispB200B, &DtcErrSaved[DispB200B], 1);
		}
		DTCConfirmThingEventIsErr(&DtcInfoConfTab[DispB200B]);
	}
    else DTCConfirmThingEventIsNoErr(&DtcInfoConfTab[DispB200B]);
}
void  DTCMicOpenProcess(u8 type)
{
	if(AL_ControlDTCSetting_OFF == AL_Process[0].DTCSetType)
	{
		return;
	}

     if(type)
     {
     	if(DtcErrSaved[DispB200E] != 1)
		{
			DtcErrSaved[DispB200E]=1;
			SystemParaSaveEeprom(member_offset(EepromPara_STypeDef, S_DTCErr)+DispB200E, &DtcErrSaved[DispB200E], 1);
		}
	 	DTCConfirmThingEventIsErr(&DtcInfoConfTab[DispB200E]);
     }
     else DTCConfirmThingEventIsNoErr(&DtcInfoConfTab[DispB200E]);
}
void  DTCMicShortToPowerProcess(u8 type)
{
	if(AL_ControlDTCSetting_OFF == AL_Process[0].DTCSetType)
	{
		return;
	}

     if(type) 
 	{
		if(DtcErrSaved[DispB200F] != 1)
		{
			DtcErrSaved[DispB200F]=1;
			SystemParaSaveEeprom(member_offset(EepromPara_STypeDef, S_DTCErr)+DispB200F, &DtcErrSaved[DispB200F], 1);
		}
		DTCConfirmThingEventIsErr(&DtcInfoConfTab[DispB200F]);
 	}
    else DTCConfirmThingEventIsNoErr(&DtcInfoConfTab[DispB200F]);
}
void  DTCEepromFailureProcess(u8 type)
{
	if(AL_ControlDTCSetting_OFF == AL_Process[0].DTCSetType)
	{
		return;
	}

	if(type) 
	{
		if(DtcErrSaved[DispB200C] != 1)
		{
			DtcErrSaved[DispB200C]=1;
			SystemParaSaveEeprom(member_offset(EepromPara_STypeDef, S_DTCErr)+DispB200C, &DtcErrSaved[DispB200C], 1);
		}
		DTCConfirmThingEventIsErr(&DtcInfoConfTab[DispB200C]);
 	}
    else DTCConfirmThingEventIsNoErr(&DtcInfoConfTab[DispB200C]);
}
void  DTCPartFaultProcess(u8 type)
{
    	if(AL_ControlDTCSetting_OFF == AL_Process[0].DTCSetType)
	{
		return;
	}

	if(type) 
	{
		if(DtcErrSaved[DispB2028] != 1)
		{
			DtcErrSaved[DispB2028]=1;
			SystemParaSaveEeprom(member_offset(EepromPara_STypeDef, S_DTCErr)+DispB2028, &DtcErrSaved[DispB2028], 1);
		}
		DTCConfirmThingEventIsErr(&DtcInfoConfTab[DispB2028]);
 	}
    else DTCConfirmThingEventIsNoErr(&DtcInfoConfTab[DispB2028]);
}
/****************************** end of DTC function *****************************/

/****************************** SID function ************************************/
void CAN_Sid_ResetEcu(u8 rxcmd)
{
	Al_Ctrl_STypeDef *AL_Ctrl;

	AL_Ctrl = &AL_Process[0];

	AL_Ctrl->EcuR.EcuReset_Type = rxcmd;
	
	if(AL_Ctrl->EcuR.EcuReset_Type == 0x01)
	{
		AL_Ctrl->EcuR.EcuReset_Timer = (100/AL_BASICTIME);
	}
	else if(AL_Ctrl->EcuR.EcuReset_Type == 0x02)
	{
	}
	else if(AL_Ctrl->EcuR.EcuReset_Type == 0x03)
	{
		AL_Ctrl->EcuR.EcuReset_Timer = (100/AL_BASICTIME);
	}
	else; 
}
void CAN_Sid_ResetEcu_Timing(void)
{
	Al_Ctrl_STypeDef *AL_Ctrl;

	AL_Ctrl = &AL_Process[0];

	if(AL_Ctrl->EcuR.EcuReset_Timer)
	{
		AL_Ctrl->EcuR.EcuReset_Timer--;
		if(AL_Ctrl->EcuR.EcuReset_Timer == 0)
		{
			if(AL_Ctrl->EcuR.EcuReset_Type == 0x01)
			{
				SoftReset();
			}
			else if(AL_Ctrl->EcuR.EcuReset_Type == 0x02)
			{
			}
			else if(AL_Ctrl->EcuR.EcuReset_Type == 0x03)
			{
				SoftReset();	
			}
			else;
		}
	}
		
}


void CAN_Sid_SetIdAppAndNMCtrlType(u8 ctrltype,u8  comtype)
{
    if(0==ctrltype)//EnableRxAndTx
    {
        if((1==comtype)||(3==comtype))
        {
            CANDiagCCCtrl.DisableAppMsgRx=false;
            CANDiagCCCtrl.DisableAppMsgTx=false;
        }
        if((2==comtype)||(3==comtype))
        {
            CANDiagCCCtrl.DisableNMMsgRx=false;
            CANDiagCCCtrl.DisableNMMsgTx=false;
        }
    }
    else if(1==ctrltype)//EnableRxAndDisableTx
    {
        if((1==comtype)||(3==comtype))
        {
            CANDiagCCCtrl.DisableAppMsgRx=false;
            CANDiagCCCtrl.DisableAppMsgTx=true;
        }
        if((2==comtype)||(3==comtype))
        {
            CANDiagCCCtrl.DisableNMMsgRx=false;
            CANDiagCCCtrl.DisableNMMsgTx=true;
        }
    }
    else if(2==ctrltype)//DisableRxAndEnableTx
    {
        if((1==comtype)||(3==comtype))
        {
            CANDiagCCCtrl.DisableAppMsgRx=true;
            CANDiagCCCtrl.DisableAppMsgTx=false;
        }
        if((2==comtype)||(3==comtype))
        {
            CANDiagCCCtrl.DisableNMMsgRx=true;
            CANDiagCCCtrl.DisableNMMsgTx=false;
        }
    }
    else if(3==ctrltype)//DisableRxAndTx
    {
        if((1==comtype)||(3==comtype))
        {
            CANDiagCCCtrl.DisableAppMsgRx=true;
            CANDiagCCCtrl.DisableAppMsgTx=true;
        }
        if((2==comtype)||(3==comtype))
        {
            CANDiagCCCtrl.DisableNMMsgRx=true;
            CANDiagCCCtrl.DisableNMMsgTx=true;
        }
    }
    else 
	{
	}

    DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"Daig Command CC  Ctrl  Apprx--%d,Apptx--%d,NMrx--%d,NMtx--%d\r\n",\
	CANDiagCCCtrl.DisableAppMsgRx, CANDiagCCCtrl.DisableAppMsgTx,CANDiagCCCtrl.DisableNMMsgRx,CANDiagCCCtrl.DisableNMMsgTx);
}

void CAN_Sid_SetDtcCtrlType(u8* prxdata,u16 rxlen)
{
    if(prxdata[0] == 0x01 )
    {
        UdsDtcCtrlIsOffFlag=false;
    }
    else
    {
        UdsDtcCtrlIsOffFlag=true;
    }
}

u8  CAN_Sid_ReadDtcInfoCheckInvalidFormat(u8 sbufunc,u16 rxlen)
{
    u8 ucSubFunction;
    ucSubFunction=sbufunc;
    if(((ucSubFunction==1)||(ucSubFunction==2))&&(rxlen!=3))
    {
        return MCURET_ERR;
    }
    //else if(((ucSubFunction==3)||(ucSubFunction==4)||(ucSubFunction==6))&&(rxlen!=6))
    //{
    //    return MCURET_ERR;
    //}
    else if((ucSubFunction==0x0A)&&(rxlen!=2))
    {
        return MCURET_ERR;
    }
    else;
    return MCURET_OK ;
}
u8  CAN_Sid_ReadDtcInfoChecksubfuncNotSupported(u8 sbufunc)
{
    u8 ucSubFunction;
    ucSubFunction=sbufunc;

    //if((ucSubFunction!=1)&&(ucSubFunction!=2)&&(ucSubFunction!=3)&&(ucSubFunction!=4)&&(ucSubFunction!=6)&&(ucSubFunction!=0x0a))
	if((ucSubFunction==1) || (ucSubFunction==2)  || (ucSubFunction==0x0a))
    {
        return MCURET_OK ;
    }
    return MCURET_ERR ;
}
u16 CheckDtcCountNum(u8 SMask)
{
	Dtc_STypeDef* ptrDtcCtrlx;
	DtcInfoConf_STypeDef*  ptrDtcInfoConf;
	u16 i,j;
	u8 temp;
	//u8 status;
	for(i=0,j=0; i<DTCLISTMAXNUM; i++)
	{
		ptrDtcCtrlx = DtcInfoConfTab[i].pDtcctrlx;
		ptrDtcInfoConf = & DtcInfoConfTab[i];

		if((void*)0 !=ptrDtcInfoConf->pDtcctrlx)
		{
			if(true == ptrDtcCtrlx->EnableFlag)
			{
				if(DtcErrSaved[i] == 1)
				{
					ptrDtcCtrlx->Status.bits.ConfirmedDTC = 1;
				}
				temp =(u8) (ptrDtcCtrlx->Status.data);
				if((temp & SMask)!=0)
				{
					j++;
				}
			}
		}
	}
	return j;
}
void CAN_Sid_ReadDtcInfo(u8 sbufunc ,u8* prxdata,u8* ptxdata,u16* ptxlen)
{
	u8 ucSubFunction;
	u8 DTCStatusAvailabilityMask;
	u8 DTCStatusMask;
	u8  SMask;
	u8 DTCFormatIdentifier;
	u16 DTCNum;
	Dtc_STypeDef* ptrDtcCtrlx;
	DtcInfoConf_STypeDef*  ptrDtcInfoConf;

	u16 i,j,len;

	ucSubFunction=sbufunc;
	len = *ptxlen;
	DTCStatusMask=prxdata[2];
	DTCStatusAvailabilityMask=0x09;//
	SMask = DTCStatusMask & DTCStatusAvailabilityMask;
	if(ucSubFunction==0x01)
	{
		// ReportNumberOfDTCByStatusMask()
		ptxdata[0]=DTCStatusAvailabilityMask;
		// 00   ISO15031-6DTCFormat
		// 01   ISO14229-1DTCFormat
		// 02   SAEJ1939-73DTCFormat
		DTCFormatIdentifier=0x01;
		ptxdata[1]=DTCFormatIdentifier;
		// DTCCount
		DTCNum=CheckDtcCountNum(SMask);
		ptxdata[2]=GetU16HighU8(DTCNum);
		ptxdata[3]=GetU16LowU8(DTCNum);
		len +=4;
		*ptxlen = len;
	} else if(ucSubFunction==0x02)
	{
		ptxdata[0]=DTCStatusAvailabilityMask;
		len +=1;
		for(i=0,j=1; i<DTCLISTMAXNUM; i++)
		{
			ptrDtcCtrlx = DtcInfoConfTab[i].pDtcctrlx;
			ptrDtcInfoConf = & DtcInfoConfTab[i];
			if((void*)0 !=ptrDtcInfoConf->pDtcctrlx)
			{
				if(true == ptrDtcCtrlx->EnableFlag)
				{
					if(DtcErrSaved[i] == 1)
					{
						ptrDtcCtrlx->Status.bits.ConfirmedDTC = 1;
					}
					if((ptrDtcCtrlx->Status.data & SMask)!=0)
					{
						ptxdata[j++] = (u8)(ptrDtcInfoConf->number >> 16);
						ptxdata[j++] = (u8)(ptrDtcInfoConf->number >> 8);
						ptxdata[j++] = (u8)(ptrDtcInfoConf->number);
						ptxdata[j++] = ptrDtcCtrlx->Status.data;
						len += 4;
					}

				}
			}
		}
		*ptxlen=len;
	} else if(ucSubFunction==0x0A)
	{
		ptxdata[0]=DTCStatusAvailabilityMask;
		len +=1;
		for(i=0,j=1; i<DTCLISTMAXNUM; i++)
		{
			ptrDtcCtrlx = DtcInfoConfTab[i].pDtcctrlx;
			ptrDtcInfoConf = & DtcInfoConfTab[i];
			if((void*)0 !=ptrDtcInfoConf->pDtcctrlx)
			{
				if(DtcErrSaved[i] == 1)
				{
					ptrDtcCtrlx->Status.bits.ConfirmedDTC = 1;
				}

				ptxdata[j++]=(u8)(ptrDtcInfoConf->number>>16);
				ptxdata[j++]=(u8)(ptrDtcInfoConf->number>>8);
				ptxdata[j++]=(u8)(ptrDtcInfoConf->number);
				ptxdata[j++]=ptrDtcCtrlx->Status.data;
				len +=4;
			}
		}
		*ptxlen=len;
	}else;

}
//ClearDiagnosticInformation
u8 CAN_Sid_ClearDtcInfo(u32 DTCIndex)
{
	Dtc_STypeDef* ptrDtcCtrlx;
	DtcInfoConf_STypeDef*  ptrDtcInfoConf;
	u16 i;
	u8 temp;
	if(DTCIndex == 0xFFFFFF) //ÁõÆÂâçÂè™ÊîØÊåÅÂÖ®?
	{
		for(i = 0; i < DTCLISTMAXNUM; i++)
		{
			ptrDtcCtrlx = DtcInfoConfTab[i].pDtcctrlx;
			ptrDtcInfoConf = & DtcInfoConfTab[i];

			if((void *)0 != ptrDtcInfoConf->pDtcctrlx)
			{
				ptrDtcCtrlx->NowInfoRecordFlag = false;
				ptrDtcCtrlx->HistoryInfoRecordFlag = false;
				ptrDtcCtrlx->WarnTimer = DtcInfoConfTab[i].WarnTimeDef;
				ptrDtcCtrlx->CancelWarnTimer = DtcInfoConfTab[i].CancelWarnTimeDef;
				DtcErrSaved[i]=0;
				ptrDtcCtrlx->Status.data = 0;
			}
		}
	//clear DTC eeprom
		SystemParaSaveEeprom(member_offset(EepromPara_STypeDef, S_DTCErr), &DtcErrSaved[0], DTCLISTMAXNUM);
	}else
	{
		temp=0;
		for(i = 0; i < DTCLISTMAXNUM; i++)
		{
			ptrDtcCtrlx = DtcInfoConfTab[i].pDtcctrlx;
			ptrDtcInfoConf = & DtcInfoConfTab[i];

			if((void *)0 != ptrDtcInfoConf->pDtcctrlx)
			{
				if(DTCIndex== DtcInfoConfTab[i].number)
				{
					ptrDtcCtrlx->NowInfoRecordFlag = false;
					ptrDtcCtrlx->HistoryInfoRecordFlag = false;
					ptrDtcCtrlx->WarnTimer = DtcInfoConfTab[i].WarnTimeDef;
					ptrDtcCtrlx->CancelWarnTimer = DtcInfoConfTab[i].CancelWarnTimeDef;
					DtcErrSaved[i]=0;
					ptrDtcCtrlx->Status.data = 0;
					SystemParaSaveEeprom(member_offset(EepromPara_STypeDef, S_DTCErr)+i, &DtcErrSaved[i], 1);
					temp=1;
					return MCURET_OK;
				}
			}
		}
		if(temp==0) return MCURET_ERR;
	}

	return  MCURET_OK;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
u8 CAN_Sid_ReadMemoryByAddress(u8* prxdata,u16 rxlen,u8* ptxdata,u16* plen)
{

    return  MCURET_OK;
}
u8 CAN_Sid_InputOutputControlByIdentifier(u8* prxdata,u16 rxlen,u8* ptxdata,u16* plen)
{
    u16 tempIOdid;
	//u8  IOdidCtrl;
	//u8  IOdidCtrlMask;
	
	tempIOdid=TwoU8ToU16(prxdata[1],prxdata[2]);
    if(tempIOdid==0xF000)
    {
    	//
    	if(rxlen != (3+2)) return 0x13;
		//
		//IOdidCtrl=prxdata[3];
		//IOdidCtrlMask=prxdata[4];
		//cyz say: Â¢ûÂä†ÁÅØÁöÑÊéßÂà∂
		//SOSÊåâÈîÆËÉåÂÖâLED ÊéßÂà∂
		//Â∑•‰ΩúÁä∂ÊÄÅÊåáÁ§∫LED ÊéßÂà∂
		//ÊïÖÈöúÁä∂ÊÄÅÊåáÁ§∫LED ÊéßÂà∂
		
		//Â§ÑÁêÜËøîÂõûÊï∞ÊçÆ
		//0:ReturnControlToECU  03:ShortTermAdjustment
		ptxdata[*plen]=3;
		*plen +=1;
    }
	else
	{
		return 0x31;
	}
    return  0;
}
void CAN_Sid_RequestDownload(u8* prxdata,u16 rxlen,u8* ptxdata,u16* plen)
{

}
void CAN_Sid_TransferData(u8* prxdata,u16 rxlen,u8* ptxdata,u16* plen)
{

}
void CAN_Sid_RequestTransferExit(u8* prxdata,u16 rxlen,u8* ptxdata,u16* plen)
{

}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
u8 CAN_Al_GetReadDataByIdentifier(u8* prxdata,u16 rxIndex,u8* ptxdata,u16 *ptxlen)
{
    u8 temp;
    u16  usDid;
    temp = MCURET_OK;
    usDid = TwoU8ToU16(prxdata[rxIndex], prxdata[rxIndex + 1]);
	// 7-1_DIDList

    switch(usDid)
    {
    case 0xF120:
    case 0xF121:
    case 0xF122:
    case 0xF189:
    case 0xF089:
    case 0xF187:
    case 0xF18A:
    case 0xF18C:
    case 0xF190:
    case 0xF170:
    case 0xF184:
   	case 0xF1A2:
    case 0xF1F3:
    case 0xF1F6:
    case 0xF1F7:
    case 0xF1F9:
    case 0xF1FA:
    case 0xF1FC:
	case 0xF1FD:	
	case 0xF210:
	case 0xF213:
	case 0xF217:
	case 0xF220:
	case 0xF227:
	case 0xF22A:
		ptxdata[0u] =prxdata[rxIndex];
		ptxdata[1u] =prxdata[rxIndex+1];
		*ptxlen +=2;
		CAN_R_DID(usDid, &ptxdata[2], ptxlen);
		break;
    default:
        temp=MCURET_ERR;
        break;
    }
    return temp;
}
/////////////////////////////////////////////////////////
u8 CAN_Al_WriteDataByIdentifier(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len)
{
	u16  usDid;
	usDid=TwoU8ToU16(pdata[1],pdata[2]);
	//  7-1_DIDList
	switch(usDid)
	{
		case 0xF089:
		case 0xF18C:
		case 0xF190:
		case 0xF184:
		case 0xF1A2:
		case 0xF1F0:
		case 0xF1FA:
		case 0xF1F3:
		case 0xF1F6:
		case 0xF210:
		case 0xF227:	
			
			if(AL_DEFAULT_SESSION==AlPROx->DsTypes)
			{
				return 0x22;//NRC
			}
		
			if(MCURET_ERR==CAN_W_DID(usDid, &pdata[CAN_DIAG_WRITE_DATA_HEAD_LENGTH], len-CAN_DIAG_WRITE_DATA_HEAD_LENGTH))
			{
				return 0x13;	//lengh error
			}

			CanDiagSendDIDdataTo4G(usDid);
			
			break;
		default:
			return 0x31;//NRC
		break;
	}
	return 0;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
u8 CAN_SID_RoutineControl(Al_Ctrl_STypeDef* AlPROx,NL_Process_STypeDef* NlPROx,u8* pdata, u16 len,u8* pmsg,u16* lenght)
{
    u16  usDid,i;
    RoutineControl_STypeDef   *tempRCctrl;
    u8 ucSubFunction;

    // u8 ucSuppressPositiveResponseMsgInd;
    //ucSubFunction = pdata[1] &(~Bit_SuppressPositiveResponseMsgInd);
    ucSubFunction= pdata[1] ;
    //ucSuppressPositiveResponseMsgInd=pdata[1] & Bit_SuppressPositiveResponseMsgInd;

    usDid=TwoU8ToU16(pdata[2],pdata[3]);


	//subfuntionNotSupported
	if((ucSubFunction<1)||(ucSubFunction>3))
	{
		return 0x12;//NRC
	}

    //TRACE_APL("\r\n##CYZ## : usDid--%4x \r\n",usDid);

    for(i=0; i<CANALRCDIDNUM; i++)
    {
        //TRACE_APL("\r\n##CYZ## : DidTab--%4x \r\n",CANDiagRCDidTab[i].Rcdid);
        if(usDid==CANDiagRCDidTab[i].Rcdid)
        {
            tempRCctrl = &CANDiagRCDidTab[i];
            //incorrectMessageLengthOrInvalidFormat
            if(tempRCctrl->rxlen != len)
            {
                return 0x13;//NRC
            }
			#if 0
            //subfuntionNotSupported
            if((ucSubFunction<1)||(ucSubFunction>3))
            {
                return 0x12;//NRC
            }
			#endif
            if(FALSE==CAN_GPRS_Interface.gblIndR4ReadyWork)
            {
                return 0x22;//NRC
            }

            DEBUG_MCU(DBG_INFO,CAN_MODULE_ID," Didsubfunc--%d \r\n",ucSubFunction);
            //
            if(1==ucSubFunction)//startRoutine
            {
                tempRCctrl->Mode=RC_START;
                tempRCctrl->ActResult=FALSE;
				(*tempRCctrl->ActOn)(pdata,len,pmsg,lenght);
                return 0;
            }
            else if (2==ucSubFunction)   //stopRoutine
            {
                if(RC_START !=  tempRCctrl->Mode)
                {
                    return 0x24;//NRC
                }
                else
                {
                    tempRCctrl->Mode=RC_STOP;
                    return 0;
                }
            }
            else if(3==ucSubFunction)//requestRoutineResults
            {
                if(RC_START !=  tempRCctrl->Mode)
                {
                    return 0x24;//NRC
                }
                if( TRUE !=  tempRCctrl->ActResult)//??
                {
                    //NRC  0x78??
                    return 0x22;//NRC
                    //start p2*can_sever_max;
                }
                else
                {
                    tempRCctrl->Mode=RC_STOP;
                    return 0;
                }
            }
            else return 0;
        }
    }

    return 0x31;
}

/****************************** end of SID function *******************************/

/****************************** Global variable function *************************/
//init variable
void CAN_DiagGlobalVariableInit(void)
{
	CANDiagCCCtrl.DisableAppMsgRx=false;
	CANDiagCCCtrl.DisableAppMsgTx=false;
	CANDiagCCCtrl.DisableNMMsgRx=false;
	CANDiagCCCtrl.DisableNMMsgTx=false;
	CANDiagCCCtrl.DisableDiagMsgRx=false;
	CANDiagCCCtrl.DisableDiagMsgTx=false;  
}

/**************************end of Global variable function *************************/
/******************************************************************************
**                            End Of File
******************************************************************************/


