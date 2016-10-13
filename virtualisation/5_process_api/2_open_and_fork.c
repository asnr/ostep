/*
    Q: Write a program that opens a file (with the open() system call)
    and then calls fork() to create a new process. Can both the child
    and parent access the file descriptor returned by open()? What
    happens when they are writing to the file concurrently, i.e., at
    the same time?

    A: Both the parent and child can access the file descriptor
    returned by open(). They can both write to the file as well; one
    writes to the file after the other.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

static const char OUT_FILENAME[] = "2_open_and_fork.out";

static const char PRE_FORK_TEMPLATE[] = "Hello from the parent (PID %d) before the fork.\n";
static const char POST_FORK_TEMPLATE_1[] = "Hello from PID %d after the fork.\n";
static const char POST_FORK_TEMPLATE_2[] = "Hello again from PID %d after the fork.\n";


int main(int argc, char const *argv[])
{
    // O_WRONLY - write only, O_CREAT - create file if it doesn't exist,
    // O_TRUNC - truncate file to 0 bytes if it already exists
    int fd = open(OUT_FILENAME, O_WRONLY | O_CREAT | O_TRUNC,
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == -1) {
        printf("Error: could not open file %s.\n", OUT_FILENAME);
        exit(1);
    }

    size_t pre_fork_msg_len = sizeof(PRE_FORK_TEMPLATE) + 10;
    char pre_fork_msg[pre_fork_msg_len];
    snprintf(pre_fork_msg, pre_fork_msg_len, PRE_FORK_TEMPLATE, (int) getpid());

    write(fd, pre_fork_msg, strnlen(pre_fork_msg, pre_fork_msg_len));

    size_t post_fork_msg_len_1 = sizeof(POST_FORK_TEMPLATE_1) + 10;
    size_t post_fork_msg_len_2 = sizeof(POST_FORK_TEMPLATE_2) + 10;
    char post_fork_msg_1[post_fork_msg_len_1];
    char post_fork_msg_2[post_fork_msg_len_2];

    int rc = fork();
    if (rc < 0) {
        printf("Error: fork failed.\n");
        exit(1);
    }

    snprintf(post_fork_msg_1, post_fork_msg_len_1, POST_FORK_TEMPLATE_1, (int) getpid());
    write(fd, post_fork_msg_1, strnlen(post_fork_msg_1, post_fork_msg_len_1));

    sleep(1);

    snprintf(post_fork_msg_2, post_fork_msg_len_2, POST_FORK_TEMPLATE_2, (int) getpid());
    write(fd, post_fork_msg_2, strnlen(post_fork_msg_2, post_fork_msg_len_2));

    return 0;
}