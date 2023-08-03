all: util main 

main: main.o util.o start.o 
	ld -m elf_i386 main.o util.o start.o  -o main

main.o: main.c
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector main.c -o main.o

util: util.o
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o

start.o: start.s
	nasm -f elf32 start.s -o start.o

.PHONY: clean

clean: 
	rm -f *.o util main 