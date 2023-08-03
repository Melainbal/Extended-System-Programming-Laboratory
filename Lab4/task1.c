#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct
{
    char debug_mode;
    char display_mode;
    char file_name[128];
    int unit_size;
    unsigned char mem_buf[10000];
    size_t mem_count;
} state;

void quit(state *s)
{
    if (s->debug_mode)
        printf("quitting\n");
    exit(0);
}

void print_debug_info(state *s)
{
    if (s->debug_mode == 1)
    {
        printf("Unit size: %d", s->unit_size);
        printf("Name of file: %s", s->file_name);
        printf("mem size: %d", s->mem_count);
    }
}

void toggle_debug_mode(state *s)
{
    if (s->debug_mode == 0)
    {
        s->debug_mode = 1;
        printf("Debug flag now on");
    }
    else
    {
        s->debug_mode = 0;
        printf("Debug flag now off");
    }
}

void set_file_name(state *s)
{
    printf("Enter the file name:\n");
    scanf("%s", s->file_name);
    getchar(); // Consume the newline character

    if (s->debug_mode)
    {
        fprintf(stderr, "Debug: file name set to '%s'\n", s->file_name);
    }
}

void set_unitsize(state *s)
{
    int size;
    printf("Enter the unit size (1, 2, or 4):\n");
    scanf("%d", &size);
    getchar(); // Consume the newline character

    if (size == 1 || size == 2 || size == 4)
    {
        s->unit_size = size;
        if (s->debug_mode)
        {
            printf("Debug: set size to %d\n", size);
        }
    }
    else
    {
        printf("Error: Invalid unit size. Size unchanged.\n");\
    }
}

void load_into_memory(state *s)
{
    if (strcmp(s->file_name, "") == 0)
    {
        printf("Error: File name is empty.\n");
        return;
    }

    FILE *file = fopen(s->file_name, "rb");
    if (file == NULL)
    {
        printf("Error: Failed to open file '%s' for reading.\n", s->file_name);
        quit(s);
        return;
    }

    char input[256];
    unsigned int location;
    int length;

    printf("Please enter <location> <length>:\n");
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%x %d", &location, &length);

    if (s->debug_mode)
    {
        printf("File: %s\n", s->file_name);
        printf("Location: 0x%x\n", location);
        printf("Length: %d\n", length);
    }

    fseek(file, location, SEEK_SET);
    size_t bytes_read = fread(s->mem_buf, s->unit_size, length*s->unit_size, file);
    s->mem_count = bytes_read/s->unit_size;
    printf("%d\n",s->mem_count);

    printf("Loaded %u units into memory\n", bytes_read/s->unit_size);

    fclose(file);
}

void toogle_display_mode(state *s)
{
    // Check if the display flag is currently on
    if (s->display_mode == 0)
    {
        // Turn on the display flag
        s->display_mode = 1;

        // Print a message indicating the change and the representation
        printf("Display flag now on, hexadecimal representation\n");
    }
    else
    {
        // Turn off the display flag
        s->display_mode = 0;

        // Print a message indicating the change and the representation
        printf("Display flag now off, decimal representation\n");
    }
}

void memory_display_mode(state *s)
{
    char input[256];
    unsigned int addr;
    int length;

    printf("Enter address and length:\n");
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%x %d", &addr, &length);

    if (addr == 0)
    {
        addr = (unsigned int)s->mem_buf;
    }

    printf("%s\n", (s->display_mode) ? "Hexadecimal" : "Decimal");
    printf("%s\n", (s->display_mode) ? "===========" : "=======");

    int u = s->unit_size;
    unsigned char *ptr = (unsigned char *)addr;
    for (int i = 0; i < length; i++)
    {
        unsigned int val = 0;
        for (int j = 0; j < u; j++)
        {
            val |= *(ptr + i * u + j) << (j * 8);
        }

        if (s->debug_mode)
        {
            printf("%#x\n", val);
        }
        else
        {
            if (u >= 1 && u <= 4)
            {
                static char *hex_formats[] = {"%#hhx\n", "%#hx\n", "No such unit", "%#x\n"};
                static char *dec_formats[] = {"%#hhd\n", "%#hd\n", "No such unit", "%#d\n"};
                if(s->display_mode==0){
                    printf(dec_formats[u-1],val);
                } else {
                    printf(hex_formats[u-1],val);
                }
            }
            else
            {
                printf("No such unit\n");
            }
        }
    }
}

