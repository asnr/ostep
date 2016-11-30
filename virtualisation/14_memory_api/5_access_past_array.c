/*
5. Write a program that creates an array of integers called data of
size 100 using malloc; then, set data[100] to zero. What happens when
you run this program? What happens when you run this program using
valgrind? Is the program correct?

The program exits normally when it is run.

Valgrind detects the invalid read:

    Invalid read of size 4
       at 0x100000F5D: main (5_access_past_array.c:28)
     Address 0x100a80690 is 0 bytes after a block of size 400 alloc'd
       at 0x100009681: malloc (in /usr/local/Cellar/valgrind/3.12.0/lib/valgrind/vgpreload_memcheck-amd64-darwin.so)
       by 0x100000F54: main (5_access_past_array.c:27)

This program is not correct.

*/

#include <stdlib.h>

static const int SIZE = 100;

int main(int argc, char const *argv[])
{
    int *arr = malloc(SIZE * sizeof(int));
    int y = arr[SIZE] + 1;
    free(arr);
    return 0;
}
