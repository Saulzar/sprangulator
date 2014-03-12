

#include "board.h"
#include "bitboard.h"
#include "transposition.h"
#include "utility.h"
#include "const.h"
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>

	int Board::search(int nodesPerUpdate) {
		currentBest = -1;
		int i = 1;
		hashHits = 0;
		returnMove = 0;

		nodes = 0;
		abortingSearch = 0;

#ifdef EXTRA_STATS
		cutoffs = 0;
		firstCutoffs = 0;
#endif


		this->nodesPerUpdate =  nodesPerUpdate;
		nodesUntilUpdate = nodesPerUpdate;

		if(isRepetition()) {
			if(currentState->halfMoveCount >= 100) return DRAW_50_MOVES;
			return DRAW_REPETITION;
		}

		while(i <= maxDepth) {
			numMoves = 0;

			int result = alphaBeta(i, -MATE , MATE , NO_NULL_MOVE);

			if(returnMove > 0) {
				currentBest = returnMove;

			/*	printMove(returnMove);

				cout << "->";
				printLine();
				cout << endl;*/
			}

			if(abortingSearch) {
				return NO_RESULT;
			}

			if(currentBest == -1) {
				if(result > 0) return sideToMove==MOVE_WHITE? WHITE_MATE : BLACK_MATE;
				else if(result < 0) return sideToMove==MOVE_WHITE? BLACK_MATE : WHITE_MATE;
				else return DRAW_STALEMATE;
			}

			i++;
		}

		return NO_RESULT;

	}

int Board::alphaBeta(int depthLeft, int alpha, int beta, int doNullMove) {
	int check = 0;
	int oldAlpha = alpha;
	int numSearched = 0;
	int bestMove = 0;
	int hashMove = 0;
	int stage;
	int i = currentState->firstMove;
	int result;

	nodes++;
	nodesUntilUpdate--;
	if(nodesUntilUpdate <= 0) {
		switch(game->interfaceUpdate()) {
			case OUT_OF_TIME:

				abortingSearch = 1;
				return 0;
			break;

			case STOP_SEARCH:
				abortingSearch = 1;
				return 0;
			break;
		}

		nodesUntilUpdate = nodesPerUpdate;
	}



	if(isRepetition()) {
		returnMove = 0;
		return DRAW;
	}


	HashEntry *e;
	e = hash->probe(currentState->hashKey);

	if(e!=NULL) {
		hashHits++;
		//Is it useful for replacing this search - otherwise just find a best move
		hashMove = e->bestMove;
		returnMove = hashMove;

#ifndef DISABLE_HASH
		if(depth > 0 && e->depth >= depthLeft) {
			switch(e->flags & 3) {
				case HASH_EXACT:
					return e->score;
				break;

				case HASH_LOWER:
					if(e->score >= beta) return e->score;
					if(e->score > alpha) alpha = e->score;
				break;

				case HASH_UPPER:
					if(e->score <= alpha) return e->score;
					if(e->score < beta) beta = e->score;
				break;
			}

			if(e->flags & HASH_NO_NULL) {
				doNullMove = 0;
			}
		}
#endif
	}

#ifdef DEBUG_HASH
	if(hashMove > 0 && !testLegality(hashMove)) {
		print();
		cout << "Illegal hash move! ";
		printMove(hashMove);
		cout << endl;
	}
#endif


	if(depthLeft == 0) {
		int value =  quiescence(alpha, beta);
		//hash->store(0, value, 0, HASH_EXACT);
		return value;
	}


	check = inCheck();

/*
	if(doNullMove && !check && !nullRisk() && depth >= 2) {
		int nullDepth = depthLeft - 2;

		makeNullMove();

		if(nullDepth > 0) {
			result = alphaBeta(nullDepth, -beta, -beta+1, NO_NULL_MOVE);
		} else {
			result = quiescence(-beta, -beta+1);
		}

		retractNullMove();

		if(nullDepth > 0 && result >= beta) {
			result = alphaBeta(nullDepth, beta-1, beta, DO_NULL_MOVE);
			numMoves = currentState->firstMove;
		}


		if(result >= beta) {
			returnMove = 0;
			return beta;
		}
	}
*/

	if(!hashMove && depthLeft >= 3) {
		result = alphaBeta(depthLeft - 2, alpha, beta, DO_NULL_MOVE);
		numMoves = currentState->firstMove;

		//Failed low so have to research with new bounds
		if(result <= alpha) result = alphaBeta(depthLeft - 2, -MATE, alpha + 1, DO_NULL_MOVE);
		numMoves = currentState->firstMove;

		hashMove = returnMove;
	}

	if(hashMove > 0) {
		stage = HASH_MOVE;
	}	else {
		stage = ALL_MOVES;
	}

	while(stage != FINISHED) {
		switch(stage) {
			case HASH_MOVE:
				moveStack[numMoves++] = hashMove;
				stage = ALL_MOVES;
			break;
			case  ALL_MOVES:
				if(check) {
					generateCheckEvasions();
					sortNonCaptures(i, hashMove);
					stage = FINISHED;
				} else {
					generateCaptures();
					sortCaptures(i, hashMove);
					stage = NON_CAPTURES;
				}
			break;
			case NON_CAPTURES:
				generateNonCaptures();
				sortNonCaptures(i, hashMove);
				stage = FINISHED;
			break;
		}

	for(; i<numMoves; i++) {
			int move = moveStack[i];
			makeMove(move);

#ifdef DEBUG_BITBOARDS
				if(isMessedUp()) {
					printMoves();
					print();
					cout << "Hash move = "; printMove(hashMove); cout << endl;
					cout << "MakeMove" << endl;
					exit(0);
				}
#endif

			if(isLegal())  {
				numSearched++;

				result = -alphaBeta(depthLeft - 1, -beta, -alpha, DO_NULL_MOVE);
				retractMove(move);

				if(abortingSearch) {
					returnMove = bestMove;
					return result;
				}

				if(result > alpha) {
					if(result >= beta) {
						returnMove = move;
						hash->store(move, result, depthLeft, HASH_LOWER);

#ifdef EXTRA_STATS
						cutoffs++;
						if(i == currentState->firstMove) firstCutoffs++;
#endif
						return result;
					}


/*					if(depth == 0) {
						printMove(move);
						cout << " ";
						printScore(result);
						cout << " hashMove: ";
						printMove(hashMove);
						cout << endl;
					}*/

					alpha = result;
					bestMove = move;
				}


			} else { //Illegal move
				retractMove(move);
			}
#ifdef DEBUG_BITBOARDS
			if(isMessedUp()) {
				printMoves();
				print();
				cout << "Hash move = "; printMove(hashMove); cout << endl;
				cout << "RetractMove" << endl;
				exit(0);
			}
#endif
		}
	}		//While we have moves

		//No legal moves..
	if(numSearched ==0) {
		returnMove = 0;

		if(check)	{
			 return -(MATE - depth);
		} else {
			return DRAW;
		}
	}


	if(alpha == oldAlpha) {
		hash->store(bestMove, alpha, depthLeft, HASH_UPPER);

	} else {
		hash->store(bestMove, alpha, depthLeft, HASH_EXACT);
	}

	returnMove = bestMove;
	return alpha;
}


