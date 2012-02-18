CC = gcc -g
LDFLAGS=-lSDL -ldl -lm -lpthread -std=c99

all: histogram.o test cursor.o circle

test: SDL_test.cc
	$(CC) -lSDL -lSDL_ttf -o SDL_test SDL_test.cc

cursor.o: cursor.cc cursor.h
	$(CC) -lX11 -c cursor.cc

circle: cursor.o cursor_test.cc
	$(CC) -lX11 -o cursor_test cursor.o cursor_test.cc 

histogram.o: histogram.c histogram.h
	$(CC) -c histogram.c $(LDFLAGS) 

histclient: histclient.cc histogram.o
	$(CC) -o histclient histclient.cc histogram.o $(LDFLAGS)

clean:
	rm -f *.o SDL_test cursor_test histogram
