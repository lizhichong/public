#include "CanNetworkManageMiddle.h"
#include "Can_config.h"
#include <stdio.h>
#include <string.h>
#include "can.h"
#include "CANAppApp.h"
#include "can_diag_al_apl.h"
#include "Debug.h"


can_frame TBOX_NM_MS;

bool gblCanSleepFlag;
extern u8 AppAllowSleepFlag;

extern AccOffJudge_t stAccOffJudge;


u8 NMPDU_TxOk;
u8 NMPDU_RxOk;
//u8 NMPDU_Err;

u8 NM_Sleep_Ack;
u8 NM_Sleep_Ind;// send message 休眠指示
u8 NM_Stop_App;

u8 NM_Goto_Awake_Called;//满足唤醒条件
//bool NM_Goto_BusSleep_Called;//系统通知NM可以休眠

u8 PRE_NMBusSleep;
u8 PRE_NMLimpHome;
u8 PRE_NMNormal;
u8 PRE_NMOff;
u8 PRE_NMLimpHomePrepSleep;

u8 NM_Ring_Sent;

u8 NM_TTyp_Enable;/* 为1才处理 TTyp-Out */ /* 对应 SetAlarm */
u8 NM_TMax_Enable;/* 为1才处理 TMax-Out */
u8 NM_TErr_Enable;/* 为1才处理 TErr-Out */
u8 NM_TTwbs_Enable;
u8 NM_TTx_Enable;

u8 NM_Sleeping = 0;
u8 NM_Sleeping2 = 0;

/*
****************************************************************************
@area       CAN_NM 服务相关
----------------------------------------------------------------------------
@release    V1.00
@author
@brief
@date       2011/3/3
****************************************************************************
*/
#if  1
typedef enum
{
    NM_OFF      = 0x00,
    NM_ON       = 0x01,
    NM_SHUTDOWN = 0x02
} NM_Module_Status_Enum;

typedef enum
{
    NM_INIT     = 0x00,
    NM_AWAKE    = 0x01,
    NM_BUSSLEEP = 0x02
} NM_On_Status_Enum;


typedef enum
{
    NM_RESET    = 0x00,
    NM_NORMAL   = 0x01,
    NM_LIMPHOME = 0x02
} NM_Awake_Status_Enum;


typedef enum
{
    NM_ERROR_MSG    = 0x00,

    ALIVE_CI_CA     = 0x01, /* 普通 Alive 消息 */
    ALIVE_SI_CA     = 0x11, /* Alive 消息请求总线睡眠 */

    RING_CI_CA      = 0x02, /* 普通 Ring 消息 */
    RING_SI_CA      = 0x12, /* Ring 消息请求总线睡眠 */
    RING_CI_SA      = 0x22, /* Ring 消息确定总线睡眠 */
    RING_SI_SA      = 0x32, /* Ring 消息确定总线睡眠 */

    LIMPHOME_CI_CA  = 0x04, /* 普通 LimpHome 消息 */
    LIMPHOME_SI_CA  = 0x14  /* LimpHome 消息请求总线睡眠 */
} NM_OpCode;


typedef struct
{
    u8  SourceID;
    u8  DestinationID;
    u8  OptCode;
    u8  Data[6];
} NM_PDU;


typedef enum
{
    BUSINIT     = 0x01,
    BUSSHUTDOWN = 0x02,
    BUSRESTART  = 0x03,
    BUSSLEEP    = 0x04,
    BUSAWAKE    = 0x05,
    BUSINVALID =0x06,
} DInitRoutine_Enum;


typedef struct
{
    u8   BusSleep;
    u8   ConfigrationStable;
    u8   NMactive;
    u8   Node_Online;
} NM_NetworkStatus_Struct;

typedef struct
{
    u8   Stable;
    u8   LimpHome;
} NM_Merker_Struct;

typedef struct
{
    u8  Limphome;
    u8  Present;
    u8  Successor;
} NM_Config_Struct;

#endif
//NM_Module_Status_Enum     G_NM_Module_Status;
//NM_On_Status_Enum         G_NM_On_Status;
//NM_Awake_Status_Enum      G_NM_Awake_Status;
NM_PDU              G_NM_PDU_Receive;
NM_PDU              G_NM_PDU_Transmit;

u8                  G_CAN_NM_TxData[8];


u8 NMrxcount = 0;
u8 NMtxcount = 0;


u32 NM_TTyp_Tick =0;
u32 NM_TMax_Tick = 0;
u32 NM_TErr_Tick =0;
u32 NM_TTwbs_Tick = 0;

u8 NM_Last_Ring_Msg;
u8 NM_Next_Node;


//u8 NMSleeptemp=0;
//u8 Busoffflag = 0;
//u8 gTime_enable_50ms = 0x00;
//u8 gTime_enable_1s = 0x00;
//u8 NM_RING_ENBLE =0;
u8 NM_Sleep_2Ind =0;
u8 NM_Sleep_3Ind =0;


u8 NMLimp_status = 0;
u8 NMNormal_Status  = 0;

u8 gTimeFlag_50ms;
u8 gTimeFlag_2ms;


void StartNM(void);
void D_Init(DInitRoutine_Enum InitRoutine);
void NMInit(void);
void NMInitReset(void);
void NMReset (void);
void NMNormal( void );
void NMLimpHome( void );
void NMNormalPrepSleep( void );
void NMLimpHomePrepSleep( void );
void NMInitBusSleep( void );
void NMTwbsNormal( void );
void NMBusSleep( void );
void NormalStandardNM( void );
void TransmitRingData( u8 Msg_Type );
void GotoMode( u8 Mode_Type );
u8 NMDetemine_NodeSkip( u8 Source, u8 Destination );
void NMTwbsLimpHome( void );

void Busoff_Handle(void);

void (*NM_Main_Program)(void) = StartNM;


NM_Config_Struct NM_Config;
NM_NetworkStatus_Struct NM_Networkstatus;
NM_Merker_Struct NM_Merker;

extern void CAN_ConfigInit(void);

void set_NM_STOP_APP(u8 Para)
{
	NM_Stop_App = Para;
}


void SendCANSleepMsg(u8 busSleep)
{
	CanNetworkEvt_t Can_Evt;
	Can_Evt.Msg.Type = EVENT_CAN_SEND_NM_ST;
	Can_Evt.busSleep = busSleep;

	SendMsgToPwrManage((u8 *)&Can_Evt, sizeof(CanNetworkEvt_t));
}

