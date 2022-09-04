CC=gcc
CFLAGS=-std=c89 -pedantic -Wall -W

BIN=bin/asm bin/emu

all: $(BIN)

bin/asm: src/asm.c
	$(CC) $(CFLAGS) src/asm.c -o bin/asm

bin/emu: src/emu.c
	$(CC) $(CFLAGS) src/emu.c -o bin/emu

clean:
	rm -f bin/asm bin/emu  