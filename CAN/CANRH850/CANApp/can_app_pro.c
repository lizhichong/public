/** 
* @file CANAppMiddle.c
* 
* @brief CAN app frame deal code
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
#include "Can_app_pro.h"
#include "CANDiagApp.h"
#include "Debug.h"

BackUpRAMPara_STypeDef* CANBackUpData;
/************************************************
*			internal macro							*
************************************************/
#define  TRACE_APPPRO   
#define IGN_AccStatusIsOn()  CAN_MCU_Interface.gblIndAccOn

#define SPEED_UP_THRESHOLD	320			///5m/S = 18Km/h   18/0.05625 = 320
#define SPEED_DOWN_THRESHOLD	320


#define TIMER_BASE_PERIOD  ((u32)10u)

#define SENDONCETIME (((u32)100u/TIMER_BASE_PERIOD) + 2)

#define MSGID296_SEND_CYCLE ((u32)100u/TIMER_BASE_PERIOD)

#define CAR_RMOTELOCK_CHKTIMEOUT	((u32)200u/TIMER_BASE_PERIOD)
#define CAR_RMOTEUNLOCK_CHKTIMEOUT	((u32)200u/TIMER_BASE_PERIOD)
#define CAR_RMOTELKFORCAR_CHKTIMEOUT	((u32)200u/TIMER_BASE_PERIOD)
#define CAR_RMOTEOPENAC_CHKTIMEOUT	((u32)200u/TIMER_BASE_PERIOD)
#define CAR_RMOTECLOSEAC_CHKTIMEOUT	((u32)200u/TIMER_BASE_PERIOD)
#define CAR_RMOTESENDTOCAR_CHKTIMEOUT	((u32)200u/TIMER_BASE_PERIOD)
#define CAR_RMOTECLOSEROOF_CHKTIMEOUT	((u32)200u/TIMER_BASE_PERIOD)
#define CAR_RMOTECARTRACK_CHKTIMEOUT	((u32)1000u/TIMER_BASE_PERIOD)
#define CAR_RMOTEUNCARTRACK_CHKTIMEOUT	((u32)1000u/TIMER_BASE_PERIOD)

#define CAR_RMOTELOCK_FDBKTIMEOUT	  ((u32)5000u/TIMER_BASE_PERIOD)
#define CAR_RMOTEUNLOCK_FDBKTIMEOUT  ((u32)5000u/TIMER_BASE_PERIOD)
#define CAR_RMOTELKFORCAR_FDBKTIMEOUT  ((u32)5000u/TIMER_BASE_PERIOD)
#define CAR_RMOTEOPENAC_FDBKTIMEOUT  ((u32)65000u/TIMER_BASE_PERIOD)
#define CAR_RMOTECLOSEAC_FDBKTIMEOUT  ((u32)65000u/TIMER_BASE_PERIOD)
#define CAR_RMOTESENDTOCAR_FDBKTIMEOUT  ((u32)30000u/TIMER_BASE_PERIOD)
#define CAR_RMOTECLOSEROOF_FDBKTIMEOUT  ((u32)30000u/TIMER_BASE_PERIOD)
#define CAR_RMOTECARTRACK_FDBKTIMEOUT  ((u32)5000u/TIMER_BASE_PERIOD)
#define CAR_RMOTECARUNTRACK_FDBKTIMEOUT  ((u32)5000u/TIMER_BASE_PERIOD)

#define CAR_RMOTEOPENACFAILEDCLOSEENGINE_TIMEOUT	  ((u32)5000u/TIMER_BASE_PERIOD)

#define MSGPROCID260_TMO	((u32)30u/TIMER_BASE_PERIOD) /*0x260 40ms */ //-10ms
#define MSGPROCID266_TMO	((u32)30u/TIMER_BASE_PERIOD) /*0x266 40ms */
#define MSGPROCID280_TMO	((u32)40u/TIMER_BASE_PERIOD) /*0x280 40ms */
#define MSGPROCID288_TMO	((u32)30u/TIMER_BASE_PERIOD) /*0x288 40ms */
#define MSGPROCID2B0_TMO	((u32)40u/TIMER_BASE_PERIOD) /*0x2B0 50ms */
#define MSGPROCID320_TMO	((u32)80u/TIMER_BASE_PERIOD) /*0x320 100ms *///-20ms
#define MSGPROCID326_TMO	((u32)80u/TIMER_BASE_PERIOD) /*0x326 100ms */
#define MSGPROCID32A_TMO	((u32)80u/TIMER_BASE_PERIOD) /*0x32A 100ms */
#define MSGPROCID330_TMO	((u32)80u/TIMER_BASE_PERIOD) /*0x330 100ms */
#define MSGPROCID340_TMO	((u32)80u/TIMER_BASE_PERIOD) /*0x340 100ms */
#define MSGPROCID347_TMO	((u32)400u/TIMER_BASE_PERIOD) /*0x347 500ms */
#define MSGPROCID380_TMO	((u32)400u/TIMER_BASE_PERIOD) /*0x380 500ms */
#define MSGPROCID384_TMO	((u32)160u/TIMER_BASE_PERIOD) /*0x384 200ms */
#define MSGPROCID500_TMO	((u32)1000u/TIMER_BASE_PERIOD) /*0x500 1000ms */
#define MSGPROCID501_TMO	((u32)800u/TIMER_BASE_PERIOD) /*0x501 1000ms */
#define MSGPROCID512_TMO    ((u32)10u/TIMER_BASE_PERIOD) /*0x512 1000ms *///EVENT
#define MSGPROCID2F8_TMO    ((u32)80u/TIMER_BASE_PERIOD) /*0x2F8 100ms *///EVENT

#define AllowSleepDelay		((u32)60000u/TIMER_BASE_PERIOD)


#define CANID296COPY() {Rte_IWrite_Fun_TboxLogic_Call_CANTX_TBox_296_Buff_CANTX_TBox_296_Buff((u8*)CanId296_Msg);}

#define TBOX_Command_LockAndUnlockNoReq()           { CanId296_Msg[0u] &= (u8)(~(1<<2u|1<<3u)); CANID296COPY();}
#define TBOX_Command_RemoteLock()                   { CanId296_Msg[0u] &= (u8)(~(1<<2u|1<<3u)); CanId296_Msg[0u] |= (u8)(1u<<2u); CANID296COPY();}
#define TBOX_Command_RemoteUnlock()                 { CanId296_Msg[0u] &= (u8)(~(1<<2u|1<<3u)); CanId296_Msg[0u] |= (u8)(2u<<2u); CANID296COPY();}
#define TBOX_Command_Invalid()                      { CanId296_Msg[0u] &= (u8)(~(1<<2u|1<<3u)); CanId296_Msg[0u] |= (u8)(3u<<2u); CANID296COPY();}


#define TBOX_Command_LkforCarNoReq()                { CanId296_Msg[4u] &= (u8)(~(1<<6u|1<<7u)); CANID296COPY();}
#define TBOX_Command_LkforCarBlink()                { CanId296_Msg[4u] &= (u8)(~(1<<6u|1<<7u)); CanId296_Msg[4u] |= (u8)(1u<<6u); CANID296COPY();}
#define TBOX_Command_LkforCarWhistle()              { CanId296_Msg[4u] &= (u8)(~(1<<6u|1<<7u)); CanId296_Msg[4u] |= (u8)(2u<<6u); CANID296COPY();}
#define TBOX_Command_LkforCarWhistleBlink()         { CanId296_Msg[4u] &= (u8)(~(1<<6u|1<<7u)); CanId296_Msg[4u] |= (u8)(3u<<6u); CANID296COPY();}

#define TBOX_Command_Send2CarNoReq()              { CanId296_Msg[4u] &= (u8)(~(3u)); CANID296COPY();}
#define TBOX_Command_Send2CarReq()                { CanId296_Msg[4u] &= (u8)(~(3u)); CanId296_Msg[4u] |= (u8)(1u); CANID296COPY();}

#define TBOX_Command_OpenAcStartEngineAndTimeNoReq()      { CanId296_Msg[0u] &= (u8)(~(3<<6u));CanId296_Msg[3u] &= (u8)(0x8f);CANID296COPY();}
#define TBOX_Command_OpenAcClearMode()                    { CanId296_Msg[2u] &= (u8)(0xe0);CANID296COPY();}
#define TBOX_Command_OpenAcReq(ucMode,ucMin)      { CanId296_Msg[0u] |= (u8)(1u<<6u);CanId296_Msg[2u] |= (u8)(ucMode&0x1f);\
													CanId296_Msg[3u] |= (u8)(ucMin<<4);CANID296COPY();}

#define TBOX_Command_CloseAcNoReq()                { CanId296_Msg[0u] &= (u8)(~(3<<6u));CANID296COPY(); }
#define TBOX_Command_CloseAcReq()                  { CanId296_Msg[0u] &= (u8)(~(3<<6u)); CanId296_Msg[0u] |= (u8)(2u<<6u); CANID296COPY();}

#define TBOX_Command_OpenSunRoofNoReq()                { CanId296_Msg[4u] &= (u8)(~(3<<2u));CANID296COPY(); }
#define TBOX_Command_OpenSunRoofReq()                  { CanId296_Msg[4u] &= (u8)(~(3<<2u)); CanId296_Msg[4u] |= (u8)(1u<<3u); CANID296COPY();}

#define TBOX_Command_OpenCloseSunRoofNoReq()                { CanId296_Msg[4u] &= (u8)(~(3<<2u));CANID296COPY(); }
#define TBOX_Command_CloseSunRoofReq()                  { CanId296_Msg[4u] &= (u8)(~(3<<2u)); CanId296_Msg[4u] |= (u8)(1u<<2u); CANID296COPY();}

#define TBOX_Command_CarTrackNoReq()                { CanId296_Msg[0u] &= (u8)(~(3<<4u));CANID296COPY(); }
#define TBOX_Command_CarTrackReq()                  { CanId296_Msg[0u] &= (u8)(~(3<<4u)); CanId296_Msg[0u] |= (u8)(1u<<4u); CANID296COPY();}

#define TBOX_Command_VinCodeNoReq()                { CanId296_Msg[3u] &= (u8)(~(3)); CANID296COPY();}
#define TBOX_Command_VinCodeReq()                  { CanId296_Msg[3u] &= (u8)(~(3)); CanId296_Msg[3u] |= (u8)(1u); CANID296COPY();}

#define TBOX_Command_VinCodeNoMatch()               { CanId296_Msg[3u] &= (u8)(~(3<<2u)); CANID296COPY();}
#define TBOX_Command_VinCodeMatched()               { CanId296_Msg[3u] &= (u8)(~(3<<2u)); CanId296_Msg[3u] |= (u8)(1u<<2u); CANID296COPY();}
#define TBOX_Command_VinCodeMatchFailed()           { CanId296_Msg[3u] &= (u8)(~(3<<2u)); CanId296_Msg[3u] |= (u8)(2u<<2u); CANID296COPY();}
#define TBOX_Command_VinCodeMatchReverse()          { CanId296_Msg[3u] &= (u8)(~(3<<2u)); CanId296_Msg[3u] |= (u8)(3u<<2u); CANID296COPY();}

#define TBOX_APP_MSGPRO_ID_MAX_NUM       (17u)

/************************************************
*			constant								*
************************************************/
/************************************************
*			internal data type						*
************************************************/
typedef struct
{
    u8 CartrackInhibitSt;

}can_app_save_info;

typedef struct
{
    rmt_ctrl rmtCurCmd;
	u32 rmtTimer;
	u8  rmtChAllow;/// 0:no allow checkfeedback;1:checkfeedback step1;2:checkfeedback step2;3:checkfeedback step3;.....
}car_rmt_ctrl_info;

typedef struct
{
    bool RequestFlag;
    u32 uiIgnOnTimer;
    u32 siRxTimer;
	u32 siTxCancelTimer;
    u32 uiMatchFailedCnt;
    u32 uiMatchSt;
} vin_chk_ctrl_t;

struct can_msg {
	u32 id;
	u32 length;
	u8 data[8];
};

typedef void (*can_pro_cb_t)(u8);

typedef struct
{
    //struct can_msg CanMsg;
    u8 CanMsg[8];
    can_pro_cb_t RdFunc;
    //can_pro_cb_t TimeOutFn;
    u32 RdTimer;
    u32 RdTimeOut;
} app_id_msg_pro;


//心跳数据上传
typedef struct{
	u8 MilAftIgOn[2];//本次行驶里程
	u8 FuelConsumptionAftIgOn[4];//本次行驶耗油
	u8 LastIgOnTime[5];//上一次点火时间
	u8 FuelLevelPercent;//剩余油量百分百
	u8 AvgFuelConsumption;//百公里平均油耗
	u8 TotalOdometer[3];//总行驶里程
	u8 DteMil[2];//余油可行驶里程
	u8 BatteryVol;//蓄电池电压
	u8 EngineCoolanTemperature;//冷却液温度
	u8 EnvironmentalTemp;//车外温度
} HeartBeat_Info_TypeDef;

typedef struct{
	u8 EMS_SysFailed_St;
	u8 TCU_SysFailed_St;
	u8 SRS_SysFailed_St;
	u8 ESC_SysFailed_St;
	u8 ABS_SysFailed_St;
	u8 EPS_SysFailed_St;
	u8 PEPS_SysFailed_St;
	u8 RCD_SysFailed_St;
	u8 RRS_SysFailed_St;
	u8 CAR_SelfTestdatachInd;//状态变化发送指示
	u32 CAR_SelfTestTimeOut;//开机60S开始上报
	u8 CAR_SelfTestAllowPop;//允许上报
	u8 EMS_SysFailed_count;
	u8 TCU_SysFailed_count;
	u8 SRS_SysFailed_count;
	u8 ESC_SysFailed_count;
	u8 ABS_SysFailed_count;
	u8 EPS_SysFailed_count;
	u8 PEPS_SysFailed_count;
	u8 RCD_SysFailed_count;
	u8 RRS_SysFailed_count;

} SelfTest_Info_TypeDef;

typedef struct{
	u8 CAR_LFTireTempWarning;
	u8 CAR_LFTireTemp;
	u8 CAR_LBTireTempWarning;
	u8 CAR_LBTireTemp;
	u8 CAR_RFTireTempWarning;
	u8 CAR_RFTireTemp;
	u8 CAR_RBTireTempWarning;
	u8 CAR_RBTireTemp;
	u8 CAR_LFLBTirePressurewarning;
	u8 CAR_RFRBTirePressurewarning;
	//u8 CAR_RBTirePressurewarning;
	//u8 CAR_LBTirePressurewarning;
	u8 CAR_LFTirePressure;
	u8 CAR_LBTirePressure;
	u8 CAR_RFTirePressure;
	u8 CAR_RBTirePressure;
	u8 CAR_TiredatachInd;//状态变化发送指示

} TireWarning_Info_TypeDef;

typedef struct{
	u8 CAR_Trunk_St;
	u8 CAR_RBdoor_St;
	u8 CAR_LBdoor_St;
	u8 CAR_RFdoor_St;
	u8 CAR_LFdoor_St;
	u8 CAR_DrDoorLock_St;
	u8 CAR_Hood_St;///引擎盖开关状态
	u8 CAR_EmergencyLight_St;///双闪开关状态
	u8 CAR_Roof_St;
	u8 CAR_PositionLight_St;
	u8 CAR_Ac_St;//空调开关状态
	u8 CAR_HighBeam_St;///大灯开关 ---远光灯开关
	u8 CAR_Body_StChInd;///状态变化发送指示
} CarBody_Info_TypeDef;

typedef struct{
	u8 CAR_Engine_St;//点火熄火状态及原因
	u8 CAR_IGOnOffTime[5];//点火熄火时间

} CarIg_Info_TypeDef;

typedef struct 
{
	u8 length[4];
	u8 channelNo[2];
}com_RX_TypeDef;

typedef struct 
{
	u8 length[4];
	u8 channelNo[2];
	u8 msgId[2];
	u8 result;
	u8 requestid[6];
	u8 reseaon;
}ctrl_TX_TypeDef;

typedef struct 
{
	u8 length[4];
	u8 channelNo[2];
	u8 requestid[6];
	u8 msgId[2];
	u8 param[1];
}ctrl_RX_TypeDef;

typedef struct
{
	
	u8 length[4];
	u8 channelNo[2];
	u8 messagehead;
	u8 message;
}bb_Net_msg_TypeDef;

/************************************************
*			Local function declare					*
************************************************/
void appTimerHandle();
static void CAN_AppProTabInit(void);
void Car_Control_Init(void);
void Car_Control_Failed_Init(void);
void Car_Control_Cancel(void);
void Tbox_CarTrackInit(void);
void CanMsgPeriodProc(void);
void Car_Remote_Contrl_Handle(void);
void Car_Remote_Contrl_Failed_Handle(void);
u8 Tbox_LocalErrConvert2BBErrCode(rmt_ctrl cmd,u8 err);
void Tbox_LockDoorAct(void);
void Tbox_UnLockDoorAct(void);
void Tbox_LkforCarAct(u8 lktype);
void Tbox_Send2CarAct(void);
void Tbox_OpenAcAct(u8 acmode,u8 runtime);
void Tbox_CloseAcAct(void);
void Tbox_OpenSunRoofAct(void);
void Tbox_CloseSunRoofAct(void);
void Tbox_CarTrackAct(void);
void Tbox_CarUnTrackAct(void);
void CarKey_Switch_Process(u8 keyst);
void CarEngine_Switch_Process(u8 engst);
void HeartBeatDataPopUp(void);
void CarEngineStPopUp(void);
void CarSelfTestPopUp(void);
void CarTireWarningStPopUp(void);
void CarBodyStPopUp(void);
void CarAlarmStPopUp(void);
void CarCrashStPopUp(void);
void HeartBeatDataHandle(void);
void CarSelfTestHandle(void);
void PowerOnSyncStHandle(void);
u8 IsEqualDefaultVincode(void);
void  CAN_VinChkCtrlTimeScan(void);
void CarVinCodeMatchHandle(void);
void CarVinCodeMatchInit(void);
void CAN_VinChkCtrlStart(void);
void CAN_VinChkCtrlStop(void);
void Sudden_Turning(u16 angle);
void Sudden_SpeedUpOrDown(u16 vspeed);

