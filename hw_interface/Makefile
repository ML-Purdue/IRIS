CC=gcc
CFLAGS=-g -std=c99
LDFLAGS=-lSDL -ldl -lm -lpthread

SOURCES=videoInterface.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLES=videoTest tracker

.PHONY: all clean
.SUFFIXES: $(SUFFIXES) .cc

.cc.o:
	$(CC) -c $^

all: $(EXECUTABLES)

videoTest: $(OBJECTS) videoTest.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

tracker: videoInterface.o tracker.o 
	make -C ../applications/ histogram.o
	$(CC) -o $@ $^ ../applications/histogram.o $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf $(EXECUTABLES) *.o

