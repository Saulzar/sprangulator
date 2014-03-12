
/* Method bodies for all functions which operate on boards - move generation and move making are in seperate files due to size */

#include "board.h"
#include "bitboard.h"
#include "transposition.h"
#include <string.h>
#include <stdlib.h>

/* Simply included here as the compiler seems to have problems with inline functions in other files */

#include "generate.cpp"
#include "move.cpp"
#include "eval.cpp"
#include "exchange.cpp"
#include "search.cpp"
#include "sort.cpp"

Board::Board(int hashSize, Game *game) {
	this->game = game;
	hash = new Transposition(hashSize, this);

	reset();
}

Board::Board() {
	game = NULL;
	hash = NULL;

	reset();

}

Board::~Board() {
	if(hash!=NULL) delete hash;
}

void Board::reset() {
	wPawns=0;
	wKnights=0;
	wBishops=0;
	wRooks=0;
	wQueens=0;
	wKing=0;

	bPawns=0;
	bKnights=0;
	bBishops=0;
	bRooks=0;
	bQueens=0;
	bKing=0;

	occupied=0;
	occupiedLeft90=0;
	occupiedLeft45=0;
	occupiedRight45=0;
	moveNumber = 0;

	sideToMove = 0;
	depth=0;

	stateStack = history;
	currentState = stateStack;

	stateStack[0].castling=0;
	stateStack[0].halfMoveCount=0;
	//Can safely use square 0 to test for validity since 0 cannot be an enPassant square.
	stateStack[0].enPassant = 0;
	stateStack[0].hashKey = 0;
	stateStack[0].firstMove = 0;
	stateStack[0].move = 0;
	numMoves = 0;

	whiteMaterial = 0;
	blackMaterial = 0;

	whitePieces = 0;
	blackPieces = 0;

	wKingSquare = 0;
	bKingSquare = 0;

	maxDepth = 256;

	if(hash!=NULL) hash->clear();

	memset(pieces, 0, sizeof(char) * 64);
}

void Board::setDefault() {
	setFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}


int Board::setFen(char *fen)
{
		reset();

        while(*fen == ' ') fen++;	//Read off leading spaces
		int file = 0;
		int rank = 0;

		int square=0;

		//Read pieces in
		while ((square = rank*8 + file) < 64) {	//While within bounds..  (file < 9) since we may be waiting for / char

                switch (*fen) {

                case 'K': putPiece(KING, wKing, square);			wKingSquare  = square;	break;
                case 'Q': putPiece(QUEEN, wQueens, square);		whiteMaterial+=QUEEN_VAL; whitePieces++;	break;
                case 'R': putPiece(ROOK, wRooks, square);	;		whiteMaterial+=ROOK_VAL;	 whitePieces++;	break;
                case 'B': putPiece(BISHOP, wBishops, square);		whiteMaterial+=BISHOP_VAL; whitePieces++;	break;
                case 'N': putPiece(KNIGHT, wKnights, square);		whiteMaterial+=KNIGHT_VAL; whitePieces++;	break;
                case 'P': putPiece(PAWN, wPawns, square);	;		whiteMaterial+=PAWN_VAL;	break;

		case 'k': putPiece(KING, bKing, square)	;			bKingSquare = square;	break;
                case 'q': putPiece(QUEEN, bQueens, square);	;	blackMaterial+=QUEEN_VAL; blackPieces++;	break;
                case 'r':  putPiece(ROOK, bRooks, square);			blackMaterial+=ROOK_VAL; blackPieces++;	break;
		case 'b': putPiece(BISHOP, bBishops, square);		blackMaterial+=BISHOP_VAL; blackPieces++; break;
                case 'n': putPiece(KNIGHT, bKnights, square);		blackMaterial+=KNIGHT_VAL; blackPieces++; break;
                case 'p': putPiece(PAWN, bPawns, square);			blackMaterial+=PAWN_VAL;	break;

                case '/': rank++; file = 0; fen++; continue;
                case '1': case '2':
				case '3': case '4':
                case '5': case '6':
				case '7': case '8':
                        file+=((*fen - '0')-1);
                        break;
                default:
					return -1;
					break;
                }
                fen++;
		file++;
        }
		while(*fen == ' ') fen++;	//Space



		if(*fen=='b') sideToMove=MOVE_BLACK;
			else sideToMove=MOVE_WHITE;


		fen++;
		while(*fen == ' ') fen++;	//Space



		if(*fen!='-') {
			int castle = 1;

			while(castle && *fen!=0) {

				switch(*fen) {
				case 'K':  stateStack[0].castling |= W_KINGSIDE; 		break;
				case 'Q': stateStack[0].castling |= W_QUEENSIDE; 	break;
				case 'k':  stateStack[0].castling |= B_KINGSIDE; 		break;
				case 'q':  stateStack[0].castling |= B_QUEENSIDE;		 break;
				default:
					castle = 0;
				break;
				}
				fen++;
			}
		} else {
			fen++;
		}

		while(*fen == ' ') fen++;	//Space

		if(*fen!='-') {
			int epFile = (*fen - 'a');
			int epRank = 0;

			if(epFile < 8 && epFile >= 0) {
				if(sideToMove==MOVE_WHITE) epRank = RANK_6;
					else epRank = RANK_3;
			}

			if(epRank > 0)  stateStack[0].enPassant = (epRank * 8 + epFile);
			else  stateStack[0].enPassant = 0;
		}

		fen++;
		while(*fen == ' ') fen++;	//Space

		int halfMoves = atoi(fen);
		if(halfMoves > 0) stateStack[0].halfMoveCount = halfMoves;

		while(*fen!=0 && *fen != ' ') fen++; //Find the end of the number
		fen++; //space

		int fullMoves = atoi(fen);
		if(fullMoves > 0) moveNumber = fullMoves;

		regenerate();
		return 0;
}

