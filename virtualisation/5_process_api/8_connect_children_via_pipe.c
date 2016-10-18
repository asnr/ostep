/*
    Q: Write a program that creates two children, and connects the
    standard output of one to the standard input of the other, using
    the pipe() system call.
*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

void write_to_pipe_as_stdout(int pipe_fds[]) {
    close(pipe_fds[0]);
    dup2(pipe_fds[1], STDOUT_FILENO);

    printf("Hello from the write child!\n");
}

void read_from_pipe_as_stdin(int pipe_fds[]) {
    close(pipe_fds[1]);
    dup2(pipe_fds[0], STDIN_FILENO);

    char msg[101];
    char endmsg;
    scanf("%100[^\n]%c", msg, &endmsg);

    printf("Read child received message from the pipe: %s\n", msg);
    if (endmsg != '\n') {
        printf("(Note: the message was over 100 characters long; only the first 100 characters were printed)\n");
    }
}

int main(int argc, char const *argv[])
{
    int pipe_fds[2];
    int piperc = pipe(pipe_fds);
    if (piperc == -1) {
        printf("Error: pipe() failed.\n");
        exit(1);
    }

    int forkrc_1 = fork();
    if (forkrc_1 == -1) {
        printf("Error: first fork() failed.\n");
        exit(1);
    } else if (forkrc_1 == 0) {
        write_to_pipe_as_stdout(pipe_fds);
    } else {
        int forkrc_2 = fork();
        if (forkrc_2 == -1) {
            printf("Error: second fork() failed.\n");
            exit(1);
        } else if (forkrc_2 == 0) {
            read_from_pipe_as_stdin(pipe_fds);
        }
    }

    return 0;
}
