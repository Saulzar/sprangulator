
#include "board.h"
#include "bitboard.h"
#include "time.h"
#include "transposition.h"
#include <string.h>
#include <stdlib.h>
#include <iostream.h>
#include <fstream.h>



//Random 64 bit values to xor together to form the hash key - extra space wasted for fast lookup
bitboard Transposition::hashMasksW[QUEEN+1][64];
bitboard Transposition::hashMasksB[QUEEN+1][64];
bitboard Transposition::castleMasks[8];
bitboard Transposition::enPassantMasks[64];


//For now just round to the nearest power of 2 hash entries - totalSize in bytes
Transposition::Transposition(int totalSize, Board *b) {
	int hashMax = totalSize / 16;
	int power;

	board = b;

	for(power=0; hashMax >= (1 << (power+1)); power++);
	hashSize = 1 << (power);

	//Hashtable basically one big array
	hashArray = new HashEntry[hashSize + 256];

	if(hashArray == NULL) {
		 cerr << "Couldn't allocate hash table of size: "  << (1<<power) << " bytes" << endl;
		 exit(-1);
	} else {
		cout << "Hash table created of size " << (hashSize * 16)/(1024 * 1024) << "Mb" << endl;
	}

	//Mask to perform hashKey % hashSize
	hashMask = 0;
	for(int i=0; i<power; i++) {
		hashMask |= SQUARE(i);
	}

	clear();
}

Transposition::~Transposition() {
	if(hashArray!=NULL) delete hashArray;
}

HashEntry *Transposition::probe(bitboard hashKey) {
	HashEntry *firstEntry = &hashArray[hashKey & hashMask];
	HashEntry *entry;
	unsigned char children = firstEntry->children;
	int child;
	unsigned int newKey = (unsigned int)(hashKey >> 32);
	int i=0;


	while(children) {
		i++;
		child = Bitboard::firstBit8[children];
		children ^= child;
		entry = firstEntry + child -1;

		if(entry->key == newKey) return entry;
	}



	return NULL;
}


void Transposition::store(int bestMove, int score, int depthSearched, int bound) {
	bitboard hashKey = board->stateStack[board->depth].hashKey;

	HashEntry *firstEntry = &hashArray[hashKey & hashMask];
	unsigned int newKey = (unsigned int)(hashKey >> 32);


	HashEntry *best = firstEntry;
	HashEntry *entry;

	int offsetBit;

	for(int child=1; child < 256; child = child << 1) {
		entry = firstEntry + (child - 1);
		if(entry->key == newKey) {

			if(depthSearched >= entry->depth) {
				if(bestMove > 0) entry->bestMove = bestMove;
				entry->key = newKey;
				entry->score = score;
				entry->depth = depthSearched;
				entry->flags = bound;
			}

			return;
		} else {
			if(entry->depth < best->depth) best = entry;
		}
	}


	//Replace the best candiate since we're out of entries
	offsetBit = (best - firstEntry) + 1;

	if(best->parent) {
		HashEntry *parent = best - (best->parent-1);
		parent->children ^= best->parent;
	}

	best->bestMove = bestMove;
	best->key = newKey;
	best->score = score;
	best->depth = depthSearched;
	best->flags = bound;

	best->parent = offsetBit;
	firstEntry->children |= offsetBit;
}

void Transposition::clear() {
	memset(hashArray, 0, sizeof(HashEntry) * hashSize);
}

//Only seems to cover 63  bits

bitboard randomBoard() {
	return rand() ^ ((bitboard)rand() << 15) ^ ((bitboard)rand() << 30) ^ ((bitboard)rand() << 45) ^ ((bitboard)rand() << 60);
}


void Transposition::saveKeys(char *filename) {
	srand(time(NULL));
	ofstream os(filename);

	for(int i=0; i<4096; i++) {
		os << randomBoard() << " ";
	}
}

void Transposition::initialiseKeys(char *filename) {
	ifstream is(filename);

	for(int i=0; i<QUEEN+1; i++) {
		for(int j=0; j<64; j++) {
			is >> hashMasksW[i][j];
		}
	}

	for(int i=0; i<QUEEN+1; i++) {
		for(int j=0; j<64; j++) {
			is >> hashMasksB[i][j];
		}
	}

	for(int i=0; i<8; i++) {
		is >> castleMasks[i];
	}

	for(int i=0; i<64; i++) {
		is >> enPassantMasks[i];
	}
}


