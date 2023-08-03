#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char *map(char *array, int array_length, char (*f)(char))
{
    char *mapped_array = (char *)(malloc(array_length * sizeof(char)));
    /* TODO: Complete during task 2.a */
    for (int i = 0; i < array_length; i++)
    {
        mapped_array[i] = (*f)(array[i]);
    }
    return mapped_array;
}

/* Ignores c, reads and returns a character from stdin using fgetc. */
char my_get(char c)
{
    return fgetc(stdin);
}

/* If c is a number between 0x20 and 0x7E, cprt prints the character of ASCII value c followed by a new line.
Otherwise, cprt prints the dot ('.') character.
After printing, cprt returns the value of c unchanged. */
char cprt(char c)
{
    if (c >= 0x20 && c <= 0x7E)
        printf("%c\n", c);
    else
    {
        printf("%c\n", '.');
    }
    return c;
}

/* Gets a char c and returns its encrypted form by adding 1 to its value.
 If c is not between 0x20 and 0x7E it is returned unchanged */
char encrypt(char c)
{
    if (c >= 0x20 && c <= 0x7E)
        return c + 1;
    else
        return c;
}

/* Gets a char c and returns its decrypted form by reducing 1 from its value.
 If c is not between 0x20 and 0x7E it is returned unchanged */
char decrypt(char c)
{
    if (c >= 0x20 && c <= 0x7E)
        return c - 1;
    else
        return c;
}

/* xprt prints the value of c (if between 0x20 and 0x7E, otherwise it prints the dot ('.') character) in a
hexadecimal representation followed by a new line, and returns c unchanged. */
char xprt(char c)
{
    if (c >= 0x20 && c <= 0x7E)
        printf("%x\n", c);
    else
    {
        printf("%c\n", '.');
    }
    return c;
}

struct fun_desc
{
    char *name;
    char (*fun)(char);
};

int main(int argc, char const *argv[])
{
    char *carray = (char *)(malloc(5 * sizeof(char)));
    carray[0] = '\0';
    struct fun_desc menu[] = {{"my_get", my_get}, {"cprt", cprt}, {"encrypt", encrypt}, {"decrypt", decrypt}, {"xprt", xprt}, {NULL, NULL}};
    int bound = sizeof(menu) / sizeof(menu[0]) - 1;

    while (1)
    {
        printf("Please elect operation from the following menu: (Ctrl^D for exit)\n");
        int j = 0;
        while (menu[j].fun != NULL)
        {
            printf("%d) %s\n", j, menu[j].name);
            j++;
        }

        char buffer[256];
        char* p = fgets(buffer, 256, stdin);
        if(p==NULL)
            break;
        int num = buffer[0] - 48;
        printf("Option : %d\n\n", num);

        if (num >= 0 && num <= bound && buffer[2] == '\0')
        {
            printf("Within bounds\n");
            carray = map(carray, 5, menu[num].fun);
            printf("DONE.\n\n");
        }
        else
        {
            printf("Not Within bounds\n");
            break;
        }
    }
    free(carray);
    return 0;
}
