all: start main

main: task2.o
	ld -o main task2.o startup.o start.o -L/usr/lib32 -lc -T linking_script -dynamic-linker /lib32/ld-linux.so.2

task2.o: task2.c
	gcc -m32 -c task2.c -o task2.o

start: start.s
	nasm -f elf32 start.s -o start.o

.PHONY: clean

clean:
	rm -f *.o main start
