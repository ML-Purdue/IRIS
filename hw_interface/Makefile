CC=gcc
CFLAGS=-g -std=gnu99
LDFLAGS=-lSDL -ldl -lm -lpthread

SOURCES=videoInterface.c
OBJECTS=$(SOURCES:.c=.o)

.PHONY: all clean

all: $(OBJECTS)

clean:
	rm -rf *.o

