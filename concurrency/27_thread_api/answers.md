###### 1 First build `main-race.c`. Examine the code so you can see the (hopefully obvious) data race in the code. Now run helgrind (by typing `valgrind --tool=helgrind ./main-race`) to see how it reports the race. Does it point to the right lines of code? What other information does it give to you?

Helgrind doesn't work with `Valgrind-3.12.0` on my El Capitan install. It reports data races in the pthread libraries, including when there are no data races in the user code.

Instead, I'll use ThreadSanitizer (TSan). To use TSan, I first had to run

```sh
$ DevToolsSecurity -enable
```

otherwise macOS complains about insufficient permissions.

The binary needs to be built with the TSan instrumentation and then run:

```sh
$ clang main-race.c -o main-race -fsanitize=thread -fPIE -g -Wall
$ ./main-race
```

This reports a warning identifying the data race as well as:
* the lines of code in each thread that dangerously access shared memory
* the symbol name of the shared memory (`balance`)
* the line of code where the thread is a created
