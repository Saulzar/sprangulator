#include "board.h"
#include "xboardplayer.h"
#include "const.h"
#include "game.h"


#include <iostream.h>
#include <fstream.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <regex.h>

void Game::findMove() {
	int estimatedMoves;
	int controlLeft = 0;
	int nodesPerUpdate;
	int bestMove;
	unsigned long long int updateTime;
	int makeMove = 0;
	int moveTime;
	int result;


	player->message("Start search", MESSAGE_CONSOLE);

	pthread_mutex_lock(&boardLock);
		estimatedMoves = ((board->whiteMaterial + board->blackMaterial)/400) + 20;

		if(movesPerControl) {
			controlLeft = movesPerControl - (board->moveNumber % movesPerControl);
			if(estimatedMoves > controlLeft) estimatedMoves = controlLeft + 1;
		}
	pthread_mutex_unlock(&boardLock);


	pthread_mutex_lock(&modeLock);
	lastUpdateOurTime = getTime();

	timeAllocated = ((ourTime-(increment+200)) / estimatedMoves); //Leave 2 seconds spare always!

	if(increment) timeAllocated += (increment - 50);
	startMoveTime = lastUpdateOurTime;

	if(timeAllocated < 1) timeAllocated = 1;

	pthread_mutex_unlock(&modeLock);

	nodesPerUpdate = (int) (((float)timeAllocated/1000.0 * (float)lastNps) / 100); //100 Updates until we move..
	if(nodesPerUpdate > lastNps/10) nodesPerUpdate = lastNps/10;

	pthread_mutex_lock(&boardLock);
		result = board->search(nodesPerUpdate);
		bestMove = board->currentBest;

	pthread_mutex_unlock(&boardLock);

	pthread_mutex_lock(&modeLock);

	if(bestMove == -1) {	//Game is in a finished state.
		pthread_mutex_unlock(&modeLock);
		mode = MODE_WAIT;
		player->result(result);

		player->message("Game over", MESSAGE_CONSOLE);
		return;
	}

	updateTime = getTime();
	moveTime = updateTime - startMoveTime;

	if(mode == MODE_PLAY) { //If we're still set to play this move
		ourTime -= moveTime;
		makeMove = 1;
	}

	lastUpdateOurTime = updateTime;
	if(moveTime) lastNps = 1000 * (board->nodes / (moveTime)); //Seconds

	pthread_mutex_unlock(&modeLock);

	if(makeMove) {
	pthread_mutex_lock(&boardLock);
		player->printLine();

		player->makeMove(bestMove);
		board->makeRealMove(bestMove);

	/*	board->print();
		board->printMove(bestMove); */

		pthread_mutex_lock(&copyLock);
			board->makeCopy(boardCopy);
		pthread_mutex_unlock(&copyLock);
	pthread_mutex_unlock(&boardLock);
	}

	//Send the move off..
	player->message("Finished search", MESSAGE_CONSOLE);
}

void Game::ponder() {
	player->message("Pondering", MESSAGE_CONSOLE);

	int nodesPerUpdate;
	int nodes = 0;

	pthread_mutex_lock(&modeLock);

	opponentMoved = 0;

	lastUpdateOpponentsTime = getTime();
	startMoveTime = lastUpdateOpponentsTime;

	pthread_mutex_unlock(&modeLock);

	nodesPerUpdate = lastNps / 100; //100 Updates per second..

	pthread_mutex_lock(&boardLock);

		int ponderMove = board->getHashMove();

		if(ponderMove > 0) board->makeRealMove(ponderMove);

		int result = board->search(nodesPerUpdate);

		if(ponderMove > 0) board->retractRealMove();

		//If the position after the ponder move is the end of the game just search from the position before..
		if(result!= NO_RESULT) {
			board->search(nodesPerUpdate);
		}

		nodes = board->nodes;

	pthread_mutex_unlock(&boardLock);

	pthread_mutex_lock(&modeLock);

	lastUpdateOpponentsTime = getTime();
	if(lastUpdateOpponentsTime - startMoveTime) lastNps = 1000 * (nodes / (lastUpdateOpponentsTime - startMoveTime)); //Seconds

	pthread_mutex_unlock(&modeLock);


	player->message("Ponder done", MESSAGE_CONSOLE);
	//Done - don't have to move all the info we want should be in the transposition table..
}


