all : task1 task4

task1 : task1.c
	gcc -g -m32 -Wall task1.c -o task1

task4 : task4.c
	gcc -g -m32 -Wall -fno-pie -fno-stack-protector task4.c -o task4

.PHONY : clean

clean : 
	rm -f *.o task1 task4