//Put piece on the board for setting up the board
void Board::putPiece(int piece, bitboard &b, int square) {
	pieces[square] = piece;
	b|= SQUARE(square);
}

void Board::print() {

	for(int i=0; i<64; i++) {
		char piece='-';
		char caseDiff = 'a' - 'A';

		switch(pieces[i]) {
			case KING:	piece = 'K';		break;
			case QUEEN:	piece = 'Q';		break;
			case ROOK:	piece = 'R';		break;
			case BISHOP:	piece = 'B';		break;
			case KNIGHT:	piece = 'N';		break;
			case PAWN:	piece = 'P';		break;
		}
		if(bPieces & SQUARE(i)) piece += caseDiff;
		cout  << "    " << piece;

		piece = '-';
		if(wPawns & SQUARE(i)) piece='P';
		if(wKnights & SQUARE(i)) piece='N';
		if(wBishops & SQUARE(i)) piece='B';
		if(wRooks & SQUARE(i)) piece='R';
		if(wQueens & SQUARE(i)) piece='Q';
		if(wKing & SQUARE(i)) piece='K';

		cout  << " " << piece;

		piece = '-';


		if(bPawns & SQUARE(i)) piece='p';
		if(bKnights & SQUARE(i)) piece='n';
		if(bBishops & SQUARE(i)) piece='b';
		if(bRooks & SQUARE(i)) piece='r';
		if(bQueens & SQUARE(i)) piece='q';
		if(bKing & SQUARE(i)) piece='k';

		cout  << " " << piece;
		if(FILE(i) == 7) cout <<    " " << 8-RANK(i) << endl;
	}


	//cout << " a b c d e f g h" << endl;
	cout << "      a        b        c        d        e        f        g        h    " << endl;

	if(sideToMove== MOVE_WHITE) cout << "-White ";
		else cout << "-Black ";
	cout << "to move." << endl;

	if(currentState->enPassant) cout << "EnPassant square: "  << squareNames[currentState->enPassant] << endl;

}

//Move trace
void Board::printMoves() {
	int i;

	for(i=0; i<depth; i++) {
		printMove(stateStack[i].move);
		cout << " ";
	}

	cout << "(";

	printMove(stateStack[i+1].move);

	cout << ")" << endl;
}


//Debug function to figure out where our board is fscking up.
int Board::isMessedUp() {

	for(int i=0; i<64; i++) {
		char piece=0;
		char caseDiff = 'a' - 'A';

		switch(pieces[i]) {
			case KING:	piece = 'K';		break;
			case QUEEN:	piece = 'Q';		break;
			case ROOK:	piece = 'R';		break;
			case BISHOP:	piece = 'B';		break;
			case KNIGHT:	piece = 'N';		break;
			case PAWN:	piece = 'P';		break;
		}
		if(bPieces & SQUARE(i)) piece += caseDiff;
		char piece1 = piece;

		piece = 0;
		if(wPawns & SQUARE(i)) piece='P';
		if(wKnights & SQUARE(i)) piece='N';
		if(wBishops & SQUARE(i)) piece='B';
		if(wRooks & SQUARE(i)) piece='R';
		if(wQueens & SQUARE(i)) piece='Q';
		if(wKing & SQUARE(i)) piece='K';
		char piece2 = piece;


		piece = 0;

		if(bPawns & SQUARE(i)) piece='p';
		if(bKnights & SQUARE(i)) piece='n';
		if(bBishops & SQUARE(i)) piece='b';
		if(bRooks & SQUARE(i)) piece='r';
		if(bQueens & SQUARE(i)) piece='q';
		if(bKing & SQUARE(i)) piece='k';
		char piece3 = piece;

		if(piece2) {
			if(piece3) return 1;
			if(piece1!=piece2) return 1;
		}

		if(piece3) {
			if(piece2) return 1;
			if(piece1!=piece3) return 1;
		}
	}

		if(wKingSquare!=Bitboard::firstSet(wKing)) {
			cout << "Bad white king square! " << wKingSquare << endl;
			return 1;
		}


		if(bKingSquare!=Bitboard::firstSet(bKing)) {
			cout << "Bad black king square! " << wKingSquare << endl;
			return 1;
		}


	if(currentState->hashKey!=calculateHash()) {
		cout << "Hash key wrong" << endl;
		cout << currentState->hashKey << " " << calculateHash() << " " << ~currentState->hashKey << endl;
		cout << depth << endl;
		return 1;
	}

	return 0;
}

