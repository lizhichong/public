/** 
* Copyright (C), CHINATSP,www.chinatsp.com.  All rights reserved.
*
* @file lifo.c
* 
* @Description LIFO
*
* @author jason
* 
* @version v1.0.0 
* 
* @date 2018/1/22
* 
* History£º
*
*/  

/************************************************
*			Include file							*
************************************************/
#include  <stdlib.h>
#include "Lifo.h"
#include <string.h>

/************************************************
*			constant								*
************************************************/

/************************************************
*			internal macro							*
************************************************/
/************************************************
*			internal data type						*
************************************************/

/************************************************
*			static global variable					*
************************************************/
/************************************************
*			Local function declare					*
************************************************/
/************************************************
*			Global function						*
************************************************/
u8 lifo_Init(lifo_TypeDef * _lifo, u8* data, u32 size)
{
    /* check for a valid pointer */
    if (_fifo == NULL)
    {
        return ERROR;
    }
    
    /* if fifo size is null nothing to do */
    if ( size == 0)
    {
        memset((u8 *)_fifo, 0x00, sizeof(fifo_TypeDef));
        return ERROR;
    }
    else
    {
        /* allocate fifo space. */
        _fifo->baseAddr = data;
        if (_fifo->baseAddr == NULL)
        {
            /* memory allocation failure. */
            #ifdef SZ_LOG
            putLine();
            putString("----> Malloc Fail!");
            #endif
            
            return ERROR;
        }
        else
        {
            memset((u8 *)_fifo->baseAddr, 0x00, sizeof(u8) * size);
            /* tail = head --> empty fifo */
            _fifo->head = 0;
            _fifo->tail = 0;
            _fifo->size = size;
            return SUCCESS;
        }
    }
}
/************************************************
*			Local function							*
************************************************/
/******************************************************************************
**                            End Of File
******************************************************************************/
