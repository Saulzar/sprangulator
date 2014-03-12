
#include <math.h>
#include <iostream.h>
#include <stdio.h>
#include <string>
#include "bitboard.h"
#include "utility.h"

const int knightOffsets[8] = {-17, -15, -10, -6, 6, 10, 15, 17};
const int bishopDirections[4]={-9, -7, 7, 9};
const int rookDirections[4]={-8,-1, 1, 8};
const int allDirs[] = {-9, -7, 7, 9, -8,-1, 1, 8};

//Standard board layout for testing
const int standardBoard[64] =  {
                     0,   1,   2 ,   3,   4,   5,   6,   7,
                     8   ,9   ,10 ,11 ,12 ,13 ,14 ,15 ,
                     16 ,17 ,18 ,19 ,20 ,21 ,22 ,23 ,
                     24 ,25 ,26 ,27 ,28 ,29 ,30 ,31 ,
                     32 ,33 ,34 ,35 ,36 ,37 ,38 ,39 ,
                     40 ,41 ,42 ,43 ,44 ,45 ,46 ,47 ,
                     48 ,49 ,50 ,51 ,52 ,53 ,54 ,55 ,
                     56 ,57 ,58 ,59 ,60 ,61 ,62 ,63 };

//Rotate right (inverse rotate left)
const int rotateLeft90Inverse[64] = {
                     56, 48, 40, 32, 24, 16,  8,  0,
                     57, 49, 41, 33, 25, 17,  9,  1,
                     58, 50, 42, 34, 26, 18, 10,  2,
                     59, 51, 43, 35, 27, 19, 11,  3,
                     60, 52, 44, 36, 28, 20, 12,  4,
                     61, 53, 45, 37, 29, 21, 13,  5,
                     62, 54, 46, 38, 30, 22, 14,  6,
                     63, 55, 47, 39, 31, 23, 15,  7 };

//Rotate left 90 lookup
const int rotateLeft90[64] = {
                      7, 15, 23, 31, 39, 47, 55, 63,
                      6, 14, 22, 30, 38, 46, 54, 62,
                      5, 13, 21, 29, 37, 45, 53, 61,
                      4, 12, 20, 28, 36, 44, 52, 60,
                      3, 11, 19, 27, 35, 43, 51, 59,
                      2, 10, 18, 26, 34, 42, 50, 58,
                      1,  9, 17, 25, 33, 41, 49, 57,
                      0,  8, 16, 24, 32, 40, 48, 56 };

//Rotate left 45 lookup
const int rotateLeft45[64] = {
                                     0,
                                  2,  5,
                                9, 14, 20,
                             27, 35,  1,  4,
                            8, 13, 19, 26, 34,
                         42,  3,  7, 12, 18, 25,
                       33, 41, 48,  6, 11, 17, 24,
                     32, 40, 47, 53, 10, 16, 23, 31,
                       39, 46, 52, 57, 15, 22, 30,
                         38, 45, 51, 56, 60, 21,
                           29, 37, 44, 50, 55,
                             59, 62, 28, 36,
                               43, 49, 54,
                                 58, 61,
                                   63 };

//Inverse rotate left 45 lookup
const int rotateLeft45Inverse[64] = {
                                     0,
                                  8,  1,
                               16,  9,  2,
                             24, 17, 10,  3,
                           32, 25, 18, 11,  4,
                         40, 33, 26, 19, 12,  5,
                       48, 41, 34, 27, 20, 13,  6,
                     56, 49, 42, 35, 28, 21, 14,  7,
                       57, 50, 43, 36, 29, 22, 15,
                         58, 51, 44, 37, 30, 23,
                           59, 52, 45, 38, 31,
                             60, 53, 46, 39,
                               61, 54, 47,
                                 62, 55,
                                   63 };

//Rotate right 45 lookup
const int rotateRight45[64] = {
                                    28,
                                 21, 15,
                               10,  6,  3,
                              1,  0, 36, 29,
                           22, 16, 11,  7,  4,
                          2, 43, 37, 30, 23, 17,
                       12,  8,  5, 49, 44, 38, 31,
                     24, 18, 13,  9, 54, 50, 45, 39,
                       32, 25, 19, 14, 58, 55, 51,
                         46, 40, 33, 26, 20, 61,
                           59, 56, 52, 47, 41,
                             34, 27, 63, 62,
                               60, 57, 53,
                                 48, 42,
                                   35  };

