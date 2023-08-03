all: myshell looper mypipe

myshell: looper myshell.o LineParser.o
	gcc -m32 -g -o myshell myshell.o LineParser.o

myshell.o: myshell.c 
	gcc -g -m32 -Wall -c -o myshell.o myshell.c

LineParser.o: LineParser.c LineParser.h
	gcc -g -m32 -Wall -c -o LineParser.o LineParser.c

looper: looper.o
	gcc -m32 -g -o looper looper.o

looper.o: looper.c
	gcc -g -m32 -Wall -c -o looper.o looper.c

mypipe: mypipe.o
	gcc -m32 -g -o mypipe mypipe.o

mypipe.o: mypipe.c
	gcc -g -m32 -Wall -c -o mypipe.o mypipe.c

clean:
	rm -f *.o looper myshell mypipe
