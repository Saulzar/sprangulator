#include "board.h"
#include "bitboard.h"
#include "transposition.h"
#include "utility.h"
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>

void bubbleSort(int n);
void bubbleSort2(int n);
void insertSort(int n);
void quickSort(int l, int r);
inline int partition(int l, int r);
inline int median3(int l, int c, int r);
inline void swap(int x, int y);



int *sortList = NULL;
int *valueArray = NULL;

void Board::sortCaptures(int firstMove, int hashMove) {
	int number = numMoves - firstMove;
	int *moveList = &moveStack[firstMove];

	for(int i=0; i<number; i++) {
		//Chuck out hash move as we already searched there..
		if(moveList[i] == hashMove) {
			moveList[i] = moveList[number-1];
			i--;
			number--;
			continue;
		}

		sortValues[i] = evaluateExchange((moveList[i] >> 6) & 63, moveList[i] & 63, (moveList[i] >> 20) & 15); //Source, dest, promote
	}

	sortList = moveList;
	valueArray = (int *)(sortValues);

	 bubbleSort(number);

	 numMoves = firstMove + number;
}

/*
	Find only good captures, used to determine when captures will stop to gain a stable board for evaluation
*/
void Board::sortGoodCaptures(int firstMove, int hashMove) {
	int number = numMoves - firstMove;
	int *moveList = &moveStack[firstMove];

	for(int i=0; i<number; i++) {
		if(moveList[i] == hashMove) {
			moveList[i] = moveList[number-1];
			i--;
			number--;
			continue;
		}

		int value = pieceValues[(moveList[i] >> 16) & 15] - pieceValues[(moveList[i] >> 12) & 15];	//Capture - type

		if(value > 0) {
			sortValues[i] = value;
		} else {
			sortValues[i] = evaluateExchange((moveList[i] >> 6) & 63, moveList[i] & 63, (moveList[i] >> 20) & 15); //Source, dest, promote
		}

		//Chuck out crap capture..
		if(sortValues[i] <= 0) {
			moveList[i] = moveList[number-1];
			i--;
			number--;
		}
	}

	sortList = moveList;
	valueArray = (int *)(sortValues);

	 bubbleSort(number);

	numMoves = firstMove + number;
}


//Nothing more than remove the hash move so far...
void Board::sortNonCaptures(int firstMove, int hashMove) {
	int number = numMoves - firstMove;
	int *moveList = &moveStack[firstMove];

	for(int i=0; i<number; i++) {
		//Chuck out hash move as we already searched there..
		if(moveList[i] == hashMove) {
			moveList[i] = moveList[number-1];
			numMoves = firstMove + number - 1; //Removed one move
			return;
		}
	}
}


void insertSort(int n) {
	int currentValue, currentItem;
	int j = 0;

	for(int i=1; i<n; i++) {
		if(valueArray[i-1] < valueArray[i]) {
			currentItem = sortList[i];
			currentValue = valueArray[i];

			for(j=i; j>0 && valueArray[j-1] <= currentValue; j--)  {
				sortList[j] = sortList[j-1];
				valueArray[j] = valueArray[j-1];
			}

			sortList[j] = currentItem;
			valueArray[j] = currentValue;
		}
	}
}

void bubbleSort(int n) {
	int done = 0;

	while(!done) {
		done = 1;
		for(int i=1; i<n; i++) {
			if(valueArray[i] > valueArray[i-1]) {
				swap(i, i-1);
				done = 0;
			}
		}
	}
}

void bubbleSort2(int n) {
	int done = 0;
	int *arrayPtr;
	int *listPtr;
	int *arrayMax = &valueArray[n];
	int temp;

	while(!done) {
		done = 1;
		arrayPtr = valueArray+1;
		listPtr = sortList + 1;

		for(;arrayPtr <= arrayMax; arrayPtr++, listPtr++) {
			if(*(arrayPtr-1) < *arrayPtr) {
				temp = *(arrayPtr);

				*(arrayPtr) = *(arrayPtr-1);
				*(arrayPtr-1) = temp;

				temp = *(listPtr);

				*(listPtr) = *(listPtr-1);
				*(listPtr-1) = temp;

				done = 0;
			}

		/*	if(valueArray[i] > valueArray[i-1]) {
				swap(i, i-1);
				done = 0;
			} */
		}
	}
}


void quickSort(int l, int r) {
        if(r-l <= 0 ) return;

       int pivot = median3(l, (l+r) >> 1, r);
       swap(pivot, r);

        pivot=partition(l, r);

        quickSort(l, pivot-1);
	quickSort(pivot+1, r);
}

inline int partition(int l, int r) {
	int pivot = valueArray[r];
	int leftPoint=l-1;
	int rightPoint=r;

	while(1) {
		while(valueArray[++leftPoint] > pivot);
		while(rightPoint > 0 && valueArray[--rightPoint] < pivot);

		if(leftPoint>=rightPoint) break;

		swap(leftPoint, rightPoint);
	}


	swap(leftPoint, r);
	return leftPoint;
}

inline int median3(int l, int c, int r) {
	if(valueArray[l] > valueArray[r]) {
		if(valueArray[r] > valueArray[c]) return r;
		if(valueArray[c] > valueArray[l]) return l;
	}
	return c;
}

inline void swap(int x, int y) {
	int temp = sortList[x];

	sortList[x] = sortList[y];
	sortList[y]= temp;

	temp = valueArray[x];

	valueArray[x] = valueArray[y];
	valueArray[y]= temp;
}



