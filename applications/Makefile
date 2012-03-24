CC = g++ -g

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
