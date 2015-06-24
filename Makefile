# Makefile for pcalc
# Copyright 2015 Jacob Wahlgren

TARGET=pcalc
CC=gcc
override CFLAGS:=-g --std=c99 -Wall -Wpedantic -Wno-parentheses $(CFLAGS)
DEPS=pcalc.h stack.h pcalc_prefix.h
OBJ=pcalc.o stack.o main.o d_array.o

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

