/*
    Q: Now write a program that uses wait() to wait for the child
    process to finish in the parent. What does wait() return? What
    happens if you use wait() in the child?

    A: Wait returns the PID of the child process that
    stopped/terminated. If you use wait() in the child, wait() returns
    -1 and sets errno to 10, "No child processes".
*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <errno.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    int forkrc = fork();
    if (forkrc == -1) {
        printf("Error: fork failed.\n");
        exit(1);
    } else if (forkrc == 0) {
        int waitrc = wait(0);
        if (waitrc -1) {
            int wait_errno = errno;
            printf("CHILD WAITED: wait returned errno %d, %s\n",
                   wait_errno, strerror(wait_errno));
        }
    } else {
        int waitrc = wait(0);
        printf("PARENT WAITING: Child pid %d, wait returned %d\n", forkrc, waitrc);
    }
    return 0;
}
