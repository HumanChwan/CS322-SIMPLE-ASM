CC=gcc
CFLAGS=-std=c89 -pedantic -Wall -W -Wpointer-arith -Wwrite-strings -Wstrict-prototypes

BIN=bin/asm bin/emu

all: $(BIN)
	@echo "\`\033[0;32masm\033[0;0m\` and \`\033[0;32memu\033[0;0m\` can be found in \033[0;34m./bin\033[0;0m directory"

bin/asm: src/asm.c
	$(CC) $(CFLAGS) src/asm.c -o bin/asm

bin/emu: src/emu.c
	$(CC) $(CFLAGS) src/emu.c -o bin/emu

clean:
	rm -f bin/asm bin/emu  