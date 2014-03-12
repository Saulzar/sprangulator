
/* Move generation functions relating to boards here */

#include "board.h"
#include "bitboard.h"
#include "transposition.h"
#include <string.h>
#include <stdlib.h>



/*
 * Generate a list of all possible moves, some illegal moves will be filtered out later
 * Generate moves in order of captures -> non captures
 */


void Board::generateCaptures() {
	bitboard epSquare =   currentState->enPassant?  SQUARE(currentState->enPassant) : 0;
	if(sideToMove==MOVE_WHITE) {

		whitePieceCaptures(bPieces);
		whitePawnCaptures(bPieces  |  epSquare );
		whiteKingCaptures(bPieces);
		whitePawnPromotions(~occupied);

	} else {

		blackPieceCaptures(wPieces);
		blackPawnCaptures(wPieces |  epSquare);
		blackKingCaptures(wPieces);
		blackPawnPromotions(~occupied);
	}
}


void Board::generateNonCaptures() {
	bitboard freeSquares = ~occupied;

	if(sideToMove==MOVE_WHITE) {
		whiteCastlingMoves();
		whitePieceMoves(freeSquares);
		whitePawnMoves(freeSquares);
		whiteKingMoves(freeSquares);
	} else {
		blackCastlingMoves();
		blackPieceMoves(freeSquares);
		blackPawnMoves(freeSquares);
		blackKingMoves(freeSquares);
	}
}


void Board::generateCheckEvasions() {

//int n = numMoves;
	if(sideToMove == MOVE_WHITE) {
		int attacker;

		bitboard attacksKing = attacksTo(wKingSquare) & bPieces;

		attacker = Bitboard::firstSet(attacksKing);
		attacksKing ^= SQUARE(attacker);

		//More than one attacker?
		if(!attacksKing) {
			bitboard epSquare =   currentState->enPassant?  SQUARE(currentState->enPassant) : 0;
			bitboard mask = Bitboard::bitsTo[wKingSquare][attacker] | SQUARE(attacker);

			whitePieceCaptures(bPieces & mask);
			whitePawnCaptures(bPieces & ( mask | epSquare));

			if(pieces[attacker] >= BISHOP) {
				bitboard freeSquares = (~occupied) & mask;

				whitePawnPromotions(freeSquares);
				whitePieceMoves(freeSquares);
				whitePawnMoves(freeSquares);
			}
		}


		bitboard dest = kingAttacksFrom(wKingSquare) & (~occupied | bPieces);

		while(dest) {
			int square = Bitboard::firstSet(dest);
			dest^=SQUARE(square);

			if(!(attacksTo(square) & bPieces)) {
				moveStack[numMoves++] = (KING << 12) | (wKingSquare << 6) | square | (pieces[square] << 16);

			}

		}

		/*for(int i=n; i< numMoves; i++) {
			printMove(moveStack[i]);
		}*/

	} else {					//MOVE_BLACK
		int attacker;

		bitboard attacksKing = attacksTo(bKingSquare) & wPieces;

		attacker = Bitboard::firstSet(attacksKing);
		attacksKing ^= SQUARE(attacker);

		//More than one attacker?
		if(!attacksKing) {
			bitboard epSquare =   currentState->enPassant?  SQUARE(currentState->enPassant) : 0;
			bitboard mask = Bitboard::bitsTo[bKingSquare][attacker] | SQUARE(attacker);

			blackPieceCaptures(wPieces & mask);
			blackPawnCaptures(wPieces & (mask | epSquare));

			if(pieces[attacker] >= BISHOP) {
				bitboard freeSquares = (~occupied) & mask;

				blackPawnPromotions(freeSquares);
				blackPieceMoves(freeSquares);
				blackPawnMoves(freeSquares);
			}
		}


		bitboard dest = kingAttacksFrom(bKingSquare) & (~occupied | wPieces);

		while(dest) {
			int square = Bitboard::firstSet(dest);
			dest^=SQUARE(square);

			if(!(attacksTo(square) & wPieces)) {
				moveStack[numMoves++] = (KING << 12) | (bKingSquare << 6) | square | (pieces[square] << 16);

			}

		}
	}

}

