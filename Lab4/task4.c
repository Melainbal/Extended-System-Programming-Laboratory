#include <stdio.h>

int count_digits ( char* str) {
    int counter =0;
    for(int j=0; str[j]!='\0'; j++){
        if(str[j]<='9'&& str[j]>='0') counter ++;
    }
    return counter;
}

int main (int argc, char** argv) {
    count_digits(argv[1]);
    return 0;
}