/////////////////////////////////////////////////////////////////////////////////
#ifndef CAN_APP_PRO_H
#define CAN_APP_PRO_H
/////////////////////////////////////////////////////////////////////////////////
//#include  "mcu_config.h"
#include  "can_config.h"
#include "base_types.h"
#include "CanAppMiddle.h"

///对应函数void COM_CmdRmtCtrlRsp(u8 ucCtrlByte, u8 ucResult, u8 ucFailedCode)中的ucCtrlByte
typedef enum{
	rmote_ctrl_none=0x0,
	rmote_ctrl_lock=0x0004,//远程闭锁
	rmote_ctrl_unlock=0x0003,//远程解锁
	rmote_ctrl_lkforcar=0x000b,//远程寻车
	rmote_ctrl_send2car=0x0011,//send2car
	rmote_ctrl_openac=0x0007,//远程开空调
	rmote_ctrl_closeac=0x0008,//远程关空调
	rmote_ctrl_opensunroof=0x0016,//远程开天窗
	rmote_ctrl_closesunroof=0x0017,//远程关天窗
	rmote_ctrl_cartrack=0x0083,//开启防盗追踪
	rmote_ctrl_caruntrack=0x0084,//解除防盗追踪
	rmote_upgrade_request=0x0015,//远程升级请求
} rmt_ctrl;

enum{
	rmote_process_noallow,
	rmote_process_allowstep1,
	rmote_process_allowstep2,
	rmote_process_allowstep3,
} ;

///对应函数void COM_CmdRmtCtrlRsp(u8 ucCtrlByte, u8 ucResult, u8 ucFailedCode)中的ucResult和ucFailedCode
#if 0
///BB端对应错误码
enum{
	
	RMTCTRL_NOERR,//远程控制无错误

	///解闭锁对应BB端错误码  ucResult
	RMTCTR_SUCCEED = 0, //远程控制成功
	RMTCTR_FAILED,//远程控制失败
	KEYNOOFF = 2,//非OFF档
	INPROCESSOFCMDL,//正在执行其他指令
	MSG_CMD_NOT_SUPPORT,//命令不支持
	ALREADY_TARGET_STAT,//当前已经处于闭锁状态
	VIN_NOT_MATE,//VIN码不匹配

	///远程寻车对应BB端错误码
	CAR_RUNNING = 2,//车辆处于运行状态

	RMTCTRL_ERR_TIMEOUT = 0xff,
};
#endif
#if 1
///BB端对应错误码
enum{
	
	RMTCTRL_NOERR,//远程控制无错误
 //-------------ucResult start----------//
	rRMTCTR_SUCCEED = 0,//远程控制成功
	rRMTCTR_FAILED,//远程控制失败
	rKEYNOOFF,//非OFF档
	rINPROCESSOFCMDL,//正在执行其他指令
	rMSG_CMD_NOT_SUPPORT,//命令不支持
	rALREADY_TARGET_STAT,//当前已经处于闭锁状态
	rVIN_NOT_MATE,//VIN码不匹配
	rSPEED_LARGE =  8,
	rNOT_REMOTE_CTRL,

//-------------ucResult end----------//

//-------------ucFailedCode start----------//
	eRMTCTRL_NOFAILEDCODE = 0,
	///解锁错误码
	eUNLOCK_KEYNOOFF = 1,//非OFF档
	eUNLOCK_HOTPROTECT,
	eUNLOCK_UNKNOWN,

	///闭锁错误码
	eLOCK_DOORDONTCLOSE = 2,
	eLOCK_BACKBOXDONTCLOSE,
	eLOCK_KEYNOOFF,//非OFF档
	eLOCK_HOTPROTECT,
	eLOCK_UNKNOWN,
	
	///控制空调错误码
	eAC_OPENTOOMUCHTIMES = 2,
	eAC_LOWBATTERY,
	eAC_FRONTBACKDOORNOTCLOSE,
	eAC_KEYINTHECAR,
	eAC_ALARMACTIVE,
	eAC_EMERGENCYLIGHTON,
	eAC_NOTATPGEAR,
	eAC_CARSPEEDNOZERO,
	eAC_ENGINEISRUNNING,
	eAC_EPBNOTHOLD,
	eAC_OILLOW,
	eAC_STARTSYSTEMBROKEN,
	eAC_ENGINEOILLOWPRESSURE,
	eAC_ENGINECOOLANTTEMTOOHIGH,
	eAC_EFISYSTEMTROUBLE,
	eAC_RUNNNIGTIMEOUTTOENGINESTOP,
	eAC_ENVIRONMENTTEMPTOOLOW,
	eAC_ENVIRONMENTAROUNDALARMNOTON,
	eAC_ROMTEENGINESTOP,
	eAC_UNKNOWN,

