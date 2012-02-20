CC=gcc
CFLAGS=-g -std=gnu99
LDFLAGS=-lSDL -ldl -lm -lpthread

.PHONY: all clean

all: videoTest

videoTest: ../videoInterface.o videoTest.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf *.o

