#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "LineParser.h"
#include "linux/limits.h"
#include "wait.h"
#include <fcntl.h>


//execute and deletes the command
void execute(cmdLine *pCmdLine){
    if(strcmp(pCmdLine->arguments[0],"quit") == 0){
        printf("bye\n");
        freeCmdLines(pCmdLine);
        exit(EXIT_SUCCESS);
    }
    printf("excutting...\n");
    //Debugger
    if (pCmdLine->argCount > 1 ) {
        for (int i=0; i<pCmdLine->argCount ; i++){
            if (strcmp(pCmdLine->arguments[i], "-d") == 0){
                fprintf(stderr,"PID: %d, executing : %s" , getpid(), pCmdLine->arguments[0]);
                break;
            }
        }
    }
    //executing

    //browse folders
    int sign = 1; // 0 for success, -1 for error
    if (strcmp(pCmdLine->arguments[0], "cd") == 0)
    {
        sign = chdir(pCmdLine->arguments[1]);
    }
    else if (strcmp(pCmdLine->arguments[0], "suspend") == 0)
    {
        sign = kill(atoi(pCmdLine->arguments[1]),SIGTSTP);
    }
    else if (strcmp(pCmdLine->arguments[0], "wake") == 0)
    {
        sign = kill(atoi(pCmdLine->arguments[1]),SIGCONT);
    }
    else if (strcmp(pCmdLine->arguments[0], "kill") == 0)
    {
        sign = kill(atoi(pCmdLine->arguments[1]),SIGTERM);
    }
    //invoke an executable
    else{
        int pId,status ;
        if((pId = fork()) == 0){
            //Child proccess

            //Redireting I/O
            if(pCmdLine->inputRedirect != NULL){
                int fdi = open(pCmdLine->inputRedirect, O_RDWR | O_CREAT, 0777);
                dup2(fdi,STDIN_FILENO); 
                close(fdi); 
            } 
            if(pCmdLine->outputRedirect != NULL){
                printf("KAKA");
                int fdo = open(pCmdLine->outputRedirect, O_RDWR | O_CREAT, 0777);
                dup2(fdo,STDOUT_FILENO);
                close(fdo);
            } 

            //invoking the executable 
            status = execvp(pCmdLine->arguments[0],pCmdLine->arguments);
            if (status == -1){
                perror("Couldn't Perform That Action");
                freeCmdLines(pCmdLine);
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }
        else {
            //Parent proccess
            if(pCmdLine->blocking == 1)
                waitpid(pId,&status,0);
            if (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_FAILURE) {
                fprintf(stderr, "execvp failed\n");
                freeCmdLines(pCmdLine);
                exit(EXIT_FAILURE);
            }
        }
    }
    if(sign==0)
        printf("succsses\n");
    freeCmdLines(pCmdLine);
}

void displayCWD(){
    char cwd[PATH_MAX];
    if(getcwd(cwd,PATH_MAX)== NULL){
        perror("couldn't get cuurent location\n");
        exit(EXIT_FAILURE);
    }
    else
        printf("%s ",cwd);
}

int main(int argc,const char **argv){
    printf("hello\n");
    char buffer[2048];
    while(1){
        displayCWD();
        char *p = fgets(buffer, 2048, stdin);
        cmdLine *com  =  parseCmdLines(p);
        execute(com);
    }
}