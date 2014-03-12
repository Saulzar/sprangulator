
/* Board functions relating to move making/unmaking */

#include "board.h"
#include "bitboard.h"
#include "transposition.h"
#include <string.h>
#include <stdlib.h>


//Returns 1 for legal 0 for no - test for move input etc..

int Board::testLegality(int i) {

	//Just generate all moves and check to see if one matches
	int firstMove = numMoves;

	generateMoves();

	for(int j=firstMove; j<numMoves; j++) {
		if(i == moveStack[j]) {
			numMoves = firstMove;
			makeMove(moveStack[j]);

			if(isLegal()) {
				retractMove(moveStack[j]);
				return 1;
			}

			retractMove(moveStack[j]);
			return 0;
		}
	}

	numMoves = firstMove;
	return 0;
}


int readSquare(char *square) {
	int file = (*square - 'a');
	int rank = 0;

	if(file < 8 && file >= 0) {
		rank = 8-(*(square+1) - '0');
		if(rank < 8 && rank >=0) return rank * 8 + file;
	}

	return -1;
}

int Board::parseMove(char *move) {
	int piece = 0;
	int source = 0;
	int dest = 0;
	int capture;
	int promotion = 0;
	int file;
	int returnMove = -1;

	bitboard pieceSet = 0;
	int length = strlen(move);

	if(length < 2) return -1;

	//White space
	while(*move==' ') move++;


	switch(*move) {
                case 'K': case 'k':
			piece = KING;

			if(sideToMove == MOVE_WHITE) pieceSet = wKing;
			else pieceSet = bKing;
		break;

                case 'Q': case 'q':
			piece = QUEEN;

			if(sideToMove == MOVE_WHITE) pieceSet = wQueens;
			else pieceSet = bQueens;
		break;
                case 'R': case 'r':
			piece = ROOK;

			if(sideToMove == MOVE_WHITE) pieceSet = wRooks;
			else pieceSet = bRooks;
		break;


                case 'N': case 'n':
			piece = KNIGHT;

			if(sideToMove == MOVE_WHITE) pieceSet = wKnights;
			else pieceSet = bKnights;
		break;



		case 'a':  case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h':
		case 'A': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H':
			piece = PAWN;

			if(sideToMove == MOVE_WHITE) pieceSet = wPawns;
			else pieceSet = bPawns;

			file = (*move - 'a');

			if(file < 8 && file >= 0) {
				pieceSet &= Bitboard::files[file];
			}


		if(file == FILE_B && !pieceSet) {
		case 'B':
				piece = BISHOP;

				if(sideToMove == MOVE_WHITE) pieceSet = wBishops;
				else pieceSet = bBishops;
		}
		break;

		case 'O':
		case 'o':
		case '0':
			if(strncmp(move, "0-0-0", 5)==0 || strncmp(move, "o-o-o", 5)==0 || strncmp(move, "O-O-O", 5)==0) {
				if(sideToMove == MOVE_WHITE) return W_CASTLE_QUEENSIDE;
					else return B_CASTLE_QUEENSIDE;
			}

			if(strncmp(move, "0-0", 3)==0 || strncmp(move, "o-o", 3)==0 || strncmp(move, "O-O", 3)==0) {
				if(sideToMove == MOVE_WHITE) return W_CASTLE_KINGSIDE;
					else return B_CASTLE_KINGSIDE;
			}

		default:
			return -1;
		break;
	}

	if(!pieceSet) return -1;

	int countsq = 0;
	int endsq = 0;

	for(int i=0; i<length-1; i++) {
		int square = readSquare(move + i);

		if(square > -1) 	{
			if(countsq == 1) {
				pieceSet &= SQUARE(dest);
			}

			dest = square;
			countsq++;

			endsq = i;
		}

		if(countsq >= 2) break;
	}

	if(countsq == 1 && piece!=PAWN) {
		for(int i=1; i<endsq; i++) {
			int square = readSquare(move + i);

			if(square == -1) {
				char qualifier =  *(move+i);

				int file = 	qualifier - 'a';
				int file2 =	qualifier - 'A';
				int rank = 	qualifier - '1';


				if(file < 8 && file >= 0) {
					pieceSet &= Bitboard::files[file];
				} else if(file2 < 8 && file2 >= 0) {
					pieceSet &= Bitboard::files[file2];
				} else if (rank < 8 && rank >= 0) {
					pieceSet &= Bitboard::ranks[7-rank];
				}

			}
		}
	}


	if(*(move+endsq + 2) == '=') {
		char promotePiece = *(move + endsq + 3);
		switch(promotePiece) {
                	case 'Q': case 'q':
				promotion = QUEEN;
			break;

                	case 'R': case 'r':
				promotion = ROOK;
			break;

                	case 'B': case 'b':
				promotion = BISHOP;


			break;

        	        case 'N': case 'n':
				promotion = KNIGHT;
			break;
		}
	}

	int countGood = 0;
	bitboard attacker = attacksTo(dest) & pieceSet;


	while(attacker) {
		source = Bitboard::firstSet(attacker);
		attacker ^= SQUARE(source);

		if(piece == PAWN && currentState->enPassant > 0 && dest == currentState->enPassant) capture = PAWN;
			else capture = pieces[dest];

		int candidateMove = dest | source << 6 | piece << 12 | capture << 16 | promotion << 20;

		if(testLegality(candidateMove)) {
			returnMove = candidateMove;
			countGood++;
		}
	}


	if(countGood==0 && piece!=PAWN) return -1;	//No move
	if(countGood > 1) return -2; //Ambiguous

	//Normal pawn moves
	if(piece == PAWN && countGood==0) {
		while(pieceSet) {
			source = Bitboard::firstSet(pieceSet);

			if((sideToMove==MOVE_WHITE && source > dest) || (sideToMove==MOVE_BLACK && source < dest))
				if(!(occupied & Bitboard::bitsTo[source][dest])) {
					returnMove = dest | source << 6 | piece << 12 | promotion << 20;

					if(testLegality(returnMove)) {
						countGood++;
					}
			}
			pieceSet ^= SQUARE(source);
		}
	}

	if(countGood == 1) return returnMove;

	return -1;
}