//Called by the search regularly to update time etc.. to check if a new move has come in
int Game::interfaceUpdate() {
	int retValue = CONTINUE_SEARCH;
	unsigned long long int updateTime;

	pthread_mutex_lock(&modeLock);
	switch(mode) {

		case MODE_TEST:
		case MODE_PLAY:
			updateTime = getTime();

/*			ourTime -= (updateTime - lastUpdateTime);
			lastUpdateTime = updateTime; */

			if(board->currentBest > 0 && (updateTime - startMoveTime) > (unsigned int)timeAllocated) {
				retValue = OUT_OF_TIME;
			}
		break;

		case MODE_PONDER:
			if(opponentMoved) {
				retValue = STOP_SEARCH;

				//Time wasted waiting to stop pondering
				updateTime = getTime();
				ourTime -= updateTime - lastUpdateOurTime;
				lastUpdateOurTime = updateTime;
			}
		break;

		case MODE_WAIT:
		case MODE_FORCE:

			retValue = STOP_SEARCH;
		break;

		case MODE_ANALYSE:

		break;
	}
	pthread_mutex_unlock(&modeLock);

	return retValue;
}

//Static function to call as a function pointer for pthread_create
void Game::runGame(Game *instance) {
	instance->run();
}


//Main control thread for searching...
void Game::run() {
	int currentMode = MODE_WAIT;
	reset();
	lastNps = 50000;

	pthread_mutex_lock(&modeLock);
		running = 1;
	pthread_mutex_unlock(&modeLock);

	while(1) {

		pthread_mutex_lock(&modeLock);
		if(currentMode!=mode) {
			player->message("Mode change", MESSAGE_CONSOLE);

			currentMode = mode;
			pthread_mutex_unlock(&modeLock);

			switch(currentMode) {
				case MODE_WAIT:
					player->message("Mode wait", MESSAGE_CONSOLE);
				break;

				case MODE_FORCE:
					player->message("Mode force", MESSAGE_CONSOLE);
				break;

				case MODE_PLAY:
					player->message("Mode play", MESSAGE_CONSOLE);
					findMove();

					//Check that the mode hasn't been changed from under us..
					pthread_mutex_lock(&modeLock);
					if(mode == MODE_PLAY) {
						//We've made a move start pondering
						mode = MODE_PONDER;
					}
					pthread_mutex_unlock(&modeLock);

				break;

				case MODE_PONDER:
					player->message("Mode ponder", MESSAGE_CONSOLE);
					ponder();

					//Check that the mode hasn't been changed from under us..
					pthread_mutex_lock(&modeLock);
					if(mode == MODE_PONDER) {
						//We've made a move start pondering
						mode = MODE_PLAY;
					}
					pthread_mutex_unlock(&modeLock);

				break;

				case MODE_ANALYSE:
					player->message("Mode analyse", MESSAGE_CONSOLE);

				break;
			}

			player->message("Finish mode change", MESSAGE_CONSOLE);
		}

		pthread_mutex_unlock(&modeLock);

	}
}

void Game::reset() {
	pthread_mutex_lock(&modeLock);

	mode = MODE_WAIT;
	colour = MOVE_BLACK;

	pthread_mutex_unlock(&modeLock);

	pthread_mutex_lock(&boardLock);

	board->reset();
	board->setDefault();

		pthread_mutex_lock(&copyLock);
			board->makeCopy(boardCopy);
		pthread_mutex_unlock(&copyLock);

	pthread_mutex_unlock(&boardLock);
}

void Game::stop() {
	pthread_mutex_lock(&modeLock);

	running = 0;

	pthread_mutex_unlock(&modeLock);
	pthread_exit(NULL);
}

int Game::isRunning() {
	int isRun;
	pthread_mutex_lock(&modeLock);
		isRun = running;
	pthread_mutex_unlock(&modeLock);

	return isRun;
}


Game::Game(Player *player) {
	board = new Board(128 * 1024 * 1024, this);
	boardCopy = new Board();

	board->setDefault();
	boardCopy->setDefault();

	colour = MOVE_BLACK;
	this->player = player;
	running = 0;
	lastNps = 0;
	mode = MODE_WAIT;
	opponentMoved = 0;

	baseTime = 0;
	increment = 0;
	movesPerControl = 0;

	ourTime = 0;
	opponentsTime = 0;

	lastUpdateOurTime = 0;
	lastUpdateOpponentsTime = 0;
	startMoveTime = 0;
	timeAllocated = 0;
	lastNps = 0;

	pthread_mutex_init(&modeLock, NULL);
	pthread_mutex_init(&boardLock, NULL);
	pthread_mutex_init(&copyLock, NULL);

}