void CAN0WakeupHandle()
{
	gblCanSleepFlag = false;
	R_INTP0_Stop();
	
	CAN_ConfigInit();
	NM_CanWkupResume();
	SendCANSleepMsg(CAN_NM_ST_WAKEUP);

	DEBUG_MCU(DBG_INFO, CAN_MODULE_ID, "Can NetManage go to wakeup\r\n");
}
void NMSleepCallBack()
{
	gblCanSleepFlag = true;

	DisableCANCtl();
	
	set_CAN0_STB(ENABLE);
	
	INTP0CtlInit();
	R_INTP0_Start();
	SendCANSleepMsg(CAN_NM_ST_SLEEP);

	DEBUG_MCU(DBG_INFO, CAN_MODULE_ID, "Can NetManage go to Sleep\r\n");
}

u8 CanEnterSleepStatusCheck(void)
{
	if((stAccOffJudge.AccOffTimeOutFlag == false)
	   ||(AppAllowSleepFlag == 1)
	   ||(CAN_MCU_Interface.gCallStatus != CALLIDLE)
	  )
	{
		if(gblCanSleepFlag == true)
		{
			CAN0WakeupHandle();
		}
		return 0;
	}
	return 1;
}

u8 CheckNMGotoSleep(void)
{
	CAN_MCU_Interface.gblRequestCANNMBusSleep = CanEnterSleepStatusCheck();// 1 need sleep
	
    if((0 == CAN_MCU_Interface.gblRequestCANNMBusSleep) 
	  ||(TRUE == CAN_MCU_Interface.gblDiagWorking))
	{	
		return 0;
	}
		
    return 1;
}

void CAN_NM_CheckGotoSleep(void)
{	
    if(CheckNMGotoSleep())
    {
        NM_SysAllowBusSleep();		
    }
    else
    {
        Nm_SysNotAllowBusSleep();
    }
}

//
/**********************************************************************
 * @brief
 * @param[in]
 * @return
 **********************************************************************/
Can_ReturnType NM_CanTransmit(can_frame* Msg)
{
    Can_ReturnType ucRet = CAN_BUSY;

    if(!CANDiagCCCtrl.DisableNMMsgTx)
    {
        ucRet = CAN_TxMsgQueue(&TBOX_NM_MS);  
        return ucRet;    
    }
    return CAN_OK;
}
void StartNM(void)
{
    //TRACE_BOOT( "StartNM\r\n" );
    PRE_NMBusSleep = 0;
    //PRE_NMLimpHome = 0;
    PRE_NMNormal = 0;
    PRE_NMOff = 1;
    G_NM_PDU_Receive.OptCode &=(u8)(~0x30);
    TalkNM();
   // NM_Main_Program = NMInit;
   NMInit();
}
//一般是涉及硬件层的 目前没啥用
void D_Init(DInitRoutine_Enum InitRoutine)
{
    if(BUSINIT == InitRoutine)
    {

    }
    else if (BUSSHUTDOWN == InitRoutine)
    {

    }
    else if (BUSRESTART == InitRoutine)
    {

    }
    else if (BUSSLEEP == InitRoutine)
    {

    }
    else if (BUSAWAKE == InitRoutine)
    {

    }
    else
    {

    }
}

