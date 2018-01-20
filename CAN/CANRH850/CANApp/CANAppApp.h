#ifndef CAN_APP_APP_H
#define CAN_APP_APP_H



#include "base_types.h"
#include "sysconfig.h"
#include "MsgDefine.h"
#include "CAN.h"

#include "CanAppMiddle.h"

#ifdef _CAN_APP_APP_ROOT
    #define EXTERN
#else
    #define EXTERN extern
#endif
#define CAN_POLLINGTIME     4

#define FRAME_SIZE 8

#if 0
typedef struct{
   // Event   Super;
	u32 id;
	u8  length;
	u8  data[FRAME_SIZE];
}can_frame;
#endif

typedef struct
{
    u32 ID;
    u8  Datas[8];
    u8  DataLen;
}CANDataType;


typedef struct
{
	u8 data;
	
}CanMsgAppFrame_t;

typedef struct
{
	u8 data;
	
}CanMsgDiagFrame_t;


typedef struct
{
	Message_t   Super;
    can_frame frame;
}CanRxDataEvt_t;

typedef struct
{
	Message_t   Super;
    can_frame frame;
}CanTxDataEvt_t;

typedef struct
{
	Message_t   Super;
    u8 data;
}CanTxOkEvt_t;

typedef struct
{
	Message_t   Super;
	union 
	{
		CanMsgAppFrame_t app;
		CanMsgDiagFrame_t diag;
	}info;
	
}CanMsgDataEvt_t;

#define CAN_TOL_CHN_NUMS   3
EXTERN u8 CAN_Err[CAN_TOL_CHN_NUMS];
#define BUSOFF  (CAN_Err[CAN_USE_CHN_NUM] == 0x01)

void CreatCanAppTask(u8 TaskPri);
void CreatCanNmProcTask(u8 TaskPri);


#define CHANNEL0    0x00
#define CHANNEL3    0x03


#define EVENT_CYCLE_50MS       50
#define EVENT_3_TIMES          3

#define EVENT_ID_600  0x600
#define EVENT_ID_601  0x601
#define EVENT_ID_515  0x515

#define PERIODIC_FRAME   0xFE
#define EVENT_FRAME      0x00


enum
{
	Closed,
	opened,
	Invalid,
	Reserved,
};

typedef struct 
{   
    
    u16 CAN_ID;
    u16 cycle;       
    u32 system_tick_times;
    u16  transmit_times;
    u8  channel;
}CAN_tx_frame_t;

typedef struct 
{
    /*****************************
    0-Luxury type
    1-Athletic
    2-Distinguished type
    3-Luxury Athletic type
    4-Zhizhong version-48V
    5-Zhizhong version
    ***************************/
    u8 models;
    u8 data[16];
}OfflineConfiguration_t ;

typedef struct 
{
    /*****************************
    Bit7～Bit6：Right front door state
    0-Closed
    1-opend
    2-invalid
    Bit5～Bit4：Left front door state
    0-Closed
    1-opend
    2-invalid
    Bit3～Bit2：Right back door state
    0-Closed
    1-opend
    2-invalid
    Bit1～Bit0：Left back door state
    0-Closed
    1-opend
    2-invalid
    ***************************/
    u8 RightFrontDoor:2;
    u8 LeftFrontDoor:2;
    u8 RightBackDoor:2;
    u8 LeftBackDoor:2;
    /*****************************
    Bit1～Bit0：Trunk Door state
    0-Closed
    1-opend
    2-invalid
    ******************************/
    u8 unused1:6;
    u8 TrunkDoor:2;
    /*****************************
    Bit3～Bit2：High beam working status
    0-Closed
    1-opend
    2-invalid
    3-Reserved
    Bit1～Bit0：Low beam working condition
    0-Closed
    1-opend
    2-Unused
    3-Reserved
    ******************************/
    u8 unused2:4;
    u8 HighBeamWorking:2;
    u8 LowBeamWorking:2;
}DoorLightsState_t;

#if 0
typedef struct 
{
    u8 LowBeamStatus:2;
    u8 HighBeamStatus:2;
    u8 FrontFoglampStatus:2;
    u8 RearFoglampStatus:2; 
    u8 TurnIndicatorLeft:2;
    u8 TurnIndicatorRight:2;
    u8 PositionLampStatus:2;    
}LightsState_t;

