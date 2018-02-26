CC =gcc
CFLAGS =-Wall -std=c99 -O3
LIBS =-lpng -lm

all: build

build:
	$(CC) $(CFLAGS) -o fractalwriter fractals.c $(LIBS)

clean:
	rm fractalwriter