void Board::makeRealMove(int i) {

	numMoves = 0;
	makeMove(i);

	//We want the changes to actually stick..

	stateStack++;

	depth = 0;
	currentState = stateStack;
}

/* Make a null move, change some states but the board doesn't change */

void Board::makeNullMove() {
	currentState->move = 0;
 	depth++;
	currentState++;

	//Some operations are pointless for the null move search
	currentState->hashKey= stateStack[depth-1].hashKey;
	currentState->castling= stateStack[depth-1].castling;
	currentState->halfMoveCount = stateStack[depth-1].halfMoveCount + 1;
	currentState->enPassant = 0;
	currentState->firstMove = numMoves;

	if(sideToMove == MOVE_BLACK) moveNumber++;


	//No more enPassant possible so remove the hash mask for it
	if(stateStack[depth-1].enPassant) {
		hashEnPassant(stateStack[depth-1].enPassant);
	}

	//Nothing moves

	//Change turn
	sideToMove=!sideToMove;
	hashChangeSide();
}



/* Make a move, update all bitboards to match, no legality testing */

void Board::makeMove(int i) {
	int promotion = (i >> 20) & 15;
	int capture = (i >> 16) & 15;
	int type = (i >> 12) & 15;
	int source = (i >> 6) & 63;
	int dest = i & 63;

	currentState->move = i;

 	depth++;
	currentState++;

	currentState->hashKey= stateStack[depth-1].hashKey;
	currentState->castling= stateStack[depth-1].castling;
	currentState->halfMoveCount = stateStack[depth-1].halfMoveCount + 1;
	currentState->enPassant = 0;
	currentState->firstMove = numMoves;

	if(sideToMove == MOVE_BLACK) moveNumber++;


	//No more enPassant possible so remove the hash mask for it
	if(stateStack[depth-1].enPassant) {
		hashEnPassant(stateStack[depth-1].enPassant);
	}

	if(capture) {
		//Capture moves
		capturePiece(type, source, dest, capture, promotion);

		//cout << "Capture" << endl;
	} else {
		//Non capture
		movePiece(type, source, dest, promotion);
	}

	//Change turn
	sideToMove=!sideToMove;
	hashChangeSide();
}




