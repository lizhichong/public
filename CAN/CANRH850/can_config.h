#ifndef __CAN_CONFIG_H
#define __CAN_CONFIG_H

#include "CanAppApp.h"
#include "base_types.h"
#include "CAN.h"

#define TRACE_BOOT

#define	CA_MCU_LIB		(0u)		//if use CA mcu lib

typedef bool MCU_bool;


typedef can_frame CAN_MSG_Type;
typedef CAN_MSG_Type NL_MSG_SType;

#define  MCURET_ERR  	(1)
#define  MCURET_OK  	(0)

#define TRACE_APL 
/*********************CAN DIAG NetLayer Config**********************/
#define  CAN_Diag_FunctionalRequests_ID (0x7DF)
#define  CAN_Diag_PhysicalRequests_ID 	(0x717)
#define  CAN_Diag_PhysicalResponses_ID  (0x71F)

#define CAN_YUANTEL_HOST_ENABLE		1

#define CAN_BASICTIME (10u)

#define NL_UNUSEDBYTE (0) // not used CAN MSG byte filled with xx
#define Nl_gIrqDis()   
#define Nl_gIrqEn()    
#define NL_BASICTIME  CAN_BASICTIME	// 10ms

#define NL_FULL_DUPLEX   0 // 1==NL_FULL_DUPLEX  0==NL_HALF_DUPLEX

#define NL_RXBUFFMAXSIZE 1024
#define NL_TXBUFFMAXSIZE 512

#define NL_WFT_MAX              0x00u //0X01U

#define NL_BS_DEFAULT           0x08u //0X01U
#define NL_STMIN_DEFAULT_TIME   (20)  //for send fc
#define NL_STMIN_DEFAULT        (NL_STMIN_DEFAULT_TIME/NL_BASICTIME) // ms
#define NL_N_Br_MAX             (70u/NL_BASICTIME) //ms 
#define NL_N_Ar_MAX             (70u/NL_BASICTIME) //ms
#define NL_N_Cr_MAX             (150u/NL_BASICTIME)//ms
#define NL_N_As_MAX             (70u/NL_BASICTIME)
#define NL_N_Bs_MAX             (150u/NL_BASICTIME)
#define NL_N_Cs_MAX             (70u/NL_BASICTIME)

#define NL_N_WAITLITTLETIME_MAX (20/NL_BASICTIME)

/**********************CAN DIAG ApplicationLayer Config*************************/
#define AL_BASICTIME           CAN_BASICTIME//10ms
#define AL_P2sever_MAX_TIME    (50)
#define AL_P2_xsever_MAX_TIME  (5000)
#define AL_S3sever_MAX_TIME    (5000)
#define AL_S3client_MAX_TIME   (4000)

#define AL_P2sever_MAX    (AL_P2sever_MAX_TIME/AL_BASICTIME)//ms
#define AL_P2_xsever_MAX  (AL_P2_xsever_MAX_TIME/AL_BASICTIME)//ms
#define AL_S3sever_MAX    (AL_S3sever_MAX_TIME/AL_BASICTIME)//ms

#define  CAN_DIAG_CHx_NUM (1)

/**********************CAN NETWORK Config*************************/
#define  LocalNMID	(0x407)

#include "CanNetworkManageMiddle.h"

/**********************system_apl.h*****************************/

#define  TwoU8ToU16(x,y)  (((u16) (x) <<8 ) + (y) )
#define  GetU16HighU8(x)  ((u8) ((u16)(x) >> 8))
#define  GetU16LowU8(x)   ((u8) ((u16)(x) & 255))
#define  FourU8ToU32(a,b,c,d)     (((u32)(a) <<24)|((u32)(b) <<16)|((u32)(c) <<8 )|((u32)(d))) 

typedef struct
{
	//indication isr
	u8  gblIndCANBusOff;//
	//
	u8  gblIndAccOn;//IGN
	u8  gblIndSystermlParaInit;
	u8  gCallStatus;
	//cmd   act
	u8  gblRequestCANNMBusSleep;
	u8  gblResponseCANNMBusSleep;
	//me state
	u8  gblDiagWorking;//Not Allow sleeping;
	u8  gblNeedMcuReset;// 
}CAN_MCU_IF_STypeDef;

typedef struct
{
	u8  gblIndR4ReadyWork;//r4 can recive info
	u8  gblIndR4DateGet;
	u8  gblIndR4SpeakerUse;
	u8  ucIndR4PhoneStatus;
}CAN_GPRS_IF_STypeDef;

extern CAN_GPRS_IF_STypeDef CAN_GPRS_Interface;
extern CAN_MCU_IF_STypeDef CAN_MCU_Interface;
#include  "can_diag_nl_fml.h"
#include  "can_diag_al_fml.h"
#include  "can_diag_al_apl.h"

#endif
