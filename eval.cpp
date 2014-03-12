

#include "board.h"
#include "bitboard.h"
#include "transposition.h"
#include <string.h>
#include <stdlib.h>


const bitboard centre = SQUARE(E5) | SQUARE(E4) | SQUARE(D5) | SQUARE(D4) |  SQUARE(F5) | SQUARE(F4) |  SQUARE(C5) | SQUARE(C4);

//Standard board layout for testing
const int kingTable[64] =  {
                     10,  0,  0 ,  10,   10,   10,   0,  10,
                     10,  10,  40 ,  40,   40,   40,   10,  10,
					 40,  40,  40 ,  40,   40,   40,  40,  40,
					 80,  80,  80 ,  80,   80,   80,  80,  80,
					 80,  80,  80 ,  80,   80,   80,  80,  80,
					 40,  40,  40 ,  40,   40,   40,  40,  40,
					 10,  10,  40 ,  40,   40,   40,   10,  10,
					 10,  0,  0 ,  10,   10,   10,   0,  10 };
					 
                     


	/* Very advanced evaluation .. err, or something */
int Board::evaluate() {
	int score;
	int totalMaterial = whiteMaterial + blackMaterial;

	score = whiteMaterial - blackMaterial;

	int wPieces = evaluateWPieces();
	int bPieces = evaluateBPieces();

	score += wPieces << 2;
	score -= bPieces << 2;
	score += evaluatePawns() << 3;

    int wb = Bitboard::countOnes(wBishops);
    int bb = Bitboard::countOnes(bBishops);

	
	if(wb > 1) score+= 50;
	if(bb > 1) score-= 50;

	if(totalMaterial > 2000) {
	
		bitboard kSquare = Bitboard::firstSet(wKing); 
		score -= kingTable[kSquare] << 1;
		
		kSquare = Bitboard::firstSet(bKing); 
		score += kingTable[kSquare] << 1;
	}
	
	score += Bitboard::countOnes(wPawns & centre) * 50;
    score -= Bitboard::countOnes(bPawns & centre) * 50;
	
	
			
	if(sideToMove == MOVE_BLACK) score = -score;

	return score;
}




int Board::evaluateWPieces() {
	int square;
	int mobility = 0;
	int component;
	int numMobile = 0;
	bitboard b;


	b = wKnights;
	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);

		component = Bitboard::knightMobility[square];

		mobility += component;
		if(component > 6) numMobile++;

	}

	b = wBishops;
	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);

		component = Bitboard::h1a8Mobility[square][(occupiedLeft45 >> diagonalStartsLeft45[square]) & 0xFF];
		component +=	Bitboard::a1h8Mobility[square][(occupiedRight45 >> diagonalStartsRight45[square]) & 0xFF];

		mobility += component;
		if(component > 6) numMobile+=2;
	}

	b = wRooks;
	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);

		component = Bitboard::rankMobility[square][(occupied >> (square & 0xF8)) & 0xFF];
		component += Bitboard::fileMobility[square][(occupiedLeft90 >> (FILE(square) << 3)) & 0xFF];

		mobility += component;
		if(component > 6) numMobile++;
	}

	b = wQueens;

	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);

		component = Bitboard::rankMobility[square][(occupied >> (square & 0xF8)) & 0xFF];
		component += Bitboard::fileMobility[square][(occupiedLeft90 >> (FILE(square) << 3)) & 0xFF];
		component += Bitboard::h1a8Mobility[square][(occupiedLeft45 >> diagonalStartsLeft45[square]) & 0xFF];
		component +=	Bitboard::a1h8Mobility[square][(occupiedRight45 >> diagonalStartsRight45[square]) & 0xFF];

		mobility += (component >> 2);
	}

	return (mobility) + (numMobile << 3);
}

