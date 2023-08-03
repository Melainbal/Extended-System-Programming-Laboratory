all : menu_map

menu_map : menu_map.c
	gcc -g -m32 -Wall menu_map.c -o menu_map

.PHONY : clean

clean : 
	rm -f *.o menu_map
