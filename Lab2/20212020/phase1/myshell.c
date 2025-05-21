#include "csapp.h"
#include <errno.h>

#define MAXARGS 128

/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv); 

int main() 
{
    char cmdline[MAXLINE]; /* Command line */

    while (1) {
        /* Read */
        printf("CSE4100-SP-P2> ");              
        char* rst = fgets(cmdline, MAXLINE, stdin); 
        if (feof(stdin) || rst == NULL)
            exit(0);
    
        /* Evaluate */
        eval(cmdline);
    } 
}

/* eval - Evaluate a command line */
void eval(char *cmdline) 
{
    char *argv[MAXARGS]; /* Argument list execve() */
    char buf[MAXLINE];   /* Holds modified command line */
    char *cmd[MAXARGS];  /* Holds individual commands separated by && */
    int bg;              /* Should the job run in bg or fg? */
    pid_t pid;           /* Process id */
    int i = 0;           /* Index for individual commands */

    strcpy(buf, cmdline);

    /* Split commands separated by && */
    cmd[i] = strtok(buf, "&&");
    while (cmd[i] != NULL) {
        i++;
        cmd[i] = strtok(NULL, "&&");
    }

    /* Execute individual commands */
    for (int j = 0; j < i; j++) {
        bg = parseline(cmd[j], argv); 
        if (argv[0] == NULL)  /* Ignore empty lines */
            continue;   
        if (!builtin_command(argv)) {
            pid = Fork(); /* Using Fork() from csapp.h to create a child process */

            if (pid == 0) { /* Child process */
                if (execvp(argv[0], argv) < 0) { /* Use execvp() to execute the command */
                    printf("%s: Command not found.\n", argv[0]);
                    exit(0);
                }
            }
        
            /* Parent waits for foreground job to terminate */
            if (!bg){ 
                int status;
                if (waitpid(pid, &status, 0) < 0) {
                    perror("waitpid error");
                }
            }
        }
    }
    return;
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv) 
{
    if (!strcmp(argv[0], "quit") || !strcmp(argv[0], "exit")) /* quit command */
        exit(0);  
    if (!strcmp(argv[0], "cd")) { /* cd command */
        if (argv[1] == NULL || !strcmp(argv[1], "~")) {
            chdir(getenv("HOME"));
        } 
        else {
            if (chdir(argv[1]) < 0) {
                printf("bash: cd: %s: No such file or directory\n", argv[1]);
            }
        }
        return 1;
    }
    if (!strcmp(argv[0], "&"))    /* Ignore singleton & */
        return 1;
    return 0;                     /* Not a builtin command */
}

/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv) 
{
    char *delim;         /* Points to first space delimiter */
    int argc;            /* Number of args */
    int bg;              /* Background job? */

    buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
        buf++;

    /* Build the argv list */
    argc = 0;

    while ((delim = strchr(buf, ' '))) {

        if (buf[0] == '\'') {
            buf++;
            delim = strchr(buf, '\'');
        }
        else if (buf[0] == '"') {
            buf++;
            delim = strchr(buf, '"');
        }

        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;

        while (*buf && (*buf == ' ')) /* Ignore spaces */
            buf++;
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* Ignore blank line */
        return 1;

    /* Should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0)
        argv[--argc] = NULL;

    return bg;
}