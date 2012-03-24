CC=gcc
CFLAGS=-g -std=gnu99
LDFLAGS=-lSDL

.PHONY: all clean

all: videoTest

videoTest: ../videoInterface.o videoTest.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf *.o

