CC= gcc
CFLAGS= -Wall -g -fopenmp -std=c99
DEPS= rsa.h utils.h
OBJ= rsa.o utils.o
LIBS= -lgmp -lm

all: rsa test

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

rsa: main.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

test: test.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

