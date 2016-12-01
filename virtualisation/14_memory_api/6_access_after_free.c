/*
6. Create a program that allocates an array of integers (as above),
frees them, and then tries to print the value of one of the elements of
the array. Does the program run? What happens when you use valgrind on
it?

The program runs and terminate normally.

Valgrind detects the read of free'd memory:

    Invalid read of size 4
       at 0x100000F50: main (6_access_after_free.c:30)
     Address 0x100a805c8 is 200 bytes inside a block of size 400 free'd
       at 0x100009ABD: free (in /usr/local/Cellar/valgrind/3.12.0/lib/valgrind/vgpreload_memcheck-amd64-darwin.so)
       by 0x100000F44: main (6_access_after_free.c:29)
     Block was alloc'd at
       at 0x100009681: malloc (in /usr/local/Cellar/valgrind/3.12.0/lib/valgrind/vgpreload_memcheck-amd64-darwin.so)
       by 0x100000F34: main (6_access_after_free.c:28)
*/

#include <stdlib.h>
#include <stdio.h>

static const int SIZE = 100;

int main(int argc, char const *argv[])
{
    int *x = (int *) malloc(SIZE * sizeof(*x));
    free(x);
    printf("%d\n", *(x + (SIZE/2)));
    return 0;
}