void save_into_file(state *s)
{
    FILE *file = fopen(s->file_name, "r+");
    if (file == NULL)
    {
        printf("Error: Failed to open file for writing.\n");
        exit(1);
        return;
    }
    char input[256];
    int source_addr =0;
    int target_loc = 0;
    int length=0;;

    printf("Please enter <source-address> <target-location> <length>:\n");
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%x %x %d", &source_addr, &target_loc, &length);
    fseek(file,0L,SEEK_END);

    if (target_loc > ftell(file))
    {
        printf("Error: Target location exceeds the size of the file.\n");
        fclose(file);
        return;
    }

    fseek(file,0,SEEK_SET);
    fseek(file, target_loc, SEEK_SET);

    if (source_addr == 0)
    {
        fwrite(&s->mem_buf, s->unit_size, length, file);
    } else {
        fwrite(&source_addr, s->unit_size, length, file);
    }
    printf("Written %d units into file.\n",length);
    fclose(file);
}

// void memory_modify(state *s)
// {
//     char input[256];
//     int location =0;
//     int val=0;

//     printf("Please enter <location> <val>\n");
//     fgets(input, sizeof(input), stdin);
//     sscanf(input, "%x %x", &location, &val);

//     if (s->debug_mode)
//         fprintf(stderr, "Debug: location = 0x%x, val = 0x%x\n", location, val);

//     // Calculate the number of units based on the unit size
//     size_t num_units = sizeof(s->mem_buf) / s->unit_size;

//     // Check if the location is within the valid range
//     if (location < num_units)
//     {
//         // Calculate the starting address of the unit to modify
//         unsigned char *addr = s->mem_buf + (location * s->unit_size);

//         // Modify the unit with the new value
//         memcpy(addr, &val, s->unit_size);

//         printf("Memory modified successfully.\n");
//     }
//     else
//     {
//         printf("Invalid location.\n");
//     }
// }

void memory_modify(state* s){
    printf("insert location and value (seperated by single space): \n");
    int location = 0;
    int val = 0;
    char input[256];
    fgets(input, sizeof(input), stdin);
    sscanf(input,"%x %x", &location, &val);
    if (s->debug_mode == 1){
        printf("location: %x, val: %x\n",location, val);
    }
    memcpy(&s->mem_buf[location],&val,s->unit_size);
}

struct fun_desc
{
    char *name;
    void (*fun)(state *s);
};

int main(int argc, char const *argv[])
{
    // Declare a variable of type 'state'
    state s;

    // Initialize the members of the 'state' struct
    s.debug_mode = 0;
    s.display_mode = 0;
    s.unit_size = 1;
    s.mem_count = 0;

    struct fun_desc menu[] = {{"Toggle Debug Mode", toggle_debug_mode}, {"Set File Name", set_file_name},
     {"Set Unit Size", set_unitsize}, {"Load Into Memory", load_into_memory},
      {"Toggle Display Mode", toogle_display_mode}, {"Memory Display", memory_display_mode},
       {"Save Into File", save_into_file}, {"Memory Modify", memory_modify}, {"Quit", quit}, {NULL, NULL}};
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
        char *p = fgets(buffer, 256, stdin);
        if (p == NULL)
            break;
        int num = buffer[0] - 48;
        printf("Option : %d\n\n", num);

        if (num >= 0 && num <= bound && buffer[2] == '\0')
        {
            printf("Within bounds\n");
            menu[num].fun(&s);
            printf("DONE.\n\n");
        }
        else
        {
            printf("Not Within bounds\n");
            break;
        }
    }
    // free(&s); 
    return 0;
}