static void AppMsg_ID260Pro(u8 listnum);
static void AppMsg_ID266Pro(u8 listnum);
static void AppMsg_ID280Pro(u8 listnum);
static void AppMsg_ID288Pro(u8 listnum);
static void AppMsg_ID2B0Pro(u8 listnum);
static void AppMsg_ID320Pro(u8 listnum);
static void AppMsg_ID326Pro(u8 listnum);
static void AppMsg_ID32APro(u8 listnum);
static void AppMsg_ID330Pro(u8 listnum);
static void AppMsg_ID340Pro(u8 listnum);
static void AppMsg_ID347Pro(u8 listnum);
static void AppMsg_ID380Pro(u8 listnum);
static void AppMsg_ID384Pro(u8 listnum);
static void AppMsg_ID500Pro(u8 listnum);
static void AppMsg_ID501Pro(u8 listnum);
static void AppMsg_ID512Pro(u8 listnum);
static void AppMsg_ID2F8Pro(u8 listnum);

void COM_CmdRmtCtrlRsp(u16 ucCtrlByte, u8 ucResult, u8 ucFailedCode);
/************************************************
*			static global variable					*
************************************************/
static u8 g_SleepFlag = 0;

static u8 CanId296_Msg[8] = {0};

can_app_save_info appsavebuf;
u8 PowerOnSyncStPopUpStep = 0;
u8 PowerOnR4RcvMsgReady = 0;
u8 PowerOnSyncVinCodePopUpTime = 0;

Car_Ctrl_Func_UTypeDef CarCtrlConfig;
u8 TboxSaveVinCode[17];

car_rmt_ctrl_info car_rmtcontrl;
car_rmt_ctrl_info car_rmtfailcontrl;

vin_chk_ctrl_t S_VinChkCtrl;

bool  AccIsOnStatusBak=false;

u16 currentangle = 0;
u16 lastangle = 0;
u16 lastspeed = 0;
u16 currentspeed = 0;
u16 dith_spd = 0;
u8 dith_spd_anti = 0;		//	0:speedup; 1:speeddown
u8 speed_up_happen = 0;
u8 speed_down_happen = 0;

u8 g_RequestId[6];

static app_id_msg_pro* CanAppProTab[TBOX_APP_MSGPRO_ID_MAX_NUM];

static app_id_msg_pro App_Id_Msg260Struct = { {0}, AppMsg_ID260Pro, MSGPROCID260_TMO,MSGPROCID260_TMO };
static app_id_msg_pro App_Id_Msg266Struct = { {0}, AppMsg_ID266Pro, MSGPROCID266_TMO,MSGPROCID266_TMO };
static app_id_msg_pro App_Id_Msg280Struct = { {0}, AppMsg_ID280Pro, MSGPROCID280_TMO,MSGPROCID280_TMO };
static app_id_msg_pro App_Id_Msg288Struct = { {0}, AppMsg_ID288Pro, MSGPROCID288_TMO,MSGPROCID288_TMO };
static app_id_msg_pro App_Id_Msg2B0Struct = { {0}, AppMsg_ID2B0Pro, MSGPROCID2B0_TMO,MSGPROCID2B0_TMO };
static app_id_msg_pro App_Id_Msg320Struct = { {0}, AppMsg_ID320Pro, MSGPROCID320_TMO,MSGPROCID320_TMO };
static app_id_msg_pro App_Id_Msg326Struct = { {0}, AppMsg_ID326Pro, MSGPROCID326_TMO,MSGPROCID326_TMO };
static app_id_msg_pro App_Id_Msg32AStruct = { {0}, AppMsg_ID32APro, MSGPROCID32A_TMO,MSGPROCID32A_TMO };
static app_id_msg_pro App_Id_Msg330Struct = { {0}, AppMsg_ID330Pro, MSGPROCID330_TMO,MSGPROCID330_TMO };
static app_id_msg_pro App_Id_Msg340Struct = { {0}, AppMsg_ID340Pro, MSGPROCID340_TMO,MSGPROCID340_TMO };
static app_id_msg_pro App_Id_Msg347Struct = { {0}, AppMsg_ID347Pro, MSGPROCID347_TMO,MSGPROCID347_TMO };
static app_id_msg_pro App_Id_Msg380Struct = { {0}, AppMsg_ID380Pro, MSGPROCID380_TMO,MSGPROCID380_TMO };
static app_id_msg_pro App_Id_Msg384Struct = { {0}, AppMsg_ID384Pro, MSGPROCID384_TMO,MSGPROCID384_TMO };
static app_id_msg_pro App_Id_Msg500Struct = { {0}, AppMsg_ID500Pro, MSGPROCID500_TMO,MSGPROCID500_TMO };
static app_id_msg_pro App_Id_Msg501Struct = { {0}, AppMsg_ID501Pro, MSGPROCID501_TMO,MSGPROCID501_TMO };
static app_id_msg_pro App_Id_Msg512Struct = { {0}, AppMsg_ID512Pro, MSGPROCID512_TMO,MSGPROCID512_TMO };
static app_id_msg_pro App_Id_Msg2F8Struct = { {0}, AppMsg_ID2F8Pro, MSGPROCID2F8_TMO,MSGPROCID2F8_TMO };


static app_id_msg_pro* CanProList[TBOX_APP_MSGPRO_ID_MAX_NUM] = {

	&App_Id_Msg260Struct,
	&App_Id_Msg266Struct,
	&App_Id_Msg280Struct,
	&App_Id_Msg288Struct,
	&App_Id_Msg2B0Struct,
	&App_Id_Msg320Struct,
	&App_Id_Msg326Struct,
	&App_Id_Msg32AStruct,
	&App_Id_Msg330Struct,
	&App_Id_Msg340Struct,
	&App_Id_Msg347Struct,
	&App_Id_Msg380Struct,
	&App_Id_Msg384Struct,
	&App_Id_Msg500Struct,
	&App_Id_Msg501Struct,
	&App_Id_Msg512Struct,
	&App_Id_Msg2F8Struct,

};

///ID260 IP
volatile u8 IP_HandBrakeSwitch;
volatile u16 IP_Dte;

///ID266 PEPS
volatile u8 PEPS_EngineStartupInhibitFeedback;
volatile u8 PEPS_PEPSFailStatus;
volatile u8 PEPS_RemoteStartReq;
volatile u8 PEPS_RemoteStartFeedback;
volatile u8 PEPS_EngineStartupInhibitSts;

volatile u8 ACOpenCommand_ClearHgup = 0;


///ID280 IP_280
volatile u8 IP_EngineOilPressureLowSts; //油压状态
volatile u8 IP_FuelLow;
volatile u8 EngineCoolanTemperatureHigh;
volatile u8 IP_FuelLevelPercent;
volatile u16 IP_DisAfterIgnOn;

///ID288 BCM_288
volatile u8 BCM_RearFoglampStatus;
volatile u8 BCM_FrontFoglampStatus;
volatile u8 BCM_HighBeamStatus;
volatile u8 BCM_LowBeamStatus;
volatile u8 BCM_PositionLampStatus;
volatile u8 BCM_TurnIndicatorLeft;
volatile u8 BCM_TurnIndicatorRight;
volatile u8 BCM_SunroofStatus;
volatile u8 BCM_HoodStatus;
volatile u8 BCM_TrunkStatus;
volatile u8 BCM_RightRearDoorStatus;
volatile u8 BCM_LeftRearDoorStatus;
volatile u8 BCM_PassengerDoorStatus;
volatile u8 BCM_DriverDoorStatus;
volatile u8	BCM_Emergrncylightstatus;
volatile u8 BCM_DriverDoorLockStatus;
volatile u8 BCM_PowerStatusFeedback;
volatile u8 BCM_RearWiperStatus;
volatile u8 BCM_FrontWiperStatus;
volatile u8 BCM_KeyAlarmStatus = 2;
volatile u8 BCM_SystemFailureFlag;

///BCM_2B0
volatile u8 EMS_EngineStatus;
volatile u8 EMS_EngineStartupEnd;
volatile u8 SRS_CrashOutputStatus = 0;
volatile u8 SRS_CrashOutputStatusCheckSum = 0;
volatile s16 SAS_SteeringAngle;

///GW_ID320
volatile u8 EMS_EngineCoolanTemperature;
volatile u8 GW_GatewayFailureFlag;
volatile u8 EMS_MIL;
volatile u8 SRS_WarningLampStatus;
volatile u8 EMS_CoolanTemperatureError;
volatile u8 ESP_TCSFailStatus ;
volatile u8 ESP_ABSFailStatus;
volatile u8 ESP_VehicleSpeedValid = 1;
volatile u16 ESP_VehicleSpeed;

///GW_ID326
volatile u8 EMS_FuelConsumption;
volatile u8	EMS_BatteryVoltage;

static u32 FuelComsuption = 0;
///GW_32A
volatile u8 AT_FailureLamp;
volatile u8 ESP_EBDFailStatus;
volatile u8 EPS_EpasFailed;

///AC_330
volatile u8 AC_AC_Req;
volatile u8 AC_AC_ReqValid;
volatile u8 AC_EnvironmentalTempVD;
volatile u8 AC_EnvironmentalTemp;
volatile u8 AC_FrReqWindLevel;

///AC_340
volatile u8 AC_Rmote_Cool_Feedback;
volatile u8	AC_Rmote_Hot_Feedback;
volatile u8 AC_Mode;


///BCM_347
volatile u8 BCM_SignalStatus;
volatile u8 BCM_LFTemperatureWarning;
volatile u8 BCM_RFTemperatureWarning;
volatile u8 BCM_RRTemperatureWarning;
volatile u8 BCM_LRTemperatureWarning;
volatile u8 BCM_TireTempPosition = 0;
volatile u8 BCM_LFTyrePressure;
volatile u8 BCM_RFTyrePressure;
volatile u8 BCM_RRTyrePressure;
volatile u8 BCM_LRTyrePressure;
volatile u8 BCM_TireTemperature;
//volatile u8 BCM_LFTireTemperature;
//volatile u8 BCM_LBTireTemperature;
//volatile u8 BCM_RFTireTemperature;
//volatile u8 BCM_RBTireTemperature;

volatile u8 BCM_LFLRPressureWarning;
volatile u8 BCM_RFRRPressureWarning;
//volatile u8 BCM_LFPressureWarning;
//volatile u8 BCM_RFPressureWarning;
//volatile u8 BCM_RRPressureWarning;
//volatile u8 BCM_LRPressureWarning;
volatile u8 BCM_SystemFailureWarning;
volatile u8 BCM_TirePresureSignalStatus;
///IP_380
volatile u32 IP_TotalOdometer;
volatile u8 IP_AvgFuelConsumption;
volatile u8 IP_FuelGageType;
volatile u8 IP_AfeAfterIgnOn;
///BCM_384
volatile u8 BCM_CarRemind;
volatile u8 BCM_RmoteLockeFeedback;
volatile u8 BCM_RmoteUnLockFeedback;
volatile u8 BCM_RmoteRoofFeedback;
volatile u8 BCM_Sunroof_Position;

///DVD_500
volatile u32 DVD_CurrentLocationLongitude;
volatile u8 DVD_CurrentLocationLatitude;
volatile u8 DVD_CurrentLocationValid;

///DVD_511
volatile u8 DVD_LocalTimeYear;
volatile u8 DVD_LocalTimeMonth;
volatile u8 DVD_LocalTimeDate;
volatile u8 DVD_LocalTimeHour;
volatile u8 DVD_LocalTimeMinute;
volatile u8 DVD_LocalTimeSecond;
volatile u8 DVD_LocalTimeValid = 1;

///PEPS_512
volatile u8 PEPS_VINFrameNumber;
volatile u8 PEPS_VIN[17];
///DVD_2F8
volatile u8 Send2car_feedback;

///popup data struct
HeartBeat_Info_TypeDef HeartBeat_Data;
SelfTest_Info_TypeDef Car_SelfTest_Data;
TireWarning_Info_TypeDef Car_TirewarningSt_Data;
CarBody_Info_TypeDef Car_BodySt_Data;
CarIg_Info_TypeDef Car_EngineSt_Data;
/************************************************
*			Global function						*
************************************************/
void initCANBackUpData(BackUpRAMPara_STypeDef* pData)
{
	CANBackUpData = pData;
}
void CAN_AppProcInit(void)
{
	u8 i;
	CAN_AppProTabInit();
	Car_Control_Init();
	for(i=0;i<8;i++)
	{
		CanId296_Msg[i] = 0;
	}
	CarVinCodeMatchInit();
	Tbox_CarTrackInit();///need to modify if using this functionality
	PowerOnSyncStPopUpStep = 0;
	PowerOnR4RcvMsgReady = FALSE;

}

void Car_Contrl_CmdRev(u8 *msg)
{
	u16 channel;
	u16 MsgId;
	ctrl_RX_TypeDef *ctrl_msg;
	com_RX_TypeDef *rxmsg;
	u8 cmdvalid = 0;
	
	rxmsg = (com_RX_TypeDef *)msg;

	channel = ((u16)rxmsg->channelNo[0]<<8) | ((u16)rxmsg->channelNo[1]);

	if(0x1112==channel)
	{
		bb_Net_msg_TypeDef *net_msg;
		net_msg = (bb_Net_msg_TypeDef *)msg;

		if(0xff == net_msg->messagehead)
		{
			CAN_Did_W_CANetWorkStatus(&(net_msg->message), 1);
		}		
	}
	else if(0x1111==channel)
	{
		ctrl_msg = (ctrl_RX_TypeDef *)msg;
		MsgId = ((u16)ctrl_msg->msgId[0]<<8) | ((u16)ctrl_msg->msgId[1]);
		

		memcpy( g_RequestId, ctrl_msg->requestid, 6);
		if(car_rmtcontrl.rmtCurCmd != rmote_ctrl_none)
		{
			COM_CmdRmtCtrlRsp(MsgId,rINPROCESSOFCMDL,eRMTCTRL_NOFAILEDCODE);
			return;
		}
		switch(MsgId)
		{
			case rmote_ctrl_lock:
				Tbox_LockDoorAct();
				break;
			case rmote_ctrl_unlock:
				Tbox_UnLockDoorAct();
				break;
			case rmote_ctrl_lkforcar:
			{
				u8 ucpara=0;
				if(ctrl_msg->param[0])
				{
					ucpara+=2;
				}
				if(ctrl_msg->param[1])
				{
					ucpara+=1;
				}
				Tbox_LkforCarAct(ucpara);
			}break;
			case rmote_ctrl_send2car:
			{
				if(CarCtrlConfig.bits.CarContrl_RemoteSend2Car == 1)
					Tbox_Send2CarAct();
				else
					cmdvalid = 1;
			}break;
			case rmote_ctrl_openac:
			{
				u8 ucpara[2];
				if(ctrl_msg->param[2] > 3)
				{
					ctrl_msg->param[2] = 3;
				}
				ucpara[0] = (ctrl_msg->param[2])+(3 * ctrl_msg->param[3]);
				ucpara[1] = 1;
				if(CarCtrlConfig.bits.CarContrl_RemoteAcCTL == 1)
					Tbox_OpenAcAct(ucpara[0], ucpara[1]);
				else
					cmdvalid = 1;
			}break;
			case rmote_ctrl_closeac:
			{
				if(CarCtrlConfig.bits.CarContrl_RemoteAcCTL == 1)
					Tbox_CloseAcAct();
				else
					cmdvalid = 1;
			}break;
			case rmote_ctrl_opensunroof:
				if(CarCtrlConfig.bits.CarContrl_RemoteSunRoofCTL == 1)
					Tbox_OpenSunRoofAct();
				else
					cmdvalid = 1;
				break;
			case rmote_ctrl_closesunroof:
				if(CarCtrlConfig.bits.CarContrl_RemoteSunRoofCTL == 1)
					Tbox_CloseSunRoofAct();
				else
					cmdvalid = 1;
				break;
			case rmote_ctrl_cartrack:
				if(CarCtrlConfig.bits.CarContrl_RemoteVehicleTrack == 1)
					Tbox_CarTrackAct();
				else
					cmdvalid = 1;
				break;
			case rmote_ctrl_caruntrack:
				if(CarCtrlConfig.bits.CarContrl_RemoteVehicleTrack == 1)
					Tbox_CarUnTrackAct();
				else
					cmdvalid = 1;
				break;
			case rmote_upgrade_request:
			{
				u8 respdata[] = {0x05,0x11,0x12,0x12,0x00,0x02};
				Rte_IWrite_Fun_TboxLogic_Call_UARTTX_Buff_UARTTX_Buff(respdata);
			}break;
			default:
				cmdvalid = 1;
			///无效命令
			break;
		}
	}
	if(cmdvalid == 1)
	{
		COM_CmdRmtCtrlRsp(MsgId,rMSG_CMD_NOT_SUPPORT,eRMTCTRL_NOFAILEDCODE);
	}
}
void CanAppProcTask(void)////10ms task
{
	CanMsgPeriodProc();
	Car_Remote_Contrl_Handle();
	Car_Remote_Contrl_Failed_Handle();
	HeartBeatDataHandle();
	CarSelfTestHandle();
	PowerOnSyncStHandle();
	CarVinCodeMatchHandle();
	appTimerHandle();
}
/************************************************
*			Local function							*
************************************************/
void appTimerHandle()
{
	static u8 SleepFlag = 0;
	static u32 SleepTimerCount = 0;

	if(car_rmtcontrl.rmtCurCmd == rmote_ctrl_none)
	{
		g_SleepFlag = 0;
	}else
	{
		g_SleepFlag = 1;
	}
	if(g_SleepFlag == 1)
	{
		if(SleepFlag == 0)
		{
			SleepTimerCount = 0;
			Rte_IWrite_Fun_TboxLogic_Call_gAppAllowSleepFlag_gAppAllowSleepFlag(1);
			SleepFlag = g_SleepFlag;
		}
	}else
	{
		if(SleepFlag == 1)
		{
			SleepTimerCount++;
			if(SleepTimerCount >= AllowSleepDelay)	
			{
				Rte_IWrite_Fun_TboxLogic_Call_gAppAllowSleepFlag_gAppAllowSleepFlag(0);
				SleepFlag = g_SleepFlag;
			}
		}
	}
}
static void CAN_AppProTabInit(void)
{
    u8 i = 0u;

    for( i = 0u; i < TBOX_APP_MSGPRO_ID_MAX_NUM; i++ )
    {
        CanAppProTab[i] = (app_id_msg_pro*)0u;
    }
    for( i = 0u; i < TBOX_APP_MSGPRO_ID_MAX_NUM; i++ )
    {
        CanAppProTab[i] = CanProList[i];
    }
}
void CanMsgPeriodProc(void)
{
	u8 i;
	app_id_msg_pro *ptr;

	for(i=0;i<TBOX_APP_MSGPRO_ID_MAX_NUM;i++)
	{
		ptr = CanAppProTab[i];

		if((app_id_msg_pro*)0u != ptr)
		{
			if(ptr->RdTimer)
			{
				ptr->RdTimer--;
				if(ptr->RdTimer == 0)
				{
					ptr->RdTimer = ptr->RdTimeOut;
					if((void*)0!= ptr->RdFunc)
					{
						(*ptr->RdFunc)(i);
					}
				}
			}
		}
	}
}

