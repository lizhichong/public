/*****************************************************************
*Company: BSJ
*Creator: LKF
*Project: 
*Module name: 
*
*########################################
*Compiler: IAR
*MCU:  STM32F103VCT6
*########################################
*
*File Name: fifo.h V1.2.2
*Purpose: This file contains all the functions prototypes for fifo management routines
*File References: 
*Create Date: Jan-10-2013
*Change History
*Description: 
******************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _FIFO_H_
#define _FIFO_H_
/* Includes ------------------------------------------------------------------*/

#include "sta10xx_type.h"
#include "macros.h"

typedef struct
{
    u32 head;  /* output conut */
    u32 tail;     /*  input count   */
    u32 size;   /*  total size  */
    u8 * baseAddr;
}fifo_TypeDef;




u8 fifo_Init(fifo_TypeDef * _fifo, u8* data, u32 size);
u8 fifo_DeInit(fifo_TypeDef * _fifo);
u8 fifo_Reset(fifo_TypeDef * _fifo);


u32 fifo_insert(fifo_TypeDef * _fifo, u8 *data, u32 len);
u32 fifo_retrieve(fifo_TypeDef * _fifo, u8 *data, u32 len);

u32 fifo_GetLen(fifo_TypeDef * _fifo);
FlagStatus fifo_NotFull(fifo_TypeDef * _fifo);
u32 fifo_GetFreeSpace(fifo_TypeDef * _fifo);
u32  Fifo_strchr(fifo_TypeDef * _fifo, u8 data);

#endif

