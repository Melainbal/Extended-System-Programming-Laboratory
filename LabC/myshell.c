#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "LineParser.h"
#include "linux/limits.h"
#include "wait.h"
#include <fcntl.h>

#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0
#define HISTLEN 20

char *history[HISTLEN];
int hist_count = 0;
int hist_newest = -1;
int hist_oldest = -1;

void addToHistory(char *input){
     // allocate space for the new entry
    char* entry = (char*) malloc(2048 * sizeof(char));
    strcpy(entry, input);

    // insert the new entry
    if(hist_count == 0){
        hist_oldest = 0;
    }
    if (hist_count < HISTLEN) {
        hist_count++;
    } else {
        free(history[hist_oldest]);
        hist_oldest = (hist_oldest + 1) % HISTLEN;
    }
    history[(hist_newest + 1) % HISTLEN] = entry;
    hist_newest = (hist_newest + 1) % HISTLEN;
}

void freeHistory(){
    for(int i=0; i<HISTLEN; i ++) {
        free(history[i]);
    }
}

void print_history() {
    int i;
    for (i = hist_oldest; i < hist_oldest + hist_count; i++) {
        printf("%d: %s\n", i - hist_oldest + 1, history[i % HISTLEN]);
    }
}

char *getStatus(int status)
{
    switch (status)
    {
    case TERMINATED:
        return "Terminated";
    case RUNNING:
        return "Running";
    case SUSPENDED:
        return "Suspended";
    default:
        return "Unknown status";
    }
}

typedef struct process
{
    cmdLine *cmd;         /* the parsed command line*/
    pid_t pid;            /* the process id that is running the command*/
    int status;           /* status of the process: RUNNING/SUSPENDED/TERMINATED */
    struct process *next; /* next process in chain */
} process;

void addProcess(process **process_list, cmdLine *cmd, pid_t pid)
{
    process *newProcess = (process *)malloc(sizeof(process));
    newProcess->cmd = cmd;
    newProcess->pid = pid;
    newProcess->next = *process_list;
    newProcess->status = 1; // default
    *process_list = newProcess;
}

void updateProcessList(process **process_list)
{
    if (process_list != NULL)
    {
        process *p = *process_list;
        while (p != NULL)
        {
            int status;
            int result = waitpid(p->pid, &status, WNOHANG | WUNTRACED | WCONTINUED);
            if (result == -1)
            {
                // Process not found, assume it has terminated
                p->status = TERMINATED;
                p = p->next;
            }
            else if (result == 0)
            {
                // Process is still running
                p = p->next;
            }
            else
            {
                // Process has exited
                if (WIFSTOPPED(status))
                {
                    p->status = SUSPENDED;
                }
                else if (WIFCONTINUED(status))
                {
                    p->status = RUNNING;
                }
                else if (WIFSIGNALED(status))
                {

                    p->status = TERMINATED;
                }
                else
                {
                    p->status = TERMINATED;
                }
                p = p->next;
            }
        }
    }
}

void printProcess(process *p)
{
    if(p!=NULL){ 
        char *c = getStatus(p->status);
        printf("%d          %s          %s\n", p->pid, p->cmd->arguments[0], c);
    }
}

void deleteProcess(process *toDelete, process *prev){
    if (prev != NULL) prev->next = toDelete->next;
    freeCmdLines(toDelete->cmd);
    free(toDelete);
}

void printProcessList(process **process_list){
    updateProcessList(process_list);
    printf("PID          Command          STATUS\n");
    if (*process_list != NULL){
        process *curr = *process_list;
        process *prev = NULL;
        while (curr != NULL)
        {
            printProcess(curr);
            if (curr->status == TERMINATED) {
                if(prev == NULL){ 
                    *process_list = curr->next;
                }
                process *toDelete = curr;
                curr = curr->next;
                deleteProcess(toDelete, prev);
                
            }
            else{ 
                prev = curr;
                curr = curr->next;
            }
        }
    }
}

void freeProcessList(process *process_list){
    if (process_list !=NULL){  
        freeProcessList(process_list->next);
        freeCmdLines(process_list->cmd);
        free(process_list);
    }
}

void updateProcessStatus(process *process_list, int pid, int status){
    while (process_list != NULL)
    {
        if (process_list->pid == pid)
        {
            process_list->status = status;
            return;
        }
        process_list = process_list->next;
    }
}

void inputRedirect(cmdLine *pCmdLine){
    int fdi = open(pCmdLine->inputRedirect, O_RDWR | O_CREAT, 0777);
    dup2(fdi, STDIN_FILENO);
    close(fdi);
}

void outputRedirect(cmdLine *pCmdLine){
    int fdo = open(pCmdLine->outputRedirect, O_RDWR | O_CREAT, 0777);
    dup2(fdo, STDOUT_FILENO);
    close(fdo);
}