void NMInit(void)//初始化
{
    //TRACE_BOOT( "NMInit\r\n" );
    if( PRE_NMOff ==1)
    {
        PRE_NMOff = 0;
        D_Init(BUSINIT);
    }
    else
    {
        if(PRE_NMBusSleep ==1)
        {
            PRE_NMBusSleep = 0;
            D_Init(BUSAWAKE);
        }
        else
        {
            ;
        }
    }
    NM_Config.Limphome = 0;
    NM_Networkstatus.BusSleep = 0;
    NM_Sleeping = 0;
    NM_Sleeping2 = 0;
	NMrxcount = 0;
    NMtxcount = 0;
    NM_Networkstatus.ConfigrationStable = 0;
    NM_Networkstatus.Node_Online = 0;
    //NM_Main_Program = NMInitReset;
    NMInitReset();

}
void NMInitReset(void)//复位初始化
{
    //TRACE_BOOT( "nminitreset\r\n" );
    if(PRE_NMLimpHome ==1) //Now
    {
        PRE_NMLimpHome = 0;
        NMrxcount = 0;
        NMtxcount = 0;
        NM_Merker.LimpHome = 0;
        NM_Networkstatus.BusSleep = 0;
        NM_TErr_Enable = 0;
        NM_TErr_Tick = 0;
        D_Online();
    }
    else
    {
        if(PRE_NMNormal ==1)
        {
            PRE_NMNormal = 0;
            NM_Networkstatus.BusSleep = 0;
        }
        else
        {
            ;
        }
    }
    NMLimp_status = 0x00;

    NM_Config.Present = OWN_NODE;
    NM_Config.Successor = OWN_NODE;
    NM_Next_Node = OWN_NODE;
    NM_Sleep_Ack = 0;
    NMrxcount += 1;
    //NM_Stop_App = 0;
    NM_Sleeping = 0;
    NM_TTyp_Tick = 0;
    NM_TMax_Tick = 0;
    NM_TTyp_Enable = 0;
    NM_TMax_Enable = 0;
    //TRACE_BOOT( "typenable1\r\n" );
	NMReset();
    //NM_Main_Program = NMReset;
}
void NMReset (void)
{
    //TRACE_BOOT( "NMReset\r\n" );
    NM_Main_Program = NMReset;
    if( NM_Networkstatus.NMactive ==1)//主动网络管理
    {
        TransmitRingData(ALIVE_CI_CA);
		//TRACE_TIME("Time is %5d.%06d\r\n",MTU_ReadTimebase(TIME_IN_SEC),MTU_ReadTimebase(TIME_IN_US));
		//LED1_Off();
        //TRACE_BOOT( "TransmitRingDataAlive_Ci_Ca\r\n" );
        NM_Stop_App = 0;
        NMtxcount++;
    }
    else
    {
        ;
    }
    if( (NMrxcount<= NM_RX_LIMIT)&&(NMtxcount<= NM_TX_LIMIT))
    {
        NM_TTyp_Tick = 0;
        NM_TTyp_Enable = 1;
        NM_Main_Program = NMNormal;
    }
    else
    {
        NM_TErr_Tick = 0;
        NM_TErr_Enable = 1;
        NM_Main_Program = NMLimpHome;
        //TRACE_BOOT( "NMLimpHome11\r\n" );
    }
    //TRACE_BOOT( "NMrxcount is:\r\n" );

}
void NMNormal( void )
{
    u8 SendMsgType = RING_CI_CA;

    //TRACE_BOOT( "NMNormal!\r\n" );

    NMNormal_Status = 1;
    if( 1 == NMPDU_RxOk )/* 接收到NM帧   nm报文接收*/
    {
        NMPDU_RxOk = 0;


        //NormalStandardNM();
        //TRACE_BOOT( "NMNormalreceiveNM!\r\n" );

        if((G_NM_PDU_Receive.OptCode&0x20) ==0x20 )
        {
            if( 1 == NM_Sleep_Ind) //允许休眠
            {
                PRE_NMNormal = 1;
                NM_Main_Program = NMInitBusSleep;
            }
            else
            {
                ;
            }
        }
        else
        {
            ;
        }
        return;
    }
    else
    {
        ;
    }

    if( ( NM_TTyp_Tick >= NM_TTyp ) && ( 1 == NM_TTyp_Enable ) )/* TTyp-Out  nm报文发送*/
    {
        NM_TTyp_Tick = 0;
        NM_TMax_Tick = 0;

        NM_TMax_Enable = 1;//
        NM_TTyp_Enable = 0;//gxc
        //TRACE_BOOT( "NMNormalsendNM!\r\n" );
        if( 1 == NM_Networkstatus.NMactive )
        {
            if( 1 == NM_Sleep_Ind )//允许网络休眠
            {
                SendMsgType  = RING_SI_CA;
                NM_Sleep_2Ind = 0x01;

            }
            else
            {
                SendMsgType = RING_CI_CA;
                NM_Sleep_2Ind = 0x00;

            }

            TransmitRingData( SendMsgType );

            //NM_RING_ENBLE = 0X01;
            NMtxcount++;
        }
        else
        {
            ;
        }


        if( NMtxcount > NM_TX_LIMIT )
        {
            NM_TErr_Tick = 0;
            NM_TErr_Enable = 1;
            NM_Main_Program = NMLimpHome;
            //TRACE_BOOT( "NMLimpHome22\r\n" );
        }
        else  //？？？
        {
            if( 1 == NM_Merker.Stable )
            {
                NM_Networkstatus.ConfigrationStable = 1;
            }
            else
            {
                NM_Merker.Stable = 1;
            }
        }

        return;
    }
    else
    {
        ;
    }
    //TRACE_BOOT( "NMPDU_TxOk:\r\n" );

    if( 1 == NMPDU_TxOk )/* 发送完成  nm报文成功发送 */
    {
        NMPDU_TxOk = 0;
        NMtxcount = 0;
        //TRACE_BOOT( "NMNormalsendok!\r\n" );
        /* 判断发送消息的类型 */
        if( NM_Last_Ring_Msg & RING_CI_CA ) /* Ring消息 */
        {
            NM_TTyp_Tick = 0;
            NM_TMax_Tick = 0;
            //TRACE_BOOT( "typenable2\r\n" );

            NM_TTyp_Enable = 0;
            NM_TMax_Enable = 1;
			
			if( 1 == NM_Sleep_2Ind )//send
			{
				NM_Sleep_2Ind = 0;
				if(1 == NM_Sleep_Ind)
				{
					NM_Main_Program = NMNormalPrepSleep;
				}
				else
				{
					;
				}
			}

			#if 0  //nongyubao change 20170506
            if( 1 == NM_Sleep_2Ind )//send
            {
                NM_Sleep_3Ind = 1;
                if( 1 == NM_Networkstatus.BusSleep )
                {
                    NM_Sleep_Ack = 1;//send
                    //TRACE_BOOT( "NMNormalsleepstart\r\n" );
                    GotoMode( BUSSLEEP );
                    NM_Main_Program = NMNormalPrepSleep;
                }
                else
                {
                    ;
                }
            }
            else
            {
                NM_Sleep_3Ind = 0;
            }
			#endif
        }
        else
        {
            ;
        }

        return;
    }
    else
    {
        ;
    }

    if( ( NM_TMax_Tick >= NM_TMax ) && ( 1 == NM_TMax_Enable ) ) //TMax-Out 超时处理
    {
        PRE_NMNormal    = 1;
        NM_Main_Program = NMInitReset;
        //TRACE_BOOT( "NMNormaltmaxout1!\r\n" );
        return;
    }
    else
    {
        ;
    }
	#if 0  //nongyubao change 20170506
    if( 1 == NM_Sleep_3Ind )
    {
        if( 1== NM_Networkstatus.NMactive )
        {
            NM_Main_Program = NMNormalPrepSleep;
        }
        NM_Sleep_3Ind = 0x00;
    }
	#endif

    /*if( 1 == NM_Goto_BusSleep_Called )// 满足睡眠
    {
        //NM_Goto_BusSleep_Called = 0;
        //GotoMode( BusSleep );
        TRACE_BOOT( "NMNormalsleep!\r\n" );
        if( 1== NM_Networkstatus.NMactive )
        {
            ;
        }
        else
        {
            NM_Main_Program = NMNormalPrepSleep;
        }
    }
    else
    {
        ;
    }*/

}


