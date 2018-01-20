#ifndef CANTESTTASK_H
#define CANTESTTASK_H

#include "base_types.h"
#ifdef _CAN_TASK_ROOT
	#define EXTERN
#else
	#define EXTERN extern
#endif
#include "sysconfig.h"
#include "MsgDefine.h"
#include "CAN.h"

#include "CanAppMiddle.h"

//#include "CanDriver.h"
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


//void CreatCanTask(u8 TaskPri);
//void CreatRunNMTask(u8 TaskPri);
EXTERN u32 SendEventToCan(const Message_t *e, bool ISRFlag);
EXTERN void InputCanTxOkQueue(u8 MsgObjNum, bool Int);
EXTERN void InputCanRxQueue(can_frame *pCanRxData, bool Int);
EXTERN u8 InputCanTxQueue(can_frame *pCanTxData, bool Int);
EXTERN void CanModeInit(void);

#undef EXTERN
#endif