static void AppMsg_ID260Pro(u8 listnum)
{
	u8 *rd;
	app_id_msg_pro *ptr;
	u8 i = 0;
	//TRACE_APPPRO("\r\n  ID260 entry ------------>");
	ptr = CanAppProTab[listnum];
	rd = Rte_IRead_Fun_TboxLogic_Call_CANRX_IP_260_Buff_CANRX_IP_260_Buff();
	for(i = 0;i < 8;i++)
	{
		ptr->CanMsg[i] = *rd++;
	}

	IP_HandBrakeSwitch = (ptr->CanMsg[1u]&0x20)>>5u;
	IP_Dte =(((u16)(ptr->CanMsg[3u]&0x03))<<8)|((u16)(ptr->CanMsg[4u]));

	HeartBeat_Data.DteMil[0] = (ptr->CanMsg[3u]&0x03);
	HeartBeat_Data.DteMil[1] = ptr->CanMsg[4u];
}

static void AppMsg_ID266Pro(u8 listnum)
{
	u8 *rd;
	app_id_msg_pro *ptr;
	u8 i = 0;
	static u8 hangupprocess = 0;
	ptr = CanAppProTab[listnum];
	rd = Rte_IRead_Fun_TboxLogic_Call_CANRX_PEPS_266_Buff_CANRX_PEPS_266_Buff();

	for(i = 0;i < 8;i++)
	{
		ptr->CanMsg[i] = *rd++;
	}

    PEPS_RemoteStartReq               = (ptr->CanMsg[4u]&0x40u)>>6u;
    PEPS_RemoteStartFeedback          = (ptr->CanMsg[4u]&0x3eu)>>1u;
    PEPS_EngineStartupInhibitSts      = (ptr->CanMsg[4u]&0x01u)>>0u;
    PEPS_EngineStartupInhibitFeedback = (ptr->CanMsg[5u]&0x04u)>>2u;
    PEPS_PEPSFailStatus               = (ptr->CanMsg[6u]&0x40u)>>6u;

	if(car_rmtcontrl.rmtChAllow == rmote_process_allowstep1)
	{
		/*if(PEPS_PEPSFailStatus == 1)
		{
			COM_CmdRmtCtrlRsp(car_rmtcontrl.rmtCurCmd,rRMTCTR_FAILED,RMTCTRL_NOERR);
			TBOX_Command_OpenAcStartEngineAndTimeNoReq();
			TBOX_Command_OpenAcClearMode();
			Car_Control_Cancel();
		}
		else */
		if(car_rmtcontrl.rmtCurCmd == rmote_ctrl_openac)
		{
			if(PEPS_RemoteStartReq == 1&&PEPS_RemoteStartFeedback == 1)
			{
				//COM_CmdRmtCtrlRsp(car_rmtcontrl.rmtCurCmd,rRMTCTR_SUCCEED,RMTCTRL_NOERR);
				TBOX_Command_OpenAcStartEngineAndTimeNoReq();
				car_rmtcontrl.rmtChAllow = rmote_process_allowstep2;
				ACOpenCommand_ClearHgup = 1;
				hangupprocess = 1;
			}
			else if(PEPS_RemoteStartFeedback>0x01&&PEPS_RemoteStartFeedback<0x15)//&&PEPS_RemoteStartFeedback!=0x11)
			{
				COM_CmdRmtCtrlRsp(car_rmtcontrl.rmtCurCmd,rRMTCTR_FAILED,PEPS_RemoteStartFeedback);
				TBOX_Command_OpenAcStartEngineAndTimeNoReq();
				TBOX_Command_OpenAcClearMode();
				Car_Control_Cancel();
				DEBUG_MCU(DBG_ERROR,CAN_MODULE_ID,"FAILED :PEPS_RemoteStartFeedback:%02x \r\n",PEPS_RemoteStartFeedback);
			}
		}
		else if(car_rmtcontrl.rmtCurCmd == rmote_ctrl_closeac)
		{
			if(PEPS_RemoteStartReq == 0)
			{
				hangupprocess = 0;
				COM_CmdRmtCtrlRsp(car_rmtcontrl.rmtCurCmd,rRMTCTR_SUCCEED,RMTCTRL_NOERR);
				Car_Control_Cancel();
				TBOX_Command_CloseAcNoReq();
			}

		}
		else if(car_rmtcontrl.rmtCurCmd == rmote_ctrl_cartrack)
		{
			if(PEPS_EngineStartupInhibitSts == 1)
			{
				COM_CmdRmtCtrlRsp(car_rmtcontrl.rmtCurCmd,rRMTCTR_SUCCEED,RMTCTRL_NOERR);
				Car_Control_Cancel();
			}
		}
		else if(car_rmtcontrl.rmtCurCmd == rmote_ctrl_caruntrack)
		{
			if(PEPS_EngineStartupInhibitSts == 0)
			{
				COM_CmdRmtCtrlRsp(car_rmtcontrl.rmtCurCmd,rRMTCTR_SUCCEED,RMTCTRL_NOERR);
				Car_Control_Cancel();
			}
		}
	}

	if(ACOpenCommand_ClearHgup == 1)
	{
		if(PEPS_RemoteStartReq == 0)
		{
			TBOX_Command_OpenAcClearMode();
			ACOpenCommand_ClearHgup = 0;
		}		
	}
	
	#if 1
	if(PEPS_RemoteStartFeedback == 0x11)
	{
		if(hangupprocess == 1)
		{
			hangupprocess = 0;
			COM_CmdRmtCtrlRsp(car_rmtcontrl.rmtCurCmd,rRMTCTR_FAILED,PEPS_RemoteStartFeedback);
			///如何反馈
		}
	}
	#endif
	#if 0
	if(Car_SelfTest_Data.PEPS_SysFailed_St != PEPS_PEPSFailStatus)
	{
		Car_SelfTest_Data.PEPS_SysFailed_St = PEPS_PEPSFailStatus;
		Car_SelfTest_Data.CAR_SelfTestdatachInd = 1;
	}
	#endif


}

static void AppMsg_ID280Pro(u8 listnum)
{
	u8 *rd;
	app_id_msg_pro *ptr;
	u8 i = 0;

	ptr = CanAppProTab[listnum];
	rd = Rte_IRead_Fun_TboxLogic_Call_CANRX_IP_280_Buff_CANRX_IP_280_Buff();

	for(i = 0;i < 8;i++)
	{
		ptr->CanMsg[i] = *rd++;
	}

	IP_EngineOilPressureLowSts = (ptr->CanMsg[1u]&0x04)>>2u; //油压状态
	IP_FuelLow = (ptr->CanMsg[1u]&0x08)>>3u;
	EngineCoolanTemperatureHigh = (ptr->CanMsg[1u]&0x02)>>1u;
	IP_FuelLevelPercent = (ptr->CanMsg[7u]&0x7F);
	IP_DisAfterIgnOn = ((u16)(ptr->CanMsg[2u]&0x3F))<<8|(u16)(ptr->CanMsg[3u]);

	if(BCM_PowerStatusFeedback != 0)
	{
		if(IP_FuelLevelPercent>0 && IP_FuelLevelPercent<=100)
		{
			HeartBeat_Data.FuelLevelPercent = IP_FuelLevelPercent;
			CANBackUpData->FuelPercent = IP_FuelLevelPercent;	
		}
	}
	
	HeartBeat_Data.MilAftIgOn[0] = (ptr->CanMsg[2u]&0x3F);
	HeartBeat_Data.MilAftIgOn[1] = ptr->CanMsg[3u];

}

static void AppMsg_ID288Pro(u8 listnum)
{
	u8 *rd;
	app_id_msg_pro *ptr;
	u8 BCM_KeyAlmStatus,i = 0;

	//TRACE_APPPRO("\r\n  ID288 entry ------------>");
	ptr = CanAppProTab[listnum];
	rd = Rte_IRead_Fun_TboxLogic_Call_CANRX_BCM_288_Buff_CANRX_BCM_288_Buff();

	for(i = 0;i < 8;i++)
	{
		ptr->CanMsg[i] = *rd++;
	}

	BCM_RearFoglampStatus  = (ptr->CanMsg[0u]&0x03u);
	BCM_FrontFoglampStatus = (ptr->CanMsg[0u]&0x0cu)>>2u;
	BCM_HighBeamStatus     = (ptr->CanMsg[0u]&0x30u)>>4u;
	BCM_LowBeamStatus      = (ptr->CanMsg[0u]&0xc0u)>>6u;

	BCM_PositionLampStatus = (ptr->CanMsg[1u]&0x0cu)>>2u;
	BCM_TurnIndicatorLeft = (ptr->CanMsg[1u]&0x30u)>>4u;
	BCM_TurnIndicatorRight = (ptr->CanMsg[1u]&0xc0u)>>6u;

	BCM_SunroofStatus = (ptr->CanMsg[2u]&0x02u)>>1u;
	BCM_HoodStatus = (ptr->CanMsg[2u]&0x04u)>>2u;
	BCM_TrunkStatus = (ptr->CanMsg[2u]&0x08u)>>3u;
	BCM_RightRearDoorStatus = (ptr->CanMsg[2u]&0x10u)>>4u;
	BCM_LeftRearDoorStatus = (ptr->CanMsg[2u]&0x20u)>>5u;
	BCM_PassengerDoorStatus = (ptr->CanMsg[2u]&0x40u)>>6u;
	BCM_DriverDoorStatus = (ptr->CanMsg[2u]&0x80u)>>7u;
	BCM_Emergrncylightstatus = (ptr->CanMsg[4u]&0x30u)>>4u;
	BCM_DriverDoorLockStatus = (ptr->CanMsg[5u]&0x03u);
	BCM_PowerStatusFeedback = (ptr->CanMsg[5u]&0x0cu)>>2u;
	BCM_RearWiperStatus = (ptr->CanMsg[5u]&0x30u)>>4u;
	BCM_FrontWiperStatus = (ptr->CanMsg[5u]&0xc0u)>>6u;
	//BCM_KeyAlarmStatus = (ptr->CanMsg[6u]&0x30u)>>4u;
	BCM_KeyAlmStatus = (ptr->CanMsg[6u]&0x30u)>>4u;
	BCM_SystemFailureFlag = (ptr->CanMsg[7u]&0x60u)>>5u;

	CarKey_Switch_Process(BCM_PowerStatusFeedback);

	/*if(car_rmtcontrl.rmtChAllow == rmote_process_allowstep1)
	{
		if(car_rmtcontrl.rmtCurCmd == rmote_ctrl_closesunroof)
		{
			if(BCM_SunroofStatus == 0)
			{
				///成功
				COM_CmdRmtCtrlRsp(car_rmtcontrl.rmtCurCmd,rRMTCTR_SUCCEED,RMTCTRL_NOERR);
				Car_Control_Cancel();
				TBOX_Command_CloseSunRoofNoReq();
			}
		}
	}*/
	
	if(Car_BodySt_Data.CAR_Trunk_St != BCM_TrunkStatus)
	{
		Car_BodySt_Data.CAR_Trunk_St = BCM_TrunkStatus;
		Car_BodySt_Data.CAR_Body_StChInd = 1;
	}

	if(Car_BodySt_Data.CAR_RBdoor_St != BCM_RightRearDoorStatus)
	{
		Car_BodySt_Data.CAR_RBdoor_St = BCM_RightRearDoorStatus;
		Car_BodySt_Data.CAR_Body_StChInd = 1;
	}

	if(Car_BodySt_Data.CAR_LBdoor_St != BCM_LeftRearDoorStatus)
	{
		Car_BodySt_Data.CAR_LBdoor_St = BCM_LeftRearDoorStatus;
		Car_BodySt_Data.CAR_Body_StChInd = 1;
	}

	if(Car_BodySt_Data.CAR_RFdoor_St != BCM_PassengerDoorStatus)
	{
		Car_BodySt_Data.CAR_RFdoor_St = BCM_PassengerDoorStatus;
		Car_BodySt_Data.CAR_Body_StChInd = 1;
	}

	if(Car_BodySt_Data.CAR_LFdoor_St != BCM_DriverDoorStatus)
	{
		Car_BodySt_Data.CAR_LFdoor_St = BCM_DriverDoorStatus;
		Car_BodySt_Data.CAR_Body_StChInd = 1;
	}

	if(Car_BodySt_Data.CAR_DrDoorLock_St != BCM_DriverDoorLockStatus)
	{
		Car_BodySt_Data.CAR_DrDoorLock_St = BCM_DriverDoorLockStatus;
		Car_BodySt_Data.CAR_Body_StChInd = 1;
	}

	if(Car_BodySt_Data.CAR_Hood_St != BCM_HoodStatus)
	{
		Car_BodySt_Data.CAR_Hood_St = BCM_HoodStatus;
		Car_BodySt_Data.CAR_Body_StChInd = 1;
	}

	if(Car_BodySt_Data.CAR_EmergencyLight_St != BCM_Emergrncylightstatus)//双闪
	{
		Car_BodySt_Data.CAR_EmergencyLight_St = BCM_Emergrncylightstatus;
		Car_BodySt_Data.CAR_Body_StChInd = 1;
	}

	if(Car_BodySt_Data.CAR_Roof_St != BCM_SunroofStatus)
	{
		Car_BodySt_Data.CAR_Roof_St = BCM_SunroofStatus;
		Car_BodySt_Data.CAR_Body_StChInd = 1;
	}

	if(Car_BodySt_Data.CAR_PositionLight_St != BCM_PositionLampStatus)///示宽灯
	{
		Car_BodySt_Data.CAR_PositionLight_St = BCM_PositionLampStatus;
		Car_BodySt_Data.CAR_Body_StChInd = 1;
	}

   	if(Car_BodySt_Data.CAR_HighBeam_St != BCM_HighBeamStatus)
	{
		Car_BodySt_Data.CAR_HighBeam_St = BCM_HighBeamStatus;
		Car_BodySt_Data.CAR_Body_StChInd = 1;
	}

	if(BCM_KeyAlarmStatus != BCM_KeyAlmStatus)
	{
		BCM_KeyAlarmStatus = BCM_KeyAlmStatus;
		CarAlarmStPopUp();
	}

	if(Car_BodySt_Data.CAR_Body_StChInd == 1)
	{
		Car_BodySt_Data.CAR_Body_StChInd = 0;
		CarBodyStPopUp();
	}

}

static void AppMsg_ID2B0Pro(u8 listnum)
{
	u8 *rd;
	app_id_msg_pro *ptr;
	u8 SRS_CshStatus,i = 0;
	u8 HardLine = 0;
	u16 SteeringAngle;
	
	ptr = CanAppProTab[listnum];
	rd = Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_2B0_Buff_CANRX_GW_2B0_Buff();

	for(i = 0;i < 8;i++)
	{
		ptr->CanMsg[i] = *rd++;
	}
	//EMS_EgStatus  = (ptr->CanMsg[0u]&0x30u)>>4u;
	SRS_CshStatus = (ptr->CanMsg[2u]&0xf0u)>>4u;
	EMS_EngineStatus = (ptr->CanMsg[0u]&0x30u)>>4u;
	//EMS_EngineStartupEnd 	       = (ptr->CanMsg[1u]&0x30u)>>4u;
	//SRS_CrashOutputStatus		   = (ptr->CanMsg[2u]&0xf0u)>>4u;
	SRS_CrashOutputStatusCheckSum  = (ptr->CanMsg[2u]&0x0fu);	//Checksum =(SRS_CrashOutputStatus) XOR 0xF
	//SAS_SteeringAngle			   = (s16)( ((u16)ptr->CanMsg[3u])<<8 + ((u16)ptr->CanMsg[4u]) );
	SteeringAngle = (((u16)ptr->CanMsg[3u])<<8)|((u16)ptr->CanMsg[4u]);
	
	CarEngine_Switch_Process(EMS_EngineStatus);
	Sudden_Turning(SteeringAngle);

	//if( EMS_EngineStatus != EMS_EgStatus )
	//{
	//	EMS_EngineStatus = EMS_EgStatus;
	//	CarEngineStPopUp();
	//	TRACE_APPPRO("CarEngineStPopUpChange------->\r\n");
	//}
	HardLine = Rte_IRead_Fun_TboxLogic_Call_CrashSignal_Crashsignal();

	if((SRS_CrashOutputStatusCheckSum == (SRS_CshStatus^0xF))||(HardLine == 1))
	{

		if((HardLine == 1)||(SRS_CshStatus == 1))
		{
			if(SRS_CrashOutputStatus != 1)
			{
				SRS_CrashOutputStatus = 1;
				CarCrashStPopUp();
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"CarCrashHappened------->\r\n");
			}
		}
		else
		{
			if(SRS_CrashOutputStatus != 0)
			{
				SRS_CrashOutputStatus = 0;
				CarCrashStPopUp();
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"CarCrashCancel------->\r\n");
			}
		}
	}

}


