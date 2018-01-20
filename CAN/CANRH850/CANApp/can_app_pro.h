/////////////////////////////////////////////////////////////////////////////////
#ifndef CAN_APP_PRO_H
#define CAN_APP_PRO_H
/////////////////////////////////////////////////////////////////////////////////
//#include  "mcu_config.h"
#include  "can_config.h"
#include "base_types.h"
#include "CanAppMiddle.h"

///��Ӧ����void COM_CmdRmtCtrlRsp(u8 ucCtrlByte, u8 ucResult, u8 ucFailedCode)�е�ucCtrlByte
typedef enum{
	rmote_ctrl_none=0x0,
	rmote_ctrl_lock=0x0004,//Զ�̱���
	rmote_ctrl_unlock=0x0003,//Զ�̽���
	rmote_ctrl_lkforcar=0x000b,//Զ��Ѱ��
	rmote_ctrl_send2car=0x0011,//send2car
	rmote_ctrl_openac=0x0007,//Զ�̿��յ�
	rmote_ctrl_closeac=0x0008,//Զ�̹ؿյ�
	rmote_ctrl_opensunroof=0x0016,//Զ�̿��촰
	rmote_ctrl_closesunroof=0x0017,//Զ�̹��촰
	rmote_ctrl_cartrack=0x0083,//��������׷��
	rmote_ctrl_caruntrack=0x0084,//�������׷��
	rmote_upgrade_request=0x0015,//Զ����������
} rmt_ctrl;

enum{
	rmote_process_noallow,
	rmote_process_allowstep1,
	rmote_process_allowstep2,
	rmote_process_allowstep3,
} ;

///��Ӧ����void COM_CmdRmtCtrlRsp(u8 ucCtrlByte, u8 ucResult, u8 ucFailedCode)�е�ucResult��ucFailedCode
#if 0
///BB�˶�Ӧ������
enum{
	
	RMTCTRL_NOERR,//Զ�̿����޴���

	///�������ӦBB�˴�����  ucResult
	RMTCTR_SUCCEED = 0, //Զ�̿��Ƴɹ�
	RMTCTR_FAILED,//Զ�̿���ʧ��
	KEYNOOFF = 2,//��OFF��
	INPROCESSOFCMDL,//����ִ������ָ��
	MSG_CMD_NOT_SUPPORT,//���֧��
	ALREADY_TARGET_STAT,//��ǰ�Ѿ����ڱ���״̬
	VIN_NOT_MATE,//VIN�벻ƥ��

	///Զ��Ѱ����ӦBB�˴�����
	CAR_RUNNING = 2,//������������״̬

	RMTCTRL_ERR_TIMEOUT = 0xff,
};
#endif
#if 1
///BB�˶�Ӧ������
enum{
	
	RMTCTRL_NOERR,//Զ�̿����޴���
 //-------------ucResult start----------//
	rRMTCTR_SUCCEED = 0,//Զ�̿��Ƴɹ�
	rRMTCTR_FAILED,//Զ�̿���ʧ��
	rKEYNOOFF,//��OFF��
	rINPROCESSOFCMDL,//����ִ������ָ��
	rMSG_CMD_NOT_SUPPORT,//���֧��
	rALREADY_TARGET_STAT,//��ǰ�Ѿ����ڱ���״̬
	rVIN_NOT_MATE,//VIN�벻ƥ��
	rSPEED_LARGE =  8,
	rNOT_REMOTE_CTRL,

//-------------ucResult end----------//

//-------------ucFailedCode start----------//
	eRMTCTRL_NOFAILEDCODE = 0,
	///����������
	eUNLOCK_KEYNOOFF = 1,//��OFF��
	eUNLOCK_HOTPROTECT,
	eUNLOCK_UNKNOWN,

	///����������
	eLOCK_DOORDONTCLOSE = 2,
	eLOCK_BACKBOXDONTCLOSE,
	eLOCK_KEYNOOFF,//��OFF��
	eLOCK_HOTPROTECT,
	eLOCK_UNKNOWN,
	
	///���ƿյ�������
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

	///Զ��Ѱ����ӦBB�˴�����
	CAR_RUNNING = 2,//������������״̬

};
#endif

#if 1
////����CANЭ���Ӧ����״̬
enum{
	
	///�������ش�����
	RMTCTRLLOCK_ERR_DOOROPEN = 2,//����δ��
	RMTCTRLLOCK_ERR_TRUNKOPEN,//������δ��
	RMTCTRLLOCK_ERR_KEYNOOFF ,//��OFF��
	RMTCTRLLOCK_ERR_HOTPROTECT,//�ȱ���

	///�������ش�����
	RMTCTRLUNLOCK_ERR_HOTPROTECT = 2,//�ȱ���
	RMTCTRLUNLOCK_ERR_KEYNOOFF,//��OFF��
	
	//RMTCTRL_ERR_OTHERCMDEXECUTING = 3,
	//RMTCTRL_ERR_TIMEOUT = 0xff,
};
#endif

////ͨѶЭ�鵽R4
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