int Board::evaluateBPieces() {
	int square;
	int mobility = 0;
	int component;
	int numMobile = 0;
	bitboard b;

	//BLACK pieces

	 b = bKnights;
	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);

		component = Bitboard::knightMobility[square];

		mobility += component;
		if(component > 6) numMobile++;
	}

	b = bBishops;
	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);

		component = Bitboard::h1a8Mobility[square][(occupiedLeft45 >> diagonalStartsLeft45[square]) & 0xFF];
		component +=	Bitboard::a1h8Mobility[square][(occupiedRight45 >> diagonalStartsRight45[square]) & 0xFF];

		mobility += component;
		if(component > 6) numMobile+=2;
	}

	b = bRooks;
	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);

		component = Bitboard::rankMobility[square][(occupied >> (square & 0xF8)) & 0xFF];
		component += Bitboard::fileMobility[square][(occupiedLeft90 >> (FILE(square) << 3)) & 0xFF];

		mobility += component;
		if(component > 6) numMobile++;
	}


	b = bQueens;

	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);

		component = Bitboard::rankMobility[square][(occupied >> (square & 0xF8)) & 0xFF];
		component += Bitboard::fileMobility[square][(occupiedLeft90 >> (FILE(square) << 3)) & 0xFF];
		component += Bitboard::h1a8Mobility[square][(occupiedLeft45 >> diagonalStartsLeft45[square]) & 0xFF];
		component +=	Bitboard::a1h8Mobility[square][(occupiedRight45 >> diagonalStartsRight45[square]) & 0xFF];

		mobility += (component >> 2);
	}

	return (mobility) + (numMobile << 3);
}

int Board::evaluateWKingSafety() {
	int kingSafety = 0;
	int kingDanger = 0;
	bitboard b;
	bitboard buisy = 0;
	int rank, file;

	rank = RANK(wKingSquare);
	file = FILE(wKingSquare);

	bitboard kingSquares = Bitboard::kingAttacks[wKingSquare];

	while(kingSquares) {
		int square = Bitboard::firstSet(kingSquares);
		kingSquares ^= SQUARE(square);

		b = attacksTo(square);

		bitboard defenders = b & wPieces;

		kingDanger += Bitboard::countOnes(defenders);
		kingDanger += Bitboard::countOnes(defenders & ~buisy);
		kingDanger -= Bitboard::countOnes(b & bPieces) << 1;
		buisy |= defenders;
	}

	kingSafety += Bitboard::edgeSquares[wKingSquare] << 3;
	return (kingSafety + (kingDanger << 1));
}


int Board::evaluateBKingSafety() {
	int kingSafety = 0;
	int kingDanger = 0;
	bitboard b;
	bitboard buisy = 0;
	int rank, file;

	rank = RANK(bKingSquare);
	file = FILE(bKingSquare);

	bitboard kingSquares = Bitboard::kingAttacks[bKingSquare];

	while(kingSquares) {
		int square = Bitboard::firstSet(kingSquares);
		kingSquares ^= SQUARE(square);

		b = attacksTo(square);

		bitboard defenders = b & bPieces;

		kingDanger += Bitboard::countOnes(defenders);
		kingDanger += Bitboard::countOnes(defenders & ~buisy);
		kingDanger -= Bitboard::countOnes(b & wPieces) << 1;
		buisy |= defenders;
	}

	kingSafety += Bitboard::edgeSquares[bKingSquare] << 3;

	return (kingSafety + (kingDanger << 1));
}


int Board::evaluatePawns() {
	int mobility = 0;

		
	bitboard b = ((wPawns >> 8) & wPieces);
	mobility -= Bitboard::countOnes(b) << 3;

	b = ((bPawns << 8) & bPieces);
	mobility += Bitboard::countOnes(b) << 3;

	b = ((wPawns >> 16) & wPawns);
	mobility -= Bitboard::countOnes(b) << 2;

	b = ((bPawns << 16) & bPawns);
	mobility += Bitboard::countOnes(b) << 2;

	return mobility;
}






