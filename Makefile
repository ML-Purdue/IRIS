CC=gcc
CFLAGS=-g
LDFLAGS=-lSDL

SOURCES=webcam.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLES=webcam

.PHONY: all clean

all: $(EXECUTABLES)

$(EXECUTABLES): $(OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf webcam *.o