Game::~Game() {

	if(isRunning()) stop();

	pthread_mutex_lock(&boardLock);

	if(board!=NULL) delete board;

	pthread_mutex_unlock(&boardLock);

	pthread_mutex_destroy(&modeLock);
	pthread_mutex_destroy(&boardLock);
	pthread_mutex_destroy(&copyLock);
}

void Game::setBoard(char *fen) {
	pthread_mutex_lock(&modeLock);
		if(mode!=MODE_FORCE && mode!=MODE_WAIT) {

			player->error("setboard", "Can't set the board outside of force mode");
			pthread_mutex_unlock(&modeLock);
			return;
		}

	pthread_mutex_unlock(&modeLock);

	pthread_mutex_lock(&boardLock);

		board->setFen(fen);

		pthread_mutex_lock(&copyLock);
			board->makeCopy(boardCopy);
		pthread_mutex_unlock(&copyLock);

	pthread_mutex_unlock(&boardLock);
}

void Game::setColour(int colour) {
	pthread_mutex_lock(&modeLock);

		colour = colour;

	pthread_mutex_unlock(&modeLock);
}


void Game::setMode(int mode) {

	pthread_mutex_lock(&modeLock);
		this->mode = mode;
	pthread_mutex_unlock(&modeLock);
}

void Game::setTime(int ourTime) {

	pthread_mutex_lock(&modeLock);

		this->ourTime = ourTime;
		lastUpdateOurTime = getTime();

	pthread_mutex_unlock(&modeLock);
}

void Game::setOpponentsTime(int opponentsTime) {
	pthread_mutex_lock(&modeLock);

		this->opponentsTime = opponentsTime;
		lastUpdateOpponentsTime = getTime();

	pthread_mutex_unlock(&modeLock);
}

void Game::setTimeControls(int movesPerControl, int baseTime, int increment) {

	pthread_mutex_lock(&modeLock);

	if(mode != MODE_FORCE && mode !=MODE_WAIT) {
		player->error("level", "Can't set the time control outside of force mode");

		pthread_mutex_unlock(&modeLock);
		return;
	}

	this->baseTime = baseTime * 1000 * 60; //Milliseconds
	this->increment = increment * 1000;
	this->movesPerControl = movesPerControl;

	pthread_mutex_unlock(&modeLock);
}

void Game::externalMove(char *move) {
	int updateTime;
	int m;

	player->message("User move", MESSAGE_CONSOLE);


	pthread_mutex_lock(&copyLock);
		m = boardCopy->parseMove(move);
	pthread_mutex_unlock(&copyLock);


	if(m == -1) {
		player->error(move, "Illegal move");
		return;
	}

	if(m == -2) {
		player->error(move, "Ambiguous move");
		return;
	}

	pthread_mutex_lock(&modeLock);

  		switch(mode) {
   			case MODE_PLAY:
				player->error(move, "It's not your turn");
				pthread_mutex_unlock(&modeLock);


				return;
			break;

			case MODE_PONDER:
				opponentMoved = 1;
				updateTime = getTime();
				opponentsTime += updateTime - lastUpdateOpponentsTime;
				lastUpdateOpponentsTime = updateTime;
			break;

			case MODE_FORCE:

			break;

			case MODE_ANALYSE:

			break;

			case MODE_WAIT:
				mode = MODE_PLAY;
			break;
		}

	pthread_mutex_unlock(&modeLock);

	pthread_mutex_lock(&boardLock);

		board->makeRealMove(m);

		pthread_mutex_lock(&copyLock);
			board->makeCopy(boardCopy);
		pthread_mutex_unlock(&copyLock);

	pthread_mutex_unlock(&boardLock);

	player->message("Updated board", MESSAGE_CONSOLE);
}


void Game::undo() {
	player->message("Undo move", MESSAGE_CONSOLE);
	int oldMode;

	pthread_mutex_lock(&modeLock);
		oldMode = mode;

  		switch(mode) {
   			case MODE_PLAY:
				mode = MODE_WAIT;
			break;

			case MODE_PONDER:
				mode = MODE_WAIT;
			break;

			case MODE_FORCE:

			break;

			case MODE_ANALYSE:

			break;

			case MODE_WAIT:

			break;
		}
	pthread_mutex_unlock(&modeLock);

	pthread_mutex_lock(&boardLock);

		board->retractRealMove();

		pthread_mutex_lock(&copyLock);
			board->makeCopy(boardCopy);
		pthread_mutex_unlock(&copyLock);

	pthread_mutex_unlock(&boardLock);

	player->message("Updated board", MESSAGE_CONSOLE);
}



