#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MSG_SIZE 6  // length of message to be sent

int main() {
    int p[2];
    char msg[] = "hello";

    // creating the pipe
    if (pipe(p) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork(); 

    if (pid == -1) {  // fork failed
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {  // child process
        close(p[0]); 
        write(p[1], msg, MSG_SIZE);  // write the message
        close(p[1]); 
        exit(EXIT_SUCCESS);
    } else {  // parent process
        close(p[1]); 
        read(p[0], msg, MSG_SIZE);  //read the message
        printf("%s\n", msg);  // print the message
        close(p[0]); 
        exit(EXIT_SUCCESS);
    }
}