typedef struct 
{
    u8 DriverDoorStatus:1;
    u8 PassengerDoorStatus:1;
    u8 LeftRearDoorStatus:1;
    u8 RightRearDoorStatus:1; 
    u8 TrunkStatus:1;
    u8 HoodStatus:1;
    u8 SunroofStatus:1; 
}DoorState_t;
#endif

typedef struct 
{
    /*****************************
    Bit1～Bit0：Engine operating conditions
    0-Stopped
    1-Ignition
    2-Run
    3-Reserved
    4-Idling
    ***************************/
    u8 unused1:6;
    u8 EngineOperation:2;
    
}EngineRunStatus_t ;

typedef struct 
{
    /*****************************
    Bit1~Bit0 Speed signal validity
    0-valid
    1-invalid
    Bit7~Bit2 Reserved
    ***************************/
    u8 valid;
    u8 high;
    u8 low;
}CarSpeed_t ;

typedef struct 
{
    /*****************************
    Bit7～Bit6：EngineFaultIndicator
    0-Closed，The indicator goes off
    1-Opened，The indicator turnes on
    2-Reversed
    3-error
    Bit5～Bit4：TCS/ESP Fault state
    0-Normal
    1-Fault
    2-invalid
    Bit3～Bit2：Engine coolant temperature fault condition
    0-Normal
    1-Fault
    2-invalid
    Bit1～Bit0：Engine speed fault condition
    0-Normal
    1-Fault
    2-invalid
    ***************************/
    u8 EngineFault:2;
    u8 TCSESPFault:2;
    u8 EngineCoolantTempFault:2;
    u8 EngineSpeedFault:2;
}EngineFailure_t ;

typedef struct 
{
    /*****************************
    Bit1～Bit0：Accelerator pedal failure status
    0-Normal
    1-Fault
    2-invalid
    Bit7～Bit2：Reversed
    ***************************/
    u8 unused0:6;
	u8 BrakePedalSta:2 ;
    
}BrakePedalStatus_t ;


typedef struct 
{
    /*****************************
    Bit3～Bit2：EBO Failure state
    0-Normal
    1-Fault
    2-invalid
    Bit1～Bit0：Power steering state
    0-Normal
    1-Fault
    2-invalid
    ***************************/
    u8 unused0:4;
    u8 EBOFailureState:2;
    u8 PowerSteeringState:2;
}EBDFailure_t; 

typedef struct 
{
    /*****************************
    Bit7~Bit6 Winter mode
    0-Normal
    1-Winter
    
    Bit5~Bit4 Singal Status
    0-Current singal
    1-Last singal
    
    Bit1~Bit0 Singal validity:
    0-Reliable signal
    1-Unreliable signal
    Bit3~Bit2 Reversed
    ***************************/
    u8 WinterMode:2;
    u8 SingalStatus:2;
    u8 unused0:2;
    u8 SingalValidity:2;
    /*****************************
    Tire temperature alarm
    Bit3: Left front wheel
    Bit2: Right front wheel
    Bit1: Left rear wheel
    Bit0: Right rear wheel
    0-Normal
    1-Temperature is too high
    Bit7~Bit4 Reversed
    ***************************/
    u8 unused1:4;
    u8 LeftFrontWheelTempAlarm:1;
    u8 RightFrontWheelTempAlarm:1;
    u8 LeftRearWheelTempAlarm:1;
    u8 RightRearWheelTempAlarm:1;
    /*****************************
    Tire Pressure Alarm
    Bit7～Bit4：Left front wheel
    Bit3～Bit0：Right front wheel
    0x1 ：Tire pressure is too high
    0x2 ：Tire pressure is too low
    0x3 ：Quick leak
    0x4 ：The sensor is missing
    0x5 ：The sensor voltage is too low
    0x6 ：Sensor failure
    ***************************/
    u8 LeftFrontWheelPressureAlarm:4;
    u8 RightFrontWheelPressureAlarm:4;

    
    /*****************************
    Tire Pressure Alarm
    Bit7～Bit4：Left Rear wheel
    Bit3～Bit0：Right Rear wheel
    0x1 ：Tire pressure is too high
    0x2 ：Tire pressure is too low
    0x3 ：Quick leak
    0x4 ：The sensor is missing
    0x5 ：The sensor voltage is too low
    0x6 ：Sensor failure
    ***************************/
    u8 LeftRearWheelPressureAlarm:4;
    u8 RightRearWheelPressureAlarm:4;

    /*****************************
    Air pressure(kpa)
    ***************************/
    u8 AirPressure;

    /*****************************
    LeftFrontTirePressure
    ***************************/
    u8 LeftFrontTirePressure;

    /*****************************
    RightFrontTirePressure
    ***************************/
    u8 RightFrontTirePressure;
    
    /*****************************
    LeftRearTirePressure
    ***************************/
    u8 LeftRearTirePressure;

    /*****************************
    LeftRearTirePressure
    ***************************/
    u8 LeftRightTirePressure;
    
}TirePressure_t; 