inline void Board::movePiece(int type, int source, int dest, int promote) {

	//Xor with occupied masks so that it creates a 0 where the piece was and a 1 where the piece is headed
	bitboard bitMove = SQUARE(source) | SQUARE(dest);
	bitboard bitMoveLeft90 = Bitboard::squaresLeft90[source] | Bitboard::squaresLeft90[dest];
	bitboard bitMoveLeft45 = Bitboard::squaresLeft45[source] | Bitboard::squaresLeft45[dest];
	bitboard bitMoveRight45 = Bitboard::squaresRight45[source] | Bitboard::squaresRight45[dest];


	Bitboard::setClear(occupied,  bitMove);
	Bitboard::setClear(occupiedLeft45 , bitMoveLeft45);
	Bitboard::setClear(occupiedRight45 ,  bitMoveRight45);
	Bitboard::setClear(occupiedLeft90 ,  bitMoveLeft90);

	pieces[dest] = type;
	pieces[source] = 0;

	if(sideToMove == MOVE_WHITE) {

		hashWPiece(type, source);
		//Destination hash done per piece type to deal with promotions

		switch(type) {
			case QUEEN:
				Bitboard::setClear(wQueens, bitMove);
				Bitboard::setClear(rookMovers, bitMove);
				Bitboard::setClear(bishopMovers, bitMove);
				hashWPiece(QUEEN, dest);
			break;

			case ROOK:
				whiteRookMove(source);

				Bitboard::setClear(wRooks, bitMove);
				Bitboard::setClear(rookMovers, bitMove);
				hashWPiece(ROOK, dest);
			break;

			case BISHOP:
				Bitboard::setClear(wBishops, bitMove);
				Bitboard::setClear(bishopMovers, bitMove);
				hashWPiece(BISHOP, dest);
			break;

			case KNIGHT:
				Bitboard::setClear(wKnights, bitMove);
				hashWPiece(KNIGHT, dest);
			break;

			case KING:
				Bitboard::setClear(wKing, bitMove);
				hashWPiece(KING, dest);
				wKingSquare = dest;

				///cout << squareNames[source] << " " << squareNames[dest] << endl;

				//King moving 2 squares must be castling
				if(abs(dest - source) == 2) {
					//Kingside
					if(dest == G1) {
						movePiece(ROOK, H1, F1, 0);

						//Rook move will cover the kingside castling
						if(currentState->castling & W_QUEENSIDE) {
							currentState->castling &= ~W_QUEENSIDE;
							hashCastle(W_QUEENSIDE);
						}

					//Queenside
					} else {
						movePiece(ROOK, A1, D1, 0);

						//Rook move will cover the queenside castling
						if(currentState->castling & W_KINGSIDE) {
							currentState->castling &= ~W_KINGSIDE;
							hashCastle(W_KINGSIDE);
						}
					}
				} else { //Remove castling flags for normal king move..

					if(currentState->castling & W_QUEENSIDE) {
						currentState->castling &= ~W_QUEENSIDE;
						hashCastle(W_QUEENSIDE);
					}

					if(currentState->castling & W_KINGSIDE) {
						currentState->castling &= ~W_KINGSIDE;
						hashCastle(W_KINGSIDE);
					}
				}

			break;

			case PAWN:
				currentState->halfMoveCount = 0;
				
				if(promote) {
					pieces[dest] = promote;
					hashWPiece(promote, dest);
					whiteMaterial += (pieceValues[promote] - PAWN_VAL);
					whitePieces++;

					Bitboard::clearSquare(wPawns, source);

					switch (promote) {
						case QUEEN:
							Bitboard::setSquare(wQueens, dest);
							Bitboard::setSquare(rookMovers, dest);
							Bitboard::setSquare(bishopMovers, dest);
						break;

						case ROOK:
							Bitboard::setSquare(wRooks, dest);
							Bitboard::setSquare(rookMovers, dest);
						break;

						case BISHOP:
							Bitboard::setSquare(wBishops, dest);
							Bitboard::setSquare(bishopMovers, dest);
						break;

						case KNIGHT:
							Bitboard::setSquare(wKnights, dest);
						break;

					}
				} else {
					//Normal pawn move
					Bitboard::setClear(wPawns, bitMove);
					if((source - dest) == 16) {
						currentState->enPassant = dest + 8;
						hashEnPassant(dest + 8);

//						cout << squareNames[source] << " " << squareNames[dest] << endl;
					}

					hashWPiece(PAWN, dest);
				}

			break;
		}

		Bitboard::setClear(wPieces ,  bitMove);

	} else {												//if(move == MOVE_BLACK)

		hashBPiece(type, source);
		//Destination hash done per piece type to deal with promotions

		switch(type) {
			case QUEEN:
				Bitboard::setClear(bQueens, bitMove);
				Bitboard::setClear(rookMovers, bitMove);
				Bitboard::setClear(bishopMovers, bitMove);
				hashBPiece(QUEEN, dest);
			break;

			case ROOK:
				blackRookMove(source);

				Bitboard::setClear(bRooks, bitMove);
				Bitboard::setClear(rookMovers, bitMove);
				hashBPiece(ROOK, dest);
			break;

			case BISHOP:
				Bitboard::setClear(bBishops, bitMove);
				Bitboard::setClear(bishopMovers, bitMove);
				hashBPiece(BISHOP, dest);
			break;

			case KNIGHT:
				Bitboard::setClear(bKnights, bitMove);
				hashBPiece(KNIGHT, dest);
			break;

			case KING:

				Bitboard::setClear(bKing, bitMove);
				hashBPiece(KING, dest);
				bKingSquare = dest;

				//King moving 2 squares must be castling
				if(abs(dest - source) == 2) {
					//Kingside
					if(dest == G8) {
						movePiece(ROOK, H8, F8, 0);

						//Rook move will cover the kingside castling
						if(currentState->castling & B_QUEENSIDE) {
							currentState->castling &= ~B_QUEENSIDE;
							hashCastle(B_QUEENSIDE);
						}

					//Queenside
					} else {
						movePiece(ROOK, A8, D8, 0);

						//Rook move will cover the queenside castling
						if(currentState->castling & B_KINGSIDE) {
							currentState->castling &= ~B_KINGSIDE;
							hashCastle(B_KINGSIDE);
						}

					}
				} else {
					if(currentState->castling & B_QUEENSIDE) {
						currentState->castling &= ~B_QUEENSIDE;
						hashCastle(B_QUEENSIDE);
					}

					if(currentState->castling & B_KINGSIDE) {
						currentState->castling &= ~B_KINGSIDE;
						hashCastle(B_KINGSIDE);
					}
				}

			break;

			case PAWN:
				currentState->halfMoveCount = 0;

				if(promote) {
					pieces[dest] = promote;
					hashBPiece(promote, dest);

					blackMaterial += (pieceValues[promote] - PAWN_VAL);
					blackPieces++;

					Bitboard::clearSquare(bPawns, source);

					switch (promote) {
						case QUEEN:
							Bitboard::setSquare(bQueens, dest);
							Bitboard::setSquare(rookMovers, dest);
							Bitboard::setSquare(bishopMovers, dest);
						break;

						case ROOK:
							Bitboard::setSquare(bRooks, dest);
							Bitboard::setSquare(rookMovers, dest);
						break;

						case BISHOP:
							Bitboard::setSquare(bBishops, dest);
							Bitboard::setSquare(bishopMovers, dest);
						break;

						case KNIGHT:
							Bitboard::setSquare(bKnights, dest);
						break;

					}
				} else {
					//Normal pawn move
					Bitboard::setClear(bPawns, bitMove);
					if((dest - source) == 16) {
						currentState->enPassant = dest - 8;
						hashEnPassant(dest - 8);

//						cout << squareNames[source] << " " << squareNames[dest] << endl;
					}

					hashBPiece(PAWN, dest);
				}

			break;
		}

		Bitboard::setClear(bPieces ,  bitMove);
	} 	//MOVE_BLACK


		//makeMove()
}