static void AppMsg_ID320Pro(u8 listnum)
{
	u8 *rd;
	app_id_msg_pro *ptr;
	u8 i = 0;
	u32 temp;
	
	ptr = CanAppProTab[listnum];
	rd = Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_320_Buff_CANRX_GW_320_Buff();

	for(i = 0;i < 8;i++)
	{
		ptr->CanMsg[i] = *rd++;
	}

	EMS_EngineCoolanTemperature = (ptr->CanMsg[0]);
	GW_GatewayFailureFlag 	  = (ptr->CanMsg[1u]&0x20)>>5u;
	EMS_MIL                   = (ptr->CanMsg[3u]&0x03u);
	SRS_WarningLampStatus 	  = (ptr->CanMsg[3]&0x0cu)>>2u;
	EMS_CoolanTemperatureError= (ptr->CanMsg[4u]&0x80u)>>7u;
	ESP_TCSFailStatus 		  = (ptr->CanMsg[4u]&0x40u)>>6u;
	ESP_ABSFailStatus 		  = (ptr->CanMsg[4u]&0x20u)>>5u;
	
	ESP_VehicleSpeedValid     = (ptr->CanMsg[1u]&0x40u)>>6u;
	ESP_VehicleSpeed                 = (((u16)(ptr->CanMsg[1u]&0x1Fu))<<8)|((u16)(ptr->CanMsg[2u]));

	temp = (u32)EMS_EngineCoolanTemperature;
	if(EMS_EngineCoolanTemperature != 0xff)
	{
		HeartBeat_Data.EngineCoolanTemperature = (u8)((temp*3)>>2);
	}
	
	if(ESP_VehicleSpeedValid == 0)
	{
		Sudden_SpeedUpOrDown(ESP_VehicleSpeed);
	}
	else
	{
		ESP_VehicleSpeed = 0;
	}

	
	if(Car_SelfTest_Data.EMS_SysFailed_St != EMS_MIL)///EMS系统故障
	{
		if(EMS_MIL == 1)
		{
			Car_SelfTest_Data.EMS_SysFailed_count++;
			if(Car_SelfTest_Data.EMS_SysFailed_count>=10)
			{
				Car_SelfTest_Data.EMS_SysFailed_St = EMS_MIL;
				Car_SelfTest_Data.CAR_SelfTestdatachInd = 1;
			}
		}else if(EMS_MIL == 0)
		{
			Car_SelfTest_Data.EMS_SysFailed_St = EMS_MIL;
			Car_SelfTest_Data.CAR_SelfTestdatachInd = 1;
			Car_SelfTest_Data.EMS_SysFailed_count = 0;
		}else
		{}		
	}else
	{
		Car_SelfTest_Data.EMS_SysFailed_count = 0;
	}
	
	#if 0
	if(Car_SelfTest_Data.ABS_SysFailed_St != ESP_ABSFailStatus)///ABS系统故障
	{
		Car_SelfTest_Data.ABS_SysFailed_St = ESP_ABSFailStatus;
		Car_SelfTest_Data.CAR_SelfTestdatachInd = 1;
	}
	#endif

	if(Car_SelfTest_Data.SRS_SysFailed_St != SRS_WarningLampStatus)///安全气囊
	{
		if(SRS_WarningLampStatus == 1)
		{
			Car_SelfTest_Data.SRS_SysFailed_count++;
			if(Car_SelfTest_Data.SRS_SysFailed_count>=10)
			{
				Car_SelfTest_Data.SRS_SysFailed_St = SRS_WarningLampStatus;
				Car_SelfTest_Data.CAR_SelfTestdatachInd = 1;
			}
		}else if(SRS_WarningLampStatus == 0)
		{
			Car_SelfTest_Data.SRS_SysFailed_St = SRS_WarningLampStatus;
			Car_SelfTest_Data.CAR_SelfTestdatachInd = 1;
			Car_SelfTest_Data.SRS_SysFailed_count = 0;
		}else
		{}		
	}else
	{
		Car_SelfTest_Data.SRS_SysFailed_count = 0;
	}

	if(Car_SelfTest_Data.ESC_SysFailed_St != ESP_TCSFailStatus) ///电子稳定系统
	{
		if(ESP_TCSFailStatus == 1)
		{
			Car_SelfTest_Data.ESC_SysFailed_count++;
			if(Car_SelfTest_Data.ESC_SysFailed_count>=10)
			{
				Car_SelfTest_Data.ESC_SysFailed_St = ESP_TCSFailStatus;
				Car_SelfTest_Data.CAR_SelfTestdatachInd = 1;
			}
		}else if(ESP_TCSFailStatus == 0)
		{
			Car_SelfTest_Data.ESC_SysFailed_St = ESP_TCSFailStatus;
			Car_SelfTest_Data.CAR_SelfTestdatachInd = 1;
			Car_SelfTest_Data.ESC_SysFailed_count = 0;
		}else
		{}
	}else
	{
		Car_SelfTest_Data.ESC_SysFailed_count = 0;
	}

}

static void AppMsg_ID326Pro(u8 listnum)
{
	u8 *rd;
	app_id_msg_pro *ptr;
	u8 i = 0;
	u32 temp;
	
	ptr = CanAppProTab[listnum];
	rd = Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_326_Buff_CANRX_GW_326_Buff();

	for(i = 0;i < 8;i++)
	{
		ptr->CanMsg[i] = *rd++;
	}
	
	EMS_FuelConsumption       = (ptr->CanMsg[1u]);
	EMS_BatteryVoltage        = (ptr->CanMsg[2u]);
	
	temp = (u32)EMS_FuelConsumption;
	if(temp!=0xff&&temp!=0)
	{
		FuelComsuption += temp*150;///0.015ml extend 10000

		if(FuelComsuption > 1000000)//大于100ml开始更新
		{			
			temp = FuelComsuption/10000;
			
			HeartBeat_Data.FuelConsumptionAftIgOn[0] = (u8)(temp>>24);
			HeartBeat_Data.FuelConsumptionAftIgOn[1] = (u8)(temp>>16);
			HeartBeat_Data.FuelConsumptionAftIgOn[2] = (u8)(temp>>8);
			HeartBeat_Data.FuelConsumptionAftIgOn[3] = (u8)temp;
		}
	}


}

static void AppMsg_ID32APro(u8 listnum)
{
	u8 *rd;
	app_id_msg_pro *ptr;
	u8 AT_FailureSt,i = 0;
	
	ptr = CanAppProTab[listnum];
	rd = Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_32A_Buff_CANRX_GW_32A_Buff();

	for(i = 0;i < 8;i++)
	{
		ptr->CanMsg[i] = *rd++;
	}

	AT_FailureLamp = (ptr->CanMsg[0]&0x80)>>7;
	
	EPS_EpasFailed = (ptr->CanMsg[4u]&0x02u)>>1u;

	if(Car_SelfTest_Data.TCU_SysFailed_St != AT_FailureLamp)///TCU系统故障
	{
		if(AT_FailureLamp == 1)
		{
			Car_SelfTest_Data.TCU_SysFailed_count++;
			if(Car_SelfTest_Data.TCU_SysFailed_count>=10)
			{
				Car_SelfTest_Data.TCU_SysFailed_St = AT_FailureLamp;
				Car_SelfTest_Data.CAR_SelfTestdatachInd = 1;
			}
		}else if(AT_FailureLamp == 0)
		{
			Car_SelfTest_Data.TCU_SysFailed_St = AT_FailureLamp;
			Car_SelfTest_Data.CAR_SelfTestdatachInd = 1;
			Car_SelfTest_Data.TCU_SysFailed_count = 0;
		}else
		{}
	}else
	{
		Car_SelfTest_Data.TCU_SysFailed_count = 0;
	}

	if(Car_SelfTest_Data.EPS_SysFailed_St != EPS_EpasFailed)///电子助力转向
	{
		if(EPS_EpasFailed == 1)
		{
			Car_SelfTest_Data.EPS_SysFailed_count++;
			if(Car_SelfTest_Data.EPS_SysFailed_count>=10)
			{
				Car_SelfTest_Data.EPS_SysFailed_St = EPS_EpasFailed;
				Car_SelfTest_Data.CAR_SelfTestdatachInd = 1;
			}
		}else if(EPS_EpasFailed == 0)
		{
			Car_SelfTest_Data.EPS_SysFailed_St = EPS_EpasFailed;
			Car_SelfTest_Data.CAR_SelfTestdatachInd = 1;
			Car_SelfTest_Data.EPS_SysFailed_count = 0;
		}else
		{}
	}else
	{
		Car_SelfTest_Data.EPS_SysFailed_count = 0;
	}

	if(Car_SelfTest_Data.CAR_SelfTestdatachInd == 1)
	{
		Car_SelfTest_Data.CAR_SelfTestdatachInd = 0;
		if(Car_SelfTest_Data.CAR_SelfTestAllowPop)
		{
			CarSelfTestPopUp();
			DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"CarSelfTestPopUpChange------->\r\n");
		}
	}

}

static void AppMsg_ID330Pro(u8 listnum)
{
	u8 *rd;
	app_id_msg_pro *ptr;
	u8 i = 0;
	u16 temp;
	
	ptr = CanAppProTab[listnum];
	rd = Rte_IRead_Fun_TboxLogic_Call_CANRX_AC_330_Buff_CANRX_AC_330_Buff();

	for(i = 0;i < 8;i++)
	{
		ptr->CanMsg[i] = *rd++;
	}

	AC_AC_Req = (ptr->CanMsg[6u]&0x80u)>>7u;
	AC_AC_ReqValid = (ptr->CanMsg[6u]&0x40u)>>6u;
	AC_EnvironmentalTempVD = (ptr->CanMsg[6u]&0x01u);
	AC_FrReqWindLevel = (ptr->CanMsg[3u]&0xf0u)>>4u;
	if(AC_EnvironmentalTempVD == 0)
	{
		AC_EnvironmentalTemp = ptr->CanMsg[7u];
		temp = (u16)AC_EnvironmentalTemp;
		HeartBeat_Data.EnvironmentalTemp = (u8)((temp*5)/10);
	}

	if((BCM_PowerStatusFeedback <= 1) || (AC_FrReqWindLevel == 0))
	{
		if(Car_BodySt_Data.CAR_Ac_St != 0)
		{
			Car_BodySt_Data.CAR_Ac_St = 0;
			Car_BodySt_Data.CAR_Body_StChInd = 1;
		}
	}
	else if(AC_FrReqWindLevel>0&&AC_FrReqWindLevel<0xA)
	{

		if(Car_BodySt_Data.CAR_Ac_St != 1)
		{
			Car_BodySt_Data.CAR_Ac_St = 1;
			Car_BodySt_Data.CAR_Body_StChInd = 1;
		}
	}

}

static void AppMsg_ID340Pro(u8 listnum)
{
	u8 *rd;
	app_id_msg_pro *ptr;
	u8 temp,i = 0;

	ptr = CanAppProTab[listnum];
	rd = Rte_IRead_Fun_TboxLogic_Call_CANRX_AC_340_Buff_CANRX_AC_340_Buff();

	for(i = 0;i < 8;i++)
	{
		ptr->CanMsg[i] = *rd++;
	}

	AC_Rmote_Cool_Feedback = (ptr->CanMsg[0u]&0x07u);
	AC_Rmote_Hot_Feedback = (ptr->CanMsg[0u]&0x38u)>>3;

	if(car_rmtcontrl.rmtChAllow == rmote_process_allowstep2)
	{
		if(car_rmtcontrl.rmtCurCmd == rmote_ctrl_openac)
		{
			if(AC_Mode<=3)
			{
				if(AC_Rmote_Cool_Feedback == 2)
				{
					COM_CmdRmtCtrlRsp(car_rmtcontrl.rmtCurCmd,rRMTCTR_SUCCEED,RMTCTRL_NOERR);
					Car_Control_Cancel();
					//???TBOX_Command_OpenAcClearMode();
				}
				else if((AC_Rmote_Cool_Feedback>=3)&&(AC_Rmote_Cool_Feedback<=7))
				{
					temp = eAC_TEMPERTRUEAIRDOOR + (AC_Rmote_Cool_Feedback - 3);
					COM_CmdRmtCtrlRsp(car_rmtcontrl.rmtCurCmd,rRMTCTR_FAILED,temp);///????
					TBOX_Command_OpenAcClearMode();
					ACOpenCommand_ClearHgup = 0;
					if(PEPS_RemoteStartReq == 1)///失败关闭发动机
					{
						TBOX_Command_CloseAcReq();
						car_rmtfailcontrl.rmtCurCmd = car_rmtcontrl.rmtCurCmd;
						car_rmtfailcontrl.rmtChAllow = 0;
						car_rmtfailcontrl.rmtTimer = 0;
					}
					Car_Control_Cancel();
					
					DEBUG_MCU(DBG_ERROR,CAN_MODULE_ID,"FAILED :AC_Rmote_Cool_Feedback:%02x \r\n",AC_Rmote_Cool_Feedback);
				}
			}
			else
			{
				if(AC_Rmote_Hot_Feedback == 2)
				{
					COM_CmdRmtCtrlRsp(car_rmtcontrl.rmtCurCmd,rRMTCTR_SUCCEED,RMTCTRL_NOERR);
					Car_Control_Cancel();
					//???TBOX_Command_OpenAcClearMode();
				}
				else if((AC_Rmote_Hot_Feedback>=3)&&(AC_Rmote_Hot_Feedback<=5))
				{
					temp = eAC_TEMPERTRUEAIRDOOR + (AC_Rmote_Cool_Feedback - 3);
					COM_CmdRmtCtrlRsp(car_rmtcontrl.rmtCurCmd,rRMTCTR_FAILED,temp);///????
					TBOX_Command_OpenAcClearMode();
					ACOpenCommand_ClearHgup = 0;
					if(PEPS_RemoteStartReq == 1)///失败关闭发动机
					{
						TBOX_Command_CloseAcReq();
						car_rmtfailcontrl.rmtCurCmd = car_rmtcontrl.rmtCurCmd;
						car_rmtfailcontrl.rmtChAllow = 0;
						car_rmtfailcontrl.rmtTimer = 0;
					}
					Car_Control_Cancel();
					
					DEBUG_MCU(DBG_ERROR,CAN_MODULE_ID,"FAILED :AC_Rmote_Hot_Feedback:%02x \r\n",AC_Rmote_Hot_Feedback);
				}
			}
		}
	}
}

static void AppMsg_ID347Pro(u8 listnum)
{
	u8 *rd;
	app_id_msg_pro *ptr;
	u8 i = 0;

	//TRACE_APPPRO("\r\n  ID347 entry ------------>");
	ptr = CanAppProTab[listnum];
	rd = Rte_IRead_Fun_TboxLogic_Call_CANRX_BCM_347_Buff_CANRX_BCM_347_Buff();

	for(i = 0;i < 8;i++)
	{
		ptr->CanMsg[i] = *rd++;
	}

	BCM_SignalStatus         = (ptr->CanMsg[0u]&0x80u)>>7u;
    BCM_LFTemperatureWarning = (ptr->CanMsg[0u]&0x40u)>>6u;
    BCM_RFTemperatureWarning = (ptr->CanMsg[0u]&0x20u)>>5u;
    BCM_RRTemperatureWarning = (ptr->CanMsg[0u]&0x10u)>>4u;
    BCM_LRTemperatureWarning = (ptr->CanMsg[0u]&0x08u)>>3u;
	BCM_TireTempPosition     = (ptr->CanMsg[0u]&0x07u);
    BCM_LFTyrePressure       = (ptr->CanMsg[1u]);
    BCM_RFTyrePressure       = (ptr->CanMsg[2u]);
    BCM_RRTyrePressure       = (ptr->CanMsg[3u]);
    BCM_LRTyrePressure       = (ptr->CanMsg[4u]);
    BCM_TireTemperature      = (ptr->CanMsg[5u]);
    //BCM_LFPressureWarning    = (ptr->CanMsg[6u]&0x70u)>>4u;
    //BCM_RFPressureWarning    = (ptr->CanMsg[6u]&0x07u);
    //BCM_RRPressureWarning    = (ptr->CanMsg[7u]&0xe0u)>>5u;
    //BCM_LRPressureWarning    = (ptr->CanMsg[7u]&0x1cu)>>2u;
	BCM_LFLRPressureWarning = ((ptr->CanMsg[6u]&0x70u)|((ptr->CanMsg[7u]&0x1cu)>>2u));
	BCM_RFRRPressureWarning = ((ptr->CanMsg[6u]&0x07u)<<4u)|((ptr->CanMsg[7u]&0xe0u)>>5u);
    BCM_SystemFailureWarning = (ptr->CanMsg[7u]&0x01u);
	BCM_TirePresureSignalStatus = ((ptr->CanMsg[6u]&0x80u)>>7);

	if((BCM_SystemFailureWarning == 0) && ( BCM_SignalStatus ==0 ) && (BCM_TirePresureSignalStatus == 0))
	{		
		/*if(Car_TirewarningSt_Data.CAR_LFTireTemp != BCM_TireTemperature) ///暂时，信号列表出来再更新
		{
			Car_TirewarningSt_Data.CAR_LFTireTemp = BCM_TireTemperature;
			Car_TirewarningSt_Data.CAR_LBTireTemp = BCM_TireTemperature;
			Car_TirewarningSt_Data.CAR_RFTireTemp = BCM_TireTemperature;
			Car_TirewarningSt_Data.CAR_RBTireTemp = BCM_TireTemperature;
			Car_TirewarningSt_Data.CAR_TiredatachInd = 1;
		}*/
		
		//0x1 ： Left Front tire.
		//0x2 ： Right Front tire;  
		//0x3 ： Right Rear tire;
		//0x4 ： Left Rear tire;
		//not popup when TireTemp change
		if(BCM_TireTempPosition == 0x00)
		{
			Car_TirewarningSt_Data.CAR_LFTireTemp = 0xff;
			Car_TirewarningSt_Data.CAR_RFTireTemp = 0xff;
			Car_TirewarningSt_Data.CAR_LBTireTemp = 0xff;
			Car_TirewarningSt_Data.CAR_RBTireTemp = 0xff;
		}
		else if(BCM_TireTempPosition == 0x01)
		{
			//if(Car_TirewarningSt_Data.CAR_LFTireTemp != BCM_TireTemperature)
			{
				Car_TirewarningSt_Data.CAR_LFTireTemp = BCM_TireTemperature;
				//Car_TirewarningSt_Data.CAR_TiredatachInd = 1;
			}
		}
		else if(BCM_TireTempPosition == 0x02)
		{
			//if(Car_TirewarningSt_Data.CAR_RFTireTemp != BCM_TireTemperature)
			{
				Car_TirewarningSt_Data.CAR_RFTireTemp = BCM_TireTemperature;
				//Car_TirewarningSt_Data.CAR_TiredatachInd = 1;
			}
		}
		else if(BCM_TireTempPosition == 0x03)
		{
			//if(Car_TirewarningSt_Data.CAR_RBTireTemp != BCM_TireTemperature)
			{
				Car_TirewarningSt_Data.CAR_RBTireTemp = BCM_TireTemperature;
				//Car_TirewarningSt_Data.CAR_TiredatachInd = 1;
			}
		}
		else if(BCM_TireTempPosition == 0x04)
		{
			//if(Car_TirewarningSt_Data.CAR_LBTireTemp != BCM_TireTemperature)
			{
				Car_TirewarningSt_Data.CAR_LBTireTemp = BCM_TireTemperature;
				//Car_TirewarningSt_Data.CAR_TiredatachInd = 1;
			}
		}

		if(Car_TirewarningSt_Data.CAR_LFTireTempWarning != BCM_LFTemperatureWarning)
		{
			Car_TirewarningSt_Data.CAR_LFTireTempWarning = BCM_LFTemperatureWarning;
			Car_TirewarningSt_Data.CAR_TiredatachInd = 1;
		}
		
		if(Car_TirewarningSt_Data.CAR_LBTireTempWarning != BCM_LRTemperatureWarning)
		{
			Car_TirewarningSt_Data.CAR_LBTireTempWarning = BCM_LRTemperatureWarning;
			Car_TirewarningSt_Data.CAR_TiredatachInd = 1;
		}

		if(Car_TirewarningSt_Data.CAR_RFTireTempWarning != BCM_RFTemperatureWarning)
		{
			Car_TirewarningSt_Data.CAR_RFTireTempWarning = BCM_RFTemperatureWarning;
			Car_TirewarningSt_Data.CAR_TiredatachInd = 1;
		}

		if(Car_TirewarningSt_Data.CAR_RBTireTempWarning != BCM_RRTemperatureWarning)
		{
			Car_TirewarningSt_Data.CAR_RBTireTempWarning = BCM_RRTemperatureWarning;
			Car_TirewarningSt_Data.CAR_TiredatachInd = 1;
		}

		////////////////////////////////////////////////////////////////////////////////////////	

		if(Car_TirewarningSt_Data.CAR_LFLBTirePressurewarning != BCM_LFLRPressureWarning)
		{
			Car_TirewarningSt_Data.CAR_LFLBTirePressurewarning = BCM_LFLRPressureWarning;
			Car_TirewarningSt_Data.CAR_TiredatachInd = 1;
		}
		
		if(Car_TirewarningSt_Data.CAR_RFRBTirePressurewarning != BCM_RFRRPressureWarning)
		{
			Car_TirewarningSt_Data.CAR_RFRBTirePressurewarning = BCM_RFRRPressureWarning;
			Car_TirewarningSt_Data.CAR_TiredatachInd = 1;
		}

		//u8 CAR_LFTirePressure;
		//u8 CAR_LBTirePressure;
		//u8 CAR_RFTirePressure;
		//u8 CAR_RBTirePressure;
		if(BCM_TirePresureSignalStatus == 0)
		{
			//if(Car_TirewarningSt_Data.CAR_LFTirePressure != BCM_LFTyrePressure)
			{
				Car_TirewarningSt_Data.CAR_LFTirePressure = BCM_LFTyrePressure;
			//	Car_TirewarningSt_Data.CAR_TiredatachInd = 1;
			}

			//if(Car_TirewarningSt_Data.CAR_LBTirePressure != BCM_LRTyrePressure)
			{
				Car_TirewarningSt_Data.CAR_LBTirePressure = BCM_LRTyrePressure;
			//	Car_TirewarningSt_Data.CAR_TiredatachInd = 1;
			}

			//if(Car_TirewarningSt_Data.CAR_RFTirePressure != BCM_RFTyrePressure)
			{
				Car_TirewarningSt_Data.CAR_RFTirePressure = BCM_RFTyrePressure;
			//	Car_TirewarningSt_Data.CAR_TiredatachInd = 1;
			}
			
			//if(Car_TirewarningSt_Data.CAR_RBTirePressure != BCM_RRTyrePressure)
			{
				Car_TirewarningSt_Data.CAR_RBTirePressure = BCM_RRTyrePressure;
			//	Car_TirewarningSt_Data.CAR_TiredatachInd = 1;
			}
		}else
		{
			Car_TirewarningSt_Data.CAR_RBTirePressure = 0xff;
			Car_TirewarningSt_Data.CAR_RFTirePressure = 0xff;
			Car_TirewarningSt_Data.CAR_LBTirePressure = 0xff;
			Car_TirewarningSt_Data.CAR_LFTirePressure = 0xff;
		}

		if(Car_TirewarningSt_Data.CAR_TiredatachInd == 1)
		{
			Car_TirewarningSt_Data.CAR_TiredatachInd = 0;
			CarTireWarningStPopUp();
		}

	}

}