typedef struct 
{
	/*****************************
	TotalMileage
	***************************/
    u8 TotalMile[4];
    
}TotalMileage_t; 


typedef struct
{
    u8 LaneChangeTurnLightSetStatus:3;
    u8 unused0:5;
    u8 FollowMeHomeLightSetStatus  :3;
    u8 unused1:5;
}CarBodyLight_t;

typedef struct 
{
    /*****************************
    Bit1~Bit0 SteeringAngel signal validity
    0-valid
    1-invalid

    Bit3~Bit2 steering
    0-left
    1-right

    Bit7~Bit4 Reversed
    *****************************/
	u8 unused0:6;
    u8 valid:2;
	
	u8 high;
	u8 low;
}SteeringAngel_t; 

typedef struct
{
    u8 TCU_Gear:4;
}TCU_GearForDisplay_t;

#if 0
typedef struct 
{
    /*****************************
    Bit1～Bit0：Accelerator pedal failure status
    0-Normal
    1-Fault
    2-invalid
    Bit7～Bit2：Reversed
    ***************************/
    u8 unused0:6;
    u8 BrakePedalFailure:2 ;
    
}BrakePedalStatus_t ;
#endif

#if 0
typedef struct
{
    u8 unused0:2;
    u8 AC_switch:2;
    u8 Defrost:2;
    u8 AUTO:2;
    u8 unused1;
    u8 unused2;
    u8 AC_DriverReqTemp;
    u8 AC_PaReqTemp;
    u8 AC_DriverReqTempModel:5;
    u8 AC_FrBlowType:3;    
}AirConditionerControlInfo_t;

#endif

typedef struct
{   
    u8 AC_system_fault        :1;
    u8 ACStatus               :1;
    u8 unused0                :6;
    u8 AC_auto_status         :2;
    u8 AC_RecycleType         :2;
    u8 AC_FrAutoSts           :2;
    u8 AC_switch_status       :2;
    u8 AC_AutoWorkStatus      :2;
    u8 AC_FrBlowType          :4;
    u8 AutowindSet1  :2;
    u8 AutowindSet2  :2;
    u8 AutowindSet3  :2;
    u8 unused1       :2;   
}AC_ControlInfo_t;


typedef struct
{
    u8 valid;
    u8 EnvironmentalTemp;  
}EnvironmentalTemp_t;

typedef struct 
{
    u8 AverageFuelConsumption;
    u8 InstantaneousFuelConsumption;
    u8 FeasibleMileageHigh;
    u8 FeasibleMileagelow;
}OilConsumption_t;
    
typedef struct
{
    u8 ObstacleDistance;
    u8 ReversRadarStatus;
    u8 SensorsClosestToObstacles;
    u8 LeftFrontSenseObstacles   :4;
    u8 RightFrontSenseObstacles  :4;
    u8 LeftRearSenseObstacles    :4;
    u8 LeftRearMiddleSenseObstacles :4;
    u8 RightRearSenseObstacles      :4;
    u8 RightRearMiddleSenseObstacles:4;
    u8 NearestSensor;
}RadarInfo_t;

typedef struct
{
    u8 data;
}CameraStatusInfo_t;

typedef struct
{
    u8 data[36];
}TUIDInfo_t;

typedef struct
{
    u8 BCM_AutoIGNOFFUnlockSetStatus  :2;
    u8 AutomaticParkingUnlock         :2;
    u8 BCM_AutoSpeedLockSetStatus     :2;
    u8 unused0 :2;
        
    u8 unused1 :2;
    u8 BCM_RKEUnlockDoorTypeSetStatus :2;
    u8 unused3 :4;
}CarBodyLockStatus_t;