void NMNormalPrepSleep( void )   //正常睡眠流程
{
    //TRACE_BOOT( "NMNormalPrepSleep!\r\n" );

    if( 1 == NM_Goto_Awake_Called ) //不满足睡眠
    {
        NM_Goto_Awake_Called    = 0;
        NM_Stop_App = 0;
        NM_Main_Program = NMNormal;
        return;
    }
    else
    {
        ;
    }

    if( ( NM_TTyp_Tick >= NM_TTyp ) && ( 1 == NM_TTyp_Enable ) )//ttyp超时
    {
        NM_TTyp_Tick    = 0;
        NM_TTyp_Enable  = 0;
        //TRACE_BOOT( "NMNormalPrepSleepttypout!\r\n" );
        if( 1 == NM_Networkstatus.NMactive )
        {
            TransmitRingData( RING_SI_SA );
            NM_Stop_App = 1;
            NM_TMax_Enable = 1;//gxc
            NM_TMax_Tick = 0;//gxc
            NM_Main_Program = NMNormalPrepSleep;//gxc
        }
        else
        {
            NM_Main_Program = NMNormalPrepSleep;//gxc
            NM_Stop_App = 0;
        }

        return;
    }
    else
    {
        ;
    }
    if( ( NM_TMax_Tick >= NM_TMax ) && ( 1 == NM_TMax_Enable ) )/* TMax-Out 超时处理 */
    {
        PRE_NMNormal    = 1;
        NM_Main_Program = NMInitReset;
        //TRACE_BOOT( "NMNormaltmaxout2!\r\n" );
        return;
    }
    else
    {
        ;
    }



    if( 1 == NMPDU_RxOk )/* 接收到NM帧 gxc*/
    {
        NMPDU_RxOk = 0;
        //NormalStandardNM();
        //TRACE_BOOT( "NMNormalPrepSleepreceiveNM!\r\n" );


        if( (G_NM_PDU_Receive.OptCode&0x10) ==0x10)//receive
        {
            if( (G_NM_PDU_Receive.OptCode&0x20) == 0x20)// || NM_Sleep_Ack ==1 ) //send
            {
                //TRACE_BOOT( "G_NM_PDU_Receive.OptCode&0x20hhh\r\n" );
                PRE_NMNormal = 1;
                NM_Stop_App = 1;
                NM_Main_Program = NMInitBusSleep;//nomal状态下睡眠初始化
            }
            else
            {
                NM_Stop_App = 0;
                NM_Main_Program = NMNormalPrepSleep;//gxc
            }

        }
        else
        {
            NM_Main_Program = NMNormal;
            NM_Stop_App = 0;
        }

        return;
    }
    else
    {
        ;
    }

    if( 1 == NMPDU_TxOk )/* 发送完成 nm报文发送成功gxc*/
    {
        NMPDU_TxOk = 0;
        //TRACE_BOOT( "NMNormalPrepSleepsendok!\r\n" );
        NM_TMax_Enable = 0;//gxc
        NM_TMax_Tick = 0;//gxc
        if( NM_Last_Ring_Msg & RING_CI_CA ) /* Ring消息 */
        {
            NM_TTwbs_Tick   = 0;
            NM_TTwbs_Enable = 1;
            NM_Stop_App = 1;


            NM_Main_Program = NMTwbsNormal; //睡眠等待状态
        }
        else
        {
            ;
        }
    }
    else
    {
        ;
    }
}

void NMInitBusSleep( void )
{
    //TRACE_BOOT( "NMInitBusSleep!\r\n" );

    D_Offline( );

    NM_Stop_App = 1;
    /* SetTwbs */
    NM_TTwbs_Tick   = 0;
    NM_TTwbs_Enable = 1;

    if( 1 == PRE_NMNormal )
    {
        NM_TMax_Enable = 0;//gxc
        NM_TMax_Tick = 0;//gxc
        PRE_NMNormal    = 0;
        NM_Main_Program = NMTwbsNormal;
        return;
    }
    else
    {
        ;
    }

    if( 1 == PRE_NMLimpHomePrepSleep )
    {
        PRE_NMLimpHomePrepSleep = 0;
        NM_Main_Program = NMTwbsLimpHome;
        return;
    }
    else
    {
        ;
    }
}

void NMTwbsNormal( void )//睡眠等待状态
{
    //TRACE_BOOT( "NMTwbsNormal!\r\n" );
#if 0
    if( ( NM_TTwbs_Tick >= NM_TTwbs ) && ( 1 == NM_TTwbs_Enable ) )
    {
        //NM_TTwbs_Tick     = 0;
        //NM_TTwbs_Enable = 0;
        //TRACE_BOOT( "NMTwbsNormaltwbsout!\r\n" );
        D_Init( BUSSLEEP );

        NM_Main_Program = NMBusSleep;

        return;
    }
    else
    {
        ;
    }
#endif
    if( 1 == NMPDU_RxOk )/* 接收到NM帧 */
    {
        NMPDU_RxOk = 0;
        //TRACE_BOOT( "NMTwbsNormalreceiveNM!\r\n" );
        if(  ( G_NM_PDU_Receive.OptCode&0x10) ==0x00 )//receive
        {
            /* CancelAlarm( Twbs ) */
            NM_TTwbs_Enable = 0;
            NM_TTwbs_Tick   = 0;
            NM_Stop_App = 0;
            NM_Main_Program = NMInitReset;
        }
        else
        {
            ;
        }

        return;
    }
    else
    {
        ;
    }

    if( 1 == NM_Goto_Awake_Called )
    {
        //TRACE_BOOT( "NMTwbsNormalnosleep!\r\n" );
        //NM_Goto_Awake_Called = 0;
        /* CancelAlarm( Twbs ) */
        NM_TTwbs_Enable = 0;
        NM_TTwbs_Tick   = 0;

        NM_Main_Program = NMInitReset;

	return;
    }
    else
    {
        ;
    }
	
    if( ( NM_TTwbs_Tick >= NM_TTwbs ) && ( 1 == NM_TTwbs_Enable ) )
    {
        //NM_TTwbs_Tick     = 0;
        //NM_TTwbs_Enable = 0;
        //TRACE_BOOT( "NMTwbsNormaltwbsout!\r\n" );
        D_Init( BUSSLEEP );

        //NM_Main_Program = NMBusSleep;
		NMBusSleep();
        return;
    }
    else
    {
        ;
    }
}
void NMBusSleep( void )
{
	NM_Main_Program = StartNM;

	//TRACE_BOOT( "\r\n ---->M3 CAN: NMSleeping! \r\n" );
    NM_Sleeping = 1;
    NM_Sleeping2 = 1;

	NMSleepCallBack();
}


