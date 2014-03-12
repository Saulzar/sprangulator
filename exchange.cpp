#include "board.h"
#include "bitboard.h"
#include "utility.h"
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>


int Board::evaluateExchange(int source, int dest, int promote) {
	int exchangeList[32];
	int attacker = 0;
	int numExchanges = 1;
	int attackedValue;

	if(promote) {
		 attackedValue = pieceValues[promote];
		 exchangeList[0] = pieceValues[int(pieces[dest])] + (pieceValues[promote] - pieceValues[PAWN]);
	} else {
		attackedValue = pieceValues[int(pieces[source])];
		exchangeList[0] = pieceValues[int(pieces[dest])];
	}

	bitboard attacks = attacksTo(dest);

		//Find extra pieces outward from the piece being attacked dest -> source
	int direction = Bitboard::directions[dest][source];

	if(direction) attacks = removeAttacker(attacks, direction, source, dest);
	attacks &= ~SQUARE(source);

	int toMove = !sideToMove;


	while(attacks) {

		if(toMove == MOVE_WHITE) {
			if(attacks & wPawns) {
				attacker = Bitboard::firstSet(wPawns & attacks);
			} else if(attacks & wKnights) {
				attacker = Bitboard::firstSet(wKnights & attacks);
			} else if(attacks & wBishops) {
				attacker = Bitboard::firstSet(wBishops & attacks);
			} else if(attacks & wRooks) {
				attacker = Bitboard::firstSet(wRooks & attacks);
			} else if(attacks & wQueens) {
				attacker = Bitboard::firstSet(wQueens & attacks);
			} else if(attacks & wKing) {
				attacker = wKingSquare;
			} else break;
		} else {
			if(attacks & bPawns) {
				attacker = Bitboard::firstSet(bPawns & attacks);
			} else if(attacks & bKnights) {
				attacker = Bitboard::firstSet(bKnights & attacks);
			} else if(attacks & bBishops) {
				attacker = Bitboard::firstSet(bBishops & attacks);
			} else if(attacks & bRooks) {
				attacker = Bitboard::firstSet(bRooks & attacks);
			} else if(attacks & bQueens) {
				attacker = Bitboard::firstSet(bQueens & attacks);
			} else if(attacks & bKing) {
				attacker = bKingSquare;
			} else break;
		}

		exchangeList[numExchanges] =  attackedValue - exchangeList[numExchanges -1];
		numExchanges++;

		//Find extra pieces outward from the piece being attacked dest -> source
		direction = Bitboard::directions[dest][attacker];
		if(direction) attacks = removeAttacker(attacks, direction, attacker, dest);

		attacks ^= SQUARE(attacker);

		attackedValue = pieceValues[int(pieces[attacker])];
		toMove = !toMove;
	}



	while(--numExchanges) {
	    	if(exchangeList[numExchanges] > -exchangeList[numExchanges-1]) exchangeList[numExchanges - 1] =- exchangeList[numExchanges];
	}
	return exchangeList[0];
}

/*
	Remove an attacker from the bitboard of attacks to a particular square
	Moved is the piece which moved - attacked is the square under attack

	const int bishopDirections[4]={-9, -7, 7, 9};
	const int rookDirections[4]={-8,-1, 1, 8};
 */
bitboard Board::removeAttacker(bitboard attacks, int direction, int moved, int attacked) {

	switch(direction) {
		//Same rank
		case DIR_RANK:	return attacks |= rankAttacks(moved) & rookMovers & Bitboard::rookDirs[moved][2];			break;
		//Same file
		case DIR_FILE:		return attacks |= fileAttacks(moved) & rookMovers & Bitboard::rookDirs[moved][3];			break;
		//a1h8 diagonal

		//Same rank
		case -DIR_RANK:	return attacks |= rankAttacks(moved) & rookMovers & Bitboard::rookDirs[moved][1];			break;
		//Same file
		case -DIR_FILE:		return attacks |= fileAttacks(moved) & rookMovers & Bitboard::rookDirs[moved][0];			break;

		//a1h8 diagonal
		case DIR_A1H8:		return attacks |= a1h8Attacks(moved) & bishopMovers & Bitboard::bishopDirs[moved][0];		break;
		//h1a8 diagonal
		case DIR_H1A8:		return attacks |= h1a8Attacks(moved) & bishopMovers & Bitboard::bishopDirs[moved][1];		break;

		//a1h8 diagonal
		case -DIR_A1H8:		return attacks |= a1h8Attacks(moved) & bishopMovers & Bitboard::bishopDirs[moved][3];		break;
		//h1a8 diagonal
		case -DIR_H1A8:		return attacks |= h1a8Attacks(moved) & bishopMovers & Bitboard::bishopDirs[moved][2];		break;

		//Can't see it
		default:

		break;
	}

	return 0;
}


/* Check to see if the piece1 is pinned to piece2 by a slider piece. Return direction pinned
 *
 * If the piece can attack the other piece in a particular direction as if it were a queen,
 * and a sliding piece on the same direction can also attack that piece then it's pinned.
 */
int Board::pinnedTo(int piece1, int piece2) {
	int direction = Bitboard::directions[piece1][piece2];
	bitboard attacks;

	switch(abs(direction)) {
		//Same rank
		case DIR_RANK:
			attacks = rankAttacks(piece1);

			if((attacks & SQUARE(piece2))
				 && (attacks & rookMovers)) return DIR_RANK;
			else return 0;

		break;

		//Same file
		case DIR_FILE:
			attacks = fileAttacks(piece1);

			if((attacks & SQUARE(piece2))
				 && (attacks & rookMovers)) return DIR_FILE;
			else return 0;
		break;

		//a1h8 diagonal
		case DIR_A1H8:
			attacks = a1h8Attacks(piece1);

			if((attacks & SQUARE(piece2))
				 && (attacks & bishopMovers)) return DIR_A1H8;
			else return 0;

		break;

		//h1a8 diagonal
		case DIR_H1A8:
			attacks = h1a8Attacks(piece1);

			if((attacks & SQUARE(piece2))
				 && (attacks & bishopMovers)) return DIR_H1A8;
			else return 0;

		break;

		//Can't see it
		default:
			return 0;
		break;
	}
}
