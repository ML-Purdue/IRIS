CC=gcc
CFLAGS=-g -std=c99
LDFLAGS=-lSDL

SOURCES=v4l2sdl.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLES=v4l2sdl

.PHONY: all clean

all: $(EXECUTABLES)

$(EXECUTABLES): $(OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf $(EXECUTABLES) $(OBJECTS)

