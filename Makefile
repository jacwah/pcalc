# Makefile for pcalc

TARGET=pcalc
CC=gcc
CFLAGS=-g -Wall
DEPS=
OBJ=pcalc.o

.PHONY: default all clean

default: $(TARGET)
all: default

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	-rm -f *.o
	-rm -f $(TARGET)
	-rm -rf $(TARGET).dSYM
