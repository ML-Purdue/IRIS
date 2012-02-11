CC=gcc
CFLAGS=-g -std=c99
LDFLAGS=-lSDL -ldl -lm -lpthread

SOURCES=videoInterface.c videoTest.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLES=videoTest

.PHONY: all clean

all: $(EXECUTABLES)

$(EXECUTABLES): $(OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf $(EXECUTABLES) $(OBJECTS)