inline void Board::capturePiece(int type, int source, int dest, int capture, int promote) {
	//Xor with occupied masks so that it creates a 0 where the piece was and a 1 where the piece is headed
	bitboard bitMove = SQUARE(source) | SQUARE(dest);

/*	bitboard bitMoveLeft90 = Bitboard::squaresLeft90[source] | Bitboard::squaresLeft90[dest];
	bitboard bitMoveLeft45 = Bitboard::squaresLeft45[source] | Bitboard::squaresLeft45[dest];
	bitboard bitMoveRight45 = Bitboard::squaresRight45[source] | Bitboard::squaresRight45[dest];*/

	/* Only need to clear the source for occupation - as there will be a piece on the destination square already -
	   except for enpassant
	*/

	currentState->halfMoveCount = 0;

	Bitboard::clearSquare(occupied,  source);
	Bitboard::clear(occupiedLeft45, Bitboard::squaresLeft45[source]);
	Bitboard::clear(occupiedRight45 ,  Bitboard::squaresRight45[source]);
	Bitboard::clear(occupiedLeft90 ,  Bitboard::squaresLeft90[source]);

	if(sideToMove == MOVE_WHITE) {

		/* Get rid of the piece being taken from specific bitboards - the
		   reason for this order is so that the rookMovers/BishopMovers
		   can be cleared without any additional if statements */
		switch (capture) {
			case QUEEN:
				Bitboard::clearSquare(bQueens, dest);

				Bitboard::clearSquare(rookMovers, dest);
				Bitboard::clearSquare(bishopMovers, dest);
				hashBPiece(QUEEN, dest);
				blackPieces--;
			break;

			case ROOK:
				blackRookMove(dest);

				Bitboard::clearSquare(bRooks, dest);
				Bitboard::clearSquare(rookMovers, dest);
				hashBPiece(ROOK, dest);
				blackPieces--;
			break;

			case BISHOP:
				Bitboard::clearSquare(bBishops, dest);
				Bitboard::clearSquare(bishopMovers, dest);
				hashBPiece(BISHOP, dest);
				blackPieces--;
			break;

			case KNIGHT:
				Bitboard::clearSquare(bKnights, dest);
				hashBPiece(KNIGHT, dest);
				blackPieces--;
			break;

			case PAWN:
				Bitboard::clearSquare(bPawns, dest);
				//cout << (int)pieces[dest] << endl;
				if(pieces[dest])  hashBPiece(PAWN, dest);  //Check is for not enPassant
			break;

			case KING:
				//Shouldn't happen, however it may require some extra error checking at some stage..
				//cerr << "Error: King was captured" << endl;
			break;
		}

		//Set pieces on board array
		pieces[source] = 0;
		pieces[dest] = type;

		hashWPiece(type, source);

		blackMaterial -= pieceValues[capture];

		switch(type) {
			case QUEEN:
				Bitboard::setClear(wQueens, bitMove);

				Bitboard::setClear(rookMovers, bitMove);
				Bitboard::setClear(bishopMovers, bitMove);

				hashWPiece(QUEEN, dest);
			break;

			case ROOK:
				whiteRookMove(source);

				Bitboard::setClear(wRooks, bitMove);
				Bitboard::setClear(rookMovers, bitMove);

				hashWPiece(ROOK, dest);
			break;

			case BISHOP:
				Bitboard::setClear(wBishops, bitMove);
				Bitboard::setClear(bishopMovers, bitMove);

				hashWPiece(BISHOP, dest);
			break;

			case KNIGHT:
				Bitboard::setClear(wKnights, bitMove);
				hashWPiece(KNIGHT, dest);
			break;

			case KING:
//cout << squareNames[source] << " " << squareNames[dest] << endl;
				wKingSquare = dest;

				//Cancel castling
				if(currentState->castling & W_KINGSIDE) {
					currentState->castling &= ~W_KINGSIDE;
					hashCastle(W_KINGSIDE);
				}

				if(currentState->castling & W_QUEENSIDE) {
					currentState->castling &= ~W_QUEENSIDE;
					hashCastle(W_QUEENSIDE);
				}

				Bitboard::setClear(wKing, bitMove);
				hashWPiece(KING, dest);

			break;

			case PAWN:

				if(promote) {
					pieces[dest] = promote;
					Bitboard::clearSquare(wPawns, source);
					hashWPiece(promote, dest);

					whiteMaterial += (pieceValues[promote] - PAWN_VAL);
					whitePieces++;

					switch (promote) {
						case QUEEN:
							Bitboard::setSquare(wQueens, dest);
							Bitboard::setSquare(rookMovers, dest);
							Bitboard::setSquare(bishopMovers, dest);
						break;

						case ROOK:
							Bitboard::setSquare(wRooks, dest);
							Bitboard::setSquare(rookMovers, dest);
						break;

						case BISHOP:
							Bitboard::setSquare(wBishops, dest);
							Bitboard::setSquare(bishopMovers, dest);
						break;

						case KNIGHT:
							Bitboard::setSquare(wKnights, dest);
						break;
					}
				} else {
					//Normal pawn capture
					Bitboard::setClear(wPawns, bitMove);

					//Enpassant if there's no piece on the destination square
					if(dest == stateStack[depth-1].enPassant) {
						//Put a piece on the destination pawn square
						int square = dest + 8;

						//Put a piece on the destination pawn square
						Bitboard::setSquare(occupied,  dest);
						Bitboard::set(occupiedLeft45, Bitboard::squaresLeft45[dest]);
						Bitboard::set(occupiedRight45 ,  Bitboard::squaresRight45[dest]);
						Bitboard::set(occupiedLeft90 ,  Bitboard::squaresLeft90[dest]);

						//Clear pawn off occupied boards..
						Bitboard::clearSquare(occupied,  square);
						Bitboard::clear(occupiedLeft45, Bitboard::squaresLeft45[square]);
						Bitboard::clear(occupiedRight45 ,  Bitboard::squaresRight45[square]);
						Bitboard::clear(occupiedLeft90 ,  Bitboard::squaresLeft90[square]);

						//Clear off pawn actually being taken for the correct square
						Bitboard::clearSquare(bPawns, square);
						Bitboard::clearSquare(bPieces, square);

						pieces[square] = 0;
						hashBPiece(PAWN, square);
					}

					hashWPiece(PAWN, dest);
				}

			break;
		}

		Bitboard::setClear(wPieces,  bitMove);
		//Remove the piece being taken
		Bitboard::clearSquare(bPieces, dest);

	}  else {  									//if(move == MOVE_BLACK)

		switch (capture) {
			case QUEEN:
				Bitboard::clearSquare(wQueens, dest);

				Bitboard::clearSquare(rookMovers, dest);
				Bitboard::clearSquare(bishopMovers, dest);
				hashWPiece(QUEEN, dest);
				whitePieces--;
			break;

			case ROOK:
				whiteRookMove(dest);

				Bitboard::clearSquare(wRooks, dest);
				Bitboard::clearSquare(rookMovers, dest);
				hashWPiece(ROOK, dest);
				whitePieces--;
			break;

			case BISHOP:
				Bitboard::clearSquare(wBishops, dest);
				Bitboard::clearSquare(bishopMovers, dest);
				hashWPiece(BISHOP, dest);
				whitePieces--;
			break;

			case KNIGHT:
				Bitboard::clearSquare(wKnights, dest);
				hashWPiece(KNIGHT, dest);
				whitePieces--;
			break;

			case PAWN:
				Bitboard::clearSquare(wPawns, dest);
				//cout << (int)pieces[dest] << endl;
				if(pieces[dest])  hashWPiece(PAWN, dest);  //Check is for not enPassant
			break;

			case KING:
				//Shouldn't happen, however it may require some extra error checking at some stage..
				//cerr << "Error: King was captured" << endl;
			break;
		}

		//Set pieces on board array
		pieces[source] = 0;
		pieces[dest] = type;

		hashBPiece(type, source);

		whiteMaterial -= pieceValues[capture];

		switch(type) {
			case QUEEN:
				Bitboard::setClear(bQueens, bitMove);

				Bitboard::setClear(rookMovers, bitMove);
				Bitboard::setClear(bishopMovers, bitMove);

				hashBPiece(QUEEN, dest);
			break;

			case ROOK:
				blackRookMove(source);

				Bitboard::setClear(bRooks, bitMove);
				Bitboard::setClear(rookMovers, bitMove);

				hashBPiece(ROOK, dest);
			break;

			case BISHOP:
				Bitboard::setClear(bBishops, bitMove);
				Bitboard::setClear(bishopMovers, bitMove);

				hashBPiece(BISHOP, dest);
			break;

			case KNIGHT:
				Bitboard::setClear(bKnights, bitMove);
				hashBPiece(KNIGHT, dest);
			break;

			case KING:
				bKingSquare = dest;

				//Cancel castling
				if(currentState->castling & B_KINGSIDE) {
					currentState->castling &= ~B_KINGSIDE;
					hashCastle(B_KINGSIDE);
				}

				if(currentState->castling & B_QUEENSIDE) {
					currentState->castling &= ~B_QUEENSIDE;
					hashCastle(B_QUEENSIDE);
				}

				Bitboard::setClear(bKing, bitMove);
				hashBPiece(KING, dest);

			break;

			case PAWN:

				if(promote) {
					pieces[dest] = promote;
					Bitboard::clearSquare(bPawns, source);
					hashBPiece(promote, dest);

					blackMaterial += (pieceValues[promote] - PAWN_VAL);
					blackPieces++;

					switch (promote) {
						case QUEEN:
							Bitboard::setSquare(bQueens, dest);
							Bitboard::setSquare(rookMovers, dest);
							Bitboard::setSquare(bishopMovers, dest);
						break;

						case ROOK:
							Bitboard::setSquare(bRooks, dest);
							Bitboard::setSquare(rookMovers, dest);
						break;

						case BISHOP:
							Bitboard::setSquare(bBishops, dest);
							Bitboard::setSquare(bishopMovers, dest);
						break;

						case KNIGHT:
							Bitboard::setSquare(bKnights, dest);
						break;
					}
				} else {
					//Normal pawn capture
					Bitboard::setClear(bPawns, bitMove);

					//Enpassant if there's no piece on the destination square
					if(dest == stateStack[depth-1].enPassant) {
						//Put a piece on the destination pawn square

						int square = dest - 8;

						//Put a piece on the destination pawn square
						Bitboard::setSquare(occupied,  dest);
						Bitboard::set(occupiedLeft45, Bitboard::squaresLeft45[dest]);
						Bitboard::set(occupiedRight45 ,  Bitboard::squaresRight45[dest]);
						Bitboard::set(occupiedLeft90 ,  Bitboard::squaresLeft90[dest]);

						//Clear pawn off occupied boards..
						Bitboard::clearSquare(occupied,  square);
						Bitboard::clear(occupiedLeft45, Bitboard::squaresLeft45[square]);
						Bitboard::clear(occupiedRight45 ,  Bitboard::squaresRight45[square]);
						Bitboard::clear(occupiedLeft90 ,  Bitboard::squaresLeft90[square]);

						//Clear off pawn actually being taken for the correct square
						Bitboard::clearSquare(wPawns, square);
						Bitboard::clearSquare(wPieces, square);

						pieces[square] = 0;
						hashWPiece(PAWN, square);
					}

					hashBPiece(PAWN, dest);
				}

			break;
		}

		Bitboard::setClear(bPieces,  bitMove);
		//Remove the piece being taken
		Bitboard::clearSquare(wPieces, dest);

	} //MOVE_BLACK


} //capturePiece()


