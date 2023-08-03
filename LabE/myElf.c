#include <stdio.h>
#include <unistd.h>
#include <elf.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

# define buffLen 100
int debug = 0;
int menu_size = 6;
#define MAX_ELF_FILES 2
Elf32_Ehdr *header[MAX_ELF_FILES]; 
void* map_start[MAX_ELF_FILES]; 
struct stat fd_stat[MAX_ELF_FILES]; 
char* file_names[MAX_ELF_FILES];
int curr_fd[MAX_ELF_FILES] = {-1, -1}; // Initialize both descriptors as -1
int curr_file = 0; // Added variable to keep track of the current file k.

void toggleDebugMode(){
    if (debug == 0) {
        debug = 1;
        printf("Debug on\n");
    } else {
        debug = 0;
        printf("Debug off\n");
  }
}

int LoadFile(){
    printf("Enter file name: ");
    char filename[buffLen];
    int fd = -1;

    if (curr_fd[0] != -1 && curr_fd[1] != -1){
        printf("Cannot open more than 2 files.\n");
        return -1;
    }
    
    fscanf(stdin,"%s",filename);
    if((fd = open(filename, O_RDWR)) < 0) {
      perror("error in open");
      return -1;
    }
    // Determine which file descriptor to use
    if(curr_fd[0] == -1)
        curr_file = 0;
    else
        curr_file = 1;
    
    curr_fd[curr_file] = fd;

    if(fstat(fd, &fd_stat[curr_file]) != 0 ) {
      perror("stat failed");
      return -1;
    }
    if ((map_start[curr_file] = mmap(0, fd_stat[curr_file].st_size, PROT_READ | PROT_WRITE , MAP_PRIVATE, fd, 0)) == MAP_FAILED ) {
      perror("mmap failed");
      return -1;
    }
    file_names[curr_file] = strdup(filename); //returning identical malloced string
    return fd;
}

char* dataType(Elf32_Ehdr* header){
    switch (header->e_ident[5]){
    case ELFDATANONE:
        return "invalid data encoding";
        break;
    case ELFDATA2LSB:
        return "2's complement, little endian";
        break;
    case ELFDATA2MSB:
        return "2's complement, big endian";
        break;
    default:
        return "NO DATA";
        break;
    }
}
void examineFile(){
    int fd = LoadFile();
    if(fd ==-1)
        return;

    header[curr_file] = (Elf32_Ehdr *) map_start[curr_file];
    if(strncmp((char*)header[curr_file]->e_ident,(char*)ELFMAG, 4)==0){ 
        printf("Magic:\t\t\t\t %X %X %X\n", header[curr_file]->e_ident[EI_MAG1],header[curr_file]->e_ident[EI_MAG2],header[curr_file]->e_ident[EI_MAG3]);
        printf("Data:\t\t\t\t %s\n",dataType(header[curr_file]));
        printf("Enty point address:\t\t 0x%x\n",header[curr_file]->e_entry);
        printf("Start of section headers:\t %d (bytes into file)\n",header[curr_file]->e_shoff);
        printf("Number of section headers:\t %d\n",  header[curr_file]->e_shnum);
        printf("Size of section headers:\t %d (bytes)\n",header[curr_file]->e_shentsize);
        printf("Start of program headers:\t %d (bytes into file)\n",header[curr_file]->e_phoff);
        printf("Number of program headers:\t %d\n",header[curr_file]->e_phnum);
        printf("Size of program headers:\t %d (bytes)\n",header[curr_file]->e_phentsize);
   }
    else{
        printf("This is not ELF file\n");
        munmap(map_start[curr_file], fd_stat[curr_file].st_size);  //Unmap the file
        close(fd); //close file
        curr_fd[curr_file] = -1; // Reset the current file descriptor to -1
    }
}

char* sectionType(int value) {
    switch (value) {
        case SHT_NULL:return "NULL";
        case SHT_PROGBITS:return "PROGBITS";
        case SHT_SYMTAB:return "SYMTAB";
        case SHT_STRTAB:return "STRTAB";
        case SHT_RELA:return "RELA";
        case SHT_HASH:return "HASH";
        case SHT_DYNAMIC:return "DYNAMIC";
        case SHT_NOTE:return "NOTE";
        case SHT_NOBITS:return "NOBITS";
        case SHT_REL:return "REL";
        case SHT_SHLIB:return "SHLIB";
        case SHT_DYNSYM:return "DYNSYM";
        default:return "Unknown";
    }
}

