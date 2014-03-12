#include "board.h"
#include "bitboard.h"
#include "time.h"
#include "const.h"
#include "xboardplayer.h"
#include <regex.h>
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <iostream.h>

char prompt[] = "command>";

XBoardPlayer::XBoardPlayer() {
//	cout.setf(ios::unitbuf);
	numCommands = 0;
	showPrompt = 1;
	game = NULL;
	pthread_mutex_init(&outputLock, NULL);
}

XBoardPlayer::~XBoardPlayer() {
	if(game!= NULL) delete game;

	pthread_mutex_destroy(&outputLock);
}

 void XBoardPlayer::addCommand(void (XBoardPlayer::*command)(int, char**), char * pattern, char *description) {
 	Command *cmd = &commandList[numCommands++];

	int error = regcomp(&cmd->regex, pattern, REG_NEWLINE|REG_EXTENDED);
	if(error) {
		char cerror[256];
		regerror(error, &cmd->regex, cerror, 256);

		cout << "Error adding command " << pattern << " " << cerror << endl;
	}

	strncpy(cmd->description, description, 256);
	strncpy(cmd->pattern, pattern, 256);
	cmd->command = command;
 }

//Main controlling input loop here
void XBoardPlayer::run() {
	char buffer[256];
	regmatch_t matches[10];
	char **args;
	int argc = 0;
	int found;

	setupCommands();
	cmd_new(0, NULL);

	cout << prompt;

	args = new char*[10];
	for(int i=0; i<10; i++) {
		args[i] = new char[128];
	}

	running = 1;
	while(cin.getline(buffer, 256)) {

		found = 0;

		for(int i=0; i<numCommands; i++) {
			if(regexec(&(commandList[i].regex), buffer, 10, matches, 0) == 0) {
				argc = 0;

				for(int j=0; j<10 && matches[j].rm_so!=-1; j++) {
					strncpy(args[j], (char*)(buffer + matches[j].rm_so), 128);
					args[j][matches[j].rm_eo - matches[j].rm_so] = 0;
					argc++;
				}

				found = 1;
				(this->*commandList[i].command)(argc, (char**)args);
			}
			if(found) break;
		}

		if(!running) break;

		if(!found && strlen(buffer) > 0) {
			error(buffer, "unknown command");
		}

		if(showPrompt) {
			pthread_mutex_lock(&outputLock);
			cout << prompt;
			pthread_mutex_unlock(&outputLock);
		}
	}


	for(int i=0; i<10; i++) {
		delete args[i];
	}
	delete args;
}

	//Server -> program commands
void XBoardPlayer::cmd_xboard(int argc, char **args) {
	showPrompt = 0;

	pthread_mutex_lock(&outputLock);

	cout << endl;

	cout << "feature myname=\"The Sprangulator -0.01\" name=1" << endl;
	cout << "feature ping=1 playother=1 san=1 setboard=1 usermove=1" << endl;
	cout << "feature time=1 draw=1 reuse= 1 analyze=0 colors=0 pause=0" << endl;
	cout << "feature sigint=0 sigterm=0" << endl;
	cout << "feature done=1" << endl;
	pthread_mutex_unlock(&outputLock);

}

void XBoardPlayer::cmd_new(int argc, char **args) {
	pthread_attr_t attributes;

	if(game==NULL) {
		if(game==NULL) game = new Game(this);
	}

	if(!game->isRunning()) {
   		if(pthread_attr_init(&attributes)) {
			perror("Error initialising thread attributes");
			exit(1);
		}

		if(pthread_create(&gameThread, &attributes, (void*(*)(void*))&Game::runGame, (void *)game)) {

			perror("Error initialising thread");
			exit(1);
		}
	} else {

		cout << "Calling reset" << endl;
		game->reset();
	}

	while(!game->isRunning());
}

void XBoardPlayer::cmd_force(int argc, char **args) {
	if(game!=NULL) game->setMode(MODE_FORCE);
}

void XBoardPlayer::cmd_time(int argc, char **args) {
	int time = atoi(args[1]) * 10;  //Centi-seconds to milliseconds

	if(game!=NULL) game->setTime(time);
}

void XBoardPlayer::cmd_otim(int argc, char **args) {
	int time = atoi(args[1]) * 10;  //Centi-seconds to milliseconds

	if(game!=NULL) game->setOpponentsTime(time);
}

void XBoardPlayer::cmd_quit(int argc, char **args) {
	running = 0;
}

