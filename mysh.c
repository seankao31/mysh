#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <pwd.h>

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
    if (chdir(path) == -1) {
        fprintf(stderr, "-mysh: cd %s: %s\n", path, strerror(errno));
        return -1;
    }
    return 0;
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
    printf("Goodbye!\n");
    // exit(EXIT_SUCCESS);
    return 0;
}

int mysh_execute_builtin_command(struct command_segment *segment) {
    /* Match if command name (i.e. segment->args[0]) is a internal command */
    if (strcmp(segment->args[0], "exit") == 0) {
        if (mysh_exit() != 0) {
            fprintf(stderr, "-mysh: exit error\n");
            exit(EXIT_FAILURE);
        }
        return -1;
    }
    else if (strcmp(segment->args[0], "cd") == 0) {
        if (mysh_cd(segment->args[1]) != 0) {
            // do something
        }
        return 1;
    }

    return 0;
}

int mysh_execute_command_segment(struct command_segment *segment, int in_fd, int out_fd, int mode, int pgid) {
    // Check if it's a null command
    if (segment->args[0] == NULL) {
        fprintf(stderr, "-mysh: No command specified\n");
        return -1;
    }

    // Check if it's a built-in command first
    int status;
    if (status = mysh_execute_builtin_command(segment)) {
        return status; // exit is -1, other builtin command is 1
    }
    /*fprintf(stderr, "-mysh: %s: Command not found\n", segment->args[0]);*/

    /* Fork a process and execute the program */
    pid_t pid;
    switch (pid = fork()) {
        case -1:
            perror("-mysh");
            break;
        case 0:
            printf("Command executed by pid=%d", getpid());
            if (mode == BACKGROUND_EXECUTION)
                printf(" in background");
            printf("\n");

            dup2(in_fd, 0);
            dup2(out_fd, 1);
            /*
             *if (mode == BACKGROUND_EXECUTION) {
             *    close(0);
             *}
             */
            if (in_fd != 0) {
                close(in_fd);
            }
            if (out_fd != 1) {
                close(out_fd);
            }
            if (execvp(segment->args[0], segment->args) == -1) {
                if (errno == ENOENT) {
                    fprintf(stderr, "-mysh: %s: Command not found\n", segment->args[0]);
                }
                else {
                    perror("-mysh");
                }
                exit(EXIT_FAILURE);
            }
        default:
            if (in_fd != 0) {
                close(in_fd);
            }
            if (out_fd != 1) {
                close(out_fd);
            }
            if (mode == BACKGROUND_EXECUTION) {
                if (waitpid(pid, &status, WNOHANG) == -1) {
                    perror("-mysh");
                }
            }
            else if (waitpid(pid, &status, 0) == -1) {
                perror("-mysh");
            }
    }

    return status;
}

int mysh_execute_command(struct command *command) {
    struct command_segment *cur;
    int status = 0;

    // Iterate the linked list of command segment
    // If this is not a pipeline command, there is only a root segment.
    for (cur = command->root; cur != NULL; cur = cur->next) {
        /* Create pipe if necessary */


        /* Call mysh_execute_command_segment(...) to execute command segment */
        status = mysh_execute_command_segment(cur, 0, 1, command->mode, cur->pgid);

    }

    // free space
    struct command_segment *p;
    p = cur = command->root;
    while (cur) {
        cur = cur->next;
        free(p);
        p = cur;
    }
    free(command);
    p = cur = NULL;
    command = NULL;

    /* Return status */
    return status;
}

struct command* mysh_parse_command(char *line) {
    /* Parse line as command structure */
    struct command *cmd = malloc(sizeof(struct command));
    struct command_segment *cur = cmd->root = malloc(sizeof(struct command_segment)); // cur is end of list
    cmd->mode = FOREGROUND_EXECUTION;

    if (!cmd || !cmd->root) {
        perror("-mysh");
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
        perror("-mysh");
        exit(EXIT_FAILURE);
    }

    while (1) {
        c = getchar();

        if (c == EOF || c == '\n') {    // read just one line per time
            buffer[position] = '\0';
            return buffer;
        }
        else {
            buffer[position] = c;
        }
        position++;

        if (position >= bufsize) {   // handle overflow case
            bufsize += COMMAND_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer) {
                perror("-mysh");
                exit(EXIT_FAILURE);
            }
        }
    }
}

void mysh_print_prompt() {
    /* Print "<username> in <current working directory>" */
    char cwd[PATH_BUFSIZE];
    struct passwd *pwd;

    if (!(pwd = getpwuid(getuid()))) {
        perror("-mysh");
    }
    if (!getcwd(cwd, sizeof(cwd))) {
        perror("-mysh");
    }

    /*printf("%s in %s\n", username, cwd);*/
    printf("%s in %s\n", pwd->pw_name, cwd);

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
        mysh_print_prompt();
        line = mysh_read_line();   // read one line from terminal
        if (strlen(line) == 0) {
            continue;
        }
        command = mysh_parse_command(line);   // parse line as command structure
        status = mysh_execute_command(command);   // execute the command
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
