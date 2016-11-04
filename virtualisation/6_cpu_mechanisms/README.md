Measuring the costs of system calls and context switches
========================================================

Note: all of the results below were generated on my laptop, an early 2015 MacBook Pro with a 3.1 GHz Intel Core i7 CPU running El Capitan.

We will use the `gettimeofday` function to measure how long system calls and context switches take. To do this, we will first need to get an idea of how precise `gettimeofday` is (we are not interested in accuracy, or how closely the time matches UTC, as we will be measuring differences in time).


Precision of `gettimeofday`
---------------------------

The function `gettimeofday` returns the number of seconds and microseconds since the Unix epoch (midnight January 1st 1970).

Without using a reference clock, we can get a rough idea of how precise `gettimeofday` is by calling it repeatedly and seeing (1) how many calls in a row return the same second, microsecond pair and (2) by how many microseconds the time advances when it does advance. (Note this approach will not detect if the clock that backs `gettimeofday` is consistently slow or consistently fast.)

The code `gettimeofday_runtimes.c` does exactly this. The number of calls to `gettimeofday` that it makes back-to-back are controlled by the static constant `NUM_SAMPLES`. Setting `NUM_SAMPLES` to 100,000 and inspecting the output (on my machine) shows that for most calls, `gettimeofday` returns the same value for 25 consecutive calls, indicating that most calls roughly 40 nanoseconds to complete. Very rarely however (on one indicative run, 215 times, or ~0.2%), between two consecutive calls there can be difference of many microseconds, with the maximum difference usually around 50 microseconds (although on the very occasional run, this will jump to almost 2000 microseconds).

In summary, the distribution of time between two calls to `gettimeofday` (and hence, a guide to the time taken to complete a single call and the precision of that call) appears to be a long-tailed distribution with the approximate shape of an exponential distribution and has a minimum of 40 nanoseconds.


The cost of a system call
-------------------------

We'd like to measure the overhead of a system call. To do this, we'll measure the *shortest* amount of time taken to `read` zero bytes. Given the constraints on the precision of `gettimeofday`, we'll measure how long it takes to execute `n` `read` operations in a row. I have implemented this strategy in `read_system_call.c`. Compiling this code and running it on my machine I get the following *minimum* times:

* 100 read operations in a row: 330 nanosecs/read
* 1000 read operations in a row: 302 nanosecs/read
* 10000 read operations in a row: 302 nanosecs/read

(One potential weakness with using the minimum is that the system clock can be adjusted backwards by [NTP](https://en.wikipedia.org/wiki/Network_Time_Protocol), causing the cost to be underestimated. However, I ran each measurement multiple times and found that all the runs took the minimum amount of time or very close to it, which suggests that clock adjustments did not affect this result.)

Thus our best estimate of the overhead of a system call is ~300 nanoseconds.


The cost of a context switch
----------------------------

To measure the cost of a context switch, we'll create a program that spawns two child processes, connects them via two pipes and has them alternately read from one pipe and write to the other. This will result in the processor rapidly context switching between the two processes (and potentially other processes as well--for example, the user might move the cursor using the mouse).

On a computer with multiple processors, the two child processes might be scheduled onto different processors, which will give us the wrong result. On macOS there is no way to guarantee that two processes get executed on the same processor; the best we can do is to give both processes the same 'affinity tag' and hope that the OS schedules the processes to the same processor.

There is another implementation detail worth mentioning: making the child that will issue the first write wait until the other child is ready to receive it (and thus avoid including the time taken to finish forking the other the child) can significantly lower measurements, especially when only a small number of context switches are measured.

Given the constraints on the precision of `gettimeofday`, we'll measure how long it takes to execute `n` context switches in a row. I have implemented this strategy in `context_switch.c`. Compiling this code and running it on my machine I get the *minimum* times outlined below.

Without modifying any affinity tags (i.e. letting the OS schedule the processes as it sees fit):

* 20 context switches in a row: 49 microseconds total, ~2.5 microseconds/switch
* 200 context switches in a row: 463 microseconds total, ~2.3 microseconds/switch
* 20000 context switches in a row: 39691 microseconds total, ~2 microseconds/switch

Modifying the affinity tags of all processes to all be nonzero and the same (i.e. indicating to the OS that the processes should be scheduled onto the same processor if possible):

* 20 context switches in a row: 49 microseconds total, ~2.5 microseconds/switch
* 200 context switches in a row: 505 microseconds total, ~2.5 microseconds/switch
* 20000 context switches in a row: 48239 microseconds total, ~2.4 microseconds/switch

Running this multiple times produces outputs that vary by as much as 20%, which suggests that either (1) regardless of the proccesses' affinity tags, both child processes are being scheduled to different processors or (2) the same processor, or (3) the processes are scheduled to the same processor for one setting of the affinity tag and different processors for the other setting, in which case a write then read on a pipe across processors takes about as long as a context switch (possibly because it involves a context switch to wake up the process that reads from the pipe).

These results should be compared against the results given by the lmbench tool.