void Board::retractRealMove() {
 	if(stateStack != history) {
		int move = stateStack[-1].move;

		depth = 1;
		stateStack--;

		retractMove(move);
		numMoves = 0;
	}
}

//Retract a null move
void Board::retractNullMove() {
	numMoves = currentState->firstMove;

	//Take last state off the stack - already initialised (should be!)
	depth--;
	currentState--;

	//Change turn back again
	sideToMove=!sideToMove;

	if(sideToMove == MOVE_BLACK) moveNumber--;

}




void Board::retractMove(int i) {
	int promotion = (i >> 20) & 15;
	int capture = (i >> 16) & 15;
	int type = (i >> 12) & 15;
	int source = (i >> 6) & 63;
	int dest = i & 63;

	numMoves = currentState->firstMove;

	//Take last state off the stack - already initialised (should be!)
	depth--;
	currentState--;

	//Change turn back again
	sideToMove=!sideToMove;

	if(sideToMove == MOVE_BLACK) moveNumber--;


	if(capture) {
		//Capture moves
		unCapturePiece(type, source, dest, capture, promotion);

	//	cout << "Capture" << endl;
	} else {
		//Non capture
		unMovePiece(type, source, dest, promotion);
	}
}


//Reverse the last move... except don't worry about the hash key as we have that on the stack
inline void Board::unMovePiece(int type, int source, int dest, int promote) {

	//Xor with occupied masks so that it creates a 0 where the piece was and a 1 where the piece is headed
	bitboard bitMove = SQUARE(source) | SQUARE(dest);
	bitboard bitMoveLeft90 = Bitboard::squaresLeft90[source] | Bitboard::squaresLeft90[dest];
	bitboard bitMoveLeft45 = Bitboard::squaresLeft45[source] | Bitboard::squaresLeft45[dest];
	bitboard bitMoveRight45 = Bitboard::squaresRight45[source] | Bitboard::squaresRight45[dest];

	//Should work just the same in reverse..

	Bitboard::setClear(occupied,  bitMove);
	Bitboard::setClear(occupiedLeft45 , bitMoveLeft45);
	Bitboard::setClear(occupiedRight45 ,  bitMoveRight45);
	Bitboard::setClear(occupiedLeft90 ,  bitMoveLeft90);

	pieces[dest] = 0;
	pieces[source] = type;

	if(sideToMove == MOVE_WHITE) {
		switch(type) {
			case QUEEN:
				Bitboard::setClear(wQueens, bitMove);
				Bitboard::setClear(rookMovers, bitMove);
				Bitboard::setClear(bishopMovers, bitMove);
			break;

			case ROOK:
				Bitboard::setClear(wRooks, bitMove);
				Bitboard::setClear(rookMovers, bitMove);
			break;

			case BISHOP:
				Bitboard::setClear(wBishops, bitMove);
				Bitboard::setClear(bishopMovers, bitMove);
			break;

			case KNIGHT:
				Bitboard::setClear(wKnights, bitMove);
			break;

			case KING:
				wKingSquare = source;

				Bitboard::setClear(wKing, bitMove);

				//King moving 2 squares must be castling
				if(abs(dest - source) == 2) {
					//Kingside
					if(dest == G1) {
						unMovePiece(ROOK, H1, F1, 0);

					//Queenside
					} else {
						unMovePiece(ROOK, A1, D1, 0);

					}
				}

			break;

			case PAWN:
				if(promote) {
//					pieces[dest] = 0;
					Bitboard::setSquare(wPawns, source);
					whiteMaterial -= (pieceValues[promote] - PAWN_VAL);
					whitePieces--;


					switch (promote) {
						case QUEEN:
							Bitboard::clearSquare(wQueens, dest);
							Bitboard::clearSquare(rookMovers, dest);
							Bitboard::clearSquare(bishopMovers, dest);
						break;

						case ROOK:
							Bitboard::clearSquare(wRooks, dest);
							Bitboard::clearSquare(rookMovers, dest);
						break;

						case BISHOP:
							Bitboard::clearSquare(wBishops, dest);
							Bitboard::clearSquare(bishopMovers, dest);
						break;

						case KNIGHT:
							Bitboard::clearSquare(wKnights, dest);
						break;

					}
				} else {
					//Normal pawn move
					Bitboard::setClear(wPawns, bitMove);
				}

			break;
		}
		Bitboard::setClear(wPieces ,  bitMove);

	} else {									//if (MOVE_BLACK)

		switch(type) {
			case QUEEN:
				Bitboard::setClear(bQueens, bitMove);
				Bitboard::setClear(rookMovers, bitMove);
				Bitboard::setClear(bishopMovers, bitMove);
			break;

			case ROOK:
				Bitboard::setClear(bRooks, bitMove);
				Bitboard::setClear(rookMovers, bitMove);
			break;

			case BISHOP:
				Bitboard::setClear(bBishops, bitMove);
				Bitboard::setClear(bishopMovers, bitMove);
			break;

			case KNIGHT:
				Bitboard::setClear(bKnights, bitMove);
			break;

			case KING:
				Bitboard::setClear(bKing, bitMove);

				bKingSquare = source;

				//King moving 2 squares must be castling
				if(abs(dest - source) == 2) {
					//Kingside
					if(dest == G8) {
						unMovePiece(ROOK, H8, F8, 0);
					//Queenside
					} else {
						unMovePiece(ROOK, A8, D8, 0);

					}
				}

			break;

			case PAWN:
				if(promote) {
//					pieces[dest] = 0;
					Bitboard::setSquare(bPawns, source);
					blackMaterial -= (pieceValues[promote] - PAWN_VAL);
					blackPieces--;

					switch (promote) {
						case QUEEN:
							Bitboard::clearSquare(bQueens, dest);
							Bitboard::clearSquare(rookMovers, dest);
							Bitboard::clearSquare(bishopMovers, dest);
						break;

						case ROOK:
							Bitboard::clearSquare(bRooks, dest);
							Bitboard::clearSquare(rookMovers, dest);
						break;

						case BISHOP:
							Bitboard::clearSquare(bBishops, dest);
							Bitboard::clearSquare(bishopMovers, dest);
						break;

						case KNIGHT:
							Bitboard::clearSquare(bKnights, dest);
						break;

					}
				} else {
					//Normal pawn move
					Bitboard::setClear(bPawns, bitMove);
				}

			break;
		}
		Bitboard::setClear(bPieces ,  bitMove);
	}	//MOVE_BLACK


}	//unMovePiece()


