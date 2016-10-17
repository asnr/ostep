/*
    Q: Write a slight modification of the previous program, this time
    using waitpid() instead of wait(). When would waitpid() be useful?

    A: When a process has more than one child.
*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char const *argv[])
{
    int forkrc = fork();
    if (forkrc == -1) {
        printf("Error: fork() failed.\n");
        exit(1);
    } else if (forkrc == 0) {
        sleep(3);
        printf("Child finished sleeping\n");
    } else {
        waitpid(forkrc, 0, 0);
        printf("Parent finished waiting\n");
    }

    return 0;
}