void printSectionNames(){
    
    for (int file = 0; file < MAX_ELF_FILES; file++){
        if (curr_fd[file] == -1){
            return;
        }
        printf("File: %s\n", file_names[file]);
        printf("--------------------------------------------------------------------------------\n");
        printf("%-10s %-30s %-10s %-10s %-10s %-10s\n", "[k]", "Name", "Address", "Offset", "Size", "Type");
        printf("--------------------------------------------------------------------------------\n");
        
        Elf32_Ehdr *header = (Elf32_Ehdr *) map_start[file];
        Elf32_Shdr *section_header_table = (Elf32_Shdr *)(map_start[file] + header->e_shoff);
        Elf32_Shdr *shstrtab_header = &section_header_table[header->e_shstrndx];
        char * const shstrtab = map_start[file] + shstrtab_header->sh_offset;

        for(int i = 0; i < header->e_shnum; i++){
            Elf32_Shdr section_header = section_header_table[i];
            printf("%-10d %-30s 0x%-8x %-10x %-10x %-10s\n",
                    i,
                    shstrtab + section_header.sh_name,
                    section_header.sh_addr,
                    section_header.sh_offset,
                    section_header.sh_size,
                    sectionType(section_header.sh_type));
        }
    }
}

void printSymbols(){
    for (int i = 0; i < 2; i++) {
        if (curr_fd[i] != -1) {
            printf("\nFile %s\n", file_names[i]);
            printf("--------------------------------------------------------------------------------\n");
            printf("[k]\tvalue\t\tsection_k\tsection_name\tsymbol_name\n");
            printf("--------------------------------------------------------------------------------\n");
            
            Elf32_Ehdr *header = (Elf32_Ehdr *)map_start[i];
            Elf32_Shdr *section_header_table = (Elf32_Shdr *)(map_start[i] + header->e_shoff);
            Elf32_Shdr *shstrtab_header = &section_header_table[header->e_shstrndx];
            char *const shstrtab = map_start[i] + shstrtab_header->sh_offset;

            for (int j = 0; j < header->e_shnum; j++) {
                if (section_header_table[j].sh_type == SHT_SYMTAB || section_header_table[j].sh_type == SHT_DYNSYM) {
                    // If the section type is SHT_SYMTAB or SHT_DYNSYM (indicating a symbol table), then it proceeds.
                    Elf32_Sym *symtab = (Elf32_Sym *)(map_start[i] + section_header_table[j].sh_offset); //  a pointer to the symbol table
                    char *const strtab = map_start[i] + section_header_table[section_header_table[j].sh_link].sh_offset; // a pointer to the string table for this symbol table,
                    int num_of_symbols = section_header_table[j].sh_size / section_header_table[j].sh_entsize; // calculating the number of symbols in the symbol table

                    for (int k = 0; k < num_of_symbols; k++) { // going over each entry in the symbol table
                        const char *sym_name = strtab + symtab[k].st_name;  //  a pointer to its name in the string table
                        const char *sec_name;
                        char section_k[16];
                         // st_shndx field of the symbol, which is the section k
                        if (symtab[k].st_shndx == SHN_ABS) { // means the symbol has an absolute value that is not affected by relocation.
                            sec_name = "ABS";  
                            strcpy(section_k, "ABS");
                        }
                        else if (symtab[k].st_shndx == SHN_UNDEF) { // means the symbol is undefined.
                            sec_name = "UND";
                            strcpy(section_k, "UND");
                        }
                        else {
                            sec_name = shstrtab + section_header_table[symtab[k].st_shndx].sh_name;
                            sprintf(section_k, "%d", symtab[k].st_shndx);
                        }
                        printf("[%d]\t%08x\t%s\t\t%s\t\t%s\n", k, symtab[k].st_value, section_k, sec_name, sym_name);
                    }
                }
            }
        }
    }
}