void NMLimpHome( void )
{
//  TRACE_BOOT( "NMLimpHome\r\n" );

    if(NMLimp_status != 0x01)
    {
        NMLimp_status = 0x01;
    }
    if( ( NM_TErr_Tick >= NM_TErr ) && ( 1 == NM_TErr_Enable ) ) //报文发送流程gxc
    {
        //TRACE_BOOT( "NMLimpHomemessagesend\r\n" );
        NM_TErr_Tick = 0;

        D_Online();
        NM_Stop_App = 0;

        if( 1 == NM_Sleep_Ind)
        {
            NM_TMax_Enable = 1;
            NM_TMax_Tick = 0;

            if( 1 == NM_Networkstatus.NMactive )
            {
                TransmitRingData( LIMPHOME_SI_CA );
                NM_Main_Program = NMLimpHomePrepSleep;
              //  TRACE_BOOT("LIMPHOME_SI_CA\n\b");
            }
            else
            {
                NM_Main_Program = NMLimpHomePrepSleep;
              //  TRACE_BOOT("LimpHome_Si_Ca2\n\b");
            }
			
			return;//add by nongyubao 2016.11.25
        }
        else
        {
            NM_TErr_Tick = 0;
            NM_TErr_Enable = 1;
            if(1 == NM_Networkstatus.NMactive)
            {
                TransmitRingData( LIMPHOME_CI_CA );
            }
            else
            {
                ;
            }
        }
    }



    if( 1 == NMPDU_TxOk )/* 发送完成 */
    {
        NMPDU_TxOk = 0;
        //TRACE_BOOT( "NMLimpHomemessagesendok\r\n" );
        /* 判断发送消息的类型 */
        if( NM_Last_Ring_Msg & LIMPHOME_CI_CA )
        {
            NM_Merker.LimpHome = 1;
            //TRACE_BOOT( "NM_Merker.LimpHome is 1\r\n" );
        }
        else
        {
            ;
        }

        return;
    }
    else
    {
        ;
    }

    if( 1 == NMPDU_RxOk )  //报文接收gxc
    {
        NMPDU_RxOk = 0;
        //TRACE_BOOT( "NMLimpHomemessagereceive\r\n" );

        if(1 == NM_Networkstatus.NMactive )
        {
            if(1 == NM_Merker.LimpHome)
            {
                if(1 == NM_Networkstatus.BusSleep)
                {
                    if ( (G_NM_PDU_Receive.OptCode&0x20) == 0x20)
                    {
                        PRE_NMLimpHomePrepSleep = 1;
                        NM_Main_Program = NMInitBusSleep;
                    }
                    else
                    {
                        PRE_NMLimpHome = 1;
                        NM_Main_Program = NMInitReset;
                    }
                }

                else
                {
                    PRE_NMLimpHome = 1;
                    NM_Main_Program = NMInitReset;
                }
            }
            else
            {
                if(1 == NM_Networkstatus.BusSleep)
                {
                    if(0x10 ==(G_NM_PDU_Receive.OptCode&0x10))//receive
                    {
                        if( (G_NM_PDU_Receive.OptCode&0x20) == 0x20)//receive
                        {
                            //PRE_NMLimpHome = 1;
                            //NM_Main_Program = NMInitReset;
                            PRE_NMLimpHomePrepSleep = 1;
                            NM_Main_Program = NMInitBusSleep;
                        }
                        else
                        {
                            //PRE_NMLimpHomePrepSleep = 1;
                            //NM_Main_Program = NMInitBusSleep;
                            NM_Main_Program = NMLimpHomePrepSleep;
                        }
                    }
                }
                else
                {
                    ;
                }
            }
        }
        else
        {
            ;
        }
    }


    else
    {
        ;
    }

}



void NMLimpHomePrepSleep( void ) //gxc
{
    //TRACE_BOOT( "NMLimpHomePrepSleep\r\n" );
	if(1 == NMPDU_TxOk) //success send   ////move from back to front modify by nongyubao 2016.11.25
	{
		//TRACE_BOOT( "NMLimpHomePrepSleepsendok\r\n" );
		NMPDU_TxOk = 0;
#if 0	
	/**************add for can nm begin -liubo *************************/
		if((NM_Last_Ring_Msg & LIMPHOME_SI_CA) == LIMPHOME_SI_CA)
		{
			NM_TMax_Enable	= 0;
			PRE_NMLimpHomePrepSleep = 1;
			NM_Main_Program = NMInitBusSleep;
		}
	/**************add for can nm end -liubo *************************/
#endif
		if(NM_Last_Ring_Msg & LIMPHOME_CI_CA)
		{
			NM_Merker.LimpHome = 1;
		}
		else
		{
			;
		}
	}
	else
	{
		;
	}

    if( 0 == NM_Sleep_Ind ) //不满足睡眠
    {
        //TRACE_BOOT( "NMLimpHomePrepSleep nosleep\r\n" );
        NM_TMax_Enable = 0;//gxc
        NM_TMax_Tick = 0;//gxc

        NM_TErr_Tick = 0;
        NM_TErr_Enable = 1;


        NM_Main_Program = NMLimpHome;
        //TRACE_BOOT( "NMLimpHome33\r\n" );
    }
    else
    {
        ;
    }

    if( 1 == NMPDU_RxOk )/* 接收到NM帧 gxc*/
    {
        //TRACE_BOOT( "NMLimpHomePrepSleep receiveNM\r\n" );
        NMPDU_RxOk = 0;

        if(1 == NM_Networkstatus.NMactive )
        {
            if (NM_Merker.LimpHome == 1)
            {
                if(1 == NM_Networkstatus.BusSleep)
                {
                    if( (G_NM_PDU_Receive.OptCode&0x20) ==0x20)//receive
                    {
                        PRE_NMLimpHomePrepSleep = 1;
                        NM_Main_Program = NMInitBusSleep;
                    }
                    else
                    {
                        PRE_NMLimpHome = 1;
                        NM_Main_Program = NMInitReset;
                    }
                }
                else
                {
                    PRE_NMLimpHome = 1;
                    NM_Main_Program = NMInitReset;
                }
            }
            else
            {
                if(1 == NM_Networkstatus.BusSleep)
                {
                    if( (G_NM_PDU_Receive.OptCode&0x10) ==0x10)
                    {
                        if((G_NM_PDU_Receive.OptCode&0x20) == 0x20)
                        {
                            PRE_NMLimpHomePrepSleep = 1;
                            NM_Main_Program = NMInitBusSleep;
                        }
                        else
                        {
                            ;
                        }
                    }
                    else
                    {
                        NM_Main_Program = NMLimpHome;
                        //TRACE_BOOT( "NMLimpHome44\r\n" );
                    }
                }
                else
                {
                    NM_Main_Program = NMLimpHome;
                    //TRACE_BOOT( "NMLimpHome55\r\n" );
                }
            }
        }
    }
    else
    {
        ;
    }
    if( ( NM_TMax_Tick >= NM_TMax ) && ( 1 == NM_TMax_Enable ) )//tmax 超时
    {
        //TRACE_BOOT( "NMLimpHomePrepSleeptmaxout\r\n" );
        NM_TMax_Tick    = 0;
        NM_TMax_Enable  = 0;

        PRE_NMLimpHomePrepSleep = 1;
        NM_Main_Program = NMInitBusSleep;
    }
    else
    {
        ;
    }

#if 0 //mask by nongyubao 2016.11.25
    if(1 == NMPDU_TxOk) //success send
    {
        //TRACE_BOOT( "NMLimpHomePrepSleepsendok\r\n" );
        NMPDU_TxOk = 0;
#if 0	
	/**************add for can nm begin -liubo *************************/
		if((NM_Last_Ring_Msg & LIMPHOME_SI_CA) == LIMPHOME_SI_CA)
		{
			NM_TMax_Enable  = 0;
			PRE_NMLimpHomePrepSleep = 1;
        	NM_Main_Program = NMInitBusSleep;
		}
	/**************add for can nm end -liubo *************************/
#endif
        if(NM_Last_Ring_Msg & LIMPHOME_CI_CA)
        {
            NM_Merker.LimpHome = 1;
        }
        else
        {
            ;
        }
    }
    else
    {
        ;
    }
#endif 
}
void NMTwbsLimpHome( void )//sleep wait
{
    //TRACE_BOOT( "NMTwbsLimpHome\r\n" );
    if( 0x00 == NM_Sleep_Ind )//no sleep
    {
        //TRACE_BOOT( "NMTwbsLimpHome NoSleep\r\n" );
        /* CancelAlarm( Twbs ) */
        NM_TTwbs_Enable = 0;
        NM_TTwbs_Tick   = 0;

        NM_TErr_Enable = 1;
        NM_TErr_Tick = 0;

        D_Online( );

        NM_Main_Program = NMLimpHome;
        //TRACE_BOOT( "NMLimpHome66\r\n" );

        return;
    }
    else
    {
        ;
    }

    if( 1 == NMPDU_RxOk )/* 接收到NM帧 gxc*/
    {
        NMPDU_RxOk = 0;
        //TRACE_BOOT( "NMTwbsLimpHome receiveNM\r\n" );
        if((G_NM_PDU_Receive.OptCode&0x10) ==0x00)
        {
            /* CancelAlarm( Twbs ) */
            NM_TTwbs_Enable = 0;
            NM_TTwbs_Tick   = 0;

            NM_TErr_Enable = 1;
            NM_TErr_Tick = 0;

            D_Online( );
            NM_Main_Program = NMLimpHome;
            //TRACE_BOOT( "NMLimpHome77\r\n" );
        }
        else
        {
            ;
        }

        return;
    }
    else
    {
        ;
    }

    if( ( NM_TTwbs_Tick >= NM_TTwbs ) && ( 1 == NM_TTwbs_Enable ) )
    {
        //TRACE_BOOT( "NMTwbsLimpHome twbsout\r\n" );
        D_Init( BUSSLEEP );
        PRE_NMLimpHome =1;
        //NM_Main_Program = NMBusSleep;
        NMBusSleep();
    }
    else
    {
        ;
    }
}


