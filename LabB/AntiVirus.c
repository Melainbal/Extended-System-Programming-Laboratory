#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct virus
{
    unsigned short SigSize;
    char virusName[16];
    unsigned char *sig;
} virus;

typedef struct link link;

struct link
{
    link *nextVirus;
    virus *vir;
};

virus *readVirus(FILE *read)
{
    struct virus *virusRead = (virus *)malloc(sizeof(virus));
    int count1 = fread(virusRead, 1, 18, read);
    if(count1!=18){
        free(virusRead);
        return NULL;
    }
    virusRead->sig = (unsigned char *)malloc(virusRead->SigSize * sizeof(char));
    int count2 = fread(virusRead->sig, 1, virusRead->SigSize, read);
    if (count2 != virusRead->SigSize)
    {
        free(virusRead->sig);
        free(virusRead);
        return NULL;
    }
    return virusRead;
}

void printVirus(virus *virus, FILE *output)
{
    fprintf(output, "Virus name: %s\n", virus->virusName);
    fprintf(output, "Virus size: %d\n", virus->SigSize);
    fprintf(output, "signature:\n");
    int counter = 0;
    for (int i = 0; i < virus->SigSize; i++)
    {
        if (counter == 20)
        { // next line
            fprintf(output, "\n");
            counter = 0;
        }
        fprintf(output, "%02X ", virus->sig[i]);
        counter++;
    }
    fprintf(output, "\n\n");
}

int test1a(int argc, char const *argv[])
{

    FILE *ptr = fopen("signatures-L", "rb");
    FILE *testOutput = fopen("outPut-1a", "w");
    char magicNumber[4];
    int read = fread(magicNumber, 1, 4, ptr);
    if (read != 4)
    {
        printf("%s\n", "error reading from file");
        return -1;
    }
    if (strncmp(magicNumber, "VISL", 4) != 0)
    {
        printf("%s\n", "error openning file");
        return -1;
    }
    else
    {
        virus *tempVirus;
        while ((tempVirus = readVirus(ptr)) != NULL)
        {
            printVirus(tempVirus, testOutput);
            free(tempVirus);
            fprintf(testOutput, "\n");
        }
    }
    fclose(ptr);
    fclose(testOutput);
    return 0;
}

void list_print(link *virus_list, FILE *stream)
{
    while (virus_list != NULL)
    {
        printVirus(virus_list->vir, stream);
        virus_list = virus_list->nextVirus;
    }
    printf("%s\n\n", "");
}

link *list_append(link *virus_list, virus *data)
{
    link *newVirus = malloc(sizeof(link));
    newVirus->vir = data;
    newVirus->nextVirus = virus_list;
    return newVirus;
}

void list_free(link *virus_list)
{
    if (virus_list == NULL)
    {
        return;
    }
    list_free(virus_list->nextVirus);
    free(virus_list->vir->sig);
    free(virus_list->vir);
    free(virus_list);
}

link* quit(link *link_list, const char* name)
{
    list_free(link_list);
    return NULL;
}

link *load_signatures(link* virustList1, const char* name)
{
    quit(virustList1, name); // If some file has been loaded before
    printf("Enter a signature file name\n");
    char buffer[256];
    char *p = fgets(buffer, 256, stdin);
    if (p == NULL)
    {
        printf("%s\n","Error reading from keyboard");
        return NULL;
    }
    buffer[strcspn(buffer, "\n")] = '\0';
    FILE *file = fopen(buffer, "rb");
    char magicNumber[4];
    int read = fread(magicNumber, 1, 4, file);
    if (read != 4)
    {
        printf("%s\n", "error reading from file");
        return NULL;
    }
    if (strncmp(magicNumber, "VISL", 4) != 0)
    {
        printf("%s\n", "error openning file");
        return NULL;
    }
    else
    {
        virus *tempVirus;
        link *virus_list = NULL;

        while ((tempVirus = readVirus(file)) != NULL)
        {
            virus_list = list_append(virus_list, tempVirus);
        }
        fclose(file);
        return virus_list;
    }
}