typedef union
{
    u8 LeaveMaintainDay_H;
    u8 LeaveMaintainDay_L;
    u8 LeaveMaintainMileageI_H;
    u8 LeaveMaintainMileageI_L;
}MainTainInfo_t;

typedef struct
{
    u8 BCM_MirrorAutoFoldSetStatus:2;
    u8 unused0:6;
}RearMirrorAutoFoldSta_t;

typedef struct
{
    u8 BCM_RearWiperSetStatus:2;
    u8 unused0:6;
}RearWiperSetSta_t;


typedef struct
{
	OfflineConfiguration_t			OfflineConfig;
	DoorLightsState_t				DoorLight;	
    AC_ControlInfo_t                AC_ControlInfo;
    EnvironmentalTemp_t             EnvironmentalTemp;
    CarSpeed_t                		speed;    
	EngineRunStatus_t				EngineRunSta;	
	EngineFailure_t				    EngineFailure;    
	BrakePedalStatus_t		        BrakePedalStatus;      
	TirePressure_t					TirePressure;    
	TotalMileage_t					TotalMileage; 
    CarBodyLight_t                  CarBodyLight;
	SteeringAngel_t					SteeringAngel;		
    OilConsumption_t                OilConsumption;
    RadarInfo_t                     RadarInfo;
    CameraStatusInfo_t              CameraStatusInfo;
    TUIDInfo_t                      TUIDInfo;
    CarBodyLockStatus_t             CarBodyLockStatus;
    MainTainInfo_t                  MainTainInfo;
    RearMirrorAutoFoldSta_t         RearMirrorAutoFoldSta;
    RearWiperSetSta_t               RearWiperSetSta;
}CarBodyInfo_t;

typedef union
{
	OfflineConfiguration_t			OfflineConfig;
	DoorLightsState_t				DoorLight;	
    AC_ControlInfo_t                AC_ControlInfo;
    EnvironmentalTemp_t             EnvironmentalTemp;
    CarSpeed_t                		speed;    
	EngineRunStatus_t				EngineRunSta;	
	EngineFailure_t				    EngineFailure;    
	BrakePedalStatus_t		        BrakePedalStatus;      
	TirePressure_t					TirePressure;    
	TotalMileage_t					TotalMileage; 
    CarBodyLight_t                  CarBodyLight;
	SteeringAngel_t					SteeringAngel;		
    OilConsumption_t                OilConsumption;
    RadarInfo_t                     RadarInfo;
    CameraStatusInfo_t              CameraStatusInfo;
    TUIDInfo_t                      TUIDInfo;
    CarBodyLockStatus_t             CarBodyLockStatus;
    MainTainInfo_t                  MainTainInfo;
    RearMirrorAutoFoldSta_t         RearMirrorAutoFoldSta;
    RearWiperSetSta_t               RearWiperSetSta;
}CarBodyInfo_tu;


typedef struct
{
    Message_t   Super;
    CarBodyInfo_tu data;
}CarBodyInfoEvt_t;


/**********************************************************/


typedef struct
{
    Message_t super;
    u8 data[4];
}HU_AC_info_RX;


#if 0
typedef struct
{   
    u8 AC_system_fault        :1;
    u8 AC_ACStatus            :1;
    u8 unused0                :6;
    u8 AC_auto_status         :2;
    u8 AC_RecycleType         :2;
    u8 AC_FrAutoSts           :2;
    u8 AC_switch_status       :2;
    u8 AC_AutoWorkStatus      :2;
    u8 AC_FrBlowType          :4;
    u8 HU_ACLFTempAdjustValueReq;
}HU_AC_info_RX;
#endif


/***********entertainment system message struct***********/

typedef struct
{
    Message_t  Super;
    u8 data[10];
}MPUSystemStatusInfo_t;

typedef struct
{
    Message_t  Super;
    u8 data[5];
}MusicStatusInfo_t;

typedef struct
{   
    Message_t  Super;
    u8 data[55];
}MusicLengthAndSingerNameInfor_t;

typedef struct
{
    Message_t  Super;
    u8 data[48];
}MusicAlbumNameInfo_t;


typedef struct
{
    Message_t  Super;
    u8 data[48];
}MusicSongNameInfo_t;


