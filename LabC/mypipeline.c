#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    int p[2];
    // creating the pipe
    if (pipe(p) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "(parent_process>forking)\n");
    pid_t pid1 = fork();
    if (pid1 == -1)
    { // fork failed
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid1 == 0)
    { // child process
        fprintf(stderr, "(child1>redirecting stdout to the write end of the pipe)\n");
        close(STDOUT_FILENO);
        dup(p[1]);
        close(p[1]);
        fprintf(stderr, "(child2>going to execute cmd: ls)\n");
        char *args[] = {"ls", "-l", NULL}; // the command to execute
        execvp(args[0], args);             // execute the command
        perror("execvp1"); // print an error message if the command fails
        exit(EXIT_SUCCESS);
    }
    else
    {
        // parent process
        fprintf(stderr, "(parent_process>created process with id: %d)\n", pid1);
        fprintf(stderr, "(parent_process>closing the write end of the pipe)\n");
        close(p[1]);
        pid_t pid2 = fork();
        if (pid2 == -1)
        { // fork failed
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid2 == 0)
        { // child process
            close(STDIN_FILENO);
            fprintf(stderr, "(child2>redirecting stdin to the read end of the pipe)\n");
            dup(p[0]);
            close(p[0]);
            fprintf(stderr, "(child2>going to execute cmd: tail)\n");
            char *args[] = {"tail", "-n", "2", NULL}; // the command to execute
            execvp(args[0], args);                    // execute the command
            perror("execvp2"); // print an error message if the command fails
            exit(EXIT_SUCCESS);
        }
        else
        {
            //parent process
            fprintf(stderr, "(parent_process>created process with id: %d)\n", pid2);
            fprintf(stderr, "(parent_process>closing the read end of the pipe)\n");
            close(p[0]);
            fprintf(stderr, "(parent_process>waiting for child processes to terminate)\n");
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
            fprintf(stderr, "(parent_process>exiting...)\n");
        }
    }
    return 0;
}