void TransmitRingData( u8 Msg_Type )
{
    //TRACE_BOOT( "TransmitRingData\r\n" );
    NM_Last_Ring_Msg = Msg_Type;

    if( 1 == NM_Networkstatus.NMactive )
    {
        NM_Ring_Sent = 1;

        G_CAN_NM_TxData[1] = Msg_Type;

        if( Msg_Type & RING_CI_CA )
        {
            G_CAN_NM_TxData[0] = NM_Next_Node;

///         CAN_Transmit( ( 0x400 | OWN_NODE ), 0x00, 0x00, 0x08, G_CAN_NM_TxData );
            TBOX_NM_MS.id     = TBOX_NM_CANID;//0x407;
            TBOX_NM_MS.length = 8u;
            memcpy(TBOX_NM_MS.data, G_CAN_NM_TxData, 8);
            NM_CanTransmit(&TBOX_NM_MS);

            if( Msg_Type == RING_SI_CA )
            {
                //TRACE_BOOT( "Transmit12ring\r\n" );
            }
        }
        else if( Msg_Type & ALIVE_CI_CA )
        {
            G_CAN_NM_TxData[0] = OWN_NODE;
            //TRACE_BOOT( "sendalive\r\n" );
            //CAN_Transmit(0x2F8, 0x00, 0x00, 0x08, G_CAN_2F8_Data );
///         CAN_Transmit(( 0x400 | OWN_NODE ), 0x00, 0x00, 0x08, G_CAN_NM_TxData );
            TBOX_NM_MS.id     = TBOX_NM_CANID;//0x407u;
            TBOX_NM_MS.length = 8u;
            memcpy(TBOX_NM_MS.data, G_CAN_NM_TxData, 8);
            NM_CanTransmit(&TBOX_NM_MS);
        }
        else if(Msg_Type & LIMPHOME_CI_CA)
        {
            G_CAN_NM_TxData[0] = OWN_NODE;
            //TRACE_BOOT( "Transmit400ownnode\r\n" );
            //CAN_Transmit(0x2F8, 0x00, 0x00, 0x08, G_CAN_2F8_Data );
///         CAN_Transmit(( 0x400 | OWN_NODE ),0x00,0x00,0x08,G_CAN_NM_TxData );
            TBOX_NM_MS.id     = TBOX_NM_CANID;//0x407u;
            TBOX_NM_MS.length = 8u;
            memcpy(TBOX_NM_MS.data, G_CAN_NM_TxData, 8);
            NM_CanTransmit(&TBOX_NM_MS);
        }
        else
        {
            G_CAN_NM_TxData[0] = OWN_NODE;

///         CAN_Transmit( ( 0x400 | OWN_NODE ), 0x00, 0x00, 0x08, G_CAN_NM_TxData );
            TBOX_NM_MS.id     = TBOX_NM_CANID;//0x407u;
            TBOX_NM_MS.length = 8u;
            memcpy(TBOX_NM_MS.data, G_CAN_NM_TxData, 8);
            NM_CanTransmit(&TBOX_NM_MS);
        }
    }
    else
    {
        ;
    }
}