//Inverse rotate right 45 lookup
const int rotateRight45Inverse[64] = {
                                     7,
                                  6, 15,
                                5, 14, 23,
                              4, 13, 22, 31,
                            3, 12, 21, 30, 39,
                          2, 11, 20, 29, 38, 47,
                        1, 10, 19, 28, 37, 46, 55,
                      0,  9, 18, 27, 36, 45, 54, 63,
                        8, 17, 26, 35, 44, 53, 62,
                         16, 25, 34, 43, 52, 61,
                           24, 33, 42, 51, 60,
                             32, 41, 50, 59,
                               40, 49, 58,
                                 48, 57,
                                   56 };

//Lengths of diagonals in rotated bitboard
const int diagonalLengths[64] = {
                                    1,
                                  2,  2,
                                3,  3,  3,
                              4,  4,  4,  4,
                            5,  5,  5,  5,  5,
                          6,  6,  6,  6,  6,  6,
                        7,  7,  7,  7,  7,  7,  7,
                      8,  8,  8,  8,  8,  8,  8,  8,
                        7,  7,  7,  7,  7,  7,  7,
                          6,  6,  6,  6,  6,  6,
                            5,  5,  5,  5,  5,
                              4,  4,  4,  4,
                                3,  3,  3,
                                  2,  2,
                                    1 };

//Start of diagonal positions for rotated bitboard
const int diagonalStarts[64] = {
				       0,
                                     1,  1,
                                   3,  3,  3,
                                 6,  6,  6,  6,
                              10, 10, 10, 10, 10,
                            15, 15, 15, 15, 15, 15,
                          21, 21, 21, 21, 21, 21, 21,
                        28, 28, 28, 28, 28, 28, 28, 28,
                          36, 36, 36, 36, 36, 36, 36,
                            43, 43, 43, 43, 43, 43,
                              49, 49, 49, 49, 49,
                                54, 54, 54, 54,
                                  58, 58, 58,
                                    61, 61,
                                      63 };

//Start of diagonal position lookup for a rotated left 45 bitboard
const int diagonalStartsLeft45[64] = {
		0,  1,  3,  6,  10, 15, 21, 28,
		1,  3,  6,  10, 15, 21, 28, 36,
		3,  6,  10, 15, 21, 28, 36, 43,
		6,  10, 15, 21, 28, 36, 43, 49,
		10, 15, 21, 28, 36, 43, 49, 54,
		15, 21, 28, 36, 43, 49, 54, 58,
		21, 28, 36, 43, 49, 54, 58, 61,
		28, 36, 43, 49, 54, 58, 61, 63 };


//Start of diagonal position lookup for a rotated right 45 bitboard
const int diagonalStartsRight45[64] = {
		28, 21, 15, 10, 6,  3,  1,  0,
		36, 28, 21, 15, 10, 6,  3,  1,
		43, 36, 28, 21, 15, 10, 6,  3,
		49, 43, 36, 28, 21, 15, 10, 6,
		54, 49, 43, 36, 28, 21, 15, 10,
		58, 54, 49, 43, 36, 28, 21, 15,
		61, 58, 54, 49, 43, 36, 28, 21,
		63, 61, 58, 54, 49, 43, 36, 28 };

const char *squareNames[64] = {
		"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
		"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
		"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
		"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
		"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
		"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
		"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
		"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
};

//Direction of one square to another for rooks/bishops
int Bitboard::directions[64][64];

//Vector of bits from one square to another
bitboard Bitboard::bitsTo[64][64];

bitboard  Bitboard::squares[64];
bitboard  Bitboard::squaresLeft90[64];
bitboard  Bitboard::squaresLeft45[64];
bitboard  Bitboard::squaresRight45[64];

bitboard  Bitboard::rankAttacks[64][256];
bitboard  Bitboard::fileAttacks[64][256];
bitboard  Bitboard::a1h8Attacks[64][256];
bitboard  Bitboard::h1a8Attacks[64][256];

char Bitboard::rankMobility[64][256];
char Bitboard::fileMobility[64][256];
char Bitboard::a1h8Mobility[64][256];
char Bitboard::h1a8Mobility[64][256];
char Bitboard::knightMobility[64];


