###### 2. Now add the `-d` flag [to `./vector-deadlock -n 2 -l 1 -v`], and change the number of loops (`-l`) from 1 to higher numbers. What happens? Does the code (always) deadlock?

The code does not always deadlock. In fact, I could not get the code to deadlock until I ran it with 1,000 loops (`-l 1000`), and even then it would terminate as often as not.

###### 3. How does changing the number of threads (`-n`) change the outcome of the program? Are there any values of `-n` that ensure no deadlock occurs?

Yes. Set `-n 1`. All other values deadlock.

###### 4. Now examine the code in `vector-global-order.c`. First, make sure you understand what the code is trying to do; do you understand why the code avoids deadlock? Also, why is there a special case in this `vector_add()` routine when the source and destination vectors are the same?

The code avoids deadlock because the order the locks are taken in is a total order, defined by the virtual memory address of the vector structure.

There is a special case when the source and destination are the same because in this case only one lock needs to be acquired. Without this special case, the code would try to acquire a lock it already held, guaranteeing a deadlock.


###### 5. Now run the code with the following flags: `-t -n 2 -l 100000 -d`. How long does the code take to complete? How does the total time change when you increase the number of loops, or the number of threads?

`./vector-global-order -t -n 2 -l 100000 -d` completes in 0.5-0.7 seconds, with the distribution skewing much closer to 0.7.

Running `-t -n 2 -d` with loops lengths 1e4, 1e5, 1e6 and 1e7 indicates that run time scales roughly linearly with loop length.

Running `-t -l 100000 -d` with number of threads 2, 20 and 99 indicates that run time scales roughly linearly with number of threads.


###### 6. What happens if you turn on the parallelism flag (`-p`)? How much would you expect performance to change when each thread is working on adding different vectors (which is what `-p` enables) versus working on the same ones?

I expect that the program will complete more quickly, but I don't know how much more quickly. I expect that having enabled parallelism, run time will continue to scale linearly with loop length. More interestingly, I expect it will be invariant with respect to the number of threads, up to the number of logical processors (4 in the case of my Macbook Pro), and will then scale linearly (in multiples of the number of logical processors) past that point.

With two threads (`-n 2`) there is a roughly 7-8 times speedup when enabling parallelism (`-p`), and this figure stays consistent with different loop lengths. Changing the number of threads changes run times as follows:

* 2 threads: ~0.9 seconds
* 3 threads: ~0.9 seconds
* 4 threads: 1.0-1.1 seconds
* 5 threads: 1.0-1.1 seconds
* 6 threads: ~1.3 seconds
* 7 threads: ~1.4 seconds
* 8 threads: ~1.6 seconds
* 16 threads: ~3.1 seconds
* 32 threads: ~6.0 seconds
* 64 threads: ~12.0 seconds

These figures don't exactly match my predictions, but they aren't too far off. Linear scaling doesn't appear to take hold until we reach 8 threads. For 5 threads or less, run time is close to constant, and scaling is sub-linear for fewer than 8 threads. 


###### 7. Now let’s study `vector-try-wait.c`. First make sure you understand the code. Is the first call to `pthread_mutex_trylock()` really needed? Now run the code. How fast does it run compared to the global order approach? How does the number of retries, as counted by the code, change as the number of threads increases?

The first call is really needed, how else are you going to lock the destination vector?

Below are run times with the options `-t -l 100000 -d`, varying the number of threads (`-n`):

* 2 threads: ~0.1 secs, ~0.6M retries
* 4 threads: ~0.4 secs, ~1.6M retries
* 8 threads: 1.9-2.0 secs, ~6.6-7.7M retries
* 16 threads: ~11-13 secs, ~27-33M retries

Below are the run times under high parallelism `-t -l 100000 -d -p` (by construction, all of the runs had zero retries):

* 2 threads: ~0.01 secs
* 4 threads: ~0.02 secs
* 8 threads: ~0.03 secs
* 16 threads: 0.04-0.05 secs
* 32 threads: ~0.08 secs
* 64 threads: 0.15-0.18 secs

Under high contention, both time taken and the number of retries scale super-linearly (it looks like time scales faster than quadratic, but slower than exponential with base 2). Although in big O notation this scaling is much worse than the global lock order solution, this solution is significantly faster in absolute time for small numbers of threads. It is at least 5 times as fast for 2 threads, and the global lock order solution only catches up at around 11 threads.

Under high parallelism (with the `-p` flag) the run times are phenomenal, and far outstrip `vector-global-order`, by first one, and then for higher numbers of threads, two orders of magnitude.


###### 8. Now let’s look at `vector-avoid-hold-and-wait.c`. What is the main problem with this approach? How does its performance compare to the other versions, when running both with `-p` and without it?

The main problem with this approach is that it is too coarse: the global lock (which protects the acquisition of all the other locks) will be under contention even when the vectors being manipulated by each thread is different.

`-t -l 100000 -d`, varying number of threads `-n`:

* 2 threads: ~0.7 secs
* 4 threads: ~1.4 secs
* 8 threads: 2.9-3.0 secs
* 16 threads: 5.9-6.2 secs
* 32 threads: ~12.5 secs

`-t -l 100000 -d -p` (with parallelism), varying number of threads `-n`:

* 2 threads: 0.6-0.7 secs
* 4 threads: ~1.4 secs
* 8 threads: ~2.8 secs
* 16 threads: 5.9-6.1 secs
* 32 threads: 12.5-12.7 secs

The run times under heavy contention (without the `-p` flag) very close to those for `vector-global-order`. For the high parallelism (with the `-p` flag) case, `vector-avoid-hold-and-wait` performs more or less as it does under high contention, but about twice as slow as `vector-global-order` under high parallelism. Consequently, the comparison between `vector-global-order` and `vector-try-wait` more or less holds here as well. 


###### 9. Finally, let’s look at `vector-nolock.c`. This version doesn’t use locks at all; does it provide the exact same semantics as the other versions? Why or why not?

No. It is only atomic with respect to adding a pair of entries, not every pair of entries.


###### 10. Now compare its performance to the other versions, both when threads are working on the same two vectors (no `-p`) and when each thread is working on separate vectors (`-p`). How does this no-lock version perform?

Run times when working on the same two vectors (high contention):

* 2 threads: 0.4-0.5 secs
* 4 threads: ~0.4 secs
* 8 threads: 0.6-0.7 secs
* 16 threads: 1.3-1.4 secs
* 32 threads: 2.6-2.7 secs
* 64 threads: 5.1-5.3 secs

This is about twice as fast as `vector-global-order`.

Run times when each thread is working on separate vectors (high parallelism):

* 2 threads: ~0.06 secs
* 4 threads: ~0.12 secs
* 8 threads: ~0.24 secs
* 16 threads: ~0.49 secs
* 32 threads: ~0.97 secs
* 64 threads: 1.95-1.98 secs

This is almost an order of magnitude faster than `vector-global-order`.