link *printSign(link *link_list, const char* name)
{
    list_print(link_list, stdout);
    return link_list;
}

void addVirusToOutput(FILE *output, int index, virus *v)
{
    fprintf(output, "Location in file is: %d\n", index);
    fprintf(output, "Virus name: %s\n", v->virusName);
    fprintf(output, "Virus name: %d\n", v->SigSize);
    printf("Location in file is: %d\n", index);
    printf("Virus name: %s\n", v->virusName);
    printf("Virus name: %d\n", v->SigSize);
    printf("\n");
}

void detect_virus2(char *buffer, unsigned int size, link *virus_list)
{
    FILE *output = fopen("outputVirsuses", "w");
    int isVirus = 1;
    while (virus_list != NULL)
    {
        isVirus=1;
        virus *v = virus_list->vir;
        unsigned short sizeOfVirus = v->SigSize;
        unsigned char *sig = v->sig;
        for (int i = 0; i <=size-sizeOfVirus; i++)
        {
            isVirus = memcmp(buffer + i, sig, sizeOfVirus);
            if (isVirus == 0){
                addVirusToOutput(output, i, v); //a virus is found
                break;
            }
                
        }
        virus_list = virus_list->nextVirus;
    }
    fclose(output);
}

link *detect_virus1(link* virusList, const char* name){
    FILE *input = fopen(name, "rb");
    fseek(input, 0, SEEK_END);
    int length = ftell(input);
    rewind(input);
    char buffer[10000];
    int b = fread(buffer, 1, length, input);
    if (b != length)
    {
        printf("%s\n", "error reading from file");
        return NULL;
    }
    detect_virus2(buffer, length, virusList);
    fclose(input);
    return virusList;
}

void neutralize_virus(const char *fileName, int signatureOffset)
{
    char buffer[] = {0xC3};
    FILE *viruses = fopen(fileName, "r+");
    fseek(viruses, signatureOffset, SEEK_SET);
    fwrite(buffer, 1, 1, viruses);
    fclose(viruses);
}

void fix_file2(char *buffer, unsigned int size, link *virus_list, const char * fileName)
{
    int isVirus = 1;
    while (virus_list != NULL)
    {
        isVirus=1;
        virus *v = virus_list->vir;
        unsigned short sizeOfVirus = v->SigSize;
        unsigned char *sig = v->sig;
        for (int i = 0; i <=size-sizeOfVirus; i++)
        {
            isVirus = memcmp(buffer + i, sig, sizeOfVirus);
            if (isVirus == 0){
                neutralize_virus(fileName, i); //a virus is found
                break;
            }
                
        }
        virus_list = virus_list->nextVirus;
    }
}

link *fix_file1(link* virusList, const char* name){
    FILE *input = fopen(name, "rb");
    fseek(input, 0, SEEK_END);
    int length = ftell(input);
    rewind(input);
    char buffer[10000];
    int b = fread(buffer, 1, length, input);
    if (b != length)
    {
        printf("%s\n", "error reading from file");
        return NULL;
    }
    fix_file2(buffer, length, virusList, name);
    fclose(input);
    return virusList;
}

struct fun_desc
{
    char *name;
    link *(*fun)(link *, const char*);
};

int main(int argc, char const *argv[])
{
    struct fun_desc menu[] = {{"Load signatures", load_signatures}, {"Print signatures", printSign},
     {"Detect viruses", detect_virus1}, {"Fix file", fix_file1}, {"Quit", quit}, {NULL, NULL}};
    int bound = sizeof(menu) / sizeof(menu[0]) - 1;
    link *virusList = NULL;
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
        char *p = fgets(buffer, 256, stdin);
        if (p == NULL)
            break;
        int num = buffer[0] - 48;
        printf("Option : %d\n\n", num);

        if (num >= 0 && num <= bound && buffer[2] == '\0')
        {
            printf("Within bounds\n");
            virusList = menu[num].fun(virusList,argv[1]);
            if(virusList == NULL)
                break;
        }
        else
        {
            printf("Not Within bounds\n");
            break;
        }
    }
}