bitboard Bitboard::knightAttacks[64];
bitboard Bitboard::rookAttacks[64];
bitboard Bitboard::bishopAttacks[64];
bitboard Bitboard::wPawnAttacks[64];
bitboard Bitboard::bPawnAttacks[64];
bitboard Bitboard::kingAttacks[64];
bitboard Bitboard::queenAttacks[64];


bitboard Bitboard::rookDirs[64][4];
bitboard Bitboard::bishopDirs[64][4];

bitboard Bitboard::files[8];
bitboard Bitboard::ranks[8];

bitboard Bitboard::inverseFiles[8];
bitboard Bitboard::inverseRanks[8];

/*Eval bitboards */
bitboard Bitboard::wPawnPasser[64];
bitboard Bitboard::bPawnPasser[64];

bitboard Bitboard::kingSurround[64];
int Bitboard::edgeSquares[64];

int Bitboard::firstSet8Bit[256];
int Bitboard::firstBit8[256];


const bitboard ONE = 1;

//Routine to precalculate all needed tables etc.
void Bitboard::initialise() {

	//Square table

	memset(files, 0, 8 * sizeof(bitboard));
	memset(ranks, 0, 8 * sizeof(bitboard));

	memset(inverseFiles, 0, 8 * sizeof(bitboard));
	memset(inverseRanks, 0, 8 * sizeof(bitboard));

	
	for(int i=0; i<64; i++) {
		squares[i] = ONE << i;
		files[FILE(i)]|= ONE << i;
		ranks[RANK(i)]|= ONE << i;
	}

	for(int i=0; i<8; i++) {
		inverseFiles[i] = ~files[i];
		inverseRanks[i] = ~ranks[i];
	}

	for(int i=0; i<64; i++) {
		squaresLeft90[i] = ONE << rotateLeft90[i];
	}

	for(int i=0; i<64; i++) {
		squaresLeft45[i] = ONE << rotateLeft45[i];
	}

	for(int i=0; i<64; i++) {
		squaresRight45[i] = ONE << rotateRight45[i];
	}

	//Directions table 0 on all squares which are not reachable by bishop or rook
	memset(directions, 0, sizeof(int) * 64 * 64);
	memset(bitsTo, 0, sizeof(bitboard) * 64 * 64);

	memset(rookDirs, 0, sizeof(bitboard) * 4 * 64);
	memset(bishopDirs, 0, sizeof(bitboard) * 4 * 64);

	memset(firstSet8Bit, 0, 256 * sizeof(int));

	for(int i=0; i<256; i++) {
		int done = 0;
		for(int j=0; j<8 && !done; j++) {
			if((unsigned char)i & (1 << j)) {
				firstSet8Bit[i] =j;
				firstBit8[i] = 1 << j;
				done = 1;
			}
		}
	}

	//Knight attacks
	for(int i=0; i<64; i++) {
		knightAttacks[i] = 0;
		int rank = RANK(i);
		int file = FILE(i);

		for(int j=0; j<8; j++) {
			int square = i + knightOffsets[j];

			if(abs(RANK(square) - rank) > 2 || abs(FILE(square) - file) > 2) continue;
			if(square < 0 || square > 63) continue;
			knightAttacks[i] |= SQUARE(square);

		}
		//print(knightAttacks[i]);

		knightMobility[i] = countOnes(knightAttacks[i]);
	}

	//Bishop attacks
	for(int i=0; i<64; i++) {
		bishopAttacks[i] = 0;
		int rank = RANK(i);
		int file = FILE(i);

		for(int j=0; j<4; j++) {
			int square = i;
			bitboard vectorTo=0;
				while(1) {
					square += bishopDirections[j];
					if((abs(RANK(square) - rank) != abs(FILE(square) - file))) break;
					if(square < 0 || square > 63) break;

					bishopAttacks[i] |=SQUARE(square);
					queenAttacks[i] |=SQUARE(square);

					bishopDirs[i][j] |=SQUARE(square);

					directions[i][square] = bishopDirections[j];
					vectorTo|= SQUARE(square);
					bitsTo[i][square] = vectorTo;

				}
		}
		//print(bishopAttacks[i]);
	}

	//Rook attacks
	for(int i=0; i<64; i++) {
		rookAttacks[i] = 0;
		int rank = RANK(i);
		int file = FILE(i);

		for(int j=0; j<4; j++) {
			int square = i;
			bitboard vectorTo=0;

				while(1) {
					square += rookDirections[j];
					if(FILE(abs(rookDirections[j])) && RANK(square) != rank) break;
					if(RANK(abs(rookDirections[j])) && FILE(square) != file) break;

					if(square < 0 || square > 63) break;

					rookAttacks[i] |=SQUARE(square);
					queenAttacks[i] |=SQUARE(square);

					rookDirs[i][j] |=SQUARE(square);


					directions[i][square] = rookDirections[j];
					vectorTo|= SQUARE(square);
					bitsTo[i][square] = vectorTo;
				}
		}
		//print(rookAttacks[i]);
	}

	//White pawn attacks
	for(int i=0; i<64; i++) {
		wPawnAttacks[i] = 0;
		int rank = RANK(i);
		int file = FILE(i);

		if(rank==0) continue;

		for(int j=0; j<2; j++) {
			int square = i + bishopDirections[j];
			if((abs(RANK(square) - rank) != abs(FILE(square) - file))) continue;
			if(square < 0 || square > 63) continue;
			wPawnAttacks[i] |= SQUARE(square);

		}
	//	print(wPawnAttacks[i]);
	}

	//Black pawn attacks
	for(int i=0; i<64; i++) {
		bPawnAttacks[i] = 0;
		int rank = RANK(i);
		int file = FILE(i);

		if(rank==7) continue;

		for(int j=0; j<2; j++) {
			int square = i + bishopDirections[j+2];
			if((abs(RANK(square) - rank) != abs(FILE(square) - file))) continue;
			if(square < 0 || square > 63) continue;
			bPawnAttacks[i] |= SQUARE(square);

		}
		//print(bPawnAttacks[i]);
	}


	//King attacks
	for(int i=0; i<64; i++) {
		kingAttacks[i] = 0;
		int rank = RANK(i);
		int file = FILE(i);

		for(int j=0; j<4; j++) {
			int square = i + bishopDirections[j];
			if((abs(RANK(square) - rank) != abs(FILE(square) - file))) continue;
			if(square < 0 || square > 63) continue;
			kingAttacks[i] |= SQUARE(square);

		}

		for(int j=0; j<4; j++) {
			int square = i + rookDirections[j];

			if(abs(RANK(square) - rank) !=0 && abs(FILE(square) - file) !=0) continue;
			if(square < 0 || square > 63) continue;

			kingAttacks[i] |=SQUARE(square);
		}

		edgeSquares[i] = 8-countOnes(kingAttacks[i]);

		//print(kingAttacks[i]);
	}

	//Rank attack lookup table
	for(int i=0; i<64; i++) {
		int rank = RANK(i);
		int file = FILE(i);

		for(int j=0; j<256; j++) {
			rankAttacks[i][j] = mapToBoard(generateSlideAttacks(j, file, 8),  (rank << 3), 8, standardBoard);
			rankMobility[i][j] = countOnes(rankAttacks[i][j]);
		}
	}

	//File attack lookup table
	for(int i=0; i<64; i++) {
		int t = rotateLeft90[i];
		int rank = RANK(t);
		int file = FILE(t);

		for(int j=0; j<256; j++) {
			fileAttacks[i][j] = mapToBoard(generateSlideAttacks(j, file, 8),  (rank << 3), 8, rotateLeft90Inverse);
			fileMobility[i][j] = countOnes(fileAttacks[i][j]);
		}
	}

		//a1 - h8 diagonal attacks..
	for(int i=0; i<64; i++) {
		int t = rotateRight45[i];
		int offset = diagonalStarts[t];
		int location = t-diagonalStarts[t];
		int length = diagonalLengths[t];

		for(int j=0; j<256; j++) {
			a1h8Attacks[i][j] = mapToBoard(generateSlideAttacks(j, location, length),  offset,  length, rotateRight45Inverse);
			a1h8Mobility[i][j] = countOnes(a1h8Attacks[i][j]);
		}
	}


		//h1 - a8 diagonal attacks..
	for(int i=0; i<64; i++) {
		int t = rotateLeft45[i];
		int offset = diagonalStarts[t];
		int location = t-diagonalStarts[t];
		int length = diagonalLengths[t];

		for(int j=0; j<256; j++) {
			h1a8Attacks[i][j] = mapToBoard(generateSlideAttacks(j, location, length),  offset,  length, rotateLeft45Inverse);
			h1a8Mobility[i][j] = countOnes(h1a8Attacks[i][j]);
		}
	}


	for(int i=8; i<64; i++) {
		wPawnPasser[i] = bitsTo[i][FILE(i)];

		if(FILE(i) > 0) wPawnPasser[i-8]|=bitsTo[i][FILE(i-1)];
		if(FILE(i) < 7) wPawnPasser[i-8]|=bitsTo[i][FILE(i+1)];
	}

	for(int i=0; i<56; i++) {
		bPawnPasser[i] = bitsTo[i][56 + FILE(i)];

		if(FILE(i) > 0) bPawnPasser[i]|=bitsTo[i+8][56 + FILE(i-1)];
		if(FILE(i) < 7) bPawnPasser[i]|=bitsTo[i+8][56 + FILE(i+1)];
	}

	/*

		for(int j=0; j<1; j++) {
			for(int i=28; i<39; i++) {

			print8(j);

			cout << FILE(i) << " " << RANK(i) << endl;
			print(rankAttacks[i][j] | fileAttacks[i][j] | a1h8Attacks[i][j] | h1a8Attacks[i][j], FILE(i), RANK(i));
			cout << endl;
			}
		}
*/


/*	for(int i=0; i<64; i++) {

		cout << "\"" << (char)(FILE(i) + 'a') << 8-RANK(i) << "\", ";

		if(FILE(i) == 7) {
			 cout << endl;
		}
	}*/


//	print(bitsTo[B8][H2]);

}

