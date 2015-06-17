# Makefile for pcalc
# Copyright 2015 Jacob Wahlgren

TARGET=pcalc
CC=gcc
override CFLAGS:=-g -Wall -Wpedantic -Wno-logical-op-parentheses $(CFLAGS)
DEPS=pcalc.h stack.h
OBJ=pcalc.o stack.o main.o

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
