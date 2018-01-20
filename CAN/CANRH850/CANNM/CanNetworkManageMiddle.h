
#ifndef __CANNETWORKMANAGEMIDDLE_H
#define __CANNETWORKMANAGEMIDDLE_H

#include "Can_config.h"
#include "MsgDefine.h"
#include "fsm.h"
#include "Vehicle.h"


#define CAN_NM_ST_SLEEP        1
#define CAN_NM_ST_WAKEUP       2

typedef struct {
	Message_t  Msg;
	u8         busSleep;
} CanNetworkEvt_t;

extern bool gblCanSleepFlag;

void CAN0WakeupHandle();
void NMSleepCallBack();

#define  	TBOX_NM_CANID   0x407
#define 	OWN_NODE				0x07u // NM偏移量 对应407的07
//#define 	NM_CAN_ID_BASE 			0x400
//#define 	NM_CAN_ID_BOTTOM  		0x480


//#define 	ID_Base_Mask 			0x700
//#define 	ID_Base_Address			0x400


#define 	NM_RX_LIMIT				0x04 // 收不到4帧进入LimpHome模式
#define 	NM_TX_LIMIT				0x08 // 发不去8帧进入LimpHome模式

#define 	NM_NODE_NOSKIP 			0
#define 	NM_NODE_SKIP 			1


#define 	SilentNM( )				NM_Networkstatus.NMactive = 0
#define 	TalkNM( )				NM_Networkstatus.NMactive = 1

#define		D_Online( )				NM_Networkstatus.Node_Online = 1
#define		D_Offline( )			NM_Networkstatus.Node_Online = 0

#define 	NM_TMER_CYC_MS      	(5u)

#define 	NM_TTyp					(100u/NM_TMER_CYC_MS)
#define 	NM_TMax					(260u/NM_TMER_CYC_MS)
#define 	NM_TErr					(1000u/NM_TMER_CYC_MS)
#define 	NM_TTwbs				(1500/NM_TMER_CYC_MS)
#define 	NM_Time_Tx				0
#define     BusoffStatus            0x04

//接口函数
void NM_SysAllowBusSleep(void);//允许CAN节点休眠
void Nm_SysNotAllowBusSleep(void);//不允许CAN节点休眠
void Process_can_NM(void);//运行扫描NM管理
void NM_RxPro( can_frame *gNmCanMsg );//接收NM 报文
void NM_TxConf(u8 ucPara);//NM报文发送完毕
void NM_TmCb(void);//计时
void NM_CanWkupResume(void);//唤醒确保变量
void NM_BusoffTmRst(void);
extern void SendCANSleepMsg(u8 busSleep);

#endif
