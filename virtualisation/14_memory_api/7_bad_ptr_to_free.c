/*
7. Now pass a funny value to free (e.g., a pointer in the middle of the
array you allocated above). What happens? Do you need tools to find
this type of problem?

When run, this program crashes (with return code 134) with a helpful
error message from malloc:

    7_bad_ptr_to_free(35929,0x7fff75cab000) malloc: *** error for object 0x7fca82c032b0: pointer being freed was not allocated
    *** set a breakpoint in malloc_error_break to debug
    Abort trap: 6

Following those instructions reveals the line of code with the bad call
to free:

    ```
    > gdb 7_bad_ptr_to_free
    ...
    > (gdb) b malloc_error_break
    Function "malloc_error_break" not defined.
    Make breakpoint pending on future shared library load? (y or [n]) y
    Breakpoint 1 (malloc_error_break) pending.
    > (gdb) run
    Starting program: /Users/.../ostep/virtualisation/14_memory_api/7_bad_ptr_to_free
    7_bad_ptr_to_free(36254,0x7fff75cab000) malloc: *** error for object 0x100300320: pointer being freed was not allocated
    *** set a breakpoint in malloc_error_break to debug

    Breakpoint 1, 0x00007fff933a5f32 in malloc_error_break () from /usr/lib/system/libsystem_malloc.dylib
    > (gdb) bt
    #0  0x00007fff933a5f32 in malloc_error_break () from /usr/lib/system/libsystem_malloc.dylib
    #1  0x00007fff93396fd2 in free () from /usr/lib/system/libsystem_malloc.dylib
    #2  0x0000000100000f79 in main (argc=1, argv=0x7fff5fbff8a0) at 7_bad_ptr_to_free.c:24
    ```

Note that gdb can't tell us what arguments were passed to free() and
malloc_error_break() because they are linked from dynamic libraries
that weren't compiled with debugging information.

Valgrind gives us this information, and more besides, without any of
the back-and-forth:

    Invalid free() / delete / delete[] / realloc()
       at 0x100009ABD: free (in /usr/local/Cellar/valgrind/3.12.0/lib/valgrind/vgpreload_memcheck-amd64-darwin.so)
       by 0x100000F78: main (7_bad_ptr_to_free.c:24)
     Address 0x100a80550 is 80 bytes inside a block of size 400 alloc'd
       at 0x100009681: malloc (in /usr/local/Cellar/valgrind/3.12.0/lib/valgrind/vgpreload_memcheck-amd64-darwin.so)
       by 0x100000F64: main (7_bad_ptr_to_free.c:23)
*/

#include <stdlib.h>
#include <stdio.h>

static const int SIZE = 100;

int main(int argc, char const *argv[])
{
    int *x = (int *) malloc(SIZE * sizeof(*x));
    free(x + 20);
    return 0;
}