void XBoardPlayer::cmd_go(int argc, char **args) {
	if(game!=NULL) game->setMode(MODE_PLAY);
}

void XBoardPlayer::cmd_playother(int argc, char **args) {
	if(game!=NULL) game->setMode(MODE_PONDER);
}

void XBoardPlayer::cmd_draw(int argc, char **args) {

}

void XBoardPlayer::cmd_result(int argc, char **args) {
	//game->reset();
	game->setMode(MODE_WAIT);
}

void XBoardPlayer::cmd_setboard(int argc, char **args) {
	if(game!=NULL) game->setBoard(args[1]);
}

void XBoardPlayer::cmd_usermove(int argc, char **args) {
	if(game!=NULL) game->externalMove(args[1]);
}

void XBoardPlayer::cmd_level(int argc, char **args) {
	int movesPerControl = atoi(args[1]);
	int base = atoi(args[2]);
	int inc = atoi(args[3]);

	if(game!=NULL) game->setTimeControls(movesPerControl, base, inc);
}

void XBoardPlayer::cmd_undo(int argc, char **args) {
	if(game!=NULL) game->undo();
}

void XBoardPlayer::cmd_remove(int argc, char **args) {
	if(game!=NULL) {
		game->undo();
		game->undo();
	}
}


void XBoardPlayer::cmd_help(int argc, char **args) {

}

void XBoardPlayer::cmd_analyze(int argc, char **args) {

}

void XBoardPlayer::cmd_hint(int argc, char **args) {

}

void XBoardPlayer::cmd_ping(int argc, char **args) {
	int n = atoi(args[1]);

	pthread_mutex_lock(&outputLock);

	cout << "pong " << n << endl;

	pthread_mutex_unlock(&outputLock);
}

void XBoardPlayer::cmd_accept(int argc, char **args) {

}

void XBoardPlayer::cmd_reject(int argc, char **args) {

}

void XBoardPlayer::cmd_protover(int argc, char **args) {

}

void XBoardPlayer::cmd_movelist(int argc, char**args) {

	pthread_mutex_lock(&outputLock);

	if(game!=NULL) game->listMoves();

	pthread_mutex_unlock(&outputLock);

}

void XBoardPlayer::cmd_printboard(int argc, char**args) {
	pthread_mutex_lock(&outputLock);

	if(game!=NULL) game->printBoard();

	pthread_mutex_unlock(&outputLock);
}

void XBoardPlayer::illegalMove(char *move) {
	pthread_mutex_lock(&outputLock);

	cout << "Illegal move: " << move << endl;

	pthread_mutex_unlock(&outputLock);

}

void XBoardPlayer::error(char *command, char *reason) {
	pthread_mutex_lock(&outputLock);

	cout << "Error (" << reason << "): " << command << endl;

	pthread_mutex_unlock(&outputLock);
}

void XBoardPlayer::cmd_loadpgn(int argc, char**args) {
	if(game!=NULL) game->readPgn(args[1]);

}

void XBoardPlayer::cmd_bench(int argc, char**args) {
	if(game!=NULL) game->benchMark(args[1]);
}


void XBoardPlayer::cmd_benchg(int argc, char**args) {
	if(game!=NULL) game->benchGenerate(args[1]);
}

void XBoardPlayer::cmd_benchm(int argc, char**args) {
	if(game!=NULL) game->benchMove(args[1]);
}

void XBoardPlayer::cmd_bencht(int argc, char**args) {
	if(game!=NULL) game->benchTreeSize(args[1]);
}


void XBoardPlayer::cmd_eval(int argc, char**args) {
	pthread_mutex_lock(&outputLock);
		game->printEval();
	pthread_mutex_unlock(&outputLock);
}


void XBoardPlayer::result(int type) {
	pthread_mutex_lock(&outputLock);

	switch(type) {

		case BLACK_MATE:
			cout << "0-1 {Black mates}" << endl;
		break;

		case  WHITE_MATE:
			cout << "1-0 {White mates}" << endl;
		break;

		case DRAW_REPETITION:
			cout << "1/2-1/2 {Draw by repetition}" << endl;
		break;

		case DRAW_STALEMATE:
			cout << "1/2-1/2 {Stalemate}" << endl;
		break;

		case DRAW_50_MOVES:
			cout << "1/2-1/2 {Draw by 50 move rule}" << endl;
		break;
	}

	pthread_mutex_unlock(&outputLock);
}