void pipeline(cmdLine *pCmdLine){
    int p[2];
    // creating the pipe
    if (pipe(p) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    pid_t pid1 = fork();
    if (pid1 == -1)
    { // fork failed
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid1 == 0)
    { // child process
        //Redireting I/O
        if (pCmdLine->inputRedirect != NULL)
        {
            inputRedirect(pCmdLine);
        }
        if (pCmdLine->outputRedirect != NULL)
        {
            perror("Could not preform this action.");
        }
        close(STDOUT_FILENO);
        dup(p[1]);
        close(p[1]);
        execvp(pCmdLine->arguments[0], pCmdLine->arguments); // execute the command
        perror("execvp1");                                   // print an error message if the command fails
        exit(EXIT_SUCCESS);
    }
    else
    {
        // parent process
        close(p[1]);
        pid_t pid2 = fork();
        if (pid2 == -1)
        { // fork failed
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid2 == 0)
        { // child process
            pCmdLine = pCmdLine->next;
            //Redireting I/O
            if (pCmdLine->inputRedirect != NULL)
            {
                perror("Could not preform this action.");
            }
            if (pCmdLine->outputRedirect != NULL)
            {
                outputRedirect(pCmdLine);
            }
            close(STDIN_FILENO);
            dup(p[0]);
            close(p[0]);
            execvp(pCmdLine->arguments[0], pCmdLine->arguments); // execute the command
            perror("execvp2");                                   // print an error message if the command fails
            exit(EXIT_SUCCESS);
        }
        else
        {
            //parent process
            close(p[0]);
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
        }
    }
}
//execute and deletes the command
void execute(cmdLine *pCmdLine, process **processList){
    if (strcmp(pCmdLine->arguments[0], "quit") == 0){
        printf("bye\n");
        freeProcessList(*processList);
        free(processList);
        freeCmdLines(pCmdLine);
        freeHistory();
        exit(EXIT_SUCCESS);
    }
    //Debugger
    if (pCmdLine->argCount > 1){
        for (int i = 0; i < pCmdLine->argCount; i++)
        {
            if (strcmp(pCmdLine->arguments[i], "-d") == 0)
            {
                fprintf(stderr, "PID: %d, executing : %s", getpid(), pCmdLine->arguments[0]);
                break;
            }
        }
    }
    //executing
    int sign = 1; // 0 for success, -1 for error
    if (strcmp(pCmdLine->arguments[0], "history") == 0){
        print_history();
        freeCmdLines(pCmdLine);
    } 
    else if (strcmp(pCmdLine->arguments[0], "procs") == 0){
        printProcessList(processList);
        freeCmdLines(pCmdLine);
    }
    else if (strcmp(pCmdLine->arguments[0], "cd") == 0){
        sign = chdir(pCmdLine->arguments[1]);
        freeCmdLines(pCmdLine);

    }
    else if (strcmp(pCmdLine->arguments[0], "suspend") == 0){
        sign = kill(atoi(pCmdLine->arguments[1]), SIGTSTP);
        freeCmdLines(pCmdLine);
    }
    else if (strcmp(pCmdLine->arguments[0], "wake") == 0){
        sign = kill(atoi(pCmdLine->arguments[1]), SIGCONT);
        freeCmdLines(pCmdLine);
    }
    else if (strcmp(pCmdLine->arguments[0], "kill") == 0){
        sign = kill(atoi(pCmdLine->arguments[1]), SIGINT);
        freeCmdLines(pCmdLine);
    }
    //invoke an executable
    else{
        if (pCmdLine->next != NULL)
        {
            pipeline(pCmdLine);
        }
        else
        {
            pid_t pId = fork();
            if (pId == -1)
            {
                perror("fork failed");
            }
            else if (pId == 0)
            {
                //Child proccess

                //Redireting I/O
                if (pCmdLine->inputRedirect != NULL)
                {
                    inputRedirect(pCmdLine);
                }
                if (pCmdLine->outputRedirect != NULL)
                {
                    outputRedirect(pCmdLine);
                }

                //invoking the executable
                int status = execvp(pCmdLine->arguments[0], pCmdLine->arguments);
                if (status == -1)
                {
                    perror("Couldn't Perform That Action");
                    freeCmdLines(pCmdLine);
                    exit(EXIT_FAILURE);
                }
                exit(EXIT_SUCCESS);
            }
            else
            {
                // Parent process
                addProcess(processList, pCmdLine, pId);
                if (pCmdLine->blocking)
                {
                    int status;
                    if (waitpid(pId, &status, 0) == -1)
                    {
                        perror("waitpid failed");
                        freeCmdLines(pCmdLine);
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }
    }
    if (sign == 0) printf("succsses\n");
}

char* handleExecMark(char* input){
    if (strcmp(input, "!!\n") == 0)
    {
        if (hist_count == 0) {
                printf("No commands in history.\n");
                return "";
            }
        else{
            input = history[hist_newest];
            // free(history[hist_newest]);
            hist_newest = (hist_newest-1)%HISTLEN;
            if(hist_count == 1) hist_oldest = -1;
            hist_count--;
            return input;
        }
    }
    int k = atoi(&input[1]);
    if (k <= 0 || k > hist_count) {
                printf("Invalid history index.\n");
                return "";
    }
    for(int i = k-1; i >= 0; i--){
        int index = (hist_newest-i)%HISTLEN;
        if(i == k-1){
            input = history[index];
            // free(history[index]);
        }
        history[index] = history[(index+1) %HISTLEN];
    }
    hist_newest = (hist_newest -1)%HISTLEN;
    hist_count --;
    return input;
}

void displayCWD()
{
    char cwd[PATH_MAX];
    if (getcwd(cwd, PATH_MAX) == NULL)
    {
        perror("couldn't get cuurent location\n");
        exit(EXIT_FAILURE);
    }
    else
        printf("%s ", cwd);
}

int main(int argc, const char **argv)
{
    char buffer[2048];
    process **processlist = (process **)malloc(sizeof(process **));
    *processlist = NULL;
    while (1)
    {
        displayCWD();
        char *p = fgets(buffer, 2048, stdin);
        if(p[0] == '!'){
            p = handleExecMark(p);
            if(strcmp(p,"") == 0) continue;
        } 
        addToHistory(p);
        cmdLine *com = parseCmdLines(p);
        execute(com, processlist);
    }
}