void NormalStandardNM( void )
{
    //TRACE_BOOT( "NormalStandardNM\r\n" );
    NMrxcount = 0;

    if( G_NM_PDU_Receive.OptCode & LIMPHOME_CI_CA )
    {
        /* Add sender to config.limphome */
        NM_Config.Limphome = G_NM_PDU_Receive.SourceID;

        return;
    }
    else/* Ring or Alive */
    {
        if( G_NM_PDU_Receive.SourceID < (u8)OWN_NODE ) /* 收到的 SID 在 L 侧 */
        {
            if( NM_Config.Present >= G_NM_PDU_Receive.SourceID )
            {
                NM_Config.Present = G_NM_PDU_Receive.SourceID;
            }
            else
            {
                ;
            }
        }
        else if( G_NM_PDU_Receive.SourceID == OWN_NODE )
        {
            ;/*  网络上有节点与自己相同 */
        }
        else    /* 收到的 SID 在 R 侧 */
        {
            if( NM_Config.Successor >= G_NM_PDU_Receive.SourceID )
            {
                NM_Config.Successor = G_NM_PDU_Receive.SourceID;
            }
            else
            {
                if( NM_Config.Successor == OWN_NODE )
                {
                    NM_Config.Successor = G_NM_PDU_Receive.SourceID;
                }
                else
                {
                    ;
                }
            }
        }

        if( NM_Config.Successor <= (u8)OWN_NODE )
        {
            NM_Next_Node = NM_Config.Present;
        }
        else
        {
            NM_Next_Node = NM_Config.Successor;
        }
        //TRACE_BOOT( "estimate alive or ring\r\n" );
        if( G_NM_PDU_Receive.OptCode & RING_CI_CA ) /* Ring */
        {
            NM_TTyp_Tick   = 0;
            NM_TMax_Tick   = 0;
            NM_TTyp_Enable = 0;
            NM_TMax_Enable = 0;
            //TRACE_BOOT( "typenable3\r\n" );

            if(OWN_NODE == G_NM_PDU_Receive.DestinationID || G_NM_PDU_Receive.SourceID == G_NM_PDU_Receive.DestinationID)
            {
                //put_hex(NM_TTyp_Tick);
                NM_TTyp_Tick   = 0;
                NM_TTyp_Enable = 1;
                //put_hex(NM_TTyp_Tick);
            }
            else
            {
                NM_TMax_Tick   = 0;
                NM_TMax_Enable = 1;
                //NM_TTyp_Tick   = 0;
                //NM_TTyp_Enable = 0;

                if( NM_NODE_NOSKIP == NMDetemine_NodeSkip( G_NM_PDU_Receive.SourceID, G_NM_PDU_Receive.DestinationID ) )/* 未跳过本节点 */
                {
                    ;
                }
                else/* 跳过本节点 */
                {
                    if( 1 == NM_Networkstatus.NMactive )
                    {
                        if( 1 == NM_Sleep_Ind )
                        {
                            TransmitRingData( ALIVE_SI_CA );
                        }
                        else
                        {
                            TransmitRingData( ALIVE_CI_CA );
                        }
                    }
                    else
                    {
                        ;
                    }
                }
            }
            //TRACE_BOOT( "ringover\r\n" );
        }
        else/* Alive */
        {
            NM_Merker.Stable = 0;
            NM_Networkstatus.ConfigrationStable = 0;
        }
    }
}

u8 NMDetemine_NodeSkip( u8 Source, u8 Destination )
{
    //TRACE_BOOT( "NMDetemine_NodeSkip\r\n" );
    if( Destination < OWN_NODE )//D<R
    {
        if( Source < Destination ) //S<D<R
        {
            return NM_NODE_NOSKIP;
        }
        else
        {
            if( Source < OWN_NODE )//D<S<R
            {
                return NM_NODE_SKIP;
            }
            else //S<D<R
            {
                return NM_NODE_NOSKIP;
            }
        }
    }
    else//D>=R
    {
        if( Source < Destination )//S<D
        {
            if( Source < OWN_NODE )//S<R<D
            {
                return NM_NODE_SKIP;
            }
            else//S>=D
            {
                return NM_NODE_NOSKIP;
            }
        }
        else//S>D
        {
            return NM_NODE_SKIP;
        }
    }
}

void GotoMode( u8 Mode_Type )
{
    if( BUSSLEEP == Mode_Type )
    {
        NM_Networkstatus.BusSleep = 1;
    }
    else if( BUSAWAKE == Mode_Type )
    {
        NM_Networkstatus.BusSleep = 0;
    }
    else
    {
        ;
    }
}


/*
    ****************************************************************************
    @function   NM_Time_Count
    @param
    @return
    ----------------------------------------------------------------------------
    @release    V1.00
    @author
    @brief
    @date       2011/3/3
    ****************************************************************************
*/
void NM_Time_Count( void )
{
    NM_TTyp_Tick    +=1;
    NM_TMax_Tick    +=1;
    NM_TErr_Tick    +=1;
    NM_TTwbs_Tick   +=1;
}

void Busoff_Handle(void)  //5ms Period
{
	static u32 BusoffEnterTimeCnt= 0;
	static u32 BusoffExitTimeCnt= 0;

	static u8 BusoffCount=0;

    #define TMER_CYC_MS     (5)
	
	#define TIME_50MS		(50/TMER_CYC_MS)
	#define TIME_150MS		(150/TMER_CYC_MS)
	#define TIME_1S		    (1000/TMER_CYC_MS)

    if(ENTER_BUSOFF == CAN_MCU_Interface.gblIndCANBusOff)
    {	
		BusoffExitTimeCnt = 0;
	
        BusoffEnterTimeCnt++;
		
        if(BusoffCount < 10u )
        {
            if(BusoffEnterTimeCnt >= TIME_50MS)
            {
                BusoffEnterTimeCnt = 0;
				
                BusoffCount++;
			
				if(BusoffCount==1)
				{
					DTCListAllResetTimer();
				}
				if(BusoffCount==3)
				{
					DTCCanBusOffProcess(1);
				}
				
                CAN_ConfigInit();

                NM_Sleeping     = 0;
				NM_Sleeping2 = 0;
                //NM_Stop_App     = 0;
                NM_TErr_Enable  = 1;
                NM_TErr_Tick    = NM_TErr;
                NM_Main_Program = NMLimpHome;

				CAN_MCU_Interface.gblIndCANBusOff = EXIT_BUSOFF;
				
            }
        }
        else
        {
            if(BusoffEnterTimeCnt >= TIME_1S)
            {
                BusoffEnterTimeCnt  = 0;
        
                CAN_ConfigInit();

                //NM_Stop_App     = 0;
                NM_Sleeping     = 0;
				NM_Sleeping2 = 0;
                NM_TErr_Enable  = 1;
                NM_TErr_Tick    = NM_TErr;
                NM_Main_Program = NMLimpHome;

				CAN_MCU_Interface.gblIndCANBusOff = EXIT_BUSOFF;
            }
        }
    }
    else
    {
        BusoffEnterTimeCnt = 0;

        BusoffExitTimeCnt++;
		
		if(BusoffExitTimeCnt > TIME_50MS)
		{
			BusoffCount = 0;
		}

		if(BusoffExitTimeCnt > TIME_150MS)
		{
			BusoffExitTimeCnt = 0;
			DTCCanBusOffProcess(0);
		}
    }
}

