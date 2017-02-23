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


###### 2. What happens when you remove one of the offending lines of code? Now add a lock around one of the updates to the shared variable, and then around both. What does helgrind report in each of these cases?

As above, we'll use TSan. Removing the racy accesses from one of the threads and running TSan produces no output and returns with exit code 0.

After adding a lock around just one of the updates to the shared variable, TSan reports a warning as in question 1 that also identifies that one of the threads was holding a lock when it updated the variable and where that lock was initialised. In this case, TSan returns with exit code 134.

After adding a lock around both the updates to the shared variables, TSan produces no output and returns with exit code 0.

###### 3. Can you see what problem `main-deadlock.c` might have?

There's a potential deadlock if the threads grab their first lock at the same time (i.e. thread 0 grabs lock 1 and thread 1 grabs lock 2, leaving both stuck waiting for the lock they don't have).


###### 4. Now run helgrind on this code. What does helgrind report?

I'll run TSan instead (see question 1).

```sh
$ make main-deadlock
$ TSAN_OPTIONS=detect_deadlocks=1 ./main-deadlock
```

TSan reports the following warning:

```
WARNING: ThreadSanitizer: lock-order-inversion (potential deadlock) (pid=71488)
  Cycle in lock order graph: M23 (0x0001001f4088) => M25 (0x0001001f40c8) => M23
```

It also details the lines of code where the mutexes were acquired and where the threads were created.


###### 5. Now run helgrind on `main-deadlock-global.c`.  Examine the code; does it have the same problem that `main-deadlock.c` has? Should helgrind be reporting the same error? What does this tell you about tools like helgrind?

I'll run TSan instead (see question 1). The new code `main-deadlock-global.c` doesn't have the same problems as `main-deadlock.c`: the global lock prevents a deadlock from occuring. Nevertheless, TSan reports the following warning:

```
WARNING: ThreadSanitizer: lock-order-inversion (potential deadlock) (pid=73168)
  Cycle in lock order graph: M25 (0x0001013b40c8) => M27 (0x0001013b4108) => M25
```

This tells us that TSan reports false-positives even for relatively simple cases. In particular, the algorithm only has a simple treatment of building a lock graph and finding cycles in it.


###### 6. Let’s next look at `main-signal.c`. This code uses a variable (`done`) to signal that the child is done and that the parent can now continue. Why is this code inefficient?

The parent spin waits, checking the value of `done` until it changes. This wastes processor time that could be used by another process (or even the worker thread if the scheduler is trying to schedule both threads onto the same processor).


###### 7. Now run helgrind on `main-signal.c`. What does it report? Is the code correct?

I'll run TSan instead (see question 1). TSan reports a data race and a thread leak (the main thread doesn't call `Pthread_join()`). TSan is correct (there is a data race), but it doesn't affect the correctness of the program, assuming that the the instructions aren't reordered so that setting `done` happens before printing to standard out.


###### 8. The version in `main-signal-cv.c` uses a condition variable to do the signaling (and associated lock). Why is this code preferred to the previous version? Is it correctness, or performance, or both?

It is a better solution for nontrivial programs, because it is less likely to introduce concurrency bugs as features are added to the signalling mechanism and it will perform better when the wait between signals increases (the condition variable doesn't spin wait or yield, but rather signals to the scheduler that the thread is blocked and shouldn't be scheduled until it receives a signal from another thread).


###### 9. Once again run helgrind on `main-signal-cv`. Does it report any errors?

I'll run TSan instead (see question 1). This code doesn't cause TSan to throw any data race warnings (it still throws the trivially fixable thread leak warning).
