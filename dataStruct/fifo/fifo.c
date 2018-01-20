/*****************************************************************
*Company: BSJ
*Creator: LKF
*Project: XT-300
*Module name: 
*
*########################################
*Compiler: IAR6.1
*MCU:  STM32F217ZGT6, 1024KROM  128KRAM  M50
*CLOCK: HSE 12Mhz   PLL: 120MHz
*########################################
*
*File Name: 
*Purpose: 
*File References: 
*Create Date: Mar-27-2014
*Change History
*Description   
******************************************************************/
  
/* Includes ------------------------------------------------------------------*/
#include  <stdlib.h>
#include "fifo.h"
#include <string.h>
//#include "header.h"

#define minimum(x, y)  (((x) < (y))? (x) : (y))


/************************************************************************
    Function : 
    Description : Init the fifo structure.
            _fifo : reference to _fifo struct.
            size : size of the fifo.
            retval ERROR if memory allocation fails or SUCCESS.
    Change history:
    Note:
    Author:     
    Date:     Jan-10-2013
************************************************************************/
u8 fifo_Init(fifo_TypeDef * _fifo, u8* data, u32 size)
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


/************************************************************************
    Function : 
    Description : DeInit the fifo structure.
            _fifo : reference to _fifo struct.
    Change history:
    Note:
    Author:     
    Date:     Jan-10-2013
************************************************************************/
u8 fifo_DeInit(fifo_TypeDef * _fifo)
{
    u8 status = ERROR;
    
    if (_fifo == NULL)
    {
        status = ERROR;
    }
    else
    {
        if (_fifo->baseAddr != NULL)
        {
            /* optional (clear memory region) */
            memset((u8 *)_fifo->baseAddr, 0x00, _fifo->size);
            memset((u8 *)_fifo, 0x00, sizeof(fifo_TypeDef));
            status = SUCCESS;
        }
        else
        {
            /* FiFo invalid base address. */
            status = ERROR;
        }
    }
    return status;
}


/************************************************************************
    Function : 
    Description : Reset the fifo structure.
            _fifo : reference to _fifo struct.
    Change history:
    Note:
    Author:     
    Date:     Jan-10-2013
************************************************************************/
u8 fifo_Reset(fifo_TypeDef * _fifo)
{
    if (_fifo == NULL)
    {
        return ERROR;
    }
    else
    {
        if (_fifo->baseAddr != NULL)
        {
            /* optional (clear memory region) */
            memset((u8 *)_fifo->baseAddr, 0x00, _fifo->size);
            
            _fifo->head = 0;
            _fifo->tail = 0;
            return SUCCESS;
        }
        else
        {
            /* FiFo invalid base address. */
            return ERROR;
        }
    }
}



/************************************************************************
    Function : 
    Description : returns how much data is stored in the fifo.
            _fifo : reference to _fifo struct.
    Change history:
    Note:
    Author:     
    Date:     Jan-10-2013
************************************************************************/
u32 fifo_GetLen(fifo_TypeDef * _fifo)
{
    if (_fifo == NULL)
    {
        return 0;
    }
    else
    {
        return (_fifo->tail - _fifo->head);
    }
}


/************************************************************************
    Function : 
    Description : returns how much free sapce in the fifo.
            _fifo : reference to _fifo struct.
    Change history:
    Note:
    Author:     
    Date:     Jan-10-2013
************************************************************************/
u32 fifo_GetFreeSpace(fifo_TypeDef * _fifo)
{
    if (_fifo == NULL)
    {
        return 0;
    }
    else
    {
        return (_fifo->size - (_fifo->tail - _fifo->head));
    }
}



/************************************************************************
    Function : 
    Description : returns SET if the FIFO is full.
            _fifo : reference to _fifo struct.
    Change history:
    Note:
    Author:     
    Date:     Jan-10-2013
************************************************************************/
FlagStatus fifo_NotFull(fifo_TypeDef * _fifo)
{
    if (_fifo == NULL)
    {
        return RESET;
    }
    else
    {
        return ((_fifo->tail - _fifo->head) != _fifo->size) ? SET : RESET;
    }
}


/************************************************************************
    Function : 
    Description : Insert data to the fifo.
            _fifo : reference to _fifo struct.
            data : reference to data buffer.
            len : data buffer length.
    Change history:
    Note:
    Author:     
    Date:     Jan-10-2013
************************************************************************/
u32 fifo_insert(fifo_TypeDef * _fifo, u8 *data, u32 len)
{
	u32 real_len;
	u32 offset;
	u32 end;


	offset   = _fifo->size - (_fifo->tail - _fifo->head);
    /*Actual size of data we can write*/
    real_len = minimum(len, offset);
    /*Offset of write pointer*/
    offset   = _fifo->tail % _fifo->size;
    /*End position before buffer being warpped*/
    end      = minimum(real_len,  _fifo->size - offset);
    
	/*Copy data to buffer before warp*/
	memcpy(_fifo->baseAddr + offset, data, end);
	
	/*Copy data to buffer after warp*/
	memcpy(_fifo->baseAddr, data + end, real_len - end);
	
	_fifo->tail += real_len;
	
	return real_len;
}


/************************************************************************
    Function : 
    Description : Retrieve data from the fifo.
            _fifo : reference to _fifo struct.
            data : reference to data buffer.
            len : data buffer length.
    Change history:
    Note:
    Author:     
    Date:     Jan-10-2013
************************************************************************/
u32 fifo_retrieve(fifo_TypeDef * _fifo, u8 *data, u32 len)
{
	u32 real_len;
	u32 offset;
	u32 end;
  
	
    offset   = _fifo->tail- _fifo->head;
	real_len = minimum(len, offset);
	
    //得到偏移量
	offset = _fifo->head % _fifo->size;
    
	end = minimum(real_len, _fifo->size - offset);
	
	memcpy(data, _fifo->baseAddr + offset, end);
	
	memcpy(data + end, _fifo->baseAddr, real_len - end);
	
	_fifo->head += real_len;
	
	return real_len;
}

/*****************************************************************************
* 函数名:  串口通道帧尾获取函数
* 函数功能: 获取当前帧帧尾所在地址
* 输入参数: _fifo - fifo结构体
* 输入参数: data-帧尾结束符
* 返回数据: fifo中接收到的数据到字节data第一次出现的长度
*****************************************************************************/ 
u32  Fifo_strchr(fifo_TypeDef * _fifo, u8 data)
{
    u32 remaining = 0;
    u32 len       = 0;
    u32 head;
    

    head      = _fifo->head;
    remaining = _fifo->tail- head;
    head      = head % _fifo->size;
	
	
	while(remaining)
	{   
        len++;
        if(data == *((_fifo->baseAddr + head)))
        {
            return len;
        }
        head++;
        remaining--;
		head %= _fifo->size;
	}
	return 0;
}