/* Used to map rotated slide attacks back onto the normal board.
	Map mask from one board layout onto another
*/
bitboard Bitboard::mapToBoard(int mask, int offset, int length, const int *boardMapping) {
	bitboard result = 0;

	for(int i=0; i< length; i++) {
		if(mask & SQUARE(i)) {
			result|=SQUARE(boardMapping[i+offset]);
		}
	}
	return result;
}

/* Map one board layout to another, used to rotate an entire bitboard */
bitboard Bitboard::mapToBoard(bitboard b, const int *boardMapping) {
	bitboard result = 0;

	for(int i=0; i< 64; i++) {
		if(b & SQUARE(i)) {
			result|=SQUARE(boardMapping[i]);
		}
	}

	return result;
}


// Find attacks for a particular rank/file/diagonal with pieces in the vector occupied, at location on that vector
int Bitboard::generateSlideAttacks(int occupied,  int location, int length) {
	int result = 0;

	if(location > length) return 0;

	for(int i=location-1; i >=0; i--) {
		result |=ONE << i;
		if(occupied &  (ONE << i)) break;
	}

	for(int i=location+1; i<length; i++) {
		result |=ONE << i;
		if(occupied &  (ONE << i)) break;
	}

	return result;
}




void Bitboard::print(char *name, bitboard b) {
	cout << name << endl;
	print(b);
}

