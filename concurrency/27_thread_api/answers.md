###### 1 First build `main-race.c`. Examine the code so you can see the (hopefully obvious) data race in the code. Now run helgrind (by typing `valgrind --tool=helgrind ./main-race`) to see how it reports the race. Does it point to the right lines of code? What other information does it give to you?

Helgrind doesn't work with `Valgrind-3.12.0` on my El Capitan install. It reports data races in the pthread libraries, including when there are no data races in the user code.

Instead, I'll use ThreadSanitizer (TSan). 