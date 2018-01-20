/*File Name:CanCom source file
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
#define _CAN_COM_ROOT
#include "CanCom.h"
//#include "Can_config.h"
#include "CAN.h"
#include "base_types.h"
#include "gpio.h"
#include "bsp.h"



CAN_HandleTypeDef sConfigCan;
//static CAN_FilterConfTypeDef sFilterCan;

static CanTxMsgTypeDef sCanTxMsg;
static CanRxMsgTypeDef sCanRxMsg;


void CanInitSetup(void *CanTxCplHandle, void *CanRxHandle,void *CanRxError)
{
	sConfigCan.Init.Prescaler = CONFIG_CAN_BAUDRATE;
	sConfigCan.Init.Channel = CONFIG_CAN_CH;
    sConfigCan.Init.BS1 = CONFIG_CAN_BS1;
	sConfigCan.Init.BS2 = CONFIG_CAN_BS2;
	sConfigCan.Init.SJW = CONFIG_CAN_SJW;
	//sConfigCan.Init.StdMode = ENABLE;
	sConfigCan.pTxMsg= &sCanTxMsg;
	sConfigCan.pRxMsg= &sCanRxMsg;
	sConfigCan.CanTxCpl = (HAL_Callback_Func1)CanTxCplHandle;
	sConfigCan.CanRxCpl= (HAL_CAN_Callback_RX)CanRxHandle;
	sConfigCan.CanRxErr = (HAL_Callback_Func1)CanRxError;
	HAL_CAN_DeInit(&sConfigCan);
	HAL_CAN_Init(&sConfigCan);

	//sFilterCan.FilterActivation = ENABLE;
	//sFilterCan.FilterIdHigh = 0x00;
	//sFilterCan.FilterMaskIdHigh = 0x00;
	//sFilterCan.FilterMaskIdLow = 0x00;
	//sFilterCan.FilterNumber = 0x0001;
	//HAL_CAN_ConfigFilter(&sConfigCan,&sFilterCan);
	EnableCANCtl();
}

/*
* set the pin of stb
*
* para:
*	ENABLE: enable CAN;	DISABLE: standby CAN	
*/
void set_CAN0_STB(u8 para)
{
    //GPIO_output_init(P0_13);
    //GPIO_output_one(P0_13); 
    CAN0STB = para;
}
void set_CAN1_STB(u8 para)
{
    //GPIO_output_init(P0_13);
    //GPIO_output_one(P0_13); 
    CAN1STB = para;
}
void set_CAN2_STB(u8 para)
{
    //GPIO_output_init(P0_13);
    //GPIO_output_one(P0_13); 
    CAN2STB = para;
}


HAL_StatusTypeDef Can_TxCanMsgProcess(can_frame* candata)
{		     
    HAL_StatusTypeDef ret = HAL_OK;

	sConfigCan.pTxMsg->IDE = CAN_ID_STD;
	sConfigCan.pTxMsg->StdId =candata->id;
	sConfigCan.pTxMsg->DLC =candata->length;
	if(sConfigCan.pTxMsg->DLC >8) 
	{
	    sConfigCan.pTxMsg->DLC =8;
	}
	memcpy(sConfigCan.pTxMsg->Data, &candata->data[0], sConfigCan.pTxMsg->DLC );
    ret  = HAL_CAN_Transmit_IT(&sConfigCan);  
    return ret;
}

#if 0
__interrupt void Can_IRQHandler(void)
{
	HAL_CAN_IRQHandler(&sConfigCan);
}
#endif

#if 0
void DisableCANCtl(void)
{
	sConfigCan.State = HAL_CAN_STATE_RESET;
	HAL_CAN_WokingStatus(sConfigCan);
}

void EnableCANCtl(void)
{
	sConfigCan.State = HAL_CAN_STATE_READY;
	HAL_CAN_WokingStatus(sConfigCan);
}
#endif

HAL_StatusTypeDef HAL_CAN_Init(CAN_HandleTypeDef *Hcan)
{
	CAN_StructInit(&(Hcan->Init));
	return HAL_OK;
}

#if 0
HAL_StatusTypeDef HAL_CAN_ConfigFilter(CAN_HandleTypeDef *Hcan,CAN_FilterConfTypeDef* sFilterConfig)
{
	return CAN_ConfigFilter(&(Hcan->Init),sFilterConfig);
}
#endif


/**
  * @brief  Deinitializes the CANx peripheral registers to their default reset values. 
  * @param  hcan: pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.  
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CAN_DeInit(CAN_HandleTypeDef* hcan)
{
  /* Check CAN handle */
  if(hcan == 0)
  {
     return HAL_ERROR;
  }
  
  /* Change CAN state */
  hcan->State = HAL_CAN_STATE_BUSY;
  

  /* Change CAN state */
  hcan->State = HAL_CAN_STATE_RESET;

  /* Release Lock */
  hcan->Lock = HAL_UNLOCKED;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Initiates and transmits a CAN frame message.
  * @param  hcan: pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.  
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CAN_Transmit_IT(CAN_HandleTypeDef* hcan)
{
	u32 label;
	switch(hcan->pTxMsg->StdId)
	{
		case 0x296:
			label = 0x01;
			break;
		case 0x71F:
			label = 0x06;
			break;
		case 0x407:
			label = 0x07;
			break;
		default:
			label = 0;
			break;
	}
	return CAN_Transmit_IT(hcan, label);
}

#if 0
/**
  * @brief  Handles CAN interrupt request  
  * @param  hcan: pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @retval None
  */
void HAL_CAN_IRQHandler(CAN_HandleTypeDef* hcan)
{
	CAN_IRQHandler(hcan);
}
#endif


/**
  * @brief  Handles CAN interrupt request  
  * @param  hcan: pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @retval None
  */
void HAL_CAN_WokingStatus(CAN_HandleTypeDef hcan)
{
	if(hcan.State == HAL_CAN_STATE_READY)
	{
		EnableCAN_INT();
	}
	else
	{
		DisableCAN_INT();
	}	
}

void DisableCANCtl(void)
{
	sConfigCan.State = HAL_CAN_STATE_RESET;
	HAL_CAN_WokingStatus(sConfigCan);
}

void EnableCANCtl(void)
{
	sConfigCan.State = HAL_CAN_STATE_READY;
	HAL_CAN_WokingStatus(sConfigCan);
}