/* Regenerate nesescary bitboards from individual piece bitboards */
void Board::regenerate() {
	wPieces = wPawns | wKnights | wBishops | wRooks | wQueens | wKing;
	bPieces = bPawns | bKnights | bBishops | bRooks | bQueens | bKing;

	bishopMovers = wBishops | bBishops | wQueens | bQueens;
	rookMovers = wRooks | bRooks | wQueens | bQueens;

	occupied = wPieces | bPieces;

	occupiedLeft90 = Bitboard::mapToBoard(occupied, rotateLeft90);
	occupiedLeft45 = Bitboard::mapToBoard(occupied, rotateLeft45);
	occupiedRight45 = Bitboard::mapToBoard(occupied, rotateRight45);

	currentState->hashKey = calculateHash();
	//Bitboard::print(occupiedLeft45);
}




//Check to see if we're in check
int Board::inCheck() {

	if(sideToMove==MOVE_WHITE) {
		if(attacksTo(wKingSquare) & bPieces) return 1;
	} else {
		if(attacksTo(bKingSquare) & wPieces) return 1;
	}


	return 0;
}

//Check to see if we're hung our king
int Board::isLegal() {

	if(sideToMove==MOVE_WHITE) {
		if(attacksTo(bKingSquare) & wPieces) return 0;
	} else {
		if(attacksTo(wKingSquare) & bPieces) return 0;
	}

	return 1;
}

int Board::isRepetition() {
	if(currentState->halfMoveCount >= 100) return 1;

 	bitboard key = currentState->hashKey;

	int n = 0;

	for(int i=1; i < currentState->halfMoveCount; i++) {
		if(key == stateStack[depth - i].hashKey) {
			if(depth - i >= 0) return 1;
			else n++;
		}
	}

	return n >= 2;
}


void Board::printMove(int i) {

	char piece=0;

	//Extract bits from move

	int promotion = (i >> 20) & 15;
	int capture = (i >> 16) & 15;
	int type = (i >> 12) & 15;
	int source = (i >> 6) & 63;
	int dest = i & 63;

	bitboard pb = 0;

	//cout << "  " << (int)pieces[source] << endl;

//	cout << "  " << i << ": ";

	//Funny exceptions for castling
	if(type == KING && abs(source - dest) == 2) {
		if(dest==G1 || dest==G8) cout << "0-0";
		else cout << "0-0-0";

		return;
	}

	switch(type) {
		case QUEEN:
			piece='Q';
			pb = wQueens | bQueens;
		break;

		case ROOK:
			piece='R';
			pb = wRooks | bRooks;
		break;

		case BISHOP:
			piece='B';
			pb = wBishops | bBishops;

		break;

		case KNIGHT:
			piece='N';
			pb = wKnights| bKnights;

		break;

		case KING:
			piece='K';
			pb=0;
		break;

		case PAWN:
			piece = (char)('a' + FILE(source));
			if(currentState->enPassant) capture = capture  || (SQUARE(dest) & SQUARE(currentState->enPassant));
		break;

		default:
			piece = 0;
		break;
	}

	if(SQUARE(source) & wPieces) pb&=wPieces;
	else pb&=bPieces;

	if(capture || type!=PAWN) cout << piece;

	//Check to see if ambiguous
	if(Bitboard::countOnes(attacksTo(dest) & pb) > 1) {
		cout << squareNames[source];
	}


	if(capture) cout << "x";
	cout << squareNames[dest];

	switch(promotion) {
		case QUEEN:
			piece='Q';
		break;

		case ROOK:
			piece='R';
		break;

		case BISHOP:
			piece='B';
		break;

		case KNIGHT:
			piece='N';
		break;

		default:
			piece = 0;
		break;
	}

	if(piece > 0) cout << "=" << piece;

}