int Board::quiescence(int alpha, int beta) {
	int check = 0;
	int numSearched = 0;
	int bestMove = 0;
	int oldAlpha = alpha;
	int hashMove = 0;
	int stage;
	int i = currentState->firstMove;

	nodes++;
	nodesUntilUpdate--;

	if(nodesUntilUpdate <= 0) {
		switch(game->interfaceUpdate()) {
			case OUT_OF_TIME:

				abortingSearch = 1;
				return 0;
			break;

			case STOP_SEARCH:
				abortingSearch = 1;
				return 0;
			break;
		}

		nodesUntilUpdate = nodesPerUpdate;
	}




	HashEntry *e;
	e = hash->probe(currentState->hashKey);

	if(e!=NULL) {
		hashHits++;
		//Is it useful for replacing this search - otherwise just find a best move
		hashMove = e->bestMove;
		returnMove = hashMove;

#ifndef DISABLE_HASH
		//No need to worry about depth >= 0...
		switch(e->flags & 3) {
			case HASH_EXACT:
				return e->score;
			break;

			case HASH_LOWER:
				if(e->score >= beta) return e->score;
				if(e->score > alpha) alpha = e->score;
			break;

			case HASH_UPPER:
				if(e->score <= alpha) return e->score;
				if(e->score < beta) beta = e->score;
			break;

		}
#endif
	}



	if(hashMove > 0  && (hashMove >> 16) & 15) {
		stage = HASH_MOVE;
	}	else {
		stage = ALL_MOVES;
	}

	while(stage != FINISHED) {
		switch(stage) {
			case HASH_MOVE:
				moveStack[numMoves++] = hashMove;
				stage = ALL_MOVES;
			break;
			case  ALL_MOVES:
				check = inCheck();

				if(check) {
					generateCheckEvasions();
					sortNonCaptures(i, hashMove);

				} else {
					alpha = evaluate();
					if(alpha > beta) {
						return beta;
					}

					generateCaptures();
					sortGoodCaptures(i, hashMove);

				}

				stage = FINISHED;
			break;
		}

		for(; i<numMoves; i++) {
			int move = moveStack[i];
			makeMove(move);

			if(isLegal())  {
				numSearched++;

				int result = -quiescence(-beta, -alpha);
				retractMove(move);

				if(abortingSearch) return 0;

				if(result > alpha) {
					if(result >= beta) {
						returnMove = move;
						hash->store(move, result, 0, HASH_LOWER);
#ifdef EXTRA_STATS
						cutoffs++;
						if(i == currentState->firstMove) firstCutoffs++;
#endif
						return result;
					}

					alpha = result;
					bestMove = move;
				}

			} else { //Illegal move
				retractMove(move);
			}
		}
	}

	//No legal moves..
	if(numSearched ==0) {
		if(check)	{
			 return -(MATE - depth);
		} else {
			//Do nothing - there may be some captures but they're no good
		}
	}

	if(alpha == oldAlpha) {
		hash->store(bestMove, alpha, 0, HASH_UPPER);

	} else {
		hash->store(bestMove, alpha, 0, HASH_EXACT);
	}

	return alpha;
}


int Board::nullRisk() {
	if(sideToMove == MOVE_WHITE) {
		if(whiteMaterial <= ZUG_MATERIAL || whitePieces <= ZUG_PIECES) return 1;
	} else if(sideToMove == MOVE_BLACK) {
		if(blackMaterial <= ZUG_MATERIAL || whitePieces <= ZUG_PIECES) return 1;
	}

	return 0;
}