void Bitboard::print(bitboard b) {
	for(int i=0; i<64; i++) {
		if(b  & (ONE << i)) cout << " X";
			else cout << " -";

		if(FILE(i) == 7) cout <<    " " << RANK(i) << endl;
	}

	cout << endl;
	cout << " 0 1 2 3 4 5 6 7" << endl;

	cout << endl;
}

void Bitboard::print(bitboard b, int x, int y) {
	for(int i=0; i<64; i++) {
		if(b  & (ONE << i)) cout << " X";
			else cout << " -";

		if(FILE(i) == 7) {
			if(RANK(i)==y) cout <<    "--" << RANK(i) << endl;
			else cout <<    "  " << RANK(i) << endl;
		}
	}

	for(int i=0; i<8; i++) {
		if(x==i) cout << " |";
		else cout << "  ";
	}
	cout << endl;
	cout << " 0 1 2 3 4 5 6 7" << endl;

	cout << endl;
}


void Bitboard::print8(int b) {

	for(int i=0; i<8; i++) {
		if(b  & (ONE << i)) cout << " X";
			else cout << " -";
	}

	cout << endl;
}

void Bitboard::printTable(int *table) {
	for(int i=0; i<64; i++) {
		
		cout << table[i] << ", ";
		if(table[i] < 10 && table[i] > 0) cout << " ";
		
		if(FILE(i) == 7) {
			 cout << endl;
		}
	}

	cout << endl;
}