void NM_SysAllowBusSleep(void)
{
    NM_Sleep_Ind = 1;
    NM_Goto_Awake_Called=0;
}
void Nm_SysNotAllowBusSleep(void)
{
    NM_Sleep_Ind = 0;
    NM_Goto_Awake_Called=1;
    NM_Stop_App=0;
}

/**
*@NOTE:this function just for test
**/
void PrintCanNmSleepSts(void)
{
	static u16 PrintTimer = 0;
	static u8 B_gCallStatus = 0;
	static u8 B_AppAllowSleepFlag = 0;
	static u8 B_AccOffTimeOutFlag = 0;
	u8 PrintFlag = 0;

	PrintTimer +=TMER_CYC_MS;
	if ((B_gCallStatus != CAN_MCU_Interface.gCallStatus) || 
		(B_AppAllowSleepFlag != AppAllowSleepFlag) ||
		(B_AccOffTimeOutFlag != stAccOffJudge.AccOffTimeOutFlag))
	{
		B_gCallStatus = CAN_MCU_Interface.gCallStatus;
		B_AppAllowSleepFlag = AppAllowSleepFlag;
		B_AccOffTimeOutFlag = stAccOffJudge.AccOffTimeOutFlag;
		PrintTimer = 0;
		PrintFlag = 1;
	}
	else if(PrintTimer >= 1000*TMER_CYC_MS)  //5 S
	{
		PrintTimer = 0;
		PrintFlag = 1;
	}

	if(CAN_MCU_Interface.gblIndAccOn == true)
	{
		return;
	}

	if(PrintFlag == 1)
	{
		DEBUG_MCU(DBG_INFO, CAN_MODULE_ID, " AccOffTimeOutFlag:%x \r\n",stAccOffJudge.AccOffTimeOutFlag);
		DEBUG_MCU(DBG_INFO, CAN_MODULE_ID, " AppAllowSleepFlag :%x \r\n",AppAllowSleepFlag);
		DEBUG_MCU(DBG_INFO, CAN_MODULE_ID, " gCallStatus:%x \r\n",CAN_MCU_Interface.gCallStatus);
	}
	
}

void Process_can_NM(void)  //5ms Period
{
	NM_TmCb();

	if( true != gblCanSleepFlag )
    {
        (*NM_Main_Program)();
    }
	Busoff_Handle();
	CAN_NM_CheckGotoSleep();
	PrintCanNmSleepSts();//just for test 20171218 add 
}

void NM_TmCb(void)
{
    NM_Time_Count();
}
//void NM_RxPro( struct can_frame* msg )
void NM_RxPro( can_frame *gNmCanMsg )//接收NM 报文
{
    if(NM_Sleeping2 == 0)
    {
        NMPDU_RxOk = 1;
        G_NM_PDU_Receive.SourceID       = (u8)gNmCanMsg->id;
        G_NM_PDU_Receive.DestinationID  = gNmCanMsg->data[0];
        G_NM_PDU_Receive.OptCode            = gNmCanMsg->data[1];
		//TRACE_BOOT("G_NM_PDU_Receive.SourceID = %x, G_NM_PDU_Receive.DestinationID  = %x, G_NM_PDU_Receive.OptCode = %x\r\n",G_NM_PDU_Receive.SourceID,G_NM_PDU_Receive.DestinationID ,G_NM_PDU_Receive.OptCode);
        NormalStandardNM();

        if( 0x01 == NMLimp_status )
        {
            if( NM_Sleep_Ind ==1)
            {
                //GotoMode( BusSleep );
                NM_Networkstatus.BusSleep = 1;
                //TRACE_BOOT( "bussleep=1\r\n" );
            }
            else
            {
                //GotoMode( BusAwake );
                NM_Networkstatus.BusSleep = 0;
                //TRACE_BOOT( "bussleep=0\r\n" );
            }
        }
    }
}

#if 0
void Can_TxErScIntCb (void)
{

    u8 chTmp;

    if( CAN->TSR & 0x07 )
    {
        CAN->TSR |= 0x07;
        RestCounter = 0;

        if( 1 == NM_Ring_Sent )
        {
            NM_Ring_Sent = 0;
            NMPDU_TxOk = 1;
        }
        else
        {

            ;//NMPDU_TxOk = 0;
        }
    }
    else
    {
        ;
    }

    if( CAN->MSR & 0x08 )
    {
        CAN->MSR |= 0x08;
        //Printf_String( "gxc1\n\r" );
        NM_Sleeping = 0;
        NM_Stop_App = 0;
        if(0x01 == Canwakeflag)
        {
            Canwakeflag = 0x00;
            //Printf_String( "gxc2\n\r" );
            Soft_Reset();
        }
    }

    return;

}
#endif
void NM_TxConf(u8 ucPara)//NM报文发送完毕
{
    if( 7 != ucPara )
    {
        return;
    }

    if( 1 == NM_Ring_Sent )
    {
        NM_Ring_Sent = 0;
        NMPDU_TxOk = 1;
    }
    else
    {
        //NMPDU_TxOk = 0;
    }
}


void NM_CanWkupResume(void)//唤醒确保变量
{
    NM_Sleeping = 0;
    NM_Stop_App = 0;
}


#if  0
u8 CAN_Error_Status(void)
{
    return 0;
}

void Nm_test(void)
{
    TRACE_INFO("typ=%d,max=%d,Err=%d,wbs=%d\r\n",NM_TTyp_Tick,NM_TMax_Tick,NM_TErr_Tick,NM_TTwbs_Tick);
}
#endif
