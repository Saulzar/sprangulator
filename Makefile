DFLAGS = #-ggdb -g3 -DDEBUG  #-DDISABLE_HASH -DDEBUG_BITBOARDS
OFLAGS = -O3 
CC       =  g++-4.0
CFLAGS  = $(DFLAGS) -Wall -Wno-deprecated $(OFLAGS) #-DEXTRA_STATS
LIBS     = -lpthread

O_FILES = sprangulator.o bitboard.o board.o transposition.o functions.o xboardplayer.o game.o

.cpp.o:
	$(CC) -c $(CFLAGS) $<

sprangulator: $(O_FILES)
	$(CC) $(CFLAGS) -o sprangulator $(O_FILES) $(LIBS)


board.o: move.cpp generate.cpp search.cpp eval.cpp board.cpp exchange.cpp sort.cpp board.h
	$(CC)  -c $(CFLAGS) board.cpp

functions.o: functions.asm
	nasm functions.asm -felf -o functions.o


clean:
	rm -f sprangulator; rm -f *.o

