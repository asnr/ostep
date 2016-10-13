/*
    Q: Write a program that calls fork() and then calls some form of
    exec() to run the program /bin/ls. See if you can try all of the
    variants of exec(), including execl(), execle(), execlp(), execv(),
    execvp(), and execvP(). Why do you think there are so many variants
    of the same basic call?

    A: There are so many variants because there are two conventions for
    passing arguments, `execle` also lets you specify environment
    variables and there are three conventions for specifying how to
    search for an executable if a full path is not specified (e.g. by
    searching the `PATH` variable).

    The two conventions for passing arguments are:
    
    1. directly, as a variable number of extra arguments to the
    `execl*` functions, e.g.

       ```
       execl('./my_prog', (char *)arg0, (char *)arg1, ..., (char *)0));
       ```
    
    2. via an array of strings to the execv* functions, e.g.

       ```
       char (*argv)[] = { "fst_arg", "snd_arg", (char *)0 };
       execv('./my_prog', argv);
       ```

    The three conventions for specifying how to search for an
    executable path if a filename, not a path, (i.e. the string does not contain a `/`) is specified are:
    
    1. don't search at all, only accept paths (`execl`, `execle`,
    `execv`);

    2. duplicate the actions of the shell, searching the `PATH`
    environment variable (`execlp`, `execvp`);

    3. duplicate the actions of the shell, but use the `PATH` specified
    by the second function argument (`execvP`).
*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char const *argv[])
{
    int forkrc = fork();
    if (forkrc == -1) {
        printf("Error: could not fork.\n");
        exit(1);
    } else if (forkrc == 0) {
        
    }
    return 0;
}