/** 
* Copyright (C), CHINATSP,www.chinatsp.com.  All rights reserved.
*
* @file heapsort.c
* 
* @Description heapsort
*
* @author jason
* 
* @version v1.0.0 
* 
* @date 2018/1/25
* 
* History£º
*
*/  

/************************************************
*			Include file							*
************************************************/
#include <stdio.h>
/************************************************
*			constant								*
************************************************/

/************************************************
*			internal macro							*
************************************************/
typedef unsigned int	u32;
typedef unsigned char u8;
typedef enum
{
	SMALL_TO_LARGE,
	LARGE_TO_SMALL,
}EN_SORT_MODE;
/************************************************
*			internal data type						*
************************************************/

/************************************************
*			static global variable					*
************************************************/
/************************************************
*			Local function declare					*
************************************************/
void MAX_HEAPIFY(u8 *pdata, u32 index, u32 size);
void build_max_heap(u8 *pdata, u32 size);
void MIN_HEAPIFY(u8 *pdata, u32 index, u32 size);
void build_min_heap(u8 *pdata, u32 size);
/************************************************
*			Global function						*
************************************************/
void heapSort(u8 *pdata, u32 size, EN_SORT_MODE mode)
{
	u32 i;
	if(mode == SMALL_TO_LARGE)
	{
		build_max_heap(pdata, size);
	}else if(mode == LARGE_TO_SMALL)
	{
		build_min_heap(pdata, size);
	}else
	{
		printf("Wrong mode!\r\n");
		return;
	}
	
	for(i = size;i>=2;i--)
	{
		u8 datatmp;
		datatmp = pdata[0];
		pdata[0] = pdata[i-1];
		pdata[i-1] = datatmp;
		if(mode == SMALL_TO_LARGE)
		{
			MAX_HEAPIFY(pdata, 0, (i-1));
		}else if(mode == LARGE_TO_SMALL)
		{
			MIN_HEAPIFY(pdata, 0, (i-1));
		}else
		{}
		
	}
}
/************************************************
*			Local function							*
************************************************/
void MAX_HEAPIFY(u8 *pdata, u32 index, u32 size)
{
	u32 left;
	u32 right;
	u32 largest;

	left = (index<<1) + 1;		//array min index is 0
	right = (index<<1) + 2;
	//printf("%d  %d  %d  \r\n", pdata[index],pdata[left],pdata[right]);
	if((left < size)&&(pdata[left] > pdata[index]))
	{
		largest = left;
	}else
	{
		largest = index;
	}

	if((right < size)&&(pdata[right] > pdata[largest]))
	{
		largest = right;
	}
	if(largest != index)
	{
		u8 datatmp;
		u8 i;
		datatmp = pdata[largest];
		pdata[largest] = pdata[index];
		pdata[index] = datatmp;
		MAX_HEAPIFY(pdata, largest, size);
	}
}
void MIN_HEAPIFY(u8 *pdata, u32 index, u32 size)
{
	u32 left;
	u32 right;
	u32 smallest;

	left = (index<<1) + 1;		//array min index is 0
	right = (index<<1) + 2;
	//printf("%d  %d  %d  \r\n", pdata[index],pdata[left],pdata[right]);
	if((left < size)&&(pdata[left] < pdata[index]))
	{
		smallest = left;
	}else
	{
		smallest = index;
	}

	if((right < size)&&(pdata[right] < pdata[smallest]))
	{
		smallest = right;
	}
	if(smallest != index)
	{
		u8 datatmp;
		u8 i;
		datatmp = pdata[smallest];
		pdata[smallest] = pdata[index];
		pdata[index] = datatmp;
		MIN_HEAPIFY(pdata, smallest, size);
	}
}
void build_max_heap(u8 *pdata, u32 size)
{
	u32 i;
	for(i=(size>>1);i>0;i--)
	{
		//printf("%d \r\n",i);
		MAX_HEAPIFY(pdata, (i-1), size);
	}
}
void build_min_heap(u8 *pdata, u32 size)
{
	u32 i;
	for(i=(size>>1);i>0;i--)
	{
		//printf("%d \r\n",i);
		MIN_HEAPIFY(pdata, (i-1), size);
	}
}
/************************************************
*			Test Main							*
************************************************/
#define TEST_CODE
#ifdef TEST_CODE
int main(int argc, char **argv)
{
	int i;
	u8 data[10] = {4,1,5,7,2,3,6,9,8,0};
	heapSort(&data[0], 10, LARGE_TO_SMALL);
	for(i = 0;i<10;i++)
	{
		printf("%d ",data[i]);
	}
}
#endif
/******************************************************************************
**                            End Of File
******************************************************************************/