void XBoardPlayer::message(char *message, int level) {
	pthread_mutex_lock(&outputLock);

	switch(level) {
		case MESSAGE_OPPONENT:
			cout << "tellopponent " << message << endl;
		break;
		case MESSAGE_OTHERS:
			cout << "tellothers " << message << endl;
		break;
		case MESAGE_ALL:
			cout << "tellall " << message << endl;
		break;
		case MESSAGE_ERROR:
			cout << "tellusererror " << message << endl;
		break;
		case MESSAGE_COMMAND:
			cout << "tellics " << message << endl;
		break;
		case MESSAGE_CONSOLE:
			cout << message << endl;
		break;
	}

	pthread_mutex_unlock(&outputLock);
}

void XBoardPlayer::makeMove(int move) {
	pthread_mutex_lock(&outputLock);

	cout << "move ";

	game->board->printMove(move);

	cout << endl;

	pthread_mutex_unlock(&outputLock);
}

void XBoardPlayer::printLine() {
	pthread_mutex_lock(&outputLock);

	cout << "tellothers ";
	game->board->printLine();
	cout << endl;

	pthread_mutex_unlock(&outputLock);
}



void XBoardPlayer::setupCommands() {
 	addCommand(&XBoardPlayer::cmd_xboard, "xboard", "Switch to xboard mode\n");
	addCommand(&XBoardPlayer::cmd_new, "new", "Reset everything to the default position \n");
	addCommand(&XBoardPlayer::cmd_force, "force", "Set the engine to play neither colour only check move legality \n");

	addCommand(&XBoardPlayer::cmd_level, "level (.*?) (.*?) (.*?)", "Set time controls <moves per control> <time> <inc> \n");
	addCommand(&XBoardPlayer::cmd_time, "time (.*?)", "Set engines time \n");
	addCommand(&XBoardPlayer::cmd_otim, "otim (.*?)", "Set opponents time \n");

	addCommand(&XBoardPlayer::cmd_go, "go", "Start the engine playing the colour to move \n");
	addCommand(&XBoardPlayer::cmd_playother, "playother", "Start the engine playing the opposite colour to the current move \n");

	addCommand(&XBoardPlayer::cmd_draw, "draw", "Offer a draw \n");
	addCommand(&XBoardPlayer::cmd_result, "result (.*?)", "Set the result of the game \n");
	addCommand(&XBoardPlayer::cmd_setboard, "setboard (.*?)", "Set the board according to a FEN string \n");

	addCommand(&XBoardPlayer::cmd_usermove, "usermove (.*?)", "Set the board according to a FEN string \n");

	addCommand(&XBoardPlayer::cmd_undo, "undo", "Undo the last move \n");
	addCommand(&XBoardPlayer::cmd_remove, "remove", "Retract the last two moves \n");

	addCommand(&XBoardPlayer::XBoardPlayer::cmd_help, "help", "Show help \n");

	addCommand(&XBoardPlayer::cmd_analyze, "analyze", "Set analysis mode \n");
	addCommand(&XBoardPlayer::cmd_hint, "hint", "Show the current line of analysis \n");
	addCommand(&XBoardPlayer::cmd_ping, "ping (.*?)", "Check to see if the other end is alive \n");

	addCommand(&XBoardPlayer::cmd_accept, "accepted", "Accept feature request \n");
	addCommand(&XBoardPlayer::cmd_reject, "rejected", "Reject feature request \n");
	addCommand(&XBoardPlayer::cmd_protover, "protover (.*?)", "Protocol version number \n");

	addCommand(&XBoardPlayer::cmd_movelist, "movelist", "Show a list of moves \n");
	addCommand(&XBoardPlayer::cmd_printboard, "printboard", "Print out the board \n");
	addCommand(&XBoardPlayer::cmd_loadpgn, "loadpgn (.*?)", "Load a pgn file\n");

	addCommand(&XBoardPlayer::cmd_bencht, "bencht (.*?)", "Benchmark performance with a series of fen positions\n");
	addCommand(&XBoardPlayer::cmd_benchm, "benchm (.*?)", "Benchmark performance with a series of fen positions\n");
	addCommand(&XBoardPlayer::cmd_benchg, "benchg (.*?)", "Benchmark performance with a series of fen positions\n");
	addCommand(&XBoardPlayer::cmd_bench, "bench (.*?)", "Benchmark performance with a series of fen positions\n");


	addCommand(&XBoardPlayer::cmd_eval, "eval", "Evaluate position\n");

	addCommand(&XBoardPlayer::cmd_quit, "quit", "Quit the engine \n");
}

