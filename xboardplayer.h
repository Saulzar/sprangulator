#ifndef PLAYER_H
#define PLAYER_H 1

#include "board.h"
#include "bitboard.h"
#include "time.h"
#include "const.h"
#include "regex.h"
#include "game.h"
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

extern char prompt[];

class XBoardPlayer;
class Game;
class Board;

struct Command {
	regex_t regex;
	void (XBoardPlayer::*command)(int, char**);
	char description[256];
	char pattern[256];
};


//Interface for the game to communicate with to get I/O
class XBoardPlayer {
	public:
	Command commandList[100];
	int numCommands;
	int showPrompt;
	int running;
	pthread_t gameThread;

	pthread_mutex_t outputLock; //Just to be safe - a lock before outputting text .. cout << etc..

	Game *game;

	XBoardPlayer();
	~XBoardPlayer();

 	void addCommand(void (XBoardPlayer::*command)(int, char**), char * pattern, char *description);

	void run();

	void setupCommands();

	//Server -> program commands
 	void cmd_xboard(int argc, char **args);
	void cmd_new(int argc, char **args);
	void cmd_force(int argc, char **args);

	void cmd_time(int argc, char **args);
	void cmd_otim(int argc, char **args);
	void cmd_level(int argc, char **args);

	void cmd_quit(int argc, char **args);

	void cmd_go(int argc, char **args);
	void cmd_playother(int argc, char **args);

	void cmd_draw(int argc, char **args);
	void cmd_result(int argc, char **args);

	void cmd_setboard(int argc, char **args);
	void cmd_usermove(int argc, char **args);

	void cmd_ping(int argc, char **args);
	void cmd_protover(int argc, char **args);

	void cmd_undo(int argc, char **args);
	void cmd_remove(int argc, char **args);

	void cmd_help(int argc, char **args);

	void cmd_analyze(int argc, char **args);
	void cmd_hint(int argc, char **args);

	void cmd_accept(int argc, char **args);
	void cmd_reject(int argc, char **args);
	
	void cmd_movelist(int argc, char**args);
	void cmd_printboard(int argc, char**args);

	void cmd_loadpgn(int argc, char**args);
	void cmd_bench(int argc, char**args);
	void cmd_benchg(int argc, char**args);
	void cmd_benchm(int argc, char**args);
	void cmd_bencht(int argc, char**args);

	void cmd_eval(int argc, char**args);

	//Program -> server commands
	void illegalMove(char *move);
	void error(char *command, char *reason);

	void result(int type);
	void message(char *message, int level);
	void makeMove(int move);

	void printLine();
};

#endif

