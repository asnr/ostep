###### 1. Run a few randomly-generated problems with just two jobs and two queues; compute the MLFQ execution trace for each. Make your life easier by limiting the length of each job and turning off I/Os.

One randomly generated problem:

```
$ ./mlfq.py --numJobs=2 --numQueues=2 --maxlen=30 --maxio=0 --seed=1
Here is the list of inputs:
OPTIONS jobs 2
OPTIONS queues 2
OPTIONS quantum length for queue  1 is  10
OPTIONS quantum length for queue  0 is  10
OPTIONS boost 0
OPTIONS ioTime 5
OPTIONS stayAfterIO False
OPTIONS iobump False


For each job, three defining characteristics are given:
  startTime : at what time does the job enter the system
  runTime   : the total CPU time needed by the job to finish
  ioFreq    : every ioFreq time units, the job issues an I/O
              (the I/O takes ioTime units to complete)

Job List:
  Job  0: startTime   0 - runTime   4 - ioFreq   0
  Job  1: startTime   0 - runTime  23 - ioFreq   0

Compute the execution trace for the given workloads.
If you would like, also compute the response and turnaround
times for each of the jobs.

Use the -c flag to get the exact results when you are finished.
```

For this problem, the execution trace will be:

* 0-3: run job 0 at priority 1.
* 4-13: run job 1 at priority 1.
* 14-26: run job 1 at priority 0.

Another randomly generated problem:

```
$ ./mlfq.py --numJobs=2 --numQueues=2 --maxlen=40 --maxio=0 --seed=2
Here is the list of inputs:
OPTIONS jobs 2
OPTIONS queues 2
OPTIONS quantum length for queue  1 is  10
OPTIONS quantum length for queue  0 is  10
OPTIONS boost 0
OPTIONS ioTime 5
OPTIONS stayAfterIO False
OPTIONS iobump False

...

Job List:
  Job  0: startTime   0 - runTime  38 - ioFreq   0
  Job  1: startTime   0 - runTime   3 - ioFreq   0
```

For this problem, the execution trace will be:

* 0-9: run job 0 at priority 1.
* 10-12: run job 1 at priority 1.
* 13-40: run job 0 at priority 0.


###### 2. How would you run the scheduler to reproduce each of the examples in the chapter?

All the examples have 3 queues.

The **first** example has one 200ms job progressing downwards through the 3 queues, the first two of which have time slices of 10ms. To reproduce this example:

```sh
$ ./mlfq.py --jlist=0,200,0 --quantumList=10,10,10
```

The **second** example has a long running job that is interrupted by a short job at T=100. This second job runs for exactly two time slices. To reproduce this example:

```sh
$ ./mlfq.py --jlist=0,180,0:100,20,0 --quantumList=10,10,10
```

The **third** example has a long running job that is interrupted by an interactive job that repeatedly issues I/O after 1ms of CPU time. To reproduce this example (note that we delay the arrival of the interactive job to give time for the long running job to get to the lowest priority queue):

```sh
$ ./mlfq.py --jlist=0,182,0:0,18,1 --quantumList=10,10,10 --iotime=9 --stay
```

The **fourth** example has two parts. In the first part, MLFQ without priority boost or storing CPU use across I/O calls starves a long running process when two interactive processes that keep issuing I/O requests before finishing a complete time slice. To reproduce this example:

```sh
$ ./mlfq.py --jlist=0,120,0:100,50,5:100,50,5 --quantumList=10,10,10 --iotime=5 --stay
```

In the second part, MLFQ with periodic priority boost does not starve the long running process in the same scenario:

```sh
$ ./mlfq.py --jlist=0,120,0:100,50,5:100,50,5 --quantumList=10,10,10 --iotime=5 --stay --boost=50
```

The **fifth** example also has two parts. In the first part, MLFQ that does not store CPU use across I/O calls is gamed by a process that makes short I/O calls immediately before its time slice finishes, thus leaving very little CPU time that a long running process can use. To reproduce this example:

```sh
$ ./mlfq.py --jlist=0,45,0:20,162,9 --quantumList=10,10,10 --iotime=1 --stay
```

In the second part, MLFQ that accounts for CPU use across I/O calls is not gamed by the same process issuing short I/O calls:

```sh
$ ./mlfq.py --jlist=0,130,0:20,70,9 --quantumList=10,10,10 --iotime=1
```

The **sixth** example has two long running jobs and queues that have longer time slices at lower priorities:

```sh
$ ./mlfq.py --jlist=0,100,0:0,100,0 --quantumList=10,20,35
```


###### 3. How would you configure the scheduler parameters to behave just like a round-robin scheduler?

```sh
TIME_SLICE=10
$ ./mlfq.py --quantumList=$TIME_SLICE
```


###### 4. Craft a workload with two jobs and scheduler parameters so that one job takes advantage of the older Rules 4a and 4b (turned on with the -S flag) to game the scheduler and obtain 99% of the CPU over a particular time interval.

```sh
$ ./mlfq.py --jlist=0,102,0:100,198,99 --quantumList=100,100 --iotime=1 --stay
```


###### 5. Given a system with a quantum length of 10ms in its highest queue, how often would you have to boost jobs back to the highest priority level (with the -B flag) in order to guarantee that a single long running (and potentially-starving) job gets at least 5% of the CPU?

200ms.


###### 6. One question that arises in scheduling is which end of a queue to add a job that just finished I/O; the -I flag changes this behavior for this scheduling simulator. Play around with some workloads and see if you can see the effect of this flag.

In a situation where there are several interactive processes that regularly issue short I/O requests, prepending a job that just finished I/O to the start of a queue can lead to jobs receiving bursts of CPU time followed by long periods of waiting for CPU. For example, running the workload

```sh
$ /mlfq.py --jlist=0,16,2:0,16,2:0,16,2:0,16,2 --quantumList=4,4,4 --iotime=1  -I
```

results in the first job being scheduled for four of the first five ticks and then not being scheduled for the next 22 ticks. In this case, processes that are the first to be scheduled at a priority level 'hog' the CPU until they move down to the next priority level where they are only serviced after all the other processes have moved down to the *third* priority level (as when the move down from the first priority level, the fact that they just finished I/O means they get pushed to the front of the second priority queue).

Alternatively, appending a job that finished I/O to the end of a queue leads to much more even scheduling. In the same scenario,

```sh
$ /mlfq.py --jlist=0,16,2:0,16,2:0,16,2:0,16,2 --quantumList=4,4,4 --iotime=1
```

the first process never waits for CPU for more than 6 ticks.
