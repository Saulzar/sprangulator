#ifndef GAME_H
#define GAME_H 1

#include "board.h"
#include "bitboard.h"
#include "time.h"
#include "const.h"
#include "xboardplayer.h"
#include <string.h>
#include <stdlib.h>

class Board;
class XBoardPlayer;

typedef XBoardPlayer Player;

//Play one game of chess..
class Game {
	public:

	Board *board;
	pthread_mutex_t boardLock; //Lock for changing the board/reading off engine structures

	Board *boardCopy; //Make a copy and use this one to check move legality etc..
	pthread_mutex_t copyLock;

	int colour;
	int mode;
	int running;
	int opponentMoved; //We're in ponder mode - has our opponent moved?

	//In milli-seconds
	int ourTime, opponentsTime;
	int baseTime, increment, movesPerControl;
	unsigned long long int lastUpdateOurTime;  //From gettimeofday
 	unsigned long long int lastUpdateOpponentsTime;  //From gettimeofday

	pthread_mutex_t modeLock; //lLock for changing game states updating time etc..

	unsigned long long int startMoveTime;
	int timeAllocated;
	int lastNps;

	Player *player;

	void findMove();
	void ponder();

	//Called by the search regularly to update time etc.. to check if a new move has come in
	int interfaceUpdate();

	static void runGame(Game *instance);
	void run();
	void stop();
	void reset();

	int isRunning();

	Game(Player *player);
	~Game();

	void setBoard(char *fen);
	void setColour(int colour);

	void setMode(int mode);
	void setTimeControls(int movesPerControl, int baseTime, int increment);

	void setTime(int ourTime);
	void setOpponentsTime(int opponentsTime);

	unsigned long long getTime();
	unsigned long long getTimeTaken();

	void externalMove(char *move);
	void Game::undo();

	void listMoves();
	void printBoard();

	void readPgn(char *filename);
	void benchMark(char *filename);
	void benchTreeSize(char *filename);
	void benchGenerate(char *filename);
	void benchMove(char *filename);

	void printEval();
};


#endif


