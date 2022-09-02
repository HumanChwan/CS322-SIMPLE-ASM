
all: asm emu

asm: asm.c
	gcc --std=c99 -pedantic -Wall asm.c -o asm

emu: emu.c
	gcc --std=c99 -pedantic -Wall emu.c -o emu