#if 0
typedef struct
{
    Message_t   Super;
    u8 MuteStatus;
    u8 CurSystemAudioSource;
    u8 CurSystemProgramSource;
    u8 SystemSpeechType;
    u8 MultimediaVolumeInfo;
    u8 MultimediaPlayControlStaInfo;
    u8 LocalMediaType;
    u8 LocalMediaStatusSignal1;
    u8 LocalMediaStatusSignal2;
    u8 RegesterStatus;
}MPUSystemStatusInfo;

typedef struct
{
    u8 CurMusicPlayStatus;
    u8 CurMusicPlayMode;
    u8 CurPlayMusicProgressTime_H;
    u8 CurPlayMusicProgressTime_M;
    u8 CurPlayMusicProgressTime_S;
}MusicStatusInfo;

typedef struct
{
    u8 MusicLenthTime_H;
    u8 MusicLenthTime_M;
    u8 MusicLenthTime_L;
    u8 CurMusicTruckNumber_H;
    u8 CurMusicTruckNumber_L;
    u8 TotalTruck_H;
    u8 TotalTruck_L;
    u8 SingerName[48];
}MusicLengthAndSingerNameInfor;

typedef struct
{
    u8 MusicAlbumName[48];
}MusicAlbumNameInfo;


typedef struct
{
    u8 SongNameInfo[48];
}MusicSongNameInfo;

#endif


/********message sent from COMMPU model for body control**********/

typedef struct
{
    Message_t  super;
    u8 data[9];
}HU_AC_info_set_t;


typedef struct
{
    Message_t  super;
    u8 data;
}HU_rearwiper_set_t;     

typedef struct
{
    Message_t  super;
    u8 data[9];
}HU_location_info_set_t;     

typedef struct
{
    Message_t  super;
    u8 data[2];
}HU_park_and_AVM_config_t;  

typedef struct
{
    Message_t  super;
    u8 data;
}HU_TUID_Info_t;  

typedef struct
{
    Message_t  super;
    u8 data;
}HU_reg_stuts_t;

typedef struct
{
    Message_t  super;
    u8 data;
}HU_bcallReq_t;

typedef struct
{
    Message_t  super;
    u8 data;
}HU_sunroofControlReq_t;


typedef struct
{
    Message_t  super;
    u8 data;
}HU_rightCameraStatus_t;

typedef struct
{
    Message_t  super;
    u8 data[6];
}HU_loca_time_info_t;


typedef struct
{
    Message_t  super;
    u8 data;
}HU_rearview_mirror_set_t;


typedef struct
{
    Message_t  super;
    u8 data;
}HU_unlock_and_latch_set_t;


typedef struct
{
    Message_t  super;
    u8 data;
}HU_car_Light_set_t;


typedef struct
{
    Message_t  super;
    u8 data[4];
}HU_coordinate_set_t;

typedef struct
{
    Message_t  super;
    u8 data[2];
}HU_AUX_driving_set_t;

typedef struct
{
    Message_t  super;
    u8 data[6];
}HU_maintenance_Info_t;

typedef struct
{
    Message_t  super;
    u8 data;
}HU_AVM_vision_angle_req_t;


/***************car body staus message to COMMPU model*************/

typedef struct
{
    Message_t super;
    u8 data;
}LatchAndLockStat_t;



typedef  void (*CarInfoManage)(void);

typedef struct
{
    u8   *flag ;
    u16  *timer;/*send to msg outimer*/
    u16   delay;
    CarInfoManage func;
}CarBodyInfoSendManage_t;

typedef struct
{
    
    u8 unused0:6;
    /**************************
    0x0 = OFF;0x1= ACC;
    0x2 = ON;0x3 = Start;
    ***************************/
    u8 status:2;
}BCM_PowerStatusFeedback_t;


typedef struct
{
    Message_t   Super;
    BCM_PowerStatusFeedback_t PowerStatus;
}CarBodyInfoToPowerEvt_t;