void Board::generateMoves() {

	generateCaptures();
	generateNonCaptures();
}

/*
 * First generators for white peices
 * Seperated into normal moves/captures
 * (by changing the mask to empty squares/black peices)
 *
 * Pawn moves/Pawn promotions/Pawn captures
 * Seperated in order to give priority to captures/promotions in search
 */

void Board::whitePieceMoves(bitboard mask) {

	bitboard dest;
	int square;
	int move;

	bitboard b = wKnights;
	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);

		move = (KNIGHT << 12) | (square << 6);

		dest = knightAttacksFrom(square) & mask;

		while(dest) {
			square = Bitboard::firstSet(dest);
			dest^=SQUARE(square);

			moveStack[numMoves++] = move | square;
		}
	}

	b = wBishops;
	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);

		move = (BISHOP << 12) | (square << 6);

		dest = bishopAttacksFrom(square) & mask;

		while(dest) {
			square = Bitboard::firstSet(dest);
			dest^=SQUARE(square);

			moveStack[numMoves++] = move | square;
		}
	}

	b = wRooks;
	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);

		move = (ROOK << 12) | (square << 6);
		dest = rookAttacksFrom(square) & mask;

		while(dest) {
			square = Bitboard::firstSet(dest);
			dest^=SQUARE(square);

			moveStack[numMoves++] = move | square;
		}
	}


	b = wQueens;

	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);

		move = (QUEEN << 12) | (square << 6);
		dest = queenAttacksFrom(square) & mask;

		while(dest) {
			square = Bitboard::firstSet(dest);
			dest^=SQUARE(square);

			moveStack[numMoves++] = move | square;
		}
	}
}


void Board::whiteKingMoves(bitboard mask) {
		bitboard dest = kingAttacksFrom(wKingSquare) & mask;

		while(dest) {
			int square = Bitboard::firstSet(dest);
			dest^=SQUARE(square);

			moveStack[numMoves++] = (KING << 12) | (wKingSquare << 6) | square;
		}
}

void Board::whitePieceCaptures(bitboard mask) {

	bitboard dest;
	int square;
	int move;

	bitboard b = wKnights;
	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);

		move = (KNIGHT << 12) | (square << 6);

		dest = knightAttacksFrom(square) & mask;

		while(dest) {
			square = Bitboard::firstSet(dest);
			dest^=SQUARE(square);

			moveStack[numMoves++] = move | square | (pieces[square] << 16);
		}
	}

	b = wBishops;
	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);

		move = (BISHOP << 12) | (square << 6);

		dest = bishopAttacksFrom(square) & mask;

		while(dest) {
			square = Bitboard::firstSet(dest);
			dest^=SQUARE(square);

			moveStack[numMoves++] = move | square  | (pieces[square] << 16);
		}
	}

	b = wRooks;
	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);

		move = (ROOK << 12) | (square << 6);
		dest = rookAttacksFrom(square) & mask;

		while(dest) {
			square = Bitboard::firstSet(dest);
			dest^=SQUARE(square);

			moveStack[numMoves++] = move | square  | (pieces[square] << 16);
		}
	}


	b = wQueens;

	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);

		move = (QUEEN << 12) | (square << 6);
		dest = queenAttacksFrom(square) & mask;

		while(dest) {
			square = Bitboard::firstSet(dest);
			dest^=SQUARE(square);

			moveStack[numMoves++] = move | square  | (pieces[square] << 16);
		}
	}
}

void Board::whiteKingCaptures(bitboard mask) {
		bitboard dest = kingAttacksFrom(wKingSquare) & mask;

		while(dest) {
			int square = Bitboard::firstSet(dest);
			dest^=SQUARE(square);

			moveStack[numMoves++] =  (KING << 12) | (wKingSquare << 6) | square | (pieces[square] << 16);
		}
}

