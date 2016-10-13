/*
    Q: Write a program that calls fork(). Before calling fork(), have the
    main process access a variable (e.g., x) and set its value to
    something (e.g., 100). What value is the variable in the child
    process? What happens to the variable when both the child and
    parent change the value of x?

    A: After the fork, the value of the variable in both the child and
    parent is 100. Each has its own copy of the variable and changing
    the value of x in one does not change the value of x in the other.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    printf("Start PID %d\n", (int) getpid());
    int x = 100;
    int rc = fork();
    if (rc < 0) {
        printf("Error: call to fork() failed.\n");
        exit(1);
    } else if (rc == 0) {
        printf("Child (pid %d) before trying to change x, x = %d\n",
               (int) getpid(), x);
        x = 1;
        printf("Child (pid %d) after trying to change x, x = %d\n",
               (int) getpid(), x);
    } else {
        sleep(2);
        printf("Parent (pid %d) before trying to change x, x = %d\n",
               (int) getpid(), x);
        x = 2;
        printf("Parent (pid %d) after trying to change x, x = %d\n",
               (int) getpid(), x);
    }
    return 0;
}
