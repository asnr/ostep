/*
    Q: Write another program using fork(). The child process should
    print “hello”; the parent process should print “goodbye”. You
    should try to ensure that the child process always prints first;
    can you do this *without* calling wait() in the parent?

    A: Use pipe()! Perhaps a similar effect could be achieved using
    signals?
*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

static const char GO = 'G';

int main(int argc, char const *argv[])
{
    int pipefd[2];
    int piperc = pipe(pipefd);
    if (piperc == -1) {
        printf("Error: could not create pipe.\n");
    }

    int forkrc = fork();
    if (forkrc == -1) {
        printf("Error: could not fork.\n");
        exit(1);
    } else if (forkrc == 0) {
        // don't need the read end
        close(pipefd[0]);
        // confirm that parent waits on child by sleeping child
        sleep(1);
        printf("hello (says child)\n");
        write(pipefd[1], &GO, 1);
    } else {
        // don't need the write end
        close(pipefd[1]);
        char msg;
        read(pipefd[0], &msg, 1);
        printf("goodbye (says parent)\n");
    }
    return 0;
}