void Board::whitePawnMoves(bitboard mask) {
	int square;


	//All but promotions, included in captures for higher priority ordering
	bitboard moveOne = (wPawns >> 8) & mask & Bitboard::inverseRanks[RANK_8];
	bitboard moveTwo = ((moveOne & Bitboard::ranks[RANK_3]) >> 8) & mask;

	while(moveOne) {
		square = Bitboard::firstSet(moveOne);
		moveOne^=SQUARE(square);

		moveStack[numMoves++] = (PAWN << 12) | ((square+8) << 6) | square;

	}

	while(moveTwo) {
		square = Bitboard::firstSet(moveTwo);
		moveTwo^=SQUARE(square);

		moveStack[numMoves++] = (PAWN << 12) | ((square+16) << 6) | square;

	}
}


void Board::whitePawnPromotions(bitboard mask) {
	int square;

	bitboard moveOne = (wPawns >> 8) & mask & Bitboard::ranks[RANK_8];

	while(moveOne) {
		square = Bitboard::firstSet(moveOne);
		moveOne^=SQUARE(square);

		moveStack[numMoves++] = (QUEEN << 20) 	| (PAWN << 12) | ((square+8) << 6) | square;
		moveStack[numMoves++] = (KNIGHT << 20)	| (PAWN << 12) | ((square+8) << 6) | square;
		moveStack[numMoves++] = (ROOK   << 20)	| (PAWN << 12) | ((square+8) << 6) | square;
		moveStack[numMoves++] = (BISHOP << 20)	| (PAWN << 12) | ((square+8) << 6) | square;

	}
}

void Board::whitePawnCaptures(bitboard mask) {
	int square;


	bitboard captures = ((wPawns & Bitboard::inverseFiles[FILE_A]) >> 9) & mask;
	while(captures) {
		square = Bitboard::firstSet(captures);
		captures^=SQUARE(square);

		int from = square+9;
		int capture = (pieces[square]? pieces[square] : PAWN)  << 16;

		if(RANK(square) == RANK_8) {
			moveStack[numMoves++] = capture | (QUEEN << 20) 	| (PAWN << 12) | (from << 6) | square;
			moveStack[numMoves++] = capture | (KNIGHT << 20) 	| (PAWN << 12) | (from << 6) | square;
			moveStack[numMoves++] = capture | (ROOK   << 20) 	| (PAWN << 12) | (from << 6) | square;
			moveStack[numMoves++] = capture | (BISHOP << 20) 	| (PAWN << 12) | (from << 6) | square;
		} else {
			moveStack[numMoves++] = capture | (PAWN << 12) | (from << 6) | square;
		}
	}



	captures= ((wPawns & Bitboard::inverseFiles[FILE_H]) >> 7) & mask;
	while(captures) {

		square = Bitboard::firstSet(captures);
		captures^=SQUARE(square);
		int from = square+7;
		int capture = (pieces[square]? pieces[square] : PAWN)  << 16;

		if(RANK(square) == RANK_8) {
			moveStack[numMoves++] = capture | (QUEEN << 20) 	| (PAWN << 12) | (from << 6) | square;
			moveStack[numMoves++] = capture | (KNIGHT << 20) 	| (PAWN << 12) | (from << 6) | square;
			moveStack[numMoves++] = capture | (ROOK   << 20) 	| (PAWN << 12) | (from << 6) | square;
			moveStack[numMoves++] = capture | (BISHOP << 20) 	| (PAWN << 12) | (from << 6) | square;
		} else {
			moveStack[numMoves++] = capture | (PAWN << 12) | (from << 6) | square;
		}
	}
}


const int W_CASTLE_KINGSIDE = (KING << 12) | (E1 << 6) | G1;
const int W_CASTLE_QUEENSIDE = (KING << 12) | (E1 << 6) | C1;

//Castling moves
void Board::whiteCastlingMoves() {

	//Don't need to check E1 or else we'd be in check anyway..
	if((currentState->castling & W_KINGSIDE) && !(Bitboard::bitsTo[E1][G1] & occupied)) {
		if(!((attacksTo(F1) | attacksTo(G1)) & bPieces))
			moveStack[numMoves++] = W_CASTLE_KINGSIDE;
	}

	if((currentState->castling & W_QUEENSIDE) && !(Bitboard::bitsTo[E1][B1] & occupied)) {
		if(!((attacksTo(D1) | attacksTo(C1)) & bPieces))
			moveStack[numMoves++] = W_CASTLE_QUEENSIDE;
	}
}