//Time in milliseconds
unsigned long long Game::getTime() {
	timeval tv;

	gettimeofday(&tv, NULL);
	return((tv.tv_sec * 1000) + (tv.tv_usec/1000)); //Milliseconds..
}

unsigned long long Game::getTimeTaken() {
	return (getTime() - startMoveTime);
}

void Game::listMoves() {
		pthread_mutex_lock(&copyLock);
			boardCopy->generateMoves();

			for(int i=0; i<boardCopy->numMoves; i++) {
				boardCopy->printMove(boardCopy->moveStack[i]);
				cout << " ";
			}

			cout << endl;

			boardCopy->numMoves = 0;
		pthread_mutex_unlock(&copyLock);
}

void Game::printBoard() {
		pthread_mutex_lock(&copyLock);
			boardCopy->print();
		pthread_mutex_unlock(&copyLock);
}


void Game::readPgn(char *filename) {
	char buffer[256];
	char attribute[128];
	char value[128];
	regmatch_t matches[10];
	regex_t regex;

	ifstream in(filename);
	int place = 0;


	char pattern[] = "\\[(.*?) \"(.*?)\"\\]";
	int error = regcomp(&regex, pattern, REG_NEWLINE|REG_EXTENDED);

	if(error) {
		char cerror[256];
		regerror(error, &regex, cerror, 256);

		cout << "Error making regex " << pattern << " " << cerror << endl;
	}

	reset(); //Reset this game

	//Read header
	while(in.getline(buffer, 256)) {
		if(regexec(&regex, buffer, 10, matches, 0) == 0) {

			strncpy(attribute, (char*)(buffer + matches[1].rm_so), 128);
			attribute[matches[1].rm_eo - matches[1].rm_so] = 0;

			strncpy(value, (char*)(buffer + matches[2].rm_so), 128);
			value[matches[2].rm_eo - matches[2].rm_so] = 0;

			if(strncmp(attribute, "FEN", 128) == 0) {
				pthread_mutex_lock(&boardLock);
					board->setFen(value);
				pthread_mutex_unlock(&boardLock);
			}

			//cout << attribute << " " << value << endl;
		} else break;

		place = in.tellg();
	}

	//Go back to the start of the line where it wasn't a header
	in.seekg(place);

	pthread_mutex_lock(&boardLock);

	//Read move list
	while(in >> buffer) {
		if(buffer[0] == '{') break;
		int m = board->parseMove(buffer);

		if(m >= -0) {
			/*cout << buffer << " ";
			board->printMove(m); */
			board->makeRealMove(m);
			//cout << " " << endl;
		}

		place = in.tellg();
	}

	in.seekg(place);

	if(in.getline(buffer, 256)) {
		//Do something with the result
	}

	pthread_mutex_lock(&copyLock);
	board->makeCopy(boardCopy);
	pthread_mutex_unlock(&copyLock);

	pthread_mutex_unlock(&boardLock);
	regfree(&regex);
}


void Game::benchMark(char *filename){
	char buffer[256];
	int nodesPerUpdate = 10000;
	int bestMove;
	unsigned long long int updateTime;
	int moveTime;
	int totalNps;
	int secondsPerPosition = 1;

	unsigned long long totalTime = 0;
	unsigned long long totalNodes =0;

	ifstream in(filename);



	pthread_mutex_lock(&modeLock);
		mode = MODE_TEST;
	pthread_mutex_unlock(&modeLock);

	pthread_mutex_lock(&boardLock);

	pthread_mutex_unlock(&boardLock);


	while(in.getline(buffer, 256)) {
		if(strlen(buffer) < 2) continue;

		board->reset();
		board->setFen(buffer);

		pthread_mutex_lock(&modeLock);
		lastUpdateOurTime = getTime();

		timeAllocated = secondsPerPosition * 1000; //100 seconds per move
		startMoveTime = lastUpdateOurTime;

		pthread_mutex_unlock(&modeLock);

		pthread_mutex_lock(&boardLock);
			bestMove = board->search(nodesPerUpdate);
		pthread_mutex_unlock(&boardLock);

		pthread_mutex_lock(&modeLock);

		if(bestMove == -1) {	//Game is in a finished state.
			player->message("Error - fen is in finished state", MESSAGE_CONSOLE);
			continue;
		}

		updateTime = getTime();
		moveTime = updateTime - startMoveTime;

		totalTime += moveTime;
		totalNodes += board->nodes;

		if(moveTime) lastNps = 1000 * (board->nodes / (moveTime)); //Seconds

		player->printLine();
		cout << board->nodes << " nodes in " << ((float)moveTime/(float)1000) << " seconds " << endl;

		pthread_mutex_unlock(&modeLock);
	}

	if(totalTime > 0) {
		totalNps = 1000 * (totalNodes / (totalTime)); //Seconds
		cout << "Benchmark: " << totalNodes << " nodes in " << ((float)totalTime/(float)1000) << " seconds " << totalNps/1000 << " kNps" << endl;
	} else {
		cout << "No tests in file" << endl;
	}

	pthread_mutex_lock(&modeLock);
		mode = MODE_FORCE;
	pthread_mutex_unlock(&modeLock);
}