static void AppMsg_ID380Pro(u8 listnum)
{
	u8 *rd;
	app_id_msg_pro *ptr;
	u8 i = 0;

	ptr = CanAppProTab[listnum];
	rd = Rte_IRead_Fun_TboxLogic_Call_CANRX_IP_380_Buff_CANRX_IP_380_Buff();

	for(i = 0;i < 8;i++)
	{
		ptr->CanMsg[i] = *rd++;
	}

	IP_TotalOdometer      = (((u32)ptr->CanMsg[6u])) + (((u32)ptr->CanMsg[5u])<<8u) + (((u32)(ptr->CanMsg[4u]))<<16u);
	IP_AvgFuelConsumption = (ptr->CanMsg[0u]);
	IP_FuelGageType  = (ptr->CanMsg[1]&0xc0u)>>6u;
	IP_AfeAfterIgnOn = ptr->CanMsg[7];

	HeartBeat_Data.TotalOdometer[0] = ptr->CanMsg[4u];
	HeartBeat_Data.TotalOdometer[1] = ptr->CanMsg[5u];
	HeartBeat_Data.TotalOdometer[2] = ptr->CanMsg[6u];
	HeartBeat_Data.AvgFuelConsumption = IP_AvgFuelConsumption;

}

static void AppMsg_ID384Pro(u8 listnum)
{
	u8 *rd;
	app_id_msg_pro *ptr;
	u8 err,i = 0;

	ptr = CanAppProTab[listnum];
	rd = Rte_IRead_Fun_TboxLogic_Call_CANRX_BCM_384_Buff_CANRX_BCM_384_Buff();

	for(i = 0;i < 8;i++)
	{
		ptr->CanMsg[i] = *rd++;
	}

	BCM_CarRemind = (ptr->CanMsg[4]&0xc0u)>>6u;
	BCM_RmoteLockeFeedback = (ptr->CanMsg[4]&0x38u)>>3u;
	BCM_RmoteUnLockFeedback = (ptr->CanMsg[4]&0x06u)>>1u;
	BCM_RmoteRoofFeedback = (ptr->CanMsg[6]&0x03u);
	BCM_Sunroof_Position = (ptr->CanMsg[7]&0x07u);
	
	if(car_rmtcontrl.rmtChAllow == rmote_process_allowstep1)
	{
		if(car_rmtcontrl.rmtCurCmd == rmote_ctrl_lock)
		{
			if(BCM_RmoteLockeFeedback>=1&&BCM_RmoteLockeFeedback<=5)
			{
				if(BCM_RmoteLockeFeedback == 1)
				{
					//回复成功
					COM_CmdRmtCtrlRsp(car_rmtcontrl.rmtCurCmd,rRMTCTR_SUCCEED,RMTCTRL_NOERR);
				}
				else
				{
					//回复失败原因
					err = Tbox_LocalErrConvert2BBErrCode(rmote_ctrl_lock,BCM_RmoteLockeFeedback);
					COM_CmdRmtCtrlRsp(car_rmtcontrl.rmtCurCmd,rRMTCTR_FAILED,err);
				}
				Car_Control_Cancel();
				TBOX_Command_LockAndUnlockNoReq();
			}

		}
		else if(car_rmtcontrl.rmtCurCmd == rmote_ctrl_unlock)
		{
			if(BCM_RmoteUnLockFeedback>=1&&BCM_RmoteUnLockFeedback<=3)
			{
				if(BCM_RmoteUnLockFeedback == 1)
				{
					//回复成功
					COM_CmdRmtCtrlRsp(car_rmtcontrl.rmtCurCmd,rRMTCTR_SUCCEED,RMTCTRL_NOERR);
				}
				else
				{
					//回复失败原因
					err = Tbox_LocalErrConvert2BBErrCode(rmote_ctrl_unlock,BCM_RmoteUnLockFeedback);
					COM_CmdRmtCtrlRsp(car_rmtcontrl.rmtCurCmd,rRMTCTR_FAILED,err);
				}
				Car_Control_Cancel();
				TBOX_Command_LockAndUnlockNoReq();
			}
		}
		else if(car_rmtcontrl.rmtCurCmd == rmote_ctrl_lkforcar)
		{
			if(BCM_CarRemind == 1)
			{
				///成功
				COM_CmdRmtCtrlRsp(car_rmtcontrl.rmtCurCmd,rRMTCTR_SUCCEED,RMTCTRL_NOERR);
				Car_Control_Cancel();
				TBOX_Command_LkforCarNoReq();
			}
			else if(BCM_CarRemind == 2)
			{
				///失败
				COM_CmdRmtCtrlRsp(car_rmtcontrl.rmtCurCmd,rRMTCTR_FAILED,RMTCTRL_NOERR);
				Car_Control_Cancel();
				TBOX_Command_LkforCarNoReq();
			}
		}
		else if(car_rmtcontrl.rmtCurCmd == rmote_ctrl_closesunroof)
		{
			if(BCM_Sunroof_Position == 3)
			{
				///成功
				COM_CmdRmtCtrlRsp(car_rmtcontrl.rmtCurCmd,rRMTCTR_SUCCEED,RMTCTRL_NOERR);
				Car_Control_Cancel();
				TBOX_Command_OpenCloseSunRoofNoReq();
			}
			/*else if(BCM_RmoteRoofFeedback == 2)
			{
				///失败
				COM_CmdRmtCtrlRsp(car_rmtcontrl.rmtCurCmd,rRMTCTR_FAILED,RMTCTRL_NOERR);
				Car_Control_Cancel();
				TBOX_Command_CloseSunRoofNoReq();
			}*/
		}
		else if(car_rmtcontrl.rmtCurCmd == rmote_ctrl_opensunroof)
		{
			if(BCM_Sunroof_Position == 6)
			{
				///成功
				COM_CmdRmtCtrlRsp(car_rmtcontrl.rmtCurCmd,rRMTCTR_SUCCEED,RMTCTRL_NOERR);
				Car_Control_Cancel();
				TBOX_Command_OpenCloseSunRoofNoReq();
			}

		}
		
	}
}


static void AppMsg_ID500Pro(u8 listnum)
{
	u8 *rd;
	app_id_msg_pro *ptr;
	u8 i = 0;

	ptr = CanAppProTab[listnum];
	rd = Rte_IRead_Fun_TboxLogic_Call_CANRX_HU_500_Buff_CANRX_HU_500_Buff();

	for(i = 0;i < 8;i++)
	{
		ptr->CanMsg[i] = *rd++;
	}
	DVD_CurrentLocationLongitude = (((u32)ptr->CanMsg[3u])) + (((u32)ptr->CanMsg[2u])<<8u) + (((u32)(ptr->CanMsg[1u]))<<16u) + (((u32)(ptr->CanMsg[0u]&0x1fu))<<24u);
	DVD_CurrentLocationLatitude = (((u32)ptr->CanMsg[7u])) + (((u32)ptr->CanMsg[6u])<<8u) + (((u32)(ptr->CanMsg[5u]))<<16u) + (((u32)(ptr->CanMsg[4u]&0x0fu))<<24u);
	DVD_CurrentLocationValid = (ptr->CanMsg[0u]&0x80u)>>7u;
}

static void AppMsg_ID501Pro(u8 listnum)
{
	u8 *rd;
	app_id_msg_pro *ptr;
	u8 i = 0;

	ptr = CanAppProTab[listnum];
	rd = Rte_IRead_Fun_TboxLogic_Call_CANRX_HU_501_Buff_CANRX_HU_501_Buff();

	for(i = 0;i < 8;i++)
	{
		ptr->CanMsg[i] = *rd++;
	}
	DVD_LocalTimeValid = (ptr->CanMsg[0u]&0x80u)>>7u;

	if(DVD_LocalTimeValid == 0)
	{
		DVD_LocalTimeYear = (ptr->CanMsg[1u]&0xf0u)>>4u;
		DVD_LocalTimeMonth = (ptr->CanMsg[1u]&0x0fu);
		DVD_LocalTimeDate = (ptr->CanMsg[2u]&0x1fu);
		DVD_LocalTimeHour = (ptr->CanMsg[3u]&0x1fu);
		DVD_LocalTimeMinute = (ptr->CanMsg[4u]&0x3fu);
		DVD_LocalTimeSecond = (ptr->CanMsg[5u]&0x3fu);
	}

}

static void AppMsg_ID512Pro(u8 listnum)
{
	u8 *rd;
	app_id_msg_pro *ptr;
	u8 i = 0;

	ptr = CanAppProTab[listnum];
	rd = Rte_IRead_Fun_TboxLogic_Call_CANRX_PEPS_512_Buff_CANRX_PEPS_512_Buff();

	for(i = 0;i < 8;i++)
	{
		ptr->CanMsg[i] = *rd++;
	}

	PEPS_VINFrameNumber = ptr->CanMsg[0u];

	if(PEPS_VINFrameNumber == 0)
	{
		for(i = 0;i < 7;i++)
		{
			PEPS_VIN[i] = ptr->CanMsg[i+1];
		}
	}
	else if(PEPS_VINFrameNumber == 1)
	{
		for(i = 0;i < 7;i++)
		{
			PEPS_VIN[i+7] = ptr->CanMsg[i+1];
		}
	}
	else if(PEPS_VINFrameNumber == 2)
	{

		for(i = 0;i < 3;i++)
		{
			PEPS_VIN[i+14] = ptr->CanMsg[i+1];
		}
		
		if(S_VinChkCtrl.RequestFlag == TRUE)
		{
			if(IsEqualDefaultVincode() == TRUE)
			{		
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	---->CAN: Equal Default Vincode------------>\r\n");
				return;
			}

			rd = Rte_IRead_Fun_TboxLogic_Call_EEPROMRX_F190_Buff_EEPROMRX_F190_Buff();//VIN

			for(i = 0;i < 17;i++)
			{
				TboxSaveVinCode[i] = *rd++;
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n  ---->TBOX SAVE VIN%d:%02x ,PEPSVIN:%02x\r\n",i,TboxSaveVinCode[i],PEPS_VIN[i]);
				
			}
			
	        if( !memcmp( (void*)&PEPS_VIN[0u], (void*)&TboxSaveVinCode[0u], 17u ) )
	        {
				///Vin码匹配成功
				//Set_VinMatchSt(TBOX_PEPS_VIN_MATCHED);
				S_VinChkCtrl.uiMatchSt = TBOX_PEPS_VIN_MATCHED;
				CAN_VinChkCtrlStop();
				TBOX_Command_VinCodeMatched();
				//CAN_Did_W_VIN_Match_Status(&S_VinChkCtrl.uiMatchSt, 1);
				if((PowerOnR4RcvMsgReady == TRUE)&&(PowerOnSyncStPopUpStep == 0||PowerOnSyncStPopUpStep == 7))
				{
					PowerOnSyncVinCodePopUpTime = 0;
					PowerOnSyncStPopUpStep = 7;
				}
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	---->CAN: Vincode Match Success------------>\r\n");
			}
			else
			{
				///Vin码匹配失败
				//Set_VinMatchSt(TBOX_PEPS_VIN_MATCHED_FAILED);
				S_VinChkCtrl.uiMatchSt = TBOX_PEPS_VIN_MATCHED_FAILED;
				CAN_VinChkCtrlStop();
				TBOX_Command_VinCodeMatchFailed();
				//CAN_Did_W_VIN_Match_Status(&S_VinChkCtrl.uiMatchSt, 1);
				if((PowerOnR4RcvMsgReady == TRUE)&&(PowerOnSyncStPopUpStep == 0||PowerOnSyncStPopUpStep == 7))
				{
					PowerOnSyncVinCodePopUpTime = 0;
					PowerOnSyncStPopUpStep = 7;
				}
				DEBUG_MCU(DBG_ERROR,CAN_MODULE_ID,"\r\n	---->CAN: Vincode Match Failed------------>\r\n");
			}

		}
	}


}

static void AppMsg_ID2F8Pro(u8 listnum)
{
	u8 *rd;
	app_id_msg_pro *ptr;
	u8 i = 0;

	//TRACE_APPPRO("\r\n  ID2F8 entry ------------>");
	ptr = CanAppProTab[listnum];
	rd = Rte_IRead_Fun_TboxLogic_Call_CANRX_HU_2F8_Buff_CANRX_HU_2F8_Buff();

	for(i = 0;i < 8;i++)
	{
		ptr->CanMsg[i] = *rd++;
	}

	Send2car_feedback = (ptr->CanMsg[1u]&0x0cu)>>2u;

	if(car_rmtcontrl.rmtChAllow == rmote_process_allowstep1)
	{
		if(car_rmtcontrl.rmtCurCmd == rmote_ctrl_send2car)
		{
			if(Send2car_feedback == 1)
			{
				//回复成功
				COM_CmdRmtCtrlRsp(car_rmtcontrl.rmtCurCmd,rRMTCTR_SUCCEED,RMTCTRL_NOERR);
				Car_Control_Cancel();
				TBOX_Command_Send2CarNoReq();
			}
			else if(Send2car_feedback == 2)
			{
				//回复失败原因
				//err = Tbox_LocalErrConvert2BBErrCode(rmote_ctrl_lock,BCM_RmoteLockeFeedback);
				COM_CmdRmtCtrlRsp(car_rmtcontrl.rmtCurCmd,rRMTCTR_FAILED,RMTCTRL_NOERR);///????
				Car_Control_Cancel();
				TBOX_Command_Send2CarNoReq();
			}
		}
	}


}

//-----------------------------------------Control-----------------------------------------------------------------

void Car_Control_Init(void)
{
	car_rmtcontrl.rmtCurCmd = rmote_ctrl_none;
	car_rmtcontrl.rmtTimer = 0;
	car_rmtcontrl.rmtChAllow = rmote_process_noallow;
}

void Car_Control_FunctionConfig(void)
{
	u8 i;
	u8 *rd;
	
	rd = Rte_IRead_Fun_TboxLogic_Call_EEPROMRX_F1F1_Buff_EEPROMRX_F1F1_Buff();
	
	CarCtrlConfig.CarContrlFunction[0] = *(rd+3);
	CarCtrlConfig.CarContrlFunction[1] = *(rd+4);

	rd = Rte_IRead_Fun_TboxLogic_Call_EEPROMRX_F190_Buff_EEPROMRX_F190_Buff();//VIN

	for(i = 0;i < 17;i++)
	{
		TboxSaveVinCode[i] = *rd++;
		DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n  ---->Init TBOX SAVE VIN%d:%02x \r\n",i,TboxSaveVinCode[i]);	
	}
		
}
void Car_Control_Cancel(void)
{
	car_rmtcontrl.rmtCurCmd = rmote_ctrl_none;
	car_rmtcontrl.rmtTimer = 0;
	car_rmtcontrl.rmtChAllow = rmote_process_noallow;
}

void Car_Control_Failed_Init(void)
{
	car_rmtfailcontrl.rmtCurCmd = rmote_ctrl_none;
	car_rmtfailcontrl.rmtTimer = 0;
	car_rmtfailcontrl.rmtChAllow = rmote_process_noallow;
}