Elf32_Half lookup_symbol(const char *sym_name, int file_idx) {
    Elf32_Ehdr *header = (Elf32_Ehdr *)map_start[file_idx];
    Elf32_Shdr *section_header_table = (Elf32_Shdr *)(map_start[file_idx] + header->e_shoff);

    // Locate the symbol table.
    for (int i = 0; i < header->e_shnum; i++) {
        if (section_header_table[i].sh_type == SHT_SYMTAB || section_header_table[i].sh_type == SHT_DYNSYM) {
            Elf32_Sym *symtab = (Elf32_Sym *)(map_start[file_idx] + section_header_table[i].sh_offset);
            char *const strtab = map_start[file_idx] + section_header_table[section_header_table[i].sh_link].sh_offset;
            int num_of_symbols = section_header_table[i].sh_size / section_header_table[i].sh_entsize;

            // Iterate over the symbols in the symbol table.
            for (int j = 1; j < num_of_symbols; j++) { // Skip the first dummy symbol.
                if (strcmp(strtab + symtab[j].st_name, sym_name) == 0) {
                    // If the symbol is found and it's not undefined, return its section k.
                    if (symtab[j].st_shndx != SHN_UNDEF) {
                        return symtab[j].st_shndx;
                    }
                    break; // Exit the loop once the symbol has been found.
                }
            }
            break; // Exit the loop once the symbol table has been processed.
        }
    }
    
    // Return SHN_UNDEF if the symbol is not found or is undefined.
    return SHN_UNDEF;
}

void CheckMerge() {
    if (curr_fd[0] == -1 || curr_fd[1] == -1) {
        printf("Two ELF files must be opened before merging\n");
        return;
    }
    
    // Loop over the two files.
    for (int file_idx = 0; file_idx < 2; file_idx++) {
        Elf32_Ehdr *header = (Elf32_Ehdr *)map_start[file_idx];
        Elf32_Shdr *section_header_table = (Elf32_Shdr *)(map_start[file_idx] + header->e_shoff);

        // Locate the symbol table.
        for (int i = 0; i < header->e_shnum; i++) {
            if (section_header_table[i].sh_type == SHT_SYMTAB || section_header_table[i].sh_type == SHT_DYNSYM) {
                Elf32_Sym *symtab = (Elf32_Sym *)(map_start[file_idx] + section_header_table[i].sh_offset);
                char *const strtab = map_start[file_idx] + section_header_table[section_header_table[i].sh_link].sh_offset;
                int num_of_symbols = section_header_table[i].sh_size / section_header_table[i].sh_entsize;

                // Iterate over the symbols in SYMTAB1 and check them against SYMTAB2.
                for (int j = 1; j < num_of_symbols; j++) { // Skip the first dummy symbol.
                    const char *sym_name = strtab + symtab[j].st_name;

                    // Ignore the dummy symbol with an empty string name.
                    if (strlen(sym_name) == 0) {
                        continue;
                    }

                    if (symtab[j].st_shndx == SHN_UNDEF) {
                        // If the symbol is undefined in SYMTAB1, search for it in SYMTAB2.
                        if (lookup_symbol(sym_name, 1 - file_idx) == SHN_UNDEF) {
                            printf("Error: Symbol %s undefined\n", sym_name);
                        }
                    } else {
                        // If the symbol is defined in SYMTAB1, search for it in SYMTAB2.
                        if (lookup_symbol(sym_name, 1 - file_idx) != SHN_UNDEF) {
                            printf("Error: Symbol %s multiply defined\n", sym_name);
                        }
                    }
                }
                break; // Exit the loop once the symbol table has been processed.
            }
        }
    }
}

char* find_section_name(void* map_start, Elf32_Ehdr* header, int section_idx) {
    // Check if the section k is valid
    if (section_idx < 0 || section_idx >= header->e_shnum) {
        printf("Invalid section k\n");
        return NULL;
    }

    // Get the section header table
    Elf32_Shdr* section_header_table = (Elf32_Shdr*)(map_start + header->e_shoff);

    // Get the section header string table
    Elf32_Shdr* shstrtab_header = &section_header_table[header->e_shstrndx];
    char* shstrtab = (char*)(map_start + shstrtab_header->sh_offset);

    // Get the name of the section
    char* section_name = shstrtab + section_header_table[section_idx].sh_name;

    return section_name;
}

Elf32_Shdr* getSection(const char* name, int numOfSecHead, Elf32_Shdr* section_header_table, char* shstrtab) {
    // Iterate over the section headers to find the section with the provided name.
    for (int i = 0; i < numOfSecHead; i++) {
        const char* section_name = shstrtab + section_header_table[i].sh_name;

        if (strcmp(name, section_name) == 0) {
            // Return a pointer to the section header if found.
            return &section_header_table[i];
        }
    }

    // Return NULL if the section was not found.
    return NULL;
}