void Game::benchTreeSize(char *filename){
	char buffer[256];
	int nodesPerUpdate = 10000;
	int bestMove;
	unsigned long long int updateTime;
	int moveTime;
	int totalNps;
	int maxDepth = 4;


	unsigned long long totalTime = 0;
	unsigned long long totalNodes =0;

	ifstream in(filename);


	pthread_mutex_lock(&modeLock);
		mode = MODE_TEST;
	pthread_mutex_unlock(&modeLock);

	pthread_mutex_lock(&boardLock);

	pthread_mutex_unlock(&boardLock);


	while(in.getline(buffer, 256)) {
		if(strlen(buffer) < 2) continue;

		board->reset();
		board->setFen(buffer);
		board->maxDepth = maxDepth;

		pthread_mutex_lock(&modeLock);
		lastUpdateOurTime = getTime();

		timeAllocated = 10000000; //100000 seconds per move
		startMoveTime = lastUpdateOurTime;

		pthread_mutex_unlock(&modeLock);

		pthread_mutex_lock(&boardLock);
			bestMove = board->search(nodesPerUpdate);
		pthread_mutex_unlock(&boardLock);

		pthread_mutex_lock(&modeLock);

		if(bestMove == -1) {	//Game is in a finished state.
			player->message("Error - fen is in finished state", MESSAGE_CONSOLE);
			continue;
		}

		updateTime = getTime();
		moveTime = updateTime - startMoveTime;

		totalTime += moveTime;
		totalNodes += board->nodes;

		if(moveTime) lastNps = 1000 * (board->nodes / (moveTime)); //Seconds

		player->printLine();
		cout << board->nodes << " nodes in " << ((float)moveTime/(float)1000) << " seconds " << endl;

		pthread_mutex_unlock(&modeLock);
	}

	if(totalTime > 0) {
		totalNps = 1000 * (totalNodes / (totalTime)); //Seconds
		cout << "Benchmark: " << totalNodes << " nodes in " << ((float)totalTime/(float)1000) << " seconds " << totalNps/1000 << " kNps" << endl;
	} else {
		cout << "No tests in file" << endl;
	}

	pthread_mutex_lock(&modeLock);
		mode = MODE_FORCE;
	pthread_mutex_unlock(&modeLock);
}


