/*
    Q: Write a program that creates a child process, and then in the
    child closes standard output (STDOUT_FILENO). What happens if the
    child calls printf() to print some output after closing the
    descriptor?

    A: If the printf call is buffered, then printf() returns with no
    error, but the message is not printed to the terminal. If the
    printf call is *not* buffered, then printf() returns -1 (signalling
    that an error has occurred) and errorno is set to 9, "bad file
    descriptor".
*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <errno.h>
#include <string.h>


void print_return_value(int fd, int printf_rc, int printf_errno) {
    dprintf(fd,
            "printf return value (called while STDOUT closed): %d.\n",
            printf_rc);

    if (printf_rc == -1) {
        dprintf(fd,
                "printf errorno %d, %s.\n",
                printf_errno, strerror(printf_errno));
    }
}


int main(int argc, char const *argv[])
{
    int forkrc = fork();
    if (forkrc == -1) {
        printf("Error: fork() failed.\n");
        exit(1);
    } else if (forkrc == 0) {

    } else {
        int dup_stdout_fd = dup(STDOUT_FILENO);

        close(STDOUT_FILENO);

        int printf_buf_rc = 
            printf("Child printing (with buffering) after closing STDOUT.\n");
        int printf_buf_errno = errno;
        print_return_value(dup_stdout_fd, printf_buf_rc, printf_buf_errno);

        // Prevent the printf call from buffering
        setvbuf(stdout, NULL, _IONBF, 0);
        int printf_no_buf_rc =
            printf("Child printing (no buffering) after closing STDOUT.\n");
        int printf_no_buf_errno = errno;
        print_return_value(dup_stdout_fd,
                           printf_no_buf_rc, printf_no_buf_errno);
    }

    return 0;
}
