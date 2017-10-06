#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

#define PATH_BUFSIZE 1024
#define COMMAND_BUFSIZE 1024
#define TOKEN_BUFSIZE 64
#define TOKEN_DELIMITERS " \t\r\n\a"
#define ARGSIZE 64
#define BACKGROUND_EXECUTION 0
#define FOREGROUND_EXECUTION 1
#define PIPELINE_EXECUTION 2

struct command_segment {
    char *args[ARGSIZE];   // arguments array
    struct command_segment *next;
    pid_t pid;   // process ID
    pid_t pgid;   // process group ID
};

struct command {
    struct command_segment *root;   // a linked list
    int mode;   // BACKGROUND_EXECUTION or FOREGROUND_EXECUTION
};

int mysh_cd(char *path) {
    /* Implement cd command */

}

int mysh_fg(pid_t pid) {
    /* Implement fg command */

}

int mysh_bg(pid_t pid) {
    /* Implement bg command */

}

int mysh_kill(pid_t pid) {
    /* Implement kill command */

}

int mysh_exit() {
    /* Release all the child processes */


    /* Exit the program */

}

int mysh_execute_builtin_command(struct command_segment *segment) {
    /* Match if command name (i.e. segment->args[0]) is a internal command */

}

int mysh_execute_command_segment(struct command_segment *segment, int in_fd, int out_fd, int mode, int pgid) {
    // Check if it's a built-in command first
    if (mysh_execute_builtin_command(segment)) {
        return 0;
    }

    /* Fork a process and execute the program */

}

int mysh_execute_command(struct command *command) {
    struct command_segment *cur_segment;

    // Iterate the linked list of command segment
    // If this is not a pipeline command, there is only a root segment.
    for (cur_segment = command->root; cur_segment != NULL; cur_segment = cur_segment->next) {
        /* Create pipe if necessary */


        /* Call mysh_execute_command_segment(...) to execute command segment */

    }

    /* Return status */

}

struct command* mysh_parse_command(char *line) {
    /* Parse line as command structure */
    struct command *cmd = malloc(sizeof(struct command));
    struct command_segment *cur = cmd->root = malloc(sizeof(struct command_segment)); // cur is end of list
    cmd->mode = FOREGROUND_EXECUTION;

    if (!cmd || !cmd->root) {
        fprintf(stderr, "-mysh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    char *p = line;
    while (*p != '\0') {
        if (*p == '&') {
            cmd->mode = BACKGROUND_EXECUTION;
            *p = '\0';
            break;
        }
        p++;
    }

    char *sep = line;
    char *segment;
    int argc = 0;
    segment = strsep(&sep, "|");
    for (argc = 0; argc < ARGSIZE - 1 && (cur->args[argc] = strtok(segment, TOKEN_DELIMITERS)) != NULL; argc++)
        segment = NULL;
    cur->args[argc] = NULL;
    while ((segment = strsep(&sep, "|")) != NULL) {
        struct command_segment *cmd_seg = malloc(sizeof(struct command_segment));
        cur = cur->next = cmd_seg;
        for (argc = 0; argc < ARGSIZE - 1 && (cur->args[argc] = strtok(segment, TOKEN_DELIMITERS)) != NULL; argc++)
            segment = NULL;
        cur->args[argc] = NULL;
    }
    cur->next = NULL;
    return cmd;
}

char* mysh_read_line() {
    int bufsize = COMMAND_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer) {
        fprintf(stderr, "-mysh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        c = getchar();

        if (c == EOF || c == '\n') {    // read just one line per time
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;

        if (position >= bufsize) {   // handle overflow case
            bufsize += COMMAND_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer) {
                fprintf(stderr, "-mysh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

void mysh_print_promt() {
    /* Print "<username> in <current working directory>" */

    /* Print "mysh> " */
    printf("mysh> ");
}

void mysh_print_welcome() {
    /* Print "Welcome to mysh by <student ID>!" */
    printf("Welcome to mysh by Sean Kao!\n");
}

void mysh_loop() {
    char *line;
    struct command *command;
    int status = 1;

    do {   // an infinite loop to handle commands
        mysh_print_promt();
        line = mysh_read_line();   // read one line from terminal
        if (strlen(line) == 0) {
            continue;
        }
        command = mysh_parse_command(line);   // parse line as command structure
        #ifdef test
        printf("mode: %d\n", command->mode);
        struct command_segment *p = command->root;
        while(p) {
            printf("%s\n", p->args[0]); // TODO: how to print all args
            p = p->next;
        }
        #else
        status = mysh_execute_command(command);   // execute the command
        #endif
        free(line);
    } while (status >= 0);
}

void mysh_init() {
    /* Do any initializations here. You may need to set handlers for signals */

}

int main(int argc, char **argv) {
    mysh_init();
    mysh_print_welcome();
    mysh_loop();

    return EXIT_SUCCESS;
}