u8 Tbox_LocalErrConvert2BBErrCode(rmt_ctrl cmd,u8 err)
{
	u8 ret = eRMTCTRL_NOFAILEDCODE;

	switch(cmd)
	{
		case rmote_ctrl_lock:
			switch(err)
			{
				case RMTCTRLLOCK_ERR_DOOROPEN:
					ret = eLOCK_DOORDONTCLOSE;
				break;
				case RMTCTRLLOCK_ERR_TRUNKOPEN:
					ret = eLOCK_BACKBOXDONTCLOSE;
				break;
				case RMTCTRLLOCK_ERR_KEYNOOFF:
					ret = eLOCK_KEYNOOFF;
				break;
				case RMTCTRLLOCK_ERR_HOTPROTECT:
					ret = eLOCK_HOTPROTECT;
				break;
			}
		break;
		case rmote_ctrl_unlock:
			switch(err)
			{

				case RMTCTRLUNLOCK_ERR_HOTPROTECT://热保护
					ret = eUNLOCK_HOTPROTECT;
				break;
				case RMTCTRLUNLOCK_ERR_KEYNOOFF://非OFF档
					ret = eUNLOCK_KEYNOOFF;
				break;
			}
		break;
	}
	return ret;

}

//	0x0:Lock,
//	0x1:Unlock
void Tbox_LockDoorAct(void)
{
	if(BCM_PowerStatusFeedback != 0 )
	{
		COM_CmdRmtCtrlRsp(rmote_ctrl_lock,rKEYNOOFF,eRMTCTRL_NOFAILEDCODE);
		return;
	}
		
	if(BCM_DriverDoorLockStatus == 0)//已经锁上
	{
		//返回错误码--当前已经在锁门状态
		COM_CmdRmtCtrlRsp(rmote_ctrl_lock,rALREADY_TARGET_STAT,eRMTCTRL_NOFAILEDCODE);
		return;
	}

	car_rmtcontrl.rmtCurCmd = rmote_ctrl_lock;
	car_rmtcontrl.rmtTimer = 0;
	car_rmtcontrl.rmtChAllow = rmote_process_allowstep1;
	TBOX_Command_RemoteLock();
}

void Tbox_UnLockDoorAct(void)
{
	if(BCM_PowerStatusFeedback != 0 )
	{
		COM_CmdRmtCtrlRsp(rmote_ctrl_unlock,rKEYNOOFF,eRMTCTRL_NOFAILEDCODE);
		return;
	}
	
	if(BCM_DriverDoorLockStatus == 1)
	{
		//返回错误码--当前已经在开门状态
		COM_CmdRmtCtrlRsp(rmote_ctrl_unlock,rALREADY_TARGET_STAT,eRMTCTRL_NOFAILEDCODE);
		return;
	}

 	car_rmtcontrl.rmtCurCmd = rmote_ctrl_unlock;
	car_rmtcontrl.rmtTimer = 0;
	car_rmtcontrl.rmtChAllow = rmote_process_allowstep1;
	TBOX_Command_RemoteUnlock();
}

void Tbox_LkforCarAct(u8 lktype)
{
	if((BCM_PowerStatusFeedback == 2)&&(ESP_VehicleSpeed>=30))
	{
		COM_CmdRmtCtrlRsp(rmote_ctrl_lkforcar,rSPEED_LARGE,eRMTCTRL_NOFAILEDCODE);////?????
		return;
	}

	car_rmtcontrl.rmtCurCmd = rmote_ctrl_lkforcar;
	car_rmtcontrl.rmtTimer = 0;
	car_rmtcontrl.rmtChAllow = rmote_process_allowstep1;

	if(lktype == 1)
	{
		TBOX_Command_LkforCarBlink();
	}
	else if(lktype == 2)
	{
		TBOX_Command_LkforCarWhistle();
	}
	else if(lktype == 3)
	{
		TBOX_Command_LkforCarWhistleBlink();
	}

}

void Tbox_Send2CarAct(void)
{
	if(PEPS_RemoteStartReq == 1 )
	{
		COM_CmdRmtCtrlRsp(rmote_ctrl_send2car,rINPROCESSOFCMDL,eRMTCTRL_NOFAILEDCODE);////?????
		return;
	}
	
	car_rmtcontrl.rmtCurCmd = rmote_ctrl_send2car;
	car_rmtcontrl.rmtTimer = 0;
	car_rmtcontrl.rmtChAllow = rmote_process_allowstep1;

	TBOX_Command_Send2CarReq();
}

void Tbox_OpenAcAct(u8 acmode,u8 runtime)
{
	if(BCM_PowerStatusFeedback != 0 )
	{
		COM_CmdRmtCtrlRsp(rmote_ctrl_openac,rKEYNOOFF,eRMTCTRL_NOFAILEDCODE);
		return;
	}
	/*if(PEPS_PEPSFailStatus == 1)
	{
		COM_CmdRmtCtrlRsp(rmote_ctrl_openac,rRMTCTR_FAILED,RMTCTRL_NOERR);
		TBOX_Command_OpenAcStartEngineAndTimeNoReq();
		TBOX_Command_OpenAcClearMode();
		Car_Control_Cancel();
	}*/

	if(PEPS_RemoteStartReq == 1 )
	{
		COM_CmdRmtCtrlRsp(rmote_ctrl_openac,rALREADY_TARGET_STAT,eRMTCTRL_NOFAILEDCODE);////?????
		return;
	}

	//?是否加入禁止发动机启动的判断澹?	car_rmtcontrl.rmtCurCmd = rmote_ctrl_openac;
	
	car_rmtcontrl.rmtTimer = 0;
	car_rmtcontrl.rmtChAllow = rmote_process_allowstep1;
	ACOpenCommand_ClearHgup = 0;

	if(acmode >6 || acmode<1)
	{
		acmode = 1;
	}

	if(runtime >5 || runtime<1)
	{
		runtime = 1;
	}

	AC_Mode = acmode;

	TBOX_Command_OpenAcStartEngineAndTimeNoReq();
	TBOX_Command_OpenAcClearMode();
	TBOX_Command_OpenAcReq(acmode,runtime);

}

void Tbox_CloseAcAct(void)
{
	/*if(BCM_PowerStatusFeedback != 0 )
	{
		COM_CmdRmtCtrlRsp(rmote_ctrl_closeac,rKEYNOOFF,eRMTCTRL_NOFAILEDCODE);
		return;
	}*/

	if(PEPS_RemoteStartReq == 0 )
	{
		COM_CmdRmtCtrlRsp(rmote_ctrl_closeac,rNOT_REMOTE_CTRL,eRMTCTRL_NOFAILEDCODE);////?????
		return;
	}
	else if(PEPS_RemoteStartReq == 1)
	{
		car_rmtcontrl.rmtCurCmd = rmote_ctrl_closeac;
		car_rmtcontrl.rmtTimer = 0;
		car_rmtcontrl.rmtChAllow = rmote_process_allowstep1;

		TBOX_Command_CloseAcReq();
	}

}
/*
//0: fully up全起翘
//1: tilt stop起翘段
//2: reserved
//3: fully close（全关）
//4: reserved
//5: slide stop（平移段开）
//6: fully open（全开）
//7: Degrade Mode（降级模式，半初始化状态，状态，不一定发出来了）
*/

void Tbox_OpenSunRoofAct(void)
{
	if(BCM_PowerStatusFeedback != 0 )
	{
		COM_CmdRmtCtrlRsp(rmote_ctrl_opensunroof,rKEYNOOFF,eRMTCTRL_NOFAILEDCODE);
		return;
	}
	
	if(BCM_Sunroof_Position == 6 )
	{
		COM_CmdRmtCtrlRsp(rmote_ctrl_opensunroof,rALREADY_TARGET_STAT,eRMTCTRL_NOFAILEDCODE);
		return;
	}

	car_rmtcontrl.rmtCurCmd = rmote_ctrl_opensunroof;
	car_rmtcontrl.rmtTimer = 0;
	car_rmtcontrl.rmtChAllow = rmote_process_allowstep1;
	
	TBOX_Command_OpenSunRoofReq();

}

void Tbox_CloseSunRoofAct(void)
{
	if(BCM_PowerStatusFeedback != 0 )
	{
		COM_CmdRmtCtrlRsp(rmote_ctrl_closesunroof,rKEYNOOFF,eRMTCTRL_NOFAILEDCODE);
		return;
	}

	//if(BCM_SunroofStatus == 0 )
		
	if(BCM_Sunroof_Position == 3 )
	{
		COM_CmdRmtCtrlRsp(rmote_ctrl_closesunroof,rALREADY_TARGET_STAT,eRMTCTRL_NOFAILEDCODE);
		return;
	}

	car_rmtcontrl.rmtCurCmd = rmote_ctrl_closesunroof;
	car_rmtcontrl.rmtTimer = 0;
	car_rmtcontrl.rmtChAllow = rmote_process_allowstep1;

	TBOX_Command_CloseSunRoofReq();

}

void Tbox_CarTrackAct(void)
{
	u8 *rptr;
	u8 *wptr;

	if(PEPS_EngineStartupInhibitSts == 1 )//已经处于禁止状态
	{
		COM_CmdRmtCtrlRsp(rmote_ctrl_cartrack,rALREADY_TARGET_STAT,eRMTCTRL_NOFAILEDCODE);
		return;
	}

	car_rmtcontrl.rmtCurCmd = rmote_ctrl_cartrack;
	car_rmtcontrl.rmtTimer = 0;
	car_rmtcontrl.rmtChAllow = rmote_process_noallow;

	TBOX_Command_CarTrackReq();
	appsavebuf.CartrackInhibitSt = 1;
	rptr = Rte_IRead_Fun_TboxLogic_Call_AppSave_ReadBuff_AppSave_ReadBuff();
	if(*rptr != 1)
	{
		wptr = (u8*)(&appsavebuf);
		Rte_IWrite_Fun_TboxLogic_Call_AppSave_WriteBuff_AppSave_WriteBuff(wptr);
	}
}

void Tbox_CarUnTrackAct(void)
{
	u8 *rptr;
	u8 *wptr;

	if(PEPS_EngineStartupInhibitSts == 0 )//已经处于禁止状态
	{
		COM_CmdRmtCtrlRsp(rmote_ctrl_caruntrack,rALREADY_TARGET_STAT,eRMTCTRL_NOFAILEDCODE);
		return;
	}

	car_rmtcontrl.rmtCurCmd = rmote_ctrl_caruntrack;
	car_rmtcontrl.rmtTimer = 0;
	car_rmtcontrl.rmtChAllow = rmote_process_noallow;

	TBOX_Command_CarTrackNoReq();
	appsavebuf.CartrackInhibitSt = 0;
	rptr = Rte_IRead_Fun_TboxLogic_Call_AppSave_ReadBuff_AppSave_ReadBuff();
	if(*rptr != 0)
	{
		wptr = (u8*)(&appsavebuf);
		Rte_IWrite_Fun_TboxLogic_Call_AppSave_WriteBuff_AppSave_WriteBuff(wptr);
	}

}

void Tbox_CarTrackInit(void)
{
	u8 *ptr;
	u8 temp;

	ptr = Rte_IRead_Fun_TboxLogic_Call_AppSave_ReadBuff_AppSave_ReadBuff();//Rte_IRead_Fun_TboxLogic_Call_CrashSignal_Crashsignal();

	temp = *ptr;

	if(temp == 1)
	{
		appsavebuf.CartrackInhibitSt = 1;
		TBOX_Command_CarTrackReq();
	}
	else if(temp == 0)
	{
		appsavebuf.CartrackInhibitSt = 0;
	}

}

void Car_Remote_Contrl_Handle(void)
{
	rmt_ctrl CurCmd;

	CurCmd = car_rmtcontrl.rmtCurCmd;

	switch(CurCmd)
	{
		case rmote_ctrl_none:
			car_rmtcontrl.rmtTimer = 0;
			car_rmtcontrl.rmtChAllow = 0;
			break;
		case rmote_ctrl_lock:
		case rmote_ctrl_unlock:
			car_rmtcontrl.rmtTimer++;
			/*if(car_rmtcontrl.rmtTimer == CAR_RMOTELOCK_CHKTIMEOUT)
			{
				car_rmtcontrl.rmtChAllow = rmote_process_allowstep1;
			}
			else */if(car_rmtcontrl.rmtTimer == (MSGID296_SEND_CYCLE*3))
			{
				TBOX_Command_LockAndUnlockNoReq();
			}
			else if(car_rmtcontrl.rmtTimer == CAR_RMOTELOCK_FDBKTIMEOUT)
			{
				//超时回复
				COM_CmdRmtCtrlRsp(car_rmtcontrl.rmtCurCmd,rRMTCTR_FAILED,RMTCTRL_ERR_TIMEOUT);
				Car_Control_Init();
			}

			break;
		case rmote_ctrl_lkforcar:
			car_rmtcontrl.rmtTimer++;
			if(car_rmtcontrl.rmtTimer == CAR_RMOTELKFORCAR_CHKTIMEOUT)
			{
				car_rmtcontrl.rmtChAllow = rmote_process_allowstep1;
			}
			else if(car_rmtcontrl.rmtTimer == (MSGID296_SEND_CYCLE*3))
			{
				TBOX_Command_LkforCarNoReq();
			}
			else if(car_rmtcontrl.rmtTimer == CAR_RMOTELKFORCAR_FDBKTIMEOUT)
			{
				//超时回复
				COM_CmdRmtCtrlRsp(car_rmtcontrl.rmtCurCmd,rRMTCTR_FAILED,RMTCTRL_ERR_TIMEOUT);
				Car_Control_Init();
			}
			break;
		case rmote_ctrl_send2car:
			car_rmtcontrl.rmtTimer++;
			if(car_rmtcontrl.rmtTimer == CAR_RMOTELKFORCAR_CHKTIMEOUT)
			{
				car_rmtcontrl.rmtChAllow = rmote_process_allowstep1;
			}
			else if(car_rmtcontrl.rmtTimer == CAR_RMOTESENDTOCAR_FDBKTIMEOUT)
			{
				//超时回复
				COM_CmdRmtCtrlRsp(car_rmtcontrl.rmtCurCmd,rRMTCTR_FAILED,RMTCTRL_ERR_TIMEOUT);
				Car_Control_Init();
				TBOX_Command_Send2CarNoReq();
			}

			break;

			case rmote_ctrl_openac:

				car_rmtcontrl.rmtTimer++;
				if(car_rmtcontrl.rmtTimer == CAR_RMOTEOPENAC_CHKTIMEOUT)
				{
					car_rmtcontrl.rmtChAllow = rmote_process_allowstep1;
				}
				else if(car_rmtcontrl.rmtTimer == (MSGID296_SEND_CYCLE*3))
				{
					TBOX_Command_OpenAcStartEngineAndTimeNoReq();
				}
				else if(car_rmtcontrl.rmtTimer == CAR_RMOTEOPENAC_FDBKTIMEOUT)
				{
					//超时回复
					COM_CmdRmtCtrlRsp(car_rmtcontrl.rmtCurCmd,rRMTCTR_FAILED,RMTCTRL_ERR_TIMEOUT);
					TBOX_Command_OpenAcClearMode();
					ACOpenCommand_ClearHgup = 0;
					if(PEPS_RemoteStartReq == 1)///失败关闭发动机
					{
						TBOX_Command_CloseAcReq();
						car_rmtfailcontrl.rmtCurCmd = car_rmtcontrl.rmtCurCmd;
						car_rmtfailcontrl.rmtChAllow = 0;
						car_rmtfailcontrl.rmtTimer = 0;
					}
					Car_Control_Init();
	
				}

				break;
			case rmote_ctrl_closeac:

				car_rmtcontrl.rmtTimer++;
				if(car_rmtcontrl.rmtTimer == CAR_RMOTEOPENAC_CHKTIMEOUT)
				{
					car_rmtcontrl.rmtChAllow = rmote_process_allowstep1;
				}
				else if(car_rmtcontrl.rmtTimer == (MSGID296_SEND_CYCLE*3))
				{
					TBOX_Command_CloseAcNoReq();
				}
				else if(car_rmtcontrl.rmtTimer == CAR_RMOTECLOSEAC_FDBKTIMEOUT)
				{
					//超时回复
					COM_CmdRmtCtrlRsp(car_rmtcontrl.rmtCurCmd,rRMTCTR_FAILED,RMTCTRL_ERR_TIMEOUT);
					Car_Control_Init();
					//TBOX_Command_CloseAcNoReq();
				}

				break;
			case rmote_ctrl_opensunroof:
			case rmote_ctrl_closesunroof:
				car_rmtcontrl.rmtTimer++;
				if(car_rmtcontrl.rmtTimer == CAR_RMOTECLOSEROOF_CHKTIMEOUT)
				{
					car_rmtcontrl.rmtChAllow = rmote_process_allowstep1;
				}
				else if(car_rmtcontrl.rmtTimer == (MSGID296_SEND_CYCLE*3))
				{
					//TBOX_Command_CloseSunRoofNoReq();
					TBOX_Command_OpenCloseSunRoofNoReq();
				}
				else if(car_rmtcontrl.rmtTimer == CAR_RMOTECLOSEROOF_FDBKTIMEOUT)
				{
					//超时回复
					COM_CmdRmtCtrlRsp(car_rmtcontrl.rmtCurCmd,rRMTCTR_FAILED,RMTCTRL_ERR_TIMEOUT);
					Car_Control_Init();
				}
				break;
			case rmote_ctrl_cartrack:
			case rmote_ctrl_caruntrack:
				car_rmtcontrl.rmtTimer++;
				if(car_rmtcontrl.rmtTimer == CAR_RMOTECARTRACK_CHKTIMEOUT)
				{
					car_rmtcontrl.rmtChAllow = rmote_process_allowstep1;
				}
				else if(car_rmtcontrl.rmtTimer == CAR_RMOTECARTRACK_FDBKTIMEOUT)
				{
					//超时回复
					COM_CmdRmtCtrlRsp(car_rmtcontrl.rmtCurCmd,rRMTCTR_FAILED,RMTCTRL_ERR_TIMEOUT);
					Car_Control_Init();
				}
				break;
			default:
				break;
	}

}

