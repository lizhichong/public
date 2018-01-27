#ifndef LIFO_H_
#define LIFO_H_
#include "Typedef.h"

typedef struct
{
    u32 head;  /* output conut */
    u32 tail;     /*  input count   */
    u32 size;   /*  total size  */
    u8 * baseAddr;
}lifo_TypeDef;
#endif