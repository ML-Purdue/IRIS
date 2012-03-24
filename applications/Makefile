CC = gcc -g
CFLAGS=-std=gnu99 -g
LDFLAGS=-lSDL -ldl -lm -lpthread

<<<<<<< HEAD
all: multithreadingtest 

multithreadingtest: multithreadingtest.cc
	$(CC) -lpthread -o multhithreadingtest multithreadingtest.cc

cursor.o: cursor.cc cursor.h
	$(CC) -lX11 -c cursor.cc 

SDL_test: SDL_test.cc
	$(CC) -lX11 -o SDL_test SDL_test.cc 

cursor_test: cursor.o
	$(CC) -lX11 -o cursor_test cursor_test.cc cursor.o	

Keylistener: Keylistener.cc cursor.o
	$(CC) -lSDL -lX11 -o Keylistener cursor.o Keylistener.cc

circle: Keylistener.o Keylistener.cc
	$(CC) -lSDL -lX11 -o Keylistener Keylistener.o Keylistener.cc 

mousestay: moursestay.cc
	$(CC) -lSDL -lX11 -o mousestay mousestay.o mousestay.cc 

clean:
	rm -rf *.o Keylistener
=======
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
>>>>>>> 909b68d43676cd1f93b3a7f0dd9d697c499e8d5d
