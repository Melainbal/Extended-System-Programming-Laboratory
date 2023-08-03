#include <stdio.h>
#include <unistd.h>
#include <elf.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

struct stat fd_stat; // struct to hold all necessary data of fd
extern int startup(int argc, char **argv, void (*start)());
void* map_start;

void print_phdr_info(Elf32_Phdr *phdr, int index) {
    if (index == 0) {
        printf("%-6s %-8s %-10s %-10s %-7s %-7s %-3s %s %s %s\n",
                "Type", "Offset", "VirtAddr", "PhysAddr",
                "FileSiz", "MemSiz", "Flg", "Align",
                "ProtFlags", "MapFlags");
    }

    const char *type;
    const char *prot_flags;
    const char *map_flags;

    switch (phdr->p_type) {
        case PT_NULL:
            type = "NULL";
            prot_flags = "";
            map_flags = "";
            break;
        case PT_LOAD:
            type = "LOAD";
            prot_flags = "R";
            map_flags = "MAP_PRIVATE";
            if (phdr->p_flags & PF_X) {
                prot_flags = "RX";
            }
            if (phdr->p_flags & PF_W) {
                prot_flags = "RW";
            }
            break;
        case PT_DYNAMIC:
            type = "DYNAMIC";
            prot_flags = "";
            map_flags = "";
            break;
        case PT_INTERP:
            type = "INTERP";
            prot_flags = "";
            map_flags = "";
            break;
        case PT_NOTE:
            type = "NOTE";
            prot_flags = "";
            map_flags = "";
            break;
        case PT_SHLIB:
            type = "SHLIB";
            prot_flags = "";
            map_flags = "";
            break;
        case PT_PHDR:
            type = "PHDR";
            prot_flags = "";
            map_flags = "";
            break;
        case PT_TLS:
            type = "TLS";
            prot_flags = "";
            map_flags = "";
            break;
        case PT_NUM:
            type = "NUM";
            prot_flags = "";
            map_flags = "";
            break;
        case PT_LOOS:
            type = "LOOS";
            prot_flags = "";
            map_flags = "";
            break;
        case PT_GNU_EH_FRAME:
            type = "GNU_EH_FRAME";
            prot_flags = "";
            map_flags = "";
            break;
        case PT_GNU_STACK:
            type = "GNU_STACK";
            prot_flags = "";
            map_flags = "";
            break;
        default:
            type = "UNKNOWN";
            prot_flags = "";
            map_flags = "";
            break;
    }

    printf("%-6s 0x%08x 0x%08x 0x%08x 0x%05x 0x%05x %c%c%c %08x %s %s\n",
           type,
           phdr->p_offset,
           phdr->p_vaddr,
           phdr->p_paddr,
           phdr->p_filesz,
           phdr->p_memsz,
           (phdr->p_flags & PF_R) ? 'R' : '-',
           (phdr->p_flags & PF_W) ? 'W' : '-',
           (phdr->p_flags & PF_X) ? 'X' : '-',
           phdr->p_align,
           prot_flags,
           map_flags);
}


int flagToInt(int flg){
    switch (flg){
        case 0x000: return 0;
        case 0x001: return PROT_EXEC;
        case 0x002: return PROT_WRITE;
        case 0x003: return PROT_EXEC | PROT_EXEC;
        case 0x004: return PROT_READ;
        case 0x005: return PROT_READ | PROT_EXEC;
        case 0x006: return PROT_READ | PROT_WRITE;
        case 0x007: return PROT_READ | PROT_WRITE | PROT_EXEC;
        default:return -1;
    }
}

void load_phdr(Elf32_Phdr *phdr, int temp_fd){
    if(phdr -> p_type != PT_LOAD)
        return;
    void *vadd = (void *)(phdr -> p_vaddr&0xfffff000);
    int offset = phdr -> p_offset&0xfffff000;
    int padding = phdr -> p_vaddr & 0xfff;
    int convertedFlag = flagToInt(phdr -> p_flags);
    void* temp;
    if ((temp = mmap(vadd, phdr -> p_memsz + padding , convertedFlag, MAP_FIXED | MAP_PRIVATE, temp_fd, offset)) == MAP_FAILED ) {
      perror("mmap fail");
      exit(-4);
   }
}

int LoadingFile(char *filename){
    int temp_fd;
    if((temp_fd = open(filename, O_RDWR)) < 0) {
      perror("error opening");
      exit(-1);
   }
    if(fstat(temp_fd, &fd_stat) != 0 ) {
      perror("stat fail");
      exit(-1);
   }
    if ((map_start = mmap(0, fd_stat.st_size, PROT_WRITE, MAP_PRIVATE , temp_fd, 0)) == MAP_FAILED ) {
      perror("mmap fail here");
      exit(-4);
   }
    return temp_fd;
}

int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *,int), int arg){
    Elf32_Ehdr *header = (Elf32_Ehdr *) map_start;
    Elf32_Off offset = header->e_phoff;
    Elf32_Half num = header->e_phnum;
    Elf32_Half size = header->e_phentsize;

    for (size_t i = 0; i < num; i++){       
        Elf32_Phdr* entry = map_start + offset + (i * size);   
        func(entry,arg);
    }
    return 0;
}

int main(int argc, char ** argv){
    if(argc < 2){
        printf("need file name");
        exit(1);
    }
    int fd  = LoadingFile(argv[1]);
    Elf32_Ehdr *header = (Elf32_Ehdr *) map_start;
    foreach_phdr(map_start, load_phdr, fd);
    startup(argc-1, argv+1, (void *)(header->e_entry));
}

