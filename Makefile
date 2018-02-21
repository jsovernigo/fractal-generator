CC =gcc
CFLAGS =-g -Wall -std=c99
LIBS =-lpng -lm

all: build

build:
	$(CC) $(CFLAGS) -o fractalwriter fractals.c $(LIBS)

clean:
	rm fractalwriter
