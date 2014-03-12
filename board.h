
#ifndef BOARD_H
#define BOARD_H 1


#include "game.h"
#include "bitboard.h"
#include "utility.h"
#include "transposition.h"
#include "const.h"



struct BoardState {
	int enPassant;	//Square of enPassant target
	int castling; //Castling flags
	int halfMoveCount; //Moves since a piece capture or pawn advance for 50 move draw
	bitboard hashKey;
	int firstMove;

	int move;
};


class Board {
	public:
	//Not sure of max number of moves, need to check
	/* Stored as 3 bytes - 24 bits,   6 bits from square,  6 bits to square, 4 bits piece, 4 bits piece attacked 4 bits promotion piece */
	int moveStack[MAX_MOVES];
	int numMoves;


	BoardState history[MAX_DEPTH];
	BoardState *stateStack; //Pointer into the history array for the current position
	BoardState *currentState;

	int depth;	//Place on the state stack

	int sortValues[256];

	int sideToMove;  //0=white, 1=black to move
	int moveNumber;		   //Move number in the game in half moves

	int whiteMaterial;
	int blackMaterial;

	int wKingSquare;
	int bKingSquare;

	int whitePieces;
	int blackPieces;

	int nodesPerUpdate;
	int nodesUntilUpdate;

	long long unsigned int nodes;
	long long unsigned int hashHits;

	int maxDepth;

#ifdef EXTRA_STATS
	long long unsigned int cutoffs;
	long long unsigned int firstCutoffs;
#endif


	//Peices
	char pieces[64];	//Array for lookup of piece at a particular square

	bitboard wPawns, bPawns;
	bitboard wKnights, bKnights;
	bitboard wBishops, bBishops;
	bitboard wRooks, bRooks;
	bitboard wQueens, bQueens;
	bitboard wKing, bKing;

	//Rotated occupied boards
	bitboard occupied, occupiedLeft90, occupiedLeft45, occupiedRight45;
	bitboard wPieces, bPieces;

	//Rooks and queens, bishops and queens
	bitboard rookMovers, bishopMovers;

	int returnMove;
	int currentBest;

	Transposition *hash;
	Game *game;



	int abortingSearch;

	void regenerate();

	Board(int hashSize, Game *game);
	Board();
	~Board();

	void reset();
	void setDefault();
	int setFen(char *fen);
	void print();		//Print the board out
	void printMoves();	//Print a trace of the current line being searched
	int isMessedUp(); 	//Check to see if the bitboards/arrays are messed up
	void printLine();	//Print out the results of a search and the PV from the hashtable
	void printHashLine();
	int getHashMove();



	void makeMove(int i);
	void retractMove(int i);

	void makeRealMove(int i);
	void retractRealMove();

	void makeNullMove();
	void retractNullMove();


	int testLegality(int i);
	int parseMove(char *move);

	inline void movePiece(int type, int source, int dest, int promote);
	inline void capturePiece(int type, int source, int dest, int capture, int promote);

	inline void unMovePiece(int type, int source, int dest, int promote);
	inline void unCapturePiece(int type, int source, int dest, int capture, int promote);

	inline void whiteRookMove(int source);
	inline void blackRookMove(int source);

	int inCheck();
	int isLegal();
	int isRepetition();

	int evaluate();
	int evaluatePawns();

	int evaluateWPieces();
	int evaluateBPieces();

	int evaluateWKingSafety();
	int evaluateBKingSafety();

	int saveLine();

	int alphaBeta(int depth, int alpha, int beta, int doNullMove);
	int search(int nodesPerUpdate);
	int quiescence(int alpha, int beta);

	int nullRisk();


	void printMove(int i);
	void printScore(int i);

	inline void whitePieceMoves(bitboard mask);
	inline void whitePieceCaptures(bitboard mask);

	inline void whiteKingMoves(bitboard mask);
	inline void whiteKingCaptures(bitboard mask);

	inline void whitePawnMoves(bitboard mask);
	inline void whitePawnCaptures(bitboard mask);
	inline void whitePawnPromotions(bitboard mask);
	inline void whiteCastlingMoves();


	inline void blackPieceMoves(bitboard mask);
	inline void blackPieceCaptures(bitboard mask);

	inline void blackKingMoves(bitboard mask);
	inline void blackKingCaptures(bitboard mask);

