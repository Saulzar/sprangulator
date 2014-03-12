
#include "bitboard.h"
#include "board.h"
#include "string.h"
#include "iostream.h"
#include "transposition.h"
#include "xboardplayer.h"
#include "game.h"


Transposition *trans;
int hits = 0;
int misses = 0;


int main(int argc, char **argv) {

/*	for(int i=0; i<64; i++) {
		bitboard b = SQUARE(i);
		cout << i << " "   <<  Bitboard::firstSet(b) << endl;

	} */
//	exit(0);

	Bitboard::initialise();	//Precalculate nesescary bitboard lookups
	Transposition::saveKeys("settings/keys");
	Transposition::initialiseKeys("keys/keys");

	Player *p = new Player();
	p->run();



/*
	Board *b = new Board(8*1024*1024, NULL);

	b->setFen("r1bqkbnr/pppp1ppp/2n5/4N3/4P3/8/PPPP1PPP/RNBQKB1R b KQkq - 0 3");
	b->setFen("r1bqkb1r/pppp1ppp/2n2n2/4N3/4P3/8/PPPP1PPP/RNBQKB1R w KQkq - 1 4");
	b->setFen("r1bqkb1r/pppN1ppp/2n2n2/8/4P3/8/PPPP1PPP/RNBQKB1R b KQkq - 0 4");
	b->setFen("r1bqkb1r/pppN1ppp/2n5/8/4n3/8/PPPP1PPP/RNBQKB1R w KQkq - 0 5");
	b->setFen("r1bqkN1r/ppp2ppp/2n5/8/4n3/8/PPPP1PPP/RNBQKB1R b KQkq - 0 5");


	b->setFen("4k2K/8/8/8/5q2/8/1p6/5R2 w - - 0 1");
	b->print();



	cout << b->quiescence(-MATE, MATE) << endl;

	b->printLine();*/

	/*b->generateMoves();
	cout << b->numMoves << endl;

	for(int i=0; i<b->numMoves; i++) {
		int source = (b->moveStack[i] >> 6) & 63;
		int dest = b->moveStack[i] & 63;
		int promote = (b->moveStack[i] >> 20) & 15;

		b->printMove(b->moveStack[i]);
		cout << " "<<  b->evaluateExchange(source, dest, promote) << endl;
	}*/
	//cout << "___________" << endl;


	/*b->numMoves = b->sortGoodCaptures(b->moveStack, b->numMoves);

	for(int i=0; i<b->numMoves; i++) {
		b->printMove(b->moveStack[i]);
		cout << " " << b->sortValues[i] << endl;
	} */

	//b->setDefault();

//	b->setFen("rnb1kb1r/5pp1/ppp1qn1p/8/3PpB2/2N2N1P/PPPQBPP1/R3K2R w KQkq - 0 1");
	//b->print();

/*	b->setFen("rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2");
	b->print();*/


//	b->setFen("1q3n2/P2P3P/8/1pPK1k2/2P5/7b/6PP/8 w - b6 0 1");
	//b->setFen("K7/1RR3QQ/8/8/8/8/1rq3qr/7k w - - 0 1");

	//b->setFen("rnb1kb1r/5pp1/ppp1qn1p/8/3PpB2/2N2N1P/PPPQBPP1/R3K2R w KQkq - 0 1");

	/* b->setFen("r3r2k/6p1/7p/3N4/8/P7/1P6/KR6 w - - 0 1"); */

	//b->setFen("7k/8/5K2/5R2/8/8/8/8 w - - 0 1");

	//b->setFen("1rb4k/2NnQ2p/6pN/p3p3/4P2P/P4P2/1PP3q1/1K5R w - - 3 27");

/*	b->setFen("1rb1Q2k/2Nn3p/6pN/p3p3/4P2P/P4P2/1PP3q1/1K5R b - - 4 27");
	b->setFen("1rb1Q3/2Nn2kp/6pN/p3p3/4P2P/P4P2/1PP3q1/1K5R w - - 5 28");
	b->setFen("1rb1Q3/2Nn2kp/6p1/p3pN2/4P2P/P4P2/1PP3q1/1K5R b - - 6 28"); */

	//b->setFen("2kr4/p6p/3bp3/1pp1N3/3qPQ2/P1K5/1P5P/5R2 w - - 0 31");
	//b->setFen("2kr4/p6p/3bp3/1pp1N3/3qPQ2/P7/1PK4P/5R2 b - - 1 31");
	//b->setFen("r1bqkbnr/1pppp3/n6p/p4ppP/P4N2/8/1PPPPPP1/RNBQKB1R w KQkq g6 0 6");
	/*b->setFen("7B/8/8/8/R7/1Pn5/BrP1p3/k1K1R3 w - - 0 1"); //Mate in 3 ply
	b->setFen("8/4K3/2k5/5Q1B/bp6/8/7B/8 w - - 0 1"); //Mate in 5 ply
	b->setFen("7K/2p5/p1kP3R/p5Q1/n3N3/N7/2b5/4B2B w - - 0 1"); // Mate 5 ply
	b->setFen("b7/1Q1r1p2/2n1B3/3qP3/3pp1N1/5kPp/2NRpPpK/4B1n1 w - - 0 1"); //very complicated mate
	b->setFen("8/6P1/8/1K6/2R5/8/1k6/8 w - - 0 1"); //Promote to rook instead of queen
	b->setFen("n1bRN1B1/1Ppp1p1r/pp2k1N1/1r2p1P1/n1P1P1Kp/8/5b2/8 w - - 0 1"); //c5 mates?!
	//b->setFen("1r4k1/6p1/4p1Qp/4bP2/3q4/1P6/R1B1b2P/1KR5 b - - 0 1");
	//b->setFen("r4k2/4b2p/q3Pp2/1p1p4/3B4/2PB4/rP3Q1P/1K1R2R1 w - - 0 1");
	//b->setFen("r5k1/r4pb1/2p1p1p1/p1q1P1P1/1p1p1Q2/1P1B3R/P2B1P2/6KR w - - 0 1");

	//b->setFen("r6k/1q1n1prp/p3pN2/1p1b4/6RQ/6P1/PP2PP1P/3R2K1 w - - 0 1");
	//b->setFen("r3r1k1/pp3ppp/3b4/3q4/1n1B1P2/3B1P2/PP2Q2P/1K1R2R1 w - - 0 1");
	//b->setFen("8/p3q2k/3p4/3P1p2/2pBbR2/2P3RK/Pr1Q3P/1r6 w - - 0 1");
	//b->setFen("3rrk2/1p3p1R/q1n5/8/p2B4/P5P1/1P2PPbP/3QR1K1 w - - 0 1");
	//b->setFen("r1k2br1/pp1q1p1p/2p2Bp1/P2n4/3PB3/3Q2P1/1P3P1P/R3R1K1 w - - 0 1");*/

	//b->print();

	//b->search();

	//test(5, b);

	/*b->print();

	char input[200];
	while(cin >> input) {

		int mov = b->parseMove(input);

		if(mov!= -1 && b->testLegality(mov)) {
			b->makeMove(mov);
			b->printMove(mov);

		} else {
			cout << "Illegal move!";
		}

		cout << endl;

		b->print();
	}*/

	//b->print();

//	cout << "Hits " << hits << " Misses  " << misses <<  " Total  " <<  (hits + misses) << endl;
/*	Bitboard::print(b->occupied);
	Bitboard::print(b->wPieces);
	Bitboard::print(b->bPieces);
	Bitboard::print(b->bishopMovers);
	Bitboard::print(b->rookMovers);*/

/*	int count = 0;
	while(count < 255 && b->numMoves > 0) {

		//b->numMoves = 0;
		int mov = ( (double)rand()/ (double)RAND_MAX) * b->numMoves;

		b->printMove(b->moveStack[mov]);
		b->makeMove(b->moveStack[mov]);
		b->move = 0;
		moves[count++] = b->moveStack[mov];

		if(b->stateStack[b->depth].hashKey != b->calculateHash()) {
			cout << "Bad!" << endl;
			b->print();
			exit(0);
		} */
		//cout << endl;

/*		b->print();

		cout << "Occupied" << endl;
		Bitboard::print(b->occupied);
		cout << "White peices" << endl;
		Bitboard::print(b->wPieces);
		cout << "White knights" << endl;
		Bitboard::print(b->wPawns);*/
/*		b->generateMoves();
	}


	cout << endl;

	while(count > 0) {
		b->move = 1;
		count--;
		b->retractMove(moves[count]);
//		b->printMove(moves[count]);
	}

	cout << endl;

	b->print(); */
/*
	Bitboard::print(b->occupied);
	Bitboard::print(b->wPieces);
	Bitboard::print(b->bPieces);
	Bitboard::print(b->bishopMovers);
	Bitboard::print(b->rookMovers);
*/

//	b->print();
/*	cout << "occupied:" << endl;
	Bitboard::print(b->occupied);*/
/*
	cout << "rookmovers:" << endl;
	Bitboard::print(b->rookMovers);

	cout << "blackpieces:" << endl;
	Bitboard::print(b->bPieces);*/

	//cout << "----------------------" << endl;;

	//cout << endl;

/*	for(int i=0; i<b->numMoves; i++) {
		Board *m = new Board();
		memcpy(m, b, sizeof(Board));

		b->printMove(m->moveStack[i]);
		cout << endl;
		m->makeMove(m->moveStack[i]);
	//	m->print();
		m->retractMove(m->moveStack[i]);
		m->print();

		cout << "occupied:" << endl;
		Bitboard::print(m->occupied);

		cout << "rookmovers:" << endl;
	Bitboard::print(m->rookMovers);

	cout << "blackpieces:" << endl;
	Bitboard::print(m->bPieces);

	cout << "----------------------" << endl;;

		cout << endl;
		cout << endl;
	}


*/

	/*b->setFen("r1bqk2r/pp2ppbN/2np1np1/8/3NP3/2N1B3/PPPQBPpP/R3K1nR w KQkq - 0 1");
	b->makeMove(16186);
	b->print();*/
/*
	while(cin >> buffer) {
		cerr << buffer << endl;
	}*/

	exit(0);
}