void Game::benchGenerate(char *filename){
	char buffer[256];
	int nodesPerUpdate = 100000;
	unsigned long long int updateTime;
	int moveTime;
	int totalNps;
	int secondsPerPosition = 1;
	int nodesUntilUpdate;
	int newNodes = 0;

	unsigned long long totalTime = 0;
	unsigned long long totalNodes =0;

	ifstream in(filename);

	pthread_mutex_lock(&modeLock);
		mode = MODE_TEST;
	pthread_mutex_unlock(&modeLock);

	pthread_mutex_lock(&boardLock);

	pthread_mutex_unlock(&boardLock);


	while(in.getline(buffer, 256)) {
		if(strlen(buffer) < 2) continue;

		board->reset();
		board->setFen(buffer);

		pthread_mutex_lock(&modeLock);
		lastUpdateOurTime = getTime();

		timeAllocated = secondsPerPosition * 1000; //100 seconds per move
		startMoveTime = lastUpdateOurTime;

		pthread_mutex_unlock(&modeLock);

		pthread_mutex_lock(&boardLock);
			nodesUntilUpdate = nodesPerUpdate;

			while(1) {
				while(nodesUntilUpdate > 0) {
					for(int i=0; i<100; i++) {
						board->generateMoves();
					}

					newNodes+=board->numMoves;
					nodesUntilUpdate -= board->numMoves;

					board->numMoves = 0;
				}

				updateTime = getTime();
				if((updateTime - startMoveTime)  > (unsigned)timeAllocated) break;

				nodesUntilUpdate = nodesPerUpdate;
			}


		pthread_mutex_unlock(&boardLock);

		pthread_mutex_lock(&modeLock);


		updateTime = getTime();
		moveTime = updateTime - startMoveTime;

		totalTime += moveTime;
		totalNodes += newNodes;

		if(moveTime) lastNps = 1000 * (newNodes / (moveTime)); //Seconds

		cout << newNodes << " moves in " << ((float)moveTime/(float)1000) << " seconds " << endl;

		newNodes = 0;

		pthread_mutex_unlock(&modeLock);
	}

	if(totalTime > 0) {
		totalNps = 1000 * (totalNodes / (totalTime)); //Seconds
		cout << "Benchmark: " << totalNodes << " moves in " << ((float)totalTime/(float)1000) << " seconds " << totalNps/1000 << " kNps" << endl;
	} else {
		cout << "No tests in file" << endl;
	}

	pthread_mutex_lock(&modeLock);
		mode = MODE_FORCE;
	pthread_mutex_unlock(&modeLock);
}

void Game::benchMove(char *filename){
	char buffer[256];
	int nodesPerUpdate = 100000;
	unsigned long long int updateTime;
	int moveTime;
	int totalNps;
	int secondsPerPosition = 1;
	int nodesUntilUpdate;
	int newNodes = 0;

	unsigned long long totalTime = 0;
	unsigned long long totalNodes =0;

	ifstream in(filename);

	pthread_mutex_lock(&modeLock);
		mode = MODE_TEST;
	pthread_mutex_unlock(&modeLock);

	pthread_mutex_lock(&boardLock);

	pthread_mutex_unlock(&boardLock);


	while(in.getline(buffer, 256)) {
		if(strlen(buffer) < 2) continue;

		board->reset();
		board->setFen(buffer);

		pthread_mutex_lock(&modeLock);
		lastUpdateOurTime = getTime();

		timeAllocated = secondsPerPosition * 1000; //100 seconds per move
		startMoveTime = lastUpdateOurTime;

		pthread_mutex_unlock(&modeLock);

		pthread_mutex_lock(&boardLock);
			nodesUntilUpdate = nodesPerUpdate;
			board->generateMoves();

			while(1) {
				while(nodesUntilUpdate > 0) {
					for(int i=0; i<board->numMoves; i++) {
						board->makeMove(board->moveStack[i]);
						board->retractMove(board->moveStack[i]);

//						board->inCheck();
					}

					newNodes+=board->numMoves;
					nodesUntilUpdate -= board->numMoves;
				}

				updateTime = getTime();
				if((updateTime - startMoveTime)  > (unsigned)timeAllocated) break;

				nodesUntilUpdate = nodesPerUpdate;
			}


		pthread_mutex_unlock(&boardLock);

		pthread_mutex_lock(&modeLock);


		updateTime = getTime();
		moveTime = updateTime - startMoveTime;

		totalTime += moveTime;
		totalNodes += newNodes;

		if(moveTime) lastNps = 1000 * (newNodes / (moveTime)); //Seconds

		cout << newNodes << " moves in " << ((float)moveTime/(float)1000) << " seconds " << endl;

		newNodes = 0;
		board->numMoves = 0;

		pthread_mutex_unlock(&modeLock);
	}

	if(totalTime > 0) {
		totalNps = 1000 * (totalNodes / (totalTime)); //Seconds
		cout << "Benchmark: " << totalNodes << " moves in " << ((float)totalTime/(float)1000) << " seconds " << totalNps/1000 << " kNps" << endl;
	} else {
		cout << "No tests in file" << endl;
	}

	pthread_mutex_lock(&modeLock);
		mode = MODE_FORCE;
	pthread_mutex_unlock(&modeLock);
}



void Game::printEval() {
	pthread_mutex_lock(&copyLock);
	boardCopy->printScore(boardCopy->evaluate());
	cout << endl;
	pthread_mutex_unlock(&copyLock);
}
