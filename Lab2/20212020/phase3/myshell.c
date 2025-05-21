#include "csapp.h"
#include <errno.h>

#define MAXARGS 128

/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);
void execute_command(char **argv, int input_fd, int output_fd);

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
    char *cmd[MAXARGS];  /* Holds individual commands separated by && and || */
    int bg;              /* Should the job run in bg or fg? */
    pid_t pid;           /* Process id */
    int i = 0;           /* Index for individual commands */

    strcpy(buf, cmdline);

    /* Parse individual commands */
    char *token = strtok(buf, "|");
    while (token != NULL && i < MAXARGS) {
        cmd[i++] = token;
        token = strtok(NULL, "|");
    }

    /* Execute commands in sequence */
    int input_fd = STDIN_FILENO; // Default input
    int pipe_fds[2]; // Pipe file descriptors
    for (int j = 0; j < i; j++) {
        char *cmdline = cmd[j];
        bg = parseline(cmdline, argv);
        for(int k=0; argv[k]!=NULL; k++) printf("%s\n", argv[k]);
        if (argv[0] == NULL)  
            continue;   /* Ignore empty lines */
        if (!builtin_command(argv)) {
            if (j < i - 1) {
                // Create pipe for intermediate commands
                pipe(pipe_fds);
                // Set output of current command to pipe write end
                execute_command(argv, input_fd, pipe_fds[1]);
                // Close write end of the pipe as it's not needed anymore
                close(pipe_fds[1]);
                // Set input of next command to pipe read end
                input_fd = pipe_fds[0];
            } else {
                // Last command, no need for pipe
                execute_command(argv, input_fd, STDOUT_FILENO);
            }
        }
    }
}

/* Execute a command with input/output redirection */
void execute_command(char **argv, int input_fd, int output_fd) {
    pid_t pid;
    pid = Fork();
    if (pid == 0) { // Child process
        // Redirect input if needed
        if (input_fd != STDIN_FILENO) {
            Dup2(input_fd, STDIN_FILENO);
            Close(input_fd); // Close the duplicate descriptor
        }
        // Redirect output if needed
        if (output_fd != STDOUT_FILENO) {
            Dup2(output_fd, STDOUT_FILENO);
            Close(output_fd); // Close the duplicate descriptor
        }
        // Execute the command
        if (execvp(argv[0], argv) < 0) {
            printf("%s: Command not found.\n", argv[0]);
            exit(0);
        }
    }
    // Parent process
    // Wait for the child process to complete
    int status;
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid error");
    }
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

        if (*buf == '\'') {
            buf++;
            delim = strchr(buf, '\'');
        }
        else if (*buf == '"') {
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