Elf32_Sym* getSymbol(const char* name, int size, Elf32_Sym* symbols, char* names_Sym) {
    // Calculate the number of symbols
    int num_of_symbols = size / sizeof(Elf32_Sym);

    // Iterate over the symbols
    for (int i = 0; i < num_of_symbols; i++) {
        const char* symbol_name = names_Sym + symbols[i].st_name;

        if (strcmp(name, symbol_name) == 0) {
            // Return a pointer to the symbol if found.
            return &symbols[i];
        }
    }

    // Return NULL if the symbol was not found.
    return NULL;
}

void copySectionSymtable(FILE* mergedFile, Elf32_Shdr *section_header_table1, Elf32_Shdr *section_header_table2,Elf32_Sym* symbols1, Elf32_Sym* symbols2,
 char* names_Sym1,char* names_Sym2,char* shstrtab2,char* shstrtab1, int size_Sym1, int size_Sym2, int num_sec_header1, int k ){
    int size = section_header_table1[k].sh_size / section_header_table1[k].sh_entsize;
    Elf32_Sym symTabNew[size];

    memcpy((char*)symTabNew, (char*)(section_header_table1[k].sh_offset + map_start[0]), section_header_table1[k].sh_size);
    
    for (int i = 0; i < size_Sym1; i++) {
        if (symbols1[i].st_shndx == SHN_UNDEF) {
            Elf32_Sym* sym = getSymbol(&names_Sym1[symbols1[i].st_name], size_Sym2, symbols2, names_Sym2);
            if (sym != NULL) {
                Elf32_Shdr* sec = getSection(&shstrtab2[section_header_table2[sym->st_shndx].sh_name], num_sec_header1, section_header_table1, shstrtab1);
                symTabNew[i].st_value = sym->st_value;
                if (sec != NULL) {
                    symTabNew[i].st_shndx = sec - section_header_table1;
                }
            }
        }
    }   
    fwrite((char*)(symTabNew), 1, section_header_table1[k].sh_size,mergedFile);
}

void mergeSections (FILE* mergedFile, Elf32_Shdr *section_header_table1, Elf32_Shdr *section_header_table2,Elf32_Shdr *shdr_new, 
int num_sec_header2,char* shstrtab2,char* shstrtab1, int k){
    fwrite((char*)(map_start[0] + section_header_table1[k].sh_offset), 1, section_header_table1[k].sh_size,mergedFile);
    Elf32_Shdr* sec = getSection(&shstrtab1[section_header_table1[k].sh_name], num_sec_header2, section_header_table2, shstrtab2);
    if (sec != NULL) {
        shdr_new[k].sh_size += sec->sh_size;
        shdr_new[k].sh_offset += sec->sh_offset;
        fwrite((char*)(map_start[1] + sec->sh_offset), 1, sec->sh_size,mergedFile);
    }
}

int getSymbolTable(char* map_start, Elf32_Ehdr *header, Elf32_Shdr *section_header_table, Elf32_Sym** symbols, char** names_Sym, int* size) {
    int num_sec_header = header->e_shnum;
    for (int i = 0; i < num_sec_header; i++) {
        if (section_header_table[i].sh_type == SHT_SYMTAB || section_header_table[i].sh_type == SHT_DYNSYM) {
            if (*symbols == NULL) {
                *symbols = (Elf32_Sym*) (map_start + section_header_table[i].sh_offset);
                *names_Sym = (char*) (map_start + section_header_table[section_header_table[i].sh_link].sh_offset);
                *size = section_header_table[i].sh_size / section_header_table[i].sh_entsize;
            }
            else {
                fprintf(stderr, "Error: More than one symbol table in the file. \n");
                return -1;
            }
        }
    }
    return 0; // Success
}

