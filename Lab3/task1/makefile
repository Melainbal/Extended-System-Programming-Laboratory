all: util task1a start 

task1a: start.s
	nasm -g -F dwarf -f elf32 start.s -o start.o 

util: util.o
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o

start: start.o
	ld -m elf_i386 start.o util.o -o start

.PHONY: clean

clean: 
	rm -f *.o start util task1a start