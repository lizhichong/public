#ifndef SORT_H_
#define SORT_H_

#define TYPE int
typedef enum{
	ASCENDING,
	DESCENDING
}SORT_MODE;

int InsertionSort(TYPE *buffer, const unsigned int length, const SORT_MODE mode);
int MergeSort(TYPE *src, int start, int end);
#endif 