/* Mouse slip we need a take back..
   Or at least something like that.
*/
inline void Board::unCapturePiece(int type, int source, int dest, int capture, int promote) {
	//Xor with occupied masks so that it creates a 0 where the piece was and a 1 where the piece is headed
	bitboard bitMove = SQUARE(source) | SQUARE(dest);

	/* bitboard bitMoveLeft90 = Bitboard::squaresLeft90[source] | Bitboard::squaresLeft90[dest];
	bitboard bitMoveLeft45 = Bitboard::squaresLeft45[source] | Bitboard::squaresLeft45[dest];
	bitboard bitMoveRight45 = Bitboard::squaresRight45[source] | Bitboard::squaresRight45[dest]; */

	/* Only need to clear the source for occupation - as there will be a piece on the destination square already -
	   except for enpassant
	*/

	Bitboard::setSquare(occupied,  source);
	Bitboard::set(occupiedLeft45, Bitboard::squaresLeft45[source]);
	Bitboard::set(occupiedRight45 ,  Bitboard::squaresRight45[source]);
	Bitboard::set(occupiedLeft90 ,  Bitboard::squaresLeft90[source]);

	pieces[source] = type;

	if(sideToMove == MOVE_WHITE) {
		switch(type) {
			case QUEEN:
				Bitboard::setClear(wQueens, bitMove);

				Bitboard::setClear(rookMovers, bitMove);
				Bitboard::setClear(bishopMovers, bitMove);
			break;

			case ROOK:

				Bitboard::setClear(wRooks, bitMove);
				Bitboard::setClear(rookMovers, bitMove);

			break;

			case BISHOP:
				Bitboard::setClear(wBishops, bitMove);
				Bitboard::setClear(bishopMovers, bitMove);

			break;

			case KNIGHT:
				Bitboard::setClear(wKnights, bitMove);
			break;

			case KING:
				wKingSquare = source;
				Bitboard::setClear(wKing, bitMove);
			break;

			case PAWN:

				if(promote) {
//					pieces[dest] = 0;
					Bitboard::setSquare(wPawns, source);
					whiteMaterial -= (pieceValues[promote] - PAWN_VAL);
					whitePieces--;

					switch (promote) {
						case QUEEN:
							Bitboard::clearSquare(wQueens, dest);
							Bitboard::clearSquare(rookMovers, dest);
							Bitboard::clearSquare(bishopMovers, dest);
						break;

						case ROOK:
							Bitboard::clearSquare(wRooks, dest);
							Bitboard::clearSquare(rookMovers, dest);
						break;

						case BISHOP:
							Bitboard::clearSquare(wBishops, dest);
							Bitboard::clearSquare(bishopMovers, dest);
						break;

						case KNIGHT:
							Bitboard::clearSquare(wKnights, dest);
						break;
					}
				} else {
					//Normal pawn capture
					Bitboard::setClear(wPawns, bitMove);

					//Enpassant if there's no piece on the destination square
					if(dest == currentState->enPassant) {

						//Take pawn off destination square occupation bitboards and array
						pieces[dest] = 0;

						Bitboard::clearSquare(occupied,  dest);
						Bitboard::clear(occupiedLeft45, Bitboard::squaresLeft45[dest]);
						Bitboard::clear(occupiedRight45 ,  Bitboard::squaresRight45[dest]);
						Bitboard::clear(occupiedLeft90 ,  Bitboard::squaresLeft90[dest]);

						dest += 8;

						//Put enPassant pawn back
						Bitboard::setSquare(occupied,  dest);
						Bitboard::set(occupiedLeft45, Bitboard::squaresLeft45[dest]);
						Bitboard::set(occupiedRight45 ,  Bitboard::squaresRight45[dest]);
						Bitboard::set(occupiedLeft90 ,  Bitboard::squaresLeft90[dest]);

					}
				}

			break; //case PAWN
		}


		Bitboard::setClear(wPieces, bitMove);

		/* Put the old piece back, set array higher up so can be overwritten for enPassant */
		Bitboard::setSquare(bPieces, dest);
		pieces[dest] = capture;
		blackMaterial += pieceValues[capture];

		switch (capture) {
			case QUEEN:
				Bitboard::setSquare(bQueens, dest);

				Bitboard::setSquare(rookMovers, dest);
				Bitboard::setSquare(bishopMovers, dest);
				blackPieces++;
			break;

			case ROOK:
				Bitboard::setSquare(bRooks, dest);
				Bitboard::setSquare(rookMovers, dest);
				blackPieces++;
			break;

			case BISHOP:
				Bitboard::setSquare(bBishops, dest);
				Bitboard::setSquare(bishopMovers, dest);
				blackPieces++;
			break;

			case KNIGHT:
				Bitboard::setSquare(bKnights, dest);
				blackPieces++;
			break;

			case PAWN:
				Bitboard::setSquare(bPawns, dest);
			break;
		}


	} else {										//if(move==MOVE_BLACK)

		switch(type) {
			case QUEEN:
				Bitboard::setClear(bQueens, bitMove);

				Bitboard::setClear(rookMovers, bitMove);
				Bitboard::setClear(bishopMovers, bitMove);
			break;

			case ROOK:

				Bitboard::setClear(bRooks, bitMove);
				Bitboard::setClear(rookMovers, bitMove);

			break;

			case BISHOP:
				Bitboard::setClear(bBishops, bitMove);
				Bitboard::setClear(bishopMovers, bitMove);

			break;

			case KNIGHT:
				Bitboard::setClear(bKnights, bitMove);
			break;

			case KING:
				bKingSquare = source;
				Bitboard::setClear(bKing, bitMove);
			break;

			case PAWN:

				if(promote) {
//					pieces[dest] = 0;
					Bitboard::setSquare(bPawns, source);
					blackMaterial -= (pieceValues[promote] - PAWN_VAL);
					blackPieces--;

					switch (promote) {
						case QUEEN:
							Bitboard::clearSquare(bQueens, dest);
							Bitboard::clearSquare(rookMovers, dest);
							Bitboard::clearSquare(bishopMovers, dest);
						break;

						case ROOK:
							Bitboard::clearSquare(bRooks, dest);
							Bitboard::clearSquare(rookMovers, dest);
						break;

						case BISHOP:
							Bitboard::clearSquare(bBishops, dest);
							Bitboard::clearSquare(bishopMovers, dest);
						break;

						case KNIGHT:
							Bitboard::clearSquare(bKnights, dest);
						break;
					}
				} else {
					//Normal pawn capture
					Bitboard::setClear(bPawns, bitMove);

					//Enpassant if there's no piece on the destination square
					if(dest == currentState->enPassant) {

						//Take pawn off destination square occupation bitboards and array
						pieces[dest] = 0;

						Bitboard::clearSquare(occupied,  dest);
						Bitboard::clear(occupiedLeft45, Bitboard::squaresLeft45[dest]);
						Bitboard::clear(occupiedRight45 ,  Bitboard::squaresRight45[dest]);
						Bitboard::clear(occupiedLeft90 ,  Bitboard::squaresLeft90[dest]);

						dest -= 8;

						//Put enPassant pawn back
						Bitboard::setSquare(occupied,  dest);
						Bitboard::set(occupiedLeft45, Bitboard::squaresLeft45[dest]);
						Bitboard::set(occupiedRight45 ,  Bitboard::squaresRight45[dest]);
						Bitboard::set(occupiedLeft90 ,  Bitboard::squaresLeft90[dest]);

					}
				}

			break; //case PAWN
		}


		Bitboard::setClear(bPieces, bitMove);

		/* Put the old piece back, set array higher up so can be overwritten for enPassant */
		Bitboard::setSquare(wPieces, dest);
		pieces[dest] = capture;
		whiteMaterial += pieceValues[capture];

		switch (capture) {
			case QUEEN:
				Bitboard::setSquare(wQueens, dest);

				Bitboard::setSquare(rookMovers, dest);
				Bitboard::setSquare(bishopMovers, dest);
				whitePieces++;
			break;

			case ROOK:
				Bitboard::setSquare(wRooks, dest);
				Bitboard::setSquare(rookMovers, dest);
				whitePieces++;
			break;

			case BISHOP:
				Bitboard::setSquare(wBishops, dest);
				Bitboard::setSquare(bishopMovers, dest);
				whitePieces++;
			break;

			case KNIGHT:
				Bitboard::setSquare(wKnights, dest);
				whitePieces++;
			break;

			case PAWN:
				Bitboard::setSquare(wPawns, dest);
			break;
		}
	}	//MOVE_BLACK


}	//unCapturePiece()

inline void Board::whiteRookMove(int source) {
	if(source == H1 && (currentState->castling & W_KINGSIDE)) {
		currentState->castling &= ~W_KINGSIDE;
		hashCastle(W_KINGSIDE);
	}

	if(source == A1 && (currentState->castling & W_QUEENSIDE)) {
		currentState->castling &= ~W_QUEENSIDE;
		hashCastle(W_QUEENSIDE);
	}
}


inline void Board::blackRookMove(int source) {

	if(source == H8 && (currentState->castling & B_KINGSIDE)) {
		currentState->castling &= ~B_KINGSIDE;
		hashCastle(B_KINGSIDE);
	}

	if(source == A8 && (currentState->castling & B_QUEENSIDE)) {
		currentState->castling &= ~B_QUEENSIDE;
		hashCastle(B_QUEENSIDE);
	}
}

