CC=gcc
CCFLAGS=-std=c89 -pedantic -Wall

all: asm emu

asm: asm.c
	$(CC) $(CCFLAGS) asm.c -o asm

emu: emu.c
	$(CC) $(CCFLAGS) emu.c -o emu