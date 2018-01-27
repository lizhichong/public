#include <stdio.h>

#define PARAERROR -2
#define ERROR -1
#define MAX_NUMBER 268435455u


int InsertionSort(TYPE *buffer, const unsigned int length, const SORT_MODE mode)
{
	int j;
	int i;
	TYPE key;
	if((buffer == NULL) || (length <= 0))
	{
	    return PARAERROR;
	}
	for(j = 1;j < length;j++)
	{
		key = buffer[j];
		i = j - 1;
		if(mode == ASCENDING)
		{
			while((i >= 0) && (buffer[i] > key))
			{
			    buffer[i + 1] = buffer[i];
				i = i - 1;
			}
			buffer[i + 1] = key;
		}else if(mode == DESCENDING)
		{
			while((i >= 0) && (buffer[i] < key))
			{
			    buffer[i + 1] = buffer[i];
				i = i - 1;
			}
			buffer[i + 1] = key;
		}
	}
	return 0;
}
//#define USE_SENTRY
int Merge(TYPE *src, int p, int q, int r)
{
	int n1 = q - p + 1;
	int n2 = r - q;
	int i;
	int j;
	int k;
	TYPE L[n1 + 1];
	TYPE R[n2 + 1];
	for(i = 0;i < n1;i++)
	{
		L[i] = src[p + i];
	}
	for(j = 0;j < n2;j++)
	{
		R[j] = src[q + j + 1];
	}
	L[n1] = MAX_NUMBER;
	R[n2] = MAX_NUMBER;
	i = 0;
	j = 0;
	k = p;
#ifdef USE_SENTRY
	for(k = p;k <= r;k++)
	{
	    if(L[i]<=R[j])
	    {
	    	src[k] = L[i];
			i++;	
	    }else
	    {
	       src[k] = R[j];
			j++;
	    }
	}
#else
	for(;i < n1;i++)
	{
		for(;j < n2;j++)
		{
			if(R[j]<=L[i])
			{
				src[k] = R[j];
				k++;
			}else
			{
				break;
			}
		}
		src[k] = L[i];
		k++;
	}
#endif
	return 0;
}
/*
* nlgn£»
*/
int MergeSort(TYPE *src, int start, int end)
{
	int q;
	if(start < end)
	{
	    q = (start + end)/2;
		MergeSort(src, start, q);
		MergeSort(src, q + 1, end);
		Merge(src, start, q, end);
	}
}
#ifdef TEST_CODE
int main(int argc, char **argv)
{
	int i;
	TYPE data[10] = {4,1,5,7,2,3,6,9,8,0};
	MergeSort(data, 0, 9);
	for(i = 0;i<10;i++)
	{
		printf("%d ",data[i]);
	}
}
#endif