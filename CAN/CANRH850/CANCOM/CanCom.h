/*File Name:CanCom include file
------------------------------------------------------------------------------
			  C O P Y R I G H T
------------------------------------------------------------------------------
Copyright (c) by CHINATSP TECHNOLOGY.,Co.Ltd. All rights reserved.

					 This software is copyright protected and prorietary
					 to CHINATSP TECHNOLOGY.,Co.Ltd.CHINATSP TECHNOLOGY.,Co.Ltd
					 grants to you only those rights as set out in the license conditions.All other 
					 rights remain with Co.Ltd.CHINATSP TECHNOLOGY.,Co.Ltd.
------------------------------------------------------------------------------
			A U T O H O R		 I D E N T I T Y
------------------------------------------------------------------------------
Initials				Name					 Company
------			  ---------------	 -------------------------------------------------
ldl 					  Lidongliang			  CHINATSP TECHNOLOGY.,Co.Ltd
-------------------------------------------------------------------------------
				R E V I S I O N 		H I S T O RY
-------------------------------------------------------------------------------
Data						vesion			  author			Description
---------				--------	  --------		 ---------------------------------------
2017-06-19			   00.01.00 		ldl 				 First Creation
******************************************************************************/

#ifndef CAN_COM_H
#define CAN_COM_H

#include "sysconfig.h"
//#include "Can_config.h"
#ifdef _CAN_COM_ROOT
	#define EXTERN
#else
	#define EXTERN extern
#endif
#define PROJECT_CAN_6X4 0x01
#define  PROJECT_CAN_S111   0x02


#define  PROJECT_CONFIG_CAN  PROJECT_CAN_S111
#if(PROJECT_CONFIG_CAN==PROJECT_CAN_S111)
#if  1
#define  CONFIG_CAN_CH				(0)
#define  CONFIG_CAN_BAUDRATE	 (125) 
#define  CONFIG_CAN_SJW	    		(CAN_SJW_1TQ) 
#define  CONFIG_CAN_BS1	    		(CAN_BS1_11TQ) 
#define  CONFIG_CAN_BS2	    		(CAN_BS2_4TQ) 

#else
#define  CONFIG_CAN_CH				(0)
#define  CONFIG_CAN_BAUDRATE	  (500)  // (125) 
#define  CONFIG_CAN_SJW	    		(CAN_SJW_4TQ) //(CAN_SJW_1TQ) 
#define  CONFIG_CAN_BS1	    		(CAN_BS1_14TQ) //(CAN_BS1_11TQ) 
#define  CONFIG_CAN_BS2	    		(CAN_BS2_3TQ)//(CAN_BS2_4TQ) 
#endif
#elif(PROJECT_CONFIG_CAN==PROJECT_CAN_6X4)

#define  CONFIG_CAN_CH				(2)
#define  CONFIG_CAN_BAUDRATE	    (500) 
#define  CONFIG_CAN_SJW	    		(CAN_SJW_4TQ) 
#define  CONFIG_CAN_BS1	    		(CAN_BS1_14TQ) 
#define  CONFIG_CAN_BS2	    		(CAN_BS2_3TQ)
#endif

#if(PROJECT_CONFIG_CAN==PROJECT_CAN_S111)
#define CAN_USE_CHN_NUM     CONFIG_CAN_CH
#elif(PROJECT_CONFIG_CAN==PROJECT_CAN_6X4)
#define CAN_USE_CHN_NUM     CONFIG_CAN_CH
#endif


#define CAN_DIAGTXMSGOBJNUM 35
#define CAN_NMTXMSGOBJNUM   36
#define CAN_APP2F8IDTXMSGOBJNUM 37
#define CAN_APP600IDTXMSGOBJNUM 38
#define CAN_APPTXMSGOBJNUM 39

EXTERN void DisableCANCtl(void);
EXTERN void EnableCANCtl(void);
EXTERN void CanInitSetup(void *	CanTxCplHandle, void *CanRxHandle,void *CanRxError);
//EXTERN void CanTxFrameMsg(can_frame*txcandata);


#undef EXTERN
#endif
