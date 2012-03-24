CC = gcc -g
CFLAGS=-std=gnu99 -g
LDFLAGS=-lSDL -ldl -lm -lpthread

all:test cursor.o circle histclient tracker Keylistener

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
	
# Dong's stuff
multithreadingtest: multithreadingtest.cc
	$(CC) -o multhithreadingtest multithreadingtest.cc -lpthread 
	
Keylistener: Keylistener.c cursor.o
	$(CC) -o Keylistener cursor.o Keylistener.c -lSDL -lX11 
	
mousestay: moursestay.cc
	$(CC) -o mousestay mousestay.o mousestay.cc -lSDL -lX11


clean:
	rm -f *.o SDL_test cursor_test histogram tracker Keylistener
