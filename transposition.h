
#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H

#include "board.h"
#include "bitboard.h"
#include "time.h"
#include "const.h"
#include <string.h>
#include <stdlib.h>

//16 byte hash entry fieild
struct HashEntry {
	unsigned int key;	//Least significant 32 bits of the hash key - most significant bits are used for finding the hash entry
	int bestMove;			//Move resulting in this position
	short score;
	unsigned char flags;
	unsigned char depth;
	unsigned char children;
	unsigned char parent;
};

class Board;

class Transposition {
public:

HashEntry *hashArray;
char *repTable;

int hashSize;
Board *board;

bitboard hashMask;
Transposition(int totalSize, Board *board);
~Transposition();

HashEntry *probe(bitboard hashKey);
void store(int bestMove, int score, int depthSearched, int bound);

void clear();


//Random 64 bit values to xor together to form the hash key
static bitboard hashMasksW[QUEEN+1][64];
static bitboard hashMasksB[QUEEN+1][64];
static bitboard castleMasks[8];
static bitboard enPassantMasks[64];


static void saveKeys(char *filename);
static void initialiseKeys(char *filename);

};


#endif

