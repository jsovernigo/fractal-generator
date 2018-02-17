CC =gcc
CFLAGS =-g -Wall -std=c99
LIBS =-lpng -lm

all: build

build:
	$(CC) $(CFLAGS) -o pngwriter mywriter.c $(LIBS)