	inline void blackPawnMoves(bitboard mask);
	inline void blackPawnCaptures(bitboard mask);
	inline void blackPawnPromotions(bitboard mask);
	inline void blackCastlingMoves();

	void generateCaptures();
	void generateNonCaptures();
	void generateMoves();
	void generateCheckEvasions();

 	void putPiece(int piece, bitboard &b, int square);

	bitboard calculateHash();

	void makeCopy(Board *b);

	int evaluateExchange(int source, int dest, int promotion);
	bitboard removeAttacker(bitboard attacks, int direction, int moved, int attacked);
	int pinnedTo(int piece1, int piece2);

	void sortGoodCaptures(int firstMove, int hashMove);

	void sortCaptures(int firstMove, int hashMove);
	void sortNonCaptures(int firstMove, int hashMove);
	
	inline void hashEnPassant(int square) {
		stateStack[depth].hashKey ^= Transposition::enPassantMasks[square];
	}

	inline void hashWPiece(int piece, int square) {
		stateStack[depth].hashKey ^= Transposition::hashMasksW[piece][square];
	}

	inline void hashBPiece(int piece, int square) {
		stateStack[depth].hashKey ^= Transposition::hashMasksB[piece][square];
	}


	inline void hashCastle(int castle) {
		stateStack[depth].hashKey ^= Transposition::castleMasks[castle];
	}

	inline void hashChangeSide() {
		stateStack[depth].hashKey = ~stateStack[depth].hashKey;
	}
	
	/* To find the attacks for each particular file/rank/diagonal
 * we need to identify the squares occupied along the particular
 * direction - to do this we lookup the rotated occupied boards.
 *
 * After we've extracted the occupied bits along the direction of the
 * attack. We lookup the precalculated table for attacks for that square
 *  and occupation
 */

	inline bitboard h1a8Attacks(int square) {
		return Bitboard::h1a8Attacks[square][(occupiedLeft45 >> diagonalStartsLeft45[square]) & 0xFF];
	}

	inline bitboard a1h8Attacks(int square) {
		return 	Bitboard::a1h8Attacks[square][(occupiedRight45 >> diagonalStartsRight45[square]) & 0xFF];
	}


	inline bitboard rankAttacks(int square) {
		return Bitboard:: rankAttacks[square][(occupied >> (square & 0xF8)) & 0xFF];
	}

	inline bitboard fileAttacks(int square) {

		return Bitboard::fileAttacks[square][(occupiedLeft90 >> (FILE(square) << 3)) & 0xFF];
	}



/*
 * Attacks from for each type of peice. Pawns it's more useful to use operations on
 * entire groups rather than one at a time - so this might not be of much use.
 * Return the bitboard for attacks of a piece of that type at that square.
 */
	inline bitboard wPawnAttacksFrom(int square) {
		return  Bitboard::wPawnAttacks[square];
	}

	inline bitboard bPawnAttacksFrom(int square) {
		return Bitboard::bPawnAttacks[square];
	}

	inline bitboard knightAttacksFrom(int square) {
		return Bitboard::knightAttacks[square];
	}

	inline bitboard bishopAttacksFrom(int square) {
		return h1a8Attacks(square) | a1h8Attacks(square);
	}

	inline bitboard rookAttacksFrom(int square) {
		return rankAttacks(square) | fileAttacks(square);
	}

	inline bitboard queenAttacksFrom(int square) {
		return  h1a8Attacks(square) | a1h8Attacks(square) | rankAttacks(square) | fileAttacks(square);
	}

	inline bitboard kingAttacksFrom(int square) {
		return Bitboard::kingAttacks[square];
	}


/* Generate attacks to a specific square
 * Doesn't handle enPassant - to consider enpassant take
 * the maximum of the (pawn square, the enpassant capture square + pawn)
 */

	inline bitboard attacksTo(int square) {
		bitboard attacksTo;

		attacksTo= wPawnAttacksFrom(square) & bPawns;
		attacksTo|= bPawnAttacksFrom(square) & wPawns;
		attacksTo|= knightAttacksFrom(square) & (wKnights | bKnights);

		//Find all attacked diagonals for the given square & all pieces in those diagonals who can move (attack) that square
		attacksTo|= bishopAttacksFrom(square) & bishopMovers;

		//Same for rooks
		attacksTo|= rookAttacksFrom(square) & rookMovers;

		//Attack of the king!
		attacksTo|= kingAttacksFrom(square) & (wKing | bKing);

		return attacksTo;
	}


};

#endif