#if 0
EVENT_CAN_OFFLINE_CONFIG,
EVENT_CAN_DOOR_LIGHT,   
EVENT_CAN_AIR_COMDITION_CONTROL_INFO,
EVENT_CAN_ENVIRON_TEMPER,
EVENT_CAN_CAR_SPEED,    
EVENT_CAN_ENGINE_RUN,   
EVENT_CAN_ENGINE_FAILURE,   
EVENT_CAN_ACC_PEDAL_FAILURE,   
EVENT_CAN_TIRE_PRESSURE,
EVENT_CAN_TOTAL_MILEAGE,
EVENT_CAN_STEERING_ANGEL,
EVENT_CAN_OIL_CONSUMPTION,
EVENT_CAN_ECU_INFO,
EVENT_CAN_RADAR_INFO,
EVENT_CAN_CAMEMA_INFO,
EVENT_CAN_TUID,
EVENT_CAN_LOCK_INFO,
EVENT_CAN_MAINTAIN,
EVENT_CAN_REARVIEW_MIRROR,

#endif
/*when soc is acking and mcu, body info isn't sent*/
typedef enum
{
	SOC_ACK_MCU_SEND_OFFLINE                 = 0x01,
    SOC_ACK_MCU_SEND_DOOR_LIGHT              = 0x02,  
    SOC_ACK_MCU_SEND_AC_INFO                 = 0x04,
    SOC_ACK_MCU_SEND_ENVIRON_TEMPER          = 0x08,
    SOC_ACK_MCU_SEND_SPEED                   = 0x10,    
    SOC_ACK_MCU_SEND_ENGINE_RUN_STA          = 0x20,    
    SOC_ACK_MCU_SEND_ENGINE_FAILURE          = 0x40,
    SOC_ACK_MCU_SEND_BRAKE_PEDAL_STA         = 0x80,
    SOC_ACK_MCU_SEND_TIRE_PRESSURE           = 0x100,
    SOC_ACK_MCU_SEND_TOTAL_MILEAGE           = 0x200,  
    SOC_ACK_MCU_SEND_BODY_LIGHT_STA          = 0x400,
    SOC_ACK_MCU_SEND_STREEN_INFO             = 0x800,
    SOC_ACK_MCU_SEND_OIL_CONSUMPTION         = 0x1000,    
    SOC_ACK_MCU_SEND_ECU_INFO                = 0x2000,     
    SOC_ACK_MCU_SEND_RADAR_INFO              = 0x4000, 
    SOC_ACK_MCU_SEND_CAMERA_INFO             = 0x8000, 
    SOC_ACK_MCU_SEND_TUID                    = 0x10000, 
    SOC_ACK_MCU_SEND_LOCK_INFO               = 0x20000, 
    SOC_ACK_MCU_SEND_MAINTAIN_INFO           = 0x40000, 
    SOC_ACK_MCU_SEND_REARVIEW_MIRROR_INFO    = 0x80000, 
    SOC_ACK_MCU_SEND_REARWIPER_STA           = 0x100000,
}CarBodyInfoSendInfoEvent_e;  

typedef struct
{
	u8 ack;/*1:ack Soc and Mcu successfully*/
    u8 event;/**/
}CarBodyInfoIfSocAck_t;

typedef struct
{
	u16 TimeOutValue;
	u16 TimeCnt;
	bool AccOffTimeOutFlag;
}AccOffJudge_t;

EXTERN CarBodyInfo_tu BodyInfo;
EXTERN CarBodyInfo_t  BodyInfoBackup;
EXTERN void CanSendCarBodyInfoToCom(u8 size,u8 type,CarBodyInfoSendInfoEvent_e event);
EXTERN void CanSendCarBodyInfoToPower(CarBodyInfoEvt_t data,u8 type);

EXTERN void CanAppAppTimerManage(void);
EXTERN void CanAppAppInit(void);
EXTERN void CarBodyInfoSendManageFunc(void);
EXTERN void CanAllBodyInfoSendToSoc(void);
EXTERN void CanBodyInfoClearSendEvent(CarBodyInfoSendInfoEvent_e event);

EXTERN u8 CanBodyInfoGetSendEvent(CarBodyInfoSendInfoEvent_e event);
EXTERN void CanBodyInfoSetSendEvent(CarBodyInfoSendInfoEvent_e event);
EXTERN  u8 CanBodyInfoGetSocAck(void);
EXTERN void CanBodyInfoSetSocAck(u8 ack);
EXTERN void CanAllBodyInfoSendToSoc(void);

EXTERN void Can_Time_1msCallBack(void);
EXTERN void Can_Time_2msCallBack(void);
EXTERN void Can_Time_10msCallBack(void);

#undef EXTERN
#endif

