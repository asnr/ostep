/*
4. Write a simple program that allocates memory using malloc() but
forgets to free it before exiting. What happens when this program runs?
Can you use gdb to find any problems with it? How about valgrind
(again with the --leak-check=yes flag)?

This program exits normally when it runs.

This program exits normally when running it under gdb. Some quick
googling did not unearth a way to use gdb to detect the leak.

Valgrind identifies the 4 bytes that were not freed, together with the
line number where they were malloc'd:

    4 bytes in 1 blocks are definitely lost in loss record 1 of 62
    at 0x100009681: malloc (in /usr/local/Cellar/valgrind/3.12.0/lib/valgrind/vgpreload_memcheck-amd64-darwin.so)
    by 0x100000F84: main (4_forget_to_free.c:16)
*/

#include <stdlib.h>

int main(int argc, char const *argv[])
{
    int *x = (int*) malloc(sizeof(int));
    return 0;
}