/*
 *  Move generators for black peices are symetrical.. hopefully :)
 *  Copy/paste/fix up of white move generators so it better be.
 */


void Board::blackPieceMoves(bitboard mask) {

	bitboard dest;
	int square;
	int move;

	bitboard b = bKnights;
	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);

		move = (KNIGHT << 12) | (square << 6);

		dest = knightAttacksFrom(square) & mask;

		while(dest) {
			square = Bitboard::firstSet(dest);
			dest^=SQUARE(square);

			moveStack[numMoves++] = move | square;
		}
	}

	b = bBishops;
	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);

		move = (BISHOP << 12) | (square << 6);

		dest = bishopAttacksFrom(square) & mask;

		while(dest) {
			square = Bitboard::firstSet(dest);
			dest^=SQUARE(square);

			moveStack[numMoves++] = move | square;
		}
	}

	b = bRooks;
	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);

		move = (ROOK << 12) | (square << 6);
		dest = rookAttacksFrom(square) & mask;

		while(dest) {
			square = Bitboard::firstSet(dest);
			dest^=SQUARE(square);

			moveStack[numMoves++] = move | square;
		}
	}


	b = bQueens;

	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);

		move = (QUEEN << 12) | (square << 6);
		dest = queenAttacksFrom(square) & mask;

		while(dest) {
			square = Bitboard::firstSet(dest);
			dest^=SQUARE(square);

			moveStack[numMoves++] = move | square;
		}
	}
}

void Board::blackKingMoves(bitboard mask) {
		bitboard dest = kingAttacksFrom(bKingSquare) & mask;

		while(dest) {
			int square = Bitboard::firstSet(dest);
			dest^=SQUARE(square);

			moveStack[numMoves++] = (KING << 12) | (bKingSquare << 6) | square;

		}
}

void Board::blackPieceCaptures(bitboard mask) {

	bitboard dest;
	int square;
	int move;

	bitboard b = bKnights;
	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);

		move =  (KNIGHT << 12) | (square << 6);

		dest = knightAttacksFrom(square) & mask;

		while(dest) {
			square = Bitboard::firstSet(dest);
			dest^=SQUARE(square);

			moveStack[numMoves++] = move | square | (pieces[square] << 16);
		}
	}

	b = bBishops;
	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);

		move = (BISHOP << 12) | (square << 6);

		dest = bishopAttacksFrom(square) & mask;

		while(dest) {
			square = Bitboard::firstSet(dest);
			dest^=SQUARE(square);

			moveStack[numMoves++] = move | square | (pieces[square] << 16);
		}
	}

	b = bRooks;
	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);

		move = (ROOK << 12) | (square << 6);
		dest = rookAttacksFrom(square) & mask;

		while(dest) {
			square = Bitboard::firstSet(dest);
			dest^=SQUARE(square);

			moveStack[numMoves++] = move | square | (pieces[square] << 16);
		}
	}


	b = bQueens;

	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);

		move = (QUEEN << 12) | (square << 6);
		dest = queenAttacksFrom(square) & mask;

		while(dest) {
			square = Bitboard::firstSet(dest);
			dest^=SQUARE(square);

			moveStack[numMoves++] = move | square | (pieces[square] << 16);
		}
	}
}

void Board::blackKingCaptures(bitboard mask) {
		bitboard dest = kingAttacksFrom(bKingSquare) & mask;

		while(dest) {
			int square = Bitboard::firstSet(dest);
			dest^=SQUARE(square);

			moveStack[numMoves++] = (KING << 12) | (bKingSquare << 6)  | square | (pieces[square] << 16);
		}
}


