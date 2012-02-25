CC = gcc -g
CFLAGS=-std=gnu99 -g
LDFLAGS=-lSDL -ldl -lm -lpthread

all: histclient histogram.o cursor.o circle tracker

test: SDL_test.c
	$(CC) -lSDL -lSDL_ttf -o SDL_test SDL_test.c $(CFLAGS)

cursor.o: cursor.c cursor.h
	$(CC) -c cursor.c $(CFLAGS) -lX11 

circle: cursor.o cursor_test.c
	$(CC) -o cursor_test cursor.o cursor_test.c $(CFLAGS) -lX11 -lm 

histogram.o: histogram.c histogram.h
	$(CC) -c histogram.c $(LDFLAGS) $(CFLAGS)

histclient: histclient.c histogram.o
	$(CC) -o histclient histclient.c histogram.o $(LDFLAGS) $(CFLAGS)

tracker: tracker.o histogram.o ../hw_interface/videoInterface.o
	$(CC) -o tracker $^ $(LDFLAGS)  $(CFLAGS)

clean:
	rm -f *.o SDL_test cursor_test histogram tracker