void Car_Remote_Contrl_Failed_Handle(void)
{
	rmt_ctrl CurCmd;

	CurCmd = car_rmtfailcontrl.rmtCurCmd;

	switch(CurCmd)
	{
		case rmote_ctrl_none:
			car_rmtfailcontrl.rmtTimer = 0;
			car_rmtfailcontrl.rmtChAllow = 0;
			break;
		case rmote_ctrl_lock:
		case rmote_ctrl_unlock:

			break;
		case rmote_ctrl_lkforcar:

			break;
		case rmote_ctrl_send2car:

			break;

			case rmote_ctrl_openac:

				car_rmtfailcontrl.rmtTimer++;
				if(car_rmtfailcontrl.rmtTimer == (MSGID296_SEND_CYCLE*3))
				{
					TBOX_Command_CloseAcNoReq();
				}
				else if(car_rmtfailcontrl.rmtTimer == CAR_RMOTEOPENACFAILEDCLOSEENGINE_TIMEOUT)
				{
					Car_Control_Failed_Init();
				}

				break;
			case rmote_ctrl_closeac:
				break;
			case rmote_ctrl_closesunroof:
				break;
			case rmote_ctrl_cartrack:
			case rmote_ctrl_caruntrack:
				break;
			default:
				break;
	}


}

//0x0:OFF 0x1:ACC 0x2:ON 0x3:Start

void CarKey_Switch_Process(u8 keyst)
{
	static u8 powerkey = 0xFF;
	#if 0
	u8 temp;
	u16 yeartemp = 0; ///from DVD time
	u8 *rdtime;  ///local time
	#endif
	
	switch(keyst)
	{
		case 0:
			if(powerkey != 0)
			{
				powerkey = 0;
				if(HeartBeat_Data.FuelConsumptionAftIgOn[0]!=0||HeartBeat_Data.FuelConsumptionAftIgOn[1]!=0||\
				   HeartBeat_Data.FuelConsumptionAftIgOn[2]!=0||HeartBeat_Data.FuelConsumptionAftIgOn[3]!=0)
				{
					HeartBeat_Data.FuelConsumptionAftIgOn[0] = 0;
					HeartBeat_Data.FuelConsumptionAftIgOn[1] = 0;
					HeartBeat_Data.FuelConsumptionAftIgOn[2] = 0;
					HeartBeat_Data.FuelConsumptionAftIgOn[3] = 0;
					FuelComsuption = 0;
				}
			}
		break;
		case 1:
		    powerkey = 1;
		break;
		case 2:
			powerkey = 2;
		break;
		case 3:
			if(powerkey == 2)
			{
				powerkey = 3;
				//记录本次点火时间点
				#if 0
				yeartemp = 2013 + DVD_LocalTimeYear;
				HeartBeat_Data.LastIgOnTime[0] = (u8)((yeartemp&0xFF00)>>8);
				HeartBeat_Data.LastIgOnTime[1] = (u8)yeartemp;
				HeartBeat_Data.LastIgOnTime[2] = DVD_LocalTimeMonth;
				HeartBeat_Data.LastIgOnTime[3] = DVD_LocalTimeDate;
				HeartBeat_Data.LastIgOnTime[4] = DVD_LocalTimeHour;
				HeartBeat_Data.LastIgOnTime[5] = DVD_LocalTimeMinute;
				HeartBeat_Data.LastIgOnTime[6] = DVD_LocalTimeSecond;
				//#else
				rdtime = Rte_IRead_Fun_TboxLogic_Call_LocalTime_LocalTime();
				/*HeartBeat_Data.LastIgOnTime[0] = *rdtime++;
				HeartBeat_Data.LastIgOnTime[1] = *rdtime++;
				HeartBeat_Data.LastIgOnTime[2] = *rdtime++;
				HeartBeat_Data.LastIgOnTime[3] = *rdtime++;
				HeartBeat_Data.LastIgOnTime[4] = *rdtime++;
				HeartBeat_Data.LastIgOnTime[5] = *rdtime++;
				HeartBeat_Data.LastIgOnTime[6] = *rdtime;*/
				temp = *rdtime++;//year high
				HeartBeat_Data.LastIgOnTime[0] = 0;
				HeartBeat_Data.LastIgOnTime[0] = (temp&0x3F)<<2;
				temp = *rdtime++;//year low :14BIT
				HeartBeat_Data.LastIgOnTime[0] |=  (temp&0xc0)>>6;
				HeartBeat_Data.LastIgOnTime[1] = 0;
				HeartBeat_Data.LastIgOnTime[1] = (temp&0x3F)<<2;
				temp = *rdtime++;//month:4BIT
				HeartBeat_Data.LastIgOnTime[1] |= (temp&0x0F)>>2;
				HeartBeat_Data.LastIgOnTime[2] = 0;
				HeartBeat_Data.LastIgOnTime[2] = (temp&0x03)<<6;
				temp = *rdtime++;//date:5BIT
				HeartBeat_Data.LastIgOnTime[2] |= (temp&0x1F)<<1;
				temp = *rdtime++;//hour:5BIT
				HeartBeat_Data.LastIgOnTime[2] |= (temp&0x1F)>>4;
				HeartBeat_Data.LastIgOnTime[3] = 0;
				HeartBeat_Data.LastIgOnTime[3] = (temp&0x0F)<<4;
				temp = *rdtime++;//minute:6BIT
				HeartBeat_Data.LastIgOnTime[3] |= (temp&0x3F)>>2;
				HeartBeat_Data.LastIgOnTime[4] = 0;
				HeartBeat_Data.LastIgOnTime[4] = (temp&0x3)<<6;
				temp = *rdtime;//second:6BIT
				HeartBeat_Data.LastIgOnTime[4] |= (temp&0x3F);
				#endif

			}

		break;
	}

}
//0x0=Stop;      0x1=Crank;
//0x2=Running;  0x3=Invalid
void TimeFormatConvt(u8*ptr)
{
	u8 timegp[5];
	u8 i,temp;
	u8 *rdtime,*wdtime;
	wdtime = ptr;
	rdtime = Rte_IRead_Fun_TboxLogic_Call_LocalTime_LocalTime();

	temp = *rdtime++;//year high
	timegp[0] = 0;
	timegp[0] = (temp&0x3F)<<2;
	temp = *rdtime++;//year low :14BIT
	timegp[0] |=  (temp&0xc0)>>6;
	timegp[1] = 0;
	timegp[1] = (temp&0x3F)<<2;
	temp = *rdtime++;//month:4BIT
	timegp[1] |= (temp&0x0F)>>2;
	timegp[2] = 0;
	timegp[2] = (temp&0x03)<<6;
	temp = *rdtime++;//date:5BIT
	timegp[2] |= (temp&0x1F)<<1;
	temp = *rdtime++;//hour:5BIT
	timegp[2] |= (temp&0x1F)>>4;
	timegp[3] = 0;
	timegp[3] = (temp&0x0F)<<4;
	temp = *rdtime++;//minute:6BIT
	timegp[3] |= (temp&0x3F)>>2;
	timegp[4] = 0;
	timegp[4] = (temp&0x3)<<6;
	temp = *rdtime;//second:6BIT
	timegp[4] |= (temp&0x3F);

	for(i=0;i<5;i++)
	{
		*(wdtime+i) = timegp[i];
	}

}

void disableSelfTest()
{
	Car_SelfTest_Data.CAR_SelfTestAllowPop = 0;
	Car_SelfTest_Data.CAR_SelfTestTimeOut = 0;
	Car_SelfTest_Data.CAR_SelfTestdatachInd = 0;
}
void CarEngine_Switch_Process(u8 engst)
{
	static u8 enginest = 0;

	switch(engst)
	{
		case 0:
			if(enginest !=0 )
			{
				enginest = 0;
				Car_EngineSt_Data.CAR_Engine_St = 0x40;
				TimeFormatConvt((u8*)(&Car_EngineSt_Data.CAR_IGOnOffTime[0]));
				CarEngineStPopUp();
				Car_SelfTest_Data.CAR_SelfTestAllowPop = 0;
				Car_SelfTest_Data.CAR_SelfTestTimeOut = 0;
				Car_SelfTest_Data.CAR_SelfTestdatachInd = 0;
			}

		break;
		case 1:
			//enginest = 1;
		break;
		case 2:
			if(enginest != 2)
			{
				enginest = 2;
				Car_SelfTest_Data.CAR_SelfTestTimeOut = ((u32)5000u/TIMER_BASE_PERIOD);
				Car_SelfTest_Data.CAR_SelfTestdatachInd = 0;
				Car_SelfTest_Data.CAR_SelfTestAllowPop = 0;

				Car_EngineSt_Data.CAR_Engine_St = 0x80;
				if(PEPS_RemoteStartReq == 1)
				{
					Car_EngineSt_Data.CAR_Engine_St = 0x81;
				}

				TimeFormatConvt((u8*)(&Car_EngineSt_Data.CAR_IGOnOffTime[0]));
				TimeFormatConvt((u8*)(&HeartBeat_Data.LastIgOnTime[0]));
				CarEngineStPopUp();
			}

		break;
		case 3:
			break;
		default:
			break;
	}
}

void HeartBeatDataPopUp(void)
{
	u8 i,Hbbuf[21+6] = {0};  //par 23bytes
	u8 *ptr;

	HeartBeat_Data.BatteryVol = Rte_IRead_Fun_TboxLogic_Call_BatteryValue_BatteryValue();
	HeartBeat_Data.FuelLevelPercent = CANBackUpData->FuelPercent;
	Hbbuf[0] = 26;///lenght
	Hbbuf[1] = 0x11;
	Hbbuf[2] = 0x12;
	Hbbuf[3] = POPTYPE_CMD1;
	Hbbuf[4] = 0x0;
	Hbbuf[5] = CAN_CMD_HAEARBEAT_POP;
	ptr = (u8*)&HeartBeat_Data;
	for(i = 6;i<27;i++)
	{
		Hbbuf[i] = *ptr++;
	}
	#if 1
	DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n  HeartBeatDataPopUp DATA: ------------>");
	for(i = 0;i < 27;i++)
	{
		DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"%02x ",Hbbuf[i]);
	}
	DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n");
	#endif
	Rte_IWrite_Fun_TboxLogic_Call_UARTTX_Buff_UARTTX_Buff(Hbbuf);

}

void CarEngineStPopUp(void)
{
	u8 i,Hbbuf[6+6] = {0};  //par 6bytes
	u8 *ptr;
	if(CANBackUpData->EngineSt == Car_EngineSt_Data.CAR_Engine_St)
	{
		return;
	}
	CANBackUpData->EngineSt = Car_EngineSt_Data.CAR_Engine_St;
	Hbbuf[0] = 11;///lenght
	Hbbuf[1] = 0x11;
	Hbbuf[2] = 0x12;
	Hbbuf[3] = POPTYPE_CMD1;
	Hbbuf[4] = 0x0;
	Hbbuf[5] = CAN_CMD_ENGINEST_POP;
	ptr = (u8*)&Car_EngineSt_Data;

	for(i = 6;i<12;i++)
	{
		Hbbuf[i] = *ptr++;
	}

#if 1
	DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n  CarEngineStPopUp DATA: ------------>");
	for(i = 0;i < 12;i++)
	{
		DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"%02x ",Hbbuf[i]);
	}
	DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n");
#endif

	Rte_IWrite_Fun_TboxLogic_Call_UARTTX_Buff_UARTTX_Buff(Hbbuf);

}


void CarSelfTestPopUp(void)///自检上报
{
	u8 i,Hbbuf[9+6] = {0};  //par 9bytes
	u8 *ptr;
	Hbbuf[0] = 14;///lenght
	Hbbuf[1] = 0x11;
	Hbbuf[2] = 0x12;
	Hbbuf[3] = POPTYPE_CMD1;
	Hbbuf[4] = 0x0;
	Hbbuf[5] = CAN_CMD_SELFTEST_POP;
	ptr = (u8*)&Car_SelfTest_Data;
	for(i = 6;i<15;i++)
	{
		Hbbuf[i] = *ptr++;
	}

	#if 1
	DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n  CarSelfTestPopUp DATA: ------------>");
	for(i = 0;i < 15;i++)
	{
		DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"%02x ",Hbbuf[i]);
	}
	DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n");
	#endif

	Rte_IWrite_Fun_TboxLogic_Call_UARTTX_Buff_UARTTX_Buff(Hbbuf);

}

void CarTireWarningStPopUp(void)///胎压胎温异常状态上报
{
	u8 i,Hbbuf[14+6] = {0};  //par 8bytes
	u8 *ptr;
	Hbbuf[0] = 19;///lenght
	Hbbuf[1] = 0x11;
	Hbbuf[2] = 0x12;
	Hbbuf[3] = POPTYPE_CMD1;
	Hbbuf[4] = 0x0;
	Hbbuf[5] = CAN_CMD_TIREWARNING_POP;
	ptr = (u8*)&Car_TirewarningSt_Data;
	for(i = 6;i<20;i++)
	{
		Hbbuf[i] = *ptr++;
	}

	#if 1
	DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n  CarTireWarningStPopUp DATA: ------------>");
	for(i = 0;i < 20;i++)
	{
		DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"%02x ",Hbbuf[i]);
	}
	DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n");
	#endif

	Rte_IWrite_Fun_TboxLogic_Call_UARTTX_Buff_UARTTX_Buff(Hbbuf);

}


void CarBodyStPopUp(void) ///车身状态上报
{
	u8 i,Hbbuf[12+6] = {0};  //par 12bytes
	u8 *ptr;
	Hbbuf[0] = 17;///lenght
	Hbbuf[1] = 0x11;
	Hbbuf[2] = 0x12;
	Hbbuf[3] = POPTYPE_CMD1;
	Hbbuf[4] = 0x0;
	Hbbuf[5] = CAN_CMD_VEHST_POP;
	ptr = (u8*)&Car_BodySt_Data;
	for(i = 6;i<18;i++)
	{
		Hbbuf[i] = *ptr++;
	}

	#if 1
	DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n  CarBodyStPopUp DATA: ------------>");
	for(i = 0;i < 18;i++)
	{
		DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"%02x ",Hbbuf[i]);
	}
	DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n");
	#endif
 	Rte_IWrite_Fun_TboxLogic_Call_UARTTX_Buff_UARTTX_Buff(Hbbuf);

}

void CarAlarmStPopUp(void)///警戒状态上报
{
	u8 Hbbuf[1+6] = {0};  //par 1bytes

	Hbbuf[0] = 6;///lenght
	Hbbuf[1] = 0x11;
	Hbbuf[2] = 0x12;
	Hbbuf[3] = POPTYPE_CMD1;
	Hbbuf[4] = 0x0;
	Hbbuf[5] = CAN_CMD_ALARM_POP;
	Hbbuf[6] = BCM_KeyAlarmStatus;

	Rte_IWrite_Fun_TboxLogic_Call_UARTTX_Buff_UARTTX_Buff(Hbbuf);

}


void CarCrashStPopUp(void)///碰撞事件
{
	u8 Hbbuf[1+6] = {0};  //par 23bytes

	Hbbuf[0] = 6;///lenght
	Hbbuf[1] = 0x11;
	Hbbuf[2] = 0x12;
	Hbbuf[3] = POPTYPE_CMD1;
	Hbbuf[4] = 0x0;
	Hbbuf[5] = CAN_CMD_CRASH_POP;
	Hbbuf[6] = SRS_CrashOutputStatus;

	Rte_IWrite_Fun_TboxLogic_Call_UARTTX_Buff_UARTTX_Buff(Hbbuf);

}

/*
* add by jason for drive behaviour
*/
void CarDriveBehaviourPopup(u8 drivetype)///三急上报
{
	u8 i,Hbbuf[1+10] = {0};  //par 23bytes
	u16 speedtmp;
	u16 dith_spd_tmp;
	Hbbuf[0] = 10;///lenght
	Hbbuf[1] = 0x11;
	Hbbuf[2] = 0x12;
	Hbbuf[3] = POPTYPE_CMD1;
	Hbbuf[4] = 0x0;
	Hbbuf[5] = CAN_CMD_DRIVER_behaviour;
	Hbbuf[6] = drivetype;
	speedtmp =  (ESP_VehicleSpeed * 45)>>3;	//  (*5625 /1000)
	Hbbuf[7] = (u8)(speedtmp>>8);	
	Hbbuf[8] = (u8)(speedtmp&0x00ff);
	dith_spd_tmp = (dith_spd * 25)>>4;	// (*5625/1000/3.6)
	if(dith_spd_anti==1)
	{
		Hbbuf[9] = (u8)((30000 - dith_spd_tmp)>>8);
		Hbbuf[10] = (u8)((30000 - dith_spd_tmp)&0x00ff);
	}else
	{
		Hbbuf[9] = (u8)((30000 + dith_spd_tmp)>>8);
		Hbbuf[10] = (u8)((30000 + dith_spd_tmp)&0x00ff);
	}
	#if 1
	DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n  CarDriveBehaviourPopup: ------------>");
	for(i = 0;i < 11;i++)
	{
		DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"%02x ",Hbbuf[i]);
	}
	DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n");
	#endif

	Rte_IWrite_Fun_TboxLogic_Call_UARTTX_Buff_UARTTX_Buff(Hbbuf);

}
void CarGPSPopup(void)///提示GPS上报
{
	u8 i,Hbbuf[1+9] = {0};  //par 23bytes
	u16 speedtmp;
	u16 dith_spd_tmp;
	Hbbuf[0] = 9;///lenght
	Hbbuf[1] = 0x11;
	Hbbuf[2] = 0x12;
	Hbbuf[3] = POPTYPE_CMD1;
	Hbbuf[4] = 0x0;
	Hbbuf[5] = CAN_CMD_GPS_POP;
	speedtmp =  (ESP_VehicleSpeed * 45)>>3;	//  (*5625 /1000)
	Hbbuf[6] = (u8)(speedtmp>>8);	
	Hbbuf[7] = (u8)(speedtmp&0x00ff);
	dith_spd_tmp = (dith_spd * 25)>>4;	// (*5625/1000/3.6)
	if(dith_spd_anti==1)
	{
		Hbbuf[8] = (u8)((30000 - dith_spd_tmp)>>8);
		Hbbuf[9] = (u8)((30000 - dith_spd_tmp)&0x00ff);
	}else
	{
		Hbbuf[8] = (u8)((30000 + dith_spd_tmp)>>8);
		Hbbuf[9] = (u8)((30000 + dith_spd_tmp)&0x00ff);
	}
	DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n  CarGPSPopup: ------------>");
	for(i = 0;i < 10;i++)
	{
		DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"%02x ",Hbbuf[i]);
	}
	DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n");

	Rte_IWrite_Fun_TboxLogic_Call_UARTTX_Buff_UARTTX_Buff(Hbbuf);

}


void CarVinMatchStPopUp(void)///VIN Matched St
{
	u8 i,Hbbuf[1+5] = {0};  //par 5bytes

	Hbbuf[0] = 5;///lenght
	Hbbuf[1] = 0x11;
	Hbbuf[2] = 0x12;
	Hbbuf[3] = 0x0f;
	Hbbuf[4] = 0x0;
	Hbbuf[5] = (u8)(S_VinChkCtrl.uiMatchSt);/// match state
	#if 1
	DEBUG_MCU(DBG_ERROR,CAN_MODULE_ID,"\r\n  CarVinMatchStPopUp: ------------>");
	for(i = 0;i < 6;i++)
	{
		DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"%02x ",Hbbuf[i]);
	}
	DEBUG_MCU(DBG_ERROR,CAN_MODULE_ID,"\r\n");
	#endif
	Rte_IWrite_Fun_TboxLogic_Call_UARTTX_Buff_UARTTX_Buff(Hbbuf);

}

void PowerOn_Sync_St(void)
{
	DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n  PowerOn_Sync_St------------------------------------>");
	PowerOnSyncStPopUpStep = 1;
	PowerOnR4RcvMsgReady = TRUE;
}

void PowerOnSyncStHandle(void)
{
	switch(PowerOnSyncStPopUpStep)
	{
		case 0:
		break;
		case 1:
			HeartBeatDataPopUp();
			PowerOnSyncStPopUpStep = 2;
		break;
		case 2:
			CarEngineStPopUp();
			PowerOnSyncStPopUpStep = 3;
		break;
		case 3:
			CarBodyStPopUp();
			PowerOnSyncStPopUpStep = 4;
		break;
		case 4:
			CarAlarmStPopUp();
			PowerOnSyncStPopUpStep = 5;
		break;
		case 5:
			CarCrashStPopUp();
			PowerOnSyncStPopUpStep = 6;
		break;
		case 6:
			CarTireWarningStPopUp();
			PowerOnSyncStPopUpStep = 7;
			PowerOnSyncVinCodePopUpTime = 0;
		break;
		case 7:
			PowerOnSyncVinCodePopUpTime++;
			if(PowerOnSyncVinCodePopUpTime<4)
			{
				CarVinMatchStPopUp();
			}
			else
			{
				PowerOnSyncStPopUpStep = 0;
			}
		break;
	}
}

void HeartBeatDataHandle(void)//心跳数据上传总处理函数
{
	static u32 caltimer = ((u32)15000u/TIMER_BASE_PERIOD);//((u32)5000u/TIMER_BASE_PERIOD);

	if(caltimer)//15s上传一次
	{
		caltimer--;
		if(caltimer==0)
		{
			caltimer = ((u32)15000u/TIMER_BASE_PERIOD);//((u32)5000u/TIMER_BASE_PERIOD);
		    HeartBeatDataPopUp();
			DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n  HeartBeatDataPopUp ------------>");
		}
	}
}

void CarSelfTestHandle(void)///自检上报处理
{
	if(Car_SelfTest_Data.CAR_SelfTestTimeOut)
	{
		Car_SelfTest_Data.CAR_SelfTestTimeOut--;
		if(Car_SelfTest_Data.CAR_SelfTestTimeOut == 0)
		{
			Car_SelfTest_Data.CAR_SelfTestAllowPop = 1;
			Car_SelfTest_Data.CAR_SelfTestdatachInd = 0;
			CarSelfTestPopUp();
			DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n  CarSelfTestPopUp ------------>");
		}
	}
}

void CarBatteryInsertVinMatchInit(void)
{
	DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n  CarBatteryInsertVinMatchInit ------------>");
//	Set_VinMatchSt(TBOX_PEPS_VIN_NOT_MATCHED);
	S_VinChkCtrl.RequestFlag = false;
    S_VinChkCtrl.uiIgnOnTimer = 0;
    S_VinChkCtrl.siRxTimer = 0;
    S_VinChkCtrl.siTxCancelTimer= 0;
    S_VinChkCtrl.uiMatchFailedCnt = 0;
    S_VinChkCtrl.uiMatchSt = TBOX_PEPS_VIN_NOT_MATCHED;

}
void CarVinCodeMatchInit(void)
{
	u8 i;
	
	S_VinChkCtrl.RequestFlag = false;
    S_VinChkCtrl.uiIgnOnTimer = 0;
    S_VinChkCtrl.siRxTimer = 0;
    S_VinChkCtrl.siTxCancelTimer= 0;
    S_VinChkCtrl.uiMatchFailedCnt = 0;
  	//S_VinChkCtrl.uiMatchSt = Get_VinMatchSt();
	
	DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	CarVinCodeMatchInit:%d ----->",S_VinChkCtrl.uiMatchSt);

	for(i = 0;i < 17;i++)
	{
		PEPS_VIN[i] = 0;
	}
	
    if( TBOX_PEPS_VIN_NOT_MATCHED == S_VinChkCtrl.uiMatchSt )
    {
        TBOX_Command_VinCodeNoMatch();
    }
    else if( TBOX_PEPS_VIN_MATCHED == S_VinChkCtrl.uiMatchSt )
    {
        TBOX_Command_VinCodeMatched();
	// CAN_Did_W_VIN_Match_Status(&S_VinChkCtrl.uiMatchSt, 1);
    }
	else if( TBOX_PEPS_VIN_MATCHED_FAILED == S_VinChkCtrl.uiMatchSt )
    {
        TBOX_Command_VinCodeMatchFailed();
	S_VinChkCtrl.uiMatchSt = TBOX_PEPS_VIN_NOT_MATCHED;
//	Set_VinMatchSt(TBOX_PEPS_VIN_NOT_MATCHED);
    }
	
	
    TBOX_Command_VinCodeNoReq();

}

void  CAN_VinChkCtrlStart(void)
{
    S_VinChkCtrl.RequestFlag=false;
    S_VinChkCtrl.uiIgnOnTimer= (10000u/TIMER_BASE_PERIOD) ;
	S_VinChkCtrl.siTxCancelTimer = 0;
    S_VinChkCtrl.siRxTimer=0;
    S_VinChkCtrl.uiMatchFailedCnt=3;
    DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n  ---->M3 CAN: Delay 10S Request PEPS VIN ------------>\r\n");
}
void CAN_VinChkCtrlStop(void)
{
    S_VinChkCtrl.RequestFlag = false;
    S_VinChkCtrl.uiIgnOnTimer = 0;
    S_VinChkCtrl.siRxTimer=0;
	S_VinChkCtrl.siTxCancelTimer = 0;
    S_VinChkCtrl.uiMatchFailedCnt=0;
	TBOX_Command_VinCodeNoReq();
    DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n  ---->M3 CAN: Stop Request PEPS VIN ------------>\r\n");
}

u8 IsEqualDefaultVincode(void)
{
	u8 i;
	
	for(i = 0;i < 17;i++)
	{
		if(TboxSaveVinCode[i] != 0)
		{
			break;
		}
	}

	if(i == 17)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}

}


void  CAN_VinChkCtrlTimeScan(void)
{
	if(S_VinChkCtrl.uiIgnOnTimer)
	{
		S_VinChkCtrl.uiIgnOnTimer--;
		if(S_VinChkCtrl.uiIgnOnTimer == 0)
		{
			S_VinChkCtrl.uiIgnOnTimer = 0;
			if(IsEqualDefaultVincode() == TRUE)
			{
				S_VinChkCtrl.RequestFlag = false;
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	---->M3 CAN: Stop By No SaveVINCODE------------>\r\n");

			}
			else
			{
				S_VinChkCtrl.RequestFlag=true;
				S_VinChkCtrl.uiMatchFailedCnt=3;
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	---->M3 CAN: Start VinChkCtrl-Request------------>\r\n");
			}			
		}
	}
	
    if(TRUE == S_VinChkCtrl.RequestFlag)
    {
        if(TBOX_PEPS_VIN_MATCHED != S_VinChkCtrl.uiMatchSt )
        {
            if(S_VinChkCtrl.siRxTimer>0) S_VinChkCtrl.siRxTimer--;
            if(0==S_VinChkCtrl.siRxTimer)
            {
                if(S_VinChkCtrl.uiMatchFailedCnt>0)
                {
                    S_VinChkCtrl.uiMatchFailedCnt--;
                    TBOX_Command_VinCodeReq();
					S_VinChkCtrl.siTxCancelTimer = SENDONCETIME;
					S_VinChkCtrl.siRxTimer = 500u/TIMER_BASE_PERIOD;
                }
                else if (0==S_VinChkCtrl.uiMatchFailedCnt)
                {
                	S_VinChkCtrl.uiMatchSt = TBOX_PEPS_VIN_NOT_MATCHED;
				//	Set_VinMatchSt(TBOX_PEPS_VIN_NOT_MATCHED);
                    CAN_VinChkCtrlStop();
					TBOX_Command_VinCodeMatchFailed();
					if((PowerOnR4RcvMsgReady == TRUE)&&(PowerOnSyncStPopUpStep == 0||PowerOnSyncStPopUpStep == 7))
					{
						PowerOnSyncVinCodePopUpTime = 0;
						PowerOnSyncStPopUpStep = 7;
					}

					DEBUG_MCU(DBG_ERROR,CAN_MODULE_ID,"\r\n	---->CAN: VinCode Match Failed ------------>\r\n");
                }
            }

        }
        else
        {
            CAN_VinChkCtrlStop();
        }

		if(S_VinChkCtrl.siTxCancelTimer)
		{
			S_VinChkCtrl.siTxCancelTimer--;
			if(S_VinChkCtrl.siTxCancelTimer == 0)
			{
				TBOX_Command_VinCodeNoReq();
			}
		}
		
    }
}

void CarVinCodeMatchHandle(void)///Vin code match process 
{
	if(false==AccIsOnStatusBak)
	{
		if(TRUE==IGN_AccStatusIsOn())
		{
			AccIsOnStatusBak=true;
			CAN_VinChkCtrlStart();
		}
	}
	else
	{
		if(FALSE==IGN_AccStatusIsOn())
		{
			AccIsOnStatusBak=false;
			CAN_VinChkCtrlStop();
		}
	}

	CAN_VinChkCtrlTimeScan();

}

// 急转弯，当车速大于9米/s,角速度>45°/s，视为急转弯
//Steering wheel angle (N : value of the message) :
//= N × 0.1 , for 0 ≤ N ≤ 32767
//= (N-65536) × 0.1 , for N ＞ 32767
//Note:        that Clockwise rotation  is ‘-‘ and anti- Clockwise rotation  is ‘+‘

void Sudden_Turning(u16 angle)///period is 40ms
{
	static u16 timecout = 0;
	static u8 gpsPopCount = 0;
	u16 temp = 0;

	if (angle >= 0xE188)
		angle -= 0xE188;	//将左转向变为0-0X1E77
	else
		angle += 0x1E77;	//将右转向变为0x1E77-0XFFFF
	
	timecout++;
	if (timecout > 25)///1.2s
	{
		timecout = 0;
		currentangle = angle;
		
		gpsPopCount++;
		if(ESP_VehicleSpeed > 320)
		{
			if((7849<currentangle)||(currentangle<7749))
			{
				if(gpsPopCount>=5)
				{
					gpsPopCount = 0;
					DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	Current wheelAngel is ------- %d -------- \r\n", currentangle);
					DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	---->CAN: GPS POPup ------------>\r\n");
					CarGPSPopup();
				}
			}else if(((7749<=currentangle)&&(currentangle<=7769))||((7829<=currentangle)&&(currentangle<=7849)))
			{
				if(gpsPopCount>=5)
				{
					gpsPopCount = 0;
					DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	Current wheelAngel is ------- %d -------- \r\n", currentangle);
					DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	---->CAN: GPS POPup ------------>\r\n");
					CarGPSPopup();
				}
			}
			
		}
		if (currentangle >= lastangle)
		{
			temp = currentangle - lastangle;
		}
		else
		{
			temp = lastangle - currentangle;
		}

		if (temp > 450)
		{
			if (ESP_VehicleSpeed > 576)//9m/s
			{
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	---->M3 CAN: Sudden_Turning ------------>\r\n");
				CarDriveBehaviourPopup(Sudden_TURN);
			}
		}
		
		lastangle = currentangle;
	}
}

//急加速，当加速度大于 5m/S2，视为急加速
//急减速，当减速度小于 -5m/S2，视为急减速
void Sudden_SpeedUpOrDown(u16 vspeed)
{
	static u8 timecout = 0;

	timecout++;
	if (timecout >= 14)///80ms*14 = 1120ms 
	{
		timecout = 0;
		currentspeed = vspeed;
		
		if (currentspeed >= lastspeed)
		{
			dith_spd_anti = 0;
			dith_spd = currentspeed - lastspeed;
			
			if((dith_spd >= 160) || ((dith_spd >= 80) && (lastspeed>640)) || ((dith_spd >= 64) && (lastspeed>1280)))
			{
				if(speed_up_happen == 0)
				{
					speed_up_happen = 1;
					CarDriveBehaviourPopup(Sudden_Speedup);
				}
			}else
			{
				speed_up_happen = 0;
			}
			
			if (dith_spd > 176)
			{	
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	---->CAN: Sudden_SpeedUp    >2.75m/S2----------------->\r\n");
			}else if(dith_spd > 160)
			{
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	---->CAN: Sudden_SpeedUp    >2.5m/S2----------------->\r\n");
			}else if(dith_spd > 144)
			{
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	---->CAN: Sudden_SpeedUp    >2.25m/S2----------------->\r\n");
			}else if(dith_spd > 128)
			{
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	---->CAN: Sudden_SpeedUp    >2m/S2----------------->\r\n");
			}else if(dith_spd > 120)
			{
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	---->CAN: Sudden_SpeedUp    >1.875m/S2----------------->\r\n");
			}else if(dith_spd > 112)
			{
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	---->CAN: Sudden_SpeedUp    >1.75m/S2----------------->\r\n");
			}else if(dith_spd > 104)
			{
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	---->CAN: Sudden_SpeedUp    >1.625m/S2----------------->\r\n");
			}else if(dith_spd > 96)
			{
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	---->CAN: Sudden_SpeedUp    >1.5m/S2----------------->\r\n");
			}else if(dith_spd > 88)
			{
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	---->CAN: Sudden_SpeedUp    >1.375m/S2----------------->\r\n");
			}else if(dith_spd > 80)
			{
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	---->CAN: Sudden_SpeedUp    >1.25m/S2----------------->\r\n");
			}else if(dith_spd > 72)
			{
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	---->CAN: Sudden_SpeedUp    >1.125m/S2----------------->\r\n");
			}else if(dith_spd > 64)
			{
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	---->CAN: Sudden_SpeedUp    >1m/S2----------------->\r\n");
			}else if(dith_spd > 48)
			{
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	---->CAN: Sudden_SpeedUp    >0.75m/S2----------------->\r\n");
			}else if(dith_spd > 32)
			{
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	---->CAN: Sudden_SpeedUp    >0.5m/S2----------------->\r\n");
			}
		}
		else
		{
			dith_spd_anti = 1;
			dith_spd = lastspeed - currentspeed;

			if(dith_spd > 192)
			{
				if(speed_down_happen == 0)
				{
					speed_down_happen = 1;
					CarDriveBehaviourPopup(Sudden_Speeddown);
				}
			}else
			{
				speed_down_happen = 0;
			}
			
			
			if (dith_spd > 320) 
			{
				DEBUG_MCU(DBG_ERROR,CAN_MODULE_ID,"\r\n	---->CAN: Sudden_SpeedDown    >5m/S2-------------->\r\n");
			}else if(dith_spd > 256)
			{
				DEBUG_MCU(DBG_ERROR,CAN_MODULE_ID,"\r\n	---->CAN: Sudden_SpeedDown    >4m/S2----------------->\r\n");
			}else if(dith_spd > 224)
			{
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	---->CAN: Sudden_SpeedDown    >3.5m/S2----------------->\r\n");
			}else if(dith_spd > 192)
			{
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	---->CAN: Sudden_SpeedDown    >3m/S2----------------->\r\n");
			}else if(dith_spd > 176)
			{
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	---->CAN: Sudden_SpeedDown    >2.75m/S2----------------->\r\n");
			}else if(dith_spd > 160)
			{
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	---->CAN: Sudden_SpeedDown    >2.5m/S2----------------->\r\n");
			}else if(dith_spd > 144)
			{
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	---->CAN: Sudden_SpeedDown    >2.25m/S2----------------->\r\n");
			}else if(dith_spd > 128)
			{
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	---->CAN: Sudden_SpeedDown    >2m/S2----------------->\r\n");
			}else if(dith_spd > 120)
			{
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	---->CAN: Sudden_SpeedDown    >1.875m/S2----------------->\r\n");
			}else if(dith_spd > 112)
			{
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	---->CAN: Sudden_SpeedDown    >1.75m/S2----------------->\r\n");
			}else if(dith_spd > 104)
			{
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	---->CAN: Sudden_SpeedDown    >1.625m/S2----------------->\r\n");
			}else if(dith_spd > 96)
			{
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	---->CAN: Sudden_SpeedDown    >1.5m/S2----------------->\r\n");
			}else if(dith_spd > 88)
			{
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	---->CAN: Sudden_SpeedDown    >1.375m/S2----------------->\r\n");
			}else if(dith_spd > 80)
			{
				DEBUG_MCU(DBG_INFO,CAN_MODULE_ID,"\r\n	---->CAN: Sudden_SpeedDown    >1.25m/S2----------------->\r\n");
			}
		}

		lastspeed = currentspeed;
	}
}

void COM_CmdRmtCtrlRsp(u16 ucCtrlByte, u8 ucResult, u8 ucFailedCode)
{
	ctrl_TX_TypeDef Buf;
	ctrl_TX_TypeDef *pBuf = &Buf;

	pBuf->length[0] = 0x00;
	pBuf->length[1] = 0x00;
	pBuf->length[2] = 0x00;
	pBuf->length[3] = 0x0c;
	pBuf->channelNo[0] = 0x11;
	pBuf->channelNo[1] = 0x11;
	pBuf->msgId[0] = (u8)(ucCtrlByte>>8);
	pBuf->msgId[1] = (u8)(ucCtrlByte&0x00ff);
	pBuf->result = ucResult;
	pBuf->reseaon = ucFailedCode;
	memcpy(pBuf->requestid, g_RequestId, 6);
	Rte_IWrite_Fun_TboxLogic_Call_UARTTX_Buff_UARTTX_Buff((u8 *)&(pBuf->length[3]));
}

/*****************************************************************************
**                            End Of File
******************************************************************************/


