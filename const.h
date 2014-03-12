
#ifndef CONST_H
#define CONST_H 1

// Piece enumeration - 4 bits
#define NO_PIECE 0
#define PAWN 1
#define KNIGHT 2
#define KING 3
#define BISHOP 4
#define ROOK 8
#define QUEEN 12

#define PAWN_VAL 100
#define KNIGHT_VAL 300
#define BISHOP_VAL 310
#define ROOK_VAL 500
#define QUEEN_VAL 920
#define KING_VAL	20000


#define ZUG_MATERIAL 1000
#define ZUG_PIECES 2

#define MODE_WAIT 0
#define MODE_FORCE 1
#define MODE_PLAY 2
#define MODE_PONDER 3
#define MODE_ANALYSE 4
#define MODE_TEST 5

#define MESSAGE_OPPONENT 	0
#define MESSAGE_OTHERS 	1
#define MESAGE_ALL 		2
#define MESSAGE_ERROR		3
#define MESSAGE_COMMAND 	4
#define MESSAGE_CONSOLE 	5

#define CONTINUE_SEARCH 	0
#define OUT_OF_TIME		1
#define STOP_SEARCH		2


 #define NO_RESULT 0
 #define BLACK_MATE 1
 #define WHITE_MATE 2
 #define DRAW_REPETITION 3
 #define DRAW_STALEMATE 4
 #define DRAW_50_MOVES 5

#define HASH_UNKNOWN 0
#define HASH_EXACT 1
#define HASH_LOWER 2
#define HASH_UPPER 3

#define HASH_NO_NULL 4


#define NO_NULL_MOVE 0
#define DO_NULL_MOVE 1

#define REP_SIZE 16384
#define REP_MASK (unsigned)16383


#define FINISHED 0
#define HASH_MOVE 1
#define NON_CAPTURES 2
#define ALL_MOVES 3


 const int pieceValues[] = { NO_PIECE, PAWN_VAL, KNIGHT_VAL, KING_VAL, BISHOP_VAL,
   NO_PIECE, NO_PIECE, NO_PIECE, ROOK_VAL, NO_PIECE, NO_PIECE, NO_PIECE, QUEEN_VAL }; 

// Flags for castling information
#define W_KINGSIDE		1
#define W_QUEENSIDE	2
#define B_KINGSIDE		4
#define B_QUEENSIDE	8

// Flags for this board position
#define MOVE_WHITE					0
#define MOVE_BLACK					1

#define MAX_MOVES 4096
#define MAX_DEPTH 512

#define MATE 32768
#define DRAW 0

//Specific move values for castling

extern const int W_CASTLE_KINGSIDE;
extern const int W_CASTLE_QUEENSIDE;
extern const int B_CASTLE_KINGSIDE;
extern const int B_CASTLE_QUEENSIDE;


#endif