void Board::blackPawnMoves(bitboard mask) {
	int square;


	//All but promotions, included in captures for higher priority ordering
	bitboard moveOne = (bPawns << 8) & mask & Bitboard::inverseRanks[RANK_1];
	bitboard moveTwo = ((moveOne & Bitboard::ranks[RANK_6]) << 8) & mask;

	while(moveOne) {
		square = Bitboard::firstSet(moveOne);
		moveOne^=SQUARE(square);

		moveStack[numMoves++] = (PAWN << 12) | ((square-8) << 6) | square;

	}

	while(moveTwo) {
		square = Bitboard::firstSet(moveTwo);
		moveTwo^=SQUARE(square);

		moveStack[numMoves++] = (PAWN << 12) | ((square-16) << 6) | square;

	}
}


void Board::blackPawnPromotions(bitboard mask) {
	int square;

	bitboard moveOne = (bPawns << 8) & mask & Bitboard::ranks[RANK_1];

	while(moveOne) {
		square = Bitboard::firstSet(moveOne);
		moveOne^=SQUARE(square);

		moveStack[numMoves++] = (QUEEN << 20) 	| (PAWN << 12) | ((square-8) << 6) | square;
		moveStack[numMoves++] = (KNIGHT << 20) 	| (PAWN << 12) | ((square-8) << 6) | square;
		moveStack[numMoves++] = (ROOK   << 20) 	| (PAWN << 12) | ((square-8) << 6) | square;
		moveStack[numMoves++] = (BISHOP << 20) 	| (PAWN << 12) | ((square-8) << 6) | square;
	}
}

void Board::blackPawnCaptures(bitboard mask) {
	int square;

	bitboard captures = ((bPawns & Bitboard::inverseFiles[FILE_H]) << 9) & mask;
	while(captures) {
		square = Bitboard::firstSet(captures);
		captures^=SQUARE(square);
		int from = square-9;
		int capture = (pieces[square]? pieces[square]  : PAWN)  << 16;

		if(RANK(square) == RANK_1) {
			moveStack[numMoves++] = capture | (QUEEN << 20) 	| (PAWN << 12) | (from << 6) | square;
			moveStack[numMoves++] = capture | (KNIGHT << 20) 	| (PAWN << 12) | (from << 6) | square;
			moveStack[numMoves++] = capture | (ROOK   << 20) 	| (PAWN << 12) | (from << 6) | square;
			moveStack[numMoves++] = capture | (BISHOP << 20) 	| (PAWN << 12) | (from << 6) | square;
		} else {
			moveStack[numMoves++] = capture | (PAWN << 12) | (from << 6) | square;
		}
	}

	captures= ((bPawns & Bitboard::inverseFiles[FILE_A]) << 7) & mask;
	while(captures) {
		square = Bitboard::firstSet(captures);
		captures^=SQUARE(square);
		int from = square-7;
		int capture = (pieces[square]? pieces[square] : PAWN)  << 16;

		if(RANK(square) == RANK_1) {
			moveStack[numMoves++] = capture | (QUEEN << 20) 	| (PAWN << 12) | (from << 6) | square;
			moveStack[numMoves++] = capture | (KNIGHT << 20) 	| (PAWN << 12) | (from << 6) | square;
			moveStack[numMoves++] = capture | (ROOK   << 20) 	| (PAWN << 12) | (from << 6) | square;
			moveStack[numMoves++] = capture | (BISHOP << 20) 	| (PAWN << 12) | (from << 6) | square;
		} else {
			moveStack[numMoves++] = capture | (PAWN << 12) | (from << 6) | square;
		}
	}
}


const int B_CASTLE_KINGSIDE = (KING << 12) | (E8 << 6) | G8;
const int B_CASTLE_QUEENSIDE = (KING << 12) | (E8 << 6) | C8;

//Castling moves
void Board::blackCastlingMoves() {

	//Don't need to check E8 or else we'd be in check anyway..
	if((currentState->castling & B_KINGSIDE) && !(Bitboard::bitsTo[E8][G8] & occupied)) {
		if(!((attacksTo(F8) | attacksTo(G8)) & wPieces))
			moveStack[numMoves++] = B_CASTLE_KINGSIDE;
	}

	if((currentState->castling & B_QUEENSIDE) && !(Bitboard::bitsTo[E8][B8] & occupied)) {
		if(!((attacksTo(D8) | attacksTo(C8)) & wPieces))
			moveStack[numMoves++] = B_CASTLE_QUEENSIDE;
	}
}


