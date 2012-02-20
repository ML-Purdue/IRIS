CC=gcc
CFLAGS=-g -std=c99
LDFLAGS=-lSDL -ldl -lm -lpthread

SOURCES=videoInterface.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLES=videoTest

.PHONY: all clean

all: $(EXECUTABLES)

videoTest: $(OBJECTS) videoTest.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf $(EXECUTABLES) *.o

