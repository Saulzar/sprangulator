
#ifndef BITBOARD_H
#define BITBOARD_H 1

#include <stdlib.h>
#include <iostream.h>

#define RANK(x) (x>>3)
#define FILE(x) (x & 0x07)

#define SQUARE(x) (bitboard)(ONE << (x))

typedef unsigned long long bitboard;			//64 bits, 64 squares..

extern const bitboard ONE;

/*Board lookup tables
  Mapping from unrotated board - > rotated boards and back again*/

extern const int standardBoard[64]; //Standard board layout for testing
extern const int rotateLeft90Inverse[64]; //Rotate right (inverse rotate left)
extern const int rotateLeft90[64]; //Rotate left 90 lookup
extern const int rotateLeft45[64]; //Rotate left 45 lookup
extern const int rotateLeft45Inverse[64]; //Inverse rotate left 45 lookup
extern const int rotateRight45[64]; //Rotate right 45 lookup
extern const int rotateRight45Inverse[64]; //Inverse rotate right 45 lookup
extern const int diagonalLengths[64]; //Lengths of diagonals in rotated bitboard
extern const int diagonalStarts[64]; //Start of diagonal positions for rotated bitboard

extern const int diagonalStartsLeft45[64]; //Start of diagonal positions for rotated bitboard
extern const int diagonalStartsRight45[64]; //Start of diagonal positions for rotated bitboard

//squares layout in algebraic notation
enum {
	A8, B8, C8, D8, E8, F8, G8, H8,
	A7, B7, C7, D7, E7, F7, G7, H7,
	A6, B6, C6, D6, E6, F6, G6, H6,
	A5, B5, C5, D5, E5, F5, G5, H5,
	A4, B4, C4, D4, E4, F4, G4, H4,
	A3, B3, C3, D3, E3, F3, G3, H3,
	A2, B2, C2, D2, E2, F2, G2, H2,
	A1, B1, C1, D1, E1, F1, G1, H1
};


enum { RANK_8, RANK_7, RANK_6, RANK_5, RANK_4, RANK_3, RANK_2, RANK_1 };

enum { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H };


extern const char *squareNames[64];

//Directions
#define DIR_H1A8	9
#define DIR_A1H8	7
#define DIR_RANK	1
#define DIR_FILE	8

extern const int knightOffsets[8];
extern const int bishopDirections[4];
extern const int rookDirections[4];
extern const int allDirs[];

extern "C"
{
	int bsf64(bitboard b);
	int bsr64(bitboard b);
};


class Bitboard {

	/* Bitboard attacks pre-calculated for each combination of peices on each file/rank  and diagonals
	   for each square */

	public:
	static bitboard rankAttacks[64][256];
	static bitboard fileAttacks[64][256];
	static bitboard a1h8Attacks[64][256];
	static bitboard h1a8Attacks[64][256];

	static char rankMobility[64][256];
	static char fileMobility[64][256];
	static char a1h8Mobility[64][256];
	static char h1a8Mobility[64][256];

	static char knightMobility[64];
	
	static  bitboard squares[64];
	static  bitboard squaresLeft90[64];
	static  bitboard squaresLeft45[64];
	static  bitboard squaresRight45[64];

	static  bitboard knightAttacks[64];
	static  bitboard rookAttacks[64];
	static  bitboard bishopAttacks[64];
	static  bitboard wPawnAttacks[64];
	static  bitboard bPawnAttacks[64];
	static  bitboard kingAttacks[64];
	static  bitboard queenAttacks[64];

	//Direction of one square to another for rooks/bishops
	static int directions[64][64];

	//Vector of bits from one square to another
	static bitboard bitsTo[64][64];

	static bitboard files[8];
	static bitboard ranks[8];

	static bitboard inverseFiles[8];
	static bitboard inverseRanks[8];

	static bitboard rookDirs[64][4];
	static bitboard bishopDirs[64][4];


	/*Eval bitboards */
	static bitboard wPawnPasser[64];
	static bitboard bPawnPasser[64];

	static bitboard kingSurround[64];
	static int edgeSquares[64];

	static int firstSet8Bit[256];
	static int firstBit8[256];

	static void initialise();
	static void print(bitboard b);
	static void print(char *name, bitboard b);
	static void print(bitboard b, int x, int y);

	static void print8(int b);

	static int generateSlideAttacks(int occupied,  int location, int length);
	static bitboard mapToBoard(int mask, int offset, int length, const int *boardMapping);
	static bitboard mapToBoard(bitboard b, const int *boardMapping);
	static void Bitboard::printTable(int *table);


	static inline int countOnes(unsigned int b) {
		int i=0;

		while(b) {
			i++;
			b &= b - 1;
		}

		return i;
	}

	static const bitboard m1 = 0x5555555555555555LL;
	static const bitboard m2 = 0x3333333333333333LL;

	static inline int countOnes(bitboard b) {
    		unsigned int n;
		const bitboard a = b - ((b >> 1) & m1);
		const bitboard c = (a & m2) + ((a >> 2) & m2);
		n = ((unsigned int) c) + ((unsigned int) (c >> 32));
		n = (n & 0x0F0F0F0F) + ((n >> 4) & 0x0F0F0F0F);
		n = (n & 0xFFFF) + (n >> 16);
		n = (n & 0xFF) + (n >> 8);
		return n;
	}

	static inline int countOnes8(char c) {
		int n;

		while(c) {
			n++;
			c&= c - 1;
		}

		return n;
	}


	//Two sets of functions - one for receiving bitboards directly one from square values

	static inline void set(bitboard &b, bitboard &mask) {
		b|=mask;
	}

	static inline void clear(bitboard &b, bitboard &mask) {
		b&=(~mask);

	}

	static inline void setClear(bitboard &b, bitboard &mask) {
		b^=mask;
	}


	static inline void setSquare(bitboard &b, int square) {
		b|=SQUARE(square);
	}


	static inline void clearSquare(bitboard &b, int square) {
		b&=(~SQUARE(square));
	}

	static inline void setClearSquare(bitboard &b, int square) {
		b^=SQUARE(square);
	}

	//Needs to be replaced with a bitsearch instruction
	static inline int  firstSet (bitboard b) {
		 return bsf64(b);
	}

	//Needs to be replaced with a bitsearch instruction
	static inline int lastSet(bitboard b) {
		 return bsr64(b);
	}

	//Needs to be replaced with a bitsearch instruction
	static inline int  firstSet (char c) {
		 return firstSet8Bit[(int)c];
	}



	
};

#endif