//Print score in decimal with Mate detection etc..

void Board::printScore(int i) {

	if(i > (MATE - MAX_DEPTH)) {
		cout << "Mate in " << (MATE - i) << " ";
	} else if(i < -(MATE - MAX_DEPTH)) {
		cout << "Mated in " << -(-MATE - i) << " ";
	} else {
		float score = (float)i/100.0;
		cout << score;
	}
}


/*
	Print out the principle variation from the hash table -
	Sometimes the first move won't be in there since it hasn't completed the root search when the search aborts.
*/


void Board::printLine() {
	HashEntry *e;
	int timeTaken = (game!=NULL) ? game->getTimeTaken() : 0;

	e = 	hash->probe(currentState->hashKey);

	if(e == NULL) {
		cout << "Bug - null PV hash move" << endl;
		return;
	}

	cout << "Score ";
	printScore(e->score);

	cout << ", Depth " << (int)e->depth;
	cout << ", Nodes " << (nodes/1000) << "k";
	cout << ", Hash hits " << (((float)hashHits/(float)nodes) * 100) << "%";

	int nps = (timeTaken > 0)? (int)((float)nodes/(float)timeTaken) : 0;
 	cout << ", Nps " << nps << "k; ";

#ifdef EXTRA_STATS

	cout << " First cutoff rate "  << (float)firstCutoffs/(float)cutoffs << " ";

#endif

	printHashLine();
}

void Board::printHashLine() {
	int moves[64];
	int num = 0;
	HashEntry *e;
	int stop = 0;

	e = 	hash->probe(currentState->hashKey);

	while(e!=NULL && e->bestMove != 0) {
		if(isRepetition()) break;

		for(int i=0; i<num; i++) {
			if(e->bestMove == moves[i]) stop = 1;
		}

		printMove(e->bestMove);
		cout << " ";

  		moves[num] = e->bestMove;

		makeMove(e->bestMove);

		e = 	hash->probe(currentState->hashKey);
		num++;
	}

	while(num > 0) {
		num--;
		retractMove(moves[num]);
	}
}

int Board::getHashMove() {
	HashEntry *e;
	e = 	hash->probe(currentState->hashKey);

	if(e == NULL) {
		return 0;
	}

	return e->bestMove;
}

//Calculate the hash key from the ground up for this board
bitboard Board::calculateHash() {
	bitboard hash = 0;
	int square;

	//White pieces
	bitboard b = wPawns;
	while(b) {
		square = Bitboard::firstSet(b);
//		cout <<  square << endl;
		b^=SQUARE(square);
		hash ^= Transposition::hashMasksW[PAWN][square];
	}

	b = wKnights;
	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);
		hash ^= Transposition::hashMasksW[KNIGHT][square];
	}

	b = wBishops;
	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);
		hash ^= Transposition::hashMasksW[BISHOP][square];
	}

	b = wRooks;
	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);
		hash ^= Transposition::hashMasksW[ROOK][square];
	}

	b = wQueens;
	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);
		hash ^= Transposition::hashMasksW[QUEEN][square];
	}

	b = wKing;
	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);
		hash ^= Transposition::hashMasksW[KING][square];
	}

	//Black pieces
	b = bPawns;
	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);
		hash ^= Transposition::hashMasksB[PAWN][square];
	}

	b = bKnights;
	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);
		hash ^= Transposition::hashMasksB[KNIGHT][square];
	}

	b = bBishops;
	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);
		hash ^= Transposition::hashMasksB[BISHOP][square];
	}

	b = bRooks;
	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);
		hash ^= Transposition::hashMasksB[ROOK][square];
	}

	b = bQueens;
	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);
		hash ^= Transposition::hashMasksB[QUEEN][square];
	}

	b = bKing;
	while(b) {
		square = Bitboard::firstSet(b);
		b^=SQUARE(square);
		hash ^= Transposition::hashMasksB[KING][square];
	}

	//1, 2, 4, 8 castling flags
	for(int i=1; i<16; i*=2) {
		if(currentState->castling & i) hash ^= Transposition::castleMasks[i];

	}


	if(sideToMove==MOVE_BLACK) hash = ~hash;

	//enPassant
	if(currentState->enPassant) hash ^= Transposition::enPassantMasks[currentState->enPassant];

	return hash;
}



void Board::makeCopy(Board *b) {
	int stackOffset = stateStack - history;

	memcpy(b, this, sizeof(Board));

	b->stateStack = b->history + stackOffset;
	b->currentState = b->stateStack;

	b->hash = NULL;
	b->game = NULL;
}

