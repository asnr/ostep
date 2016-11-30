/*
    1. First, write a simple program called null.c that creates a
    pointer to an integer, sets it to NULL, and then tries to
    dereference it. Compile this into an executable called null. What
    happens when you run this program?

    Segmentation fault.

    2. Next, compile this program with symbol information included
    (with the -g flag). Doing so let’s put more information into the
    executable, enabling the debugger to access more useful information
    about variable names and the like. Run the program under the
    debugger by typing gdb null and then, once gdb is running, typing
    run. What does gdb show you?

        Starting program: .../ostep/virtualisation/14_memory_api/1_null

        Program received signal SIGSEGV, Segmentation fault.
        0x0000000100000fa0 in main (argc=1, argv=0x7fff5fbff8b0) at 1_null.c:25
        25      int y = *x + 1;

    3. Finally, use the valgrind tool on this program. We’ll use the
    memcheck tool that is a part of valgrind to analyze what happens.
    Run this by typing in the following:
      valgrind --leak-check=yes null
    What happens when you run this? Can you interpret the output from
    the tool?

    A lot of things. Some of those things. 
*/

int main(int argc, char const *argv[])
{
    int *x = 0;
    int y = *x + 1;
    return 0;
}