void mergeFiles() {

    if (curr_fd[0] == -1 || curr_fd[1] == -1) {
        printf("Two ELF files must be opened before merging\n");
        return;
    }
    
    FILE* mergedFile = fopen("out.ro", "w");
    if (!mergedFile) {
        printf("Failed to open merged file\n");
        return;
    }

    Elf32_Ehdr *header1 = (Elf32_Ehdr *)map_start[0];
    Elf32_Ehdr *header2 = (Elf32_Ehdr *)map_start[1];

    //section header string tables 
    Elf32_Shdr *section_header_table1 = (Elf32_Shdr *)(map_start[0] + header1->e_shoff);
    Elf32_Shdr *section_header_table2 = (Elf32_Shdr *)(map_start[1] + header2->e_shoff);

    Elf32_Sym* symbols1 = NULL;
    Elf32_Sym* symbols2 = NULL;

    char* shstrtab1 = (char*)(map_start[0] + section_header_table1[header1->e_shstrndx].sh_offset);
    char* shstrtab2 = (char*)(map_start[1] + section_header_table2[header2->e_shstrndx].sh_offset);

    //symbols1 and symbols2 are populated with pointers to the symbol tables of the two files.
    //names_Sym1 and names_Sym2 are populated with pointers to the symbol names.
    char* names_Sym1;
    int size_Sym1;
    if(getSymbolTable(map_start[0], header1, section_header_table1, &symbols1, &names_Sym1, &size_Sym1) == -1) {
        return;
    }

    char* names_Sym2;
    int size_Sym2;
    if(getSymbolTable(map_start[1], header2, section_header_table2, &symbols2, &names_Sym2, &size_Sym2) == -1) {
        return;
    }

    int num_sec_header1 = header1->e_shnum;
    int num_sec_header2 = header2->e_shnum;

    //declaring an array that will hold the section headers for the merged file
    Elf32_Shdr shdr_new[num_sec_header1];

    // writes the ELF header of the first file into the mergedFile
    fwrite((char*) header1, 1, header1->e_ehsize, mergedFile); 

    // copies the section headers of the first file into shdr_new
    memcpy((char*)shdr_new, (char*)section_header_table1, num_sec_header1 * header1->e_shentsize); 
    
    for(int p = 0; p < num_sec_header1; p++) {
        shdr_new[p].sh_offset = ftell(mergedFile);
        char* name1 = find_section_name(map_start[0], header1, p);
        
        if (strcmp(name1, ".symtab") == 0) { // Copy over  ".symtab" without changes
            copySectionSymtable(mergedFile,section_header_table1,section_header_table2,symbols1,symbols2,
            names_Sym1,names_Sym2,shstrtab2,shstrtab1,size_Sym1,size_Sym2,num_sec_header1,p);
        }
        else if (strcmp(name1, ".data") == 0 || strcmp(name1, ".rodata") == 0 || strcmp(name1, ".text") == 0) { // Merge sections: ".text", ".data", ".rodata".
            mergeSections(mergedFile,section_header_table1,section_header_table2,shdr_new,num_sec_header2,shstrtab2,shstrtab1,p);
        }
        else if (strcmp(name1, ".shstrtab") == 0) { // Copy over ".shstrtab" without changes
            fwrite((char*)(map_start[0] + section_header_table1[p].sh_offset), 1, section_header_table1[p].sh_size, mergedFile);
        }
        else { // Relocation sections without changes
            fwrite((char*)(map_start[0] + section_header_table1[p].sh_offset), 1, section_header_table1[p].sh_size, mergedFile);
        }
    }


    //Update the "e_shoff" field in ELF header to point to the new location of the section header table.
    int offset = ftell(mergedFile);
    fwrite((char*)(shdr_new), 1, num_sec_header1 * header1->e_shentsize, mergedFile);
    fseek(mergedFile, 32, SEEK_SET); 
    fwrite((char*)(&offset), 1, 4, mergedFile);
    fclose(mergedFile);
}

void quit(){
    if (debug) printf("quitting\n");
    for(int file = 0; file < MAX_ELF_FILES; file++){
        if(curr_fd[file] != -1){
            if (munmap(map_start[file], fd_stat[file].st_size) == -1) {
                perror("munmap failed");
            }
            close(curr_fd[file]);
        }
        if(file_names[file] != NULL){
            free(file_names[file]);
            file_names[file] = NULL;
        }
    }
    exit(0);
}

typedef struct {
  char *name;
  void (*fun)();
}fun_desc;

int main(int argc, char const *argv[])
{

    fun_desc menu[] = {{"Toggle Debug Mode",toggleDebugMode},{"Examine ELF File",examineFile},
                    {"Print Section Names",printSectionNames},{"Print Symbols",printSymbols},
                    {"Check Files for Merge",CheckMerge},{"Merge ELF Files",mergeFiles},{"Quit",quit}, {NULL, NULL}};
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
        int num;
        printf("Option: ");
        int numRead = scanf("%d", &num);

        if (numRead == 1) {
            if (num >= 0 && num <= bound) {
                printf("Within bounds\n");
                menu[num].fun();
                printf("DONE.\n\n");
            } else {
                printf("Not within bounds\n");
                break;
            }
        } else if (numRead == EOF) {
            break;
        } else {
            printf("Invalid input\n");
            break;
        }
    }
    return 0;
}