	eAC_TEMPERTRUEAIRDOOR = 22,
	eAC_MODEAIRDOOR,
	eAC_BLOWERTROUBLE,
	eAC_TRISTATETROUBLE,
	eAC_EVAPORATORTEMPTROUBLE,


//-------------ucFailedCode end----------//


	RMTCTRL_ERR_TIMEOUT = 0xff,

	///远程寻车对应BB端错误码
	CAR_RUNNING = 2,//车辆处于运行状态

};
#endif

#if 1
////本地CAN协议对应错误状态
enum{
	
	///闭锁本地错误码
	RMTCTRLLOCK_ERR_DOOROPEN = 2,//车门未关
	RMTCTRLLOCK_ERR_TRUNKOPEN,//行李箱未关
	RMTCTRLLOCK_ERR_KEYNOOFF ,//非OFF档
	RMTCTRLLOCK_ERR_HOTPROTECT,//热保护

	///解锁本地错误码
	RMTCTRLUNLOCK_ERR_HOTPROTECT = 2,//热保护
	RMTCTRLUNLOCK_ERR_KEYNOOFF,//非OFF档
	
	//RMTCTRL_ERR_OTHERCMDEXECUTING = 3,
	//RMTCTRL_ERR_TIMEOUT = 0xff,
};
#endif

////通讯协议到R4
#define POPTYPE_CMD1 ((u8)(0x09))

enum{
	CAN_CMD_HAEARBEAT_POP,
	CAN_CMD_ENGINEST_POP,
	CAN_CMD_SELFTEST_POP,
	CAN_CMD_VEHST_POP,
	CAN_CMD_ALARM_POP,
	CAN_CMD_CRASH_POP,
	CAN_CMD_TIREWARNING_POP,
	CAN_CMD_DRIVER_behaviour,
	CAN_CMD_GPS_POP,
};

//drive behivaiour type
enum{
	Sudden_Speedup,
	Sudden_Speeddown,
	Sudden_TURN,
};

typedef union
{
	u8  CarContrlFunction[2];
	struct
	{
		u8	CarContrl_EmergencyRescue  :1;  // Bit0 E-CALL
		u8	CarContrl_RoadRescue	:1;  // Bit1   B-CALL
		u8	CarContrl_RemoteVehicleImage  :1;  // Bit2 veh around Image
		u8	CarContrl_RemoteVehicleVedio  :1;  // Bit3 veh around Vedio
		u8	CarContrl_RemoteAcCTL  :1;  // Bit4 Remote AirCondiciton Ctrl
		u8	CarContrl_RemoteSunRoofCTL  :1;  // Bit5  Remote SunRoof Ctrl
		u8	CarContrl_RemoteSend2Car  :1;  // Bit6 Remote Send2Car Ctrl
		u8	CarContrl_RemoteBackTrunk  :1;  // Bit7 Remote Trunk Ctrl
		
		u8	CarContrl_RemoteVehicleTrack  :1;	// Bit7  Remote Vehc Track
		u8  CarContrl_Reverse:7;

	} bits;
	
} Car_Ctrl_Func_UTypeDef;


#define TBOX_PEPS_VIN_NOT_MATCHED           ((u32)2)//((u32)0)
#define TBOX_PEPS_VIN_MATCHED               ((u32)1)
#define TBOX_PEPS_VIN_MATCHED_FAILED        ((u32)0)//((u32)2)

void initCANBackUpData(BackUpRAMPara_STypeDef* pData);
void CAN_AppProcInit(void);
void CanAppProcTask(void);
void Car_Contrl_CmdRev(u8 *msg);

void PowerOn_Sync_St(void);
void Car_Control_FunctionConfig(void);
void CarBatteryInsertVinMatchInit(void);


/////////////////////////////////////////////////////////////////////////////////
#endif
/*****************************************************************************
**                            End Of File
******************************************************************************/

