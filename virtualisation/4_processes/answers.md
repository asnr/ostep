
###### 1 `./process-run.py -l 5:100,5:100`. What should the CPU utilization be (e.g., the percent of time the CPU is in use? ######

CPU utilisation should be 100%. Both processes do not call IO, so the CPU will never wait.


###### 2. `./process-run.py -l 4:100,1:0`. How long does it take to complete both processes?

Default IO length is 5 ticks. The processes start in the order the are listed in the `-l` option. The scheduler is not pre-emptive. This will take 9 ticks to complete: the first process completes in 4 ticks and the second will then be scheduled and complete in 5 ticks. The simulator adds a final "empty tick" to make 10 ticks.


###### 3. `./process-run.py -l 1:0,4:100`. What happens now? Does switching the order matter? Why?

The 2 processes now take 5 ticks to complete as the second process can run while the first is waiting on IO. The simulator adds a final "empty tick" to make 6 ticks.


###### 4. We’ll now explore some of the other flags. One important flag is `-S`, which determines how the system reacts when a process issues an I/O. With the flag set to `SWITCH_ON_END`, the system will NOT switch to another process while one is doing I/O, instead waiting until the process is completely finished. What happens when you run the following two processes, one doing I/O and the other doing CPU work? (`-l 1:0,4:100 -c -S SWITCH_ON_END`)

The resource usage statistics are now almost identical to that in Question 2 (9 ticks for both processes to complete), the only difference being that there is no final "empty tick" as the process that does IO runs first.


###### 5. Now, run the same processes, but with the switching behavior set to switch to another process whenever one is WAITING for I/O (`-l 1:0,4:100 -c -S SWITCH_ON_IO`). What happens?

This is exactly Question 3, as `-S SWITCH_ON_IO` is the default setting.


###### 6. One other important behavior is what to do when an I/O completes. With `-I IO_RUN_LATER`, when an I/O completes, the process that issued it is not necessarily run right away; rather, whatever was running at the time keeps running. What happens when you run this combination of processes? (`./process-run.py -l 3:0,5:100,5:100,5:100 -S SWITCH_ON_IO -I IO_RUN_LATER -c -p`) Are system resources being effectively utilized?

The first process will wait on IO, and the next 3 processes will run to completion, one after the other. Total running time will be 26 ticks with one extra "empty tick" at the end. This is not an effective usage of resources as the first process could have been running *all* the IO operations while the other processes were using CPU.


###### 7. Now run the same processes, but with `-I IO_RUN_IMMEDIATE` set, which immediately runs the process that issued the I/O. How does this behavior differ? Why might running a process that just completed an I/O again be a good idea?

In this case the first process will interrupt the other processes in order to start it's IO operations, resulting in a reduced computation time of 18 ticks. Resource usage will similarly improve, with CPU usage at 100%.

Running a process that just completed an I/O again is a good idea because that process is more likely to issue another I/O statement than other processes (this is a statistical argument) and in the context of this simulation it is better to kick off I/O operations as soon as possible.


###### 8. Now run with some randomly generated processes, e.g., `-s 1 -l 3:50,3:50`, `-s 2 -l 3:50,3:50`, `-s 3 -l 3:50,3:50`. See if you can predict how the trace will turn out. What happens when you use `-I IO_RUN_IMMEDIATE` vs. `-I IO_RUN_LATER`? What happens when you use `-S SWITCH_ON_IO` vs. `-S SWITCH_ON_END`

`SWITCH_ON_IO` always results in faster run times than `SWITCH_ON_END`. For the 3 examples given, `-I IO_RUN_IMMEDIATE` results in exactly the same order of execution as `-I IO_RUN_LATER`. In all 3 cases, the few possible CPU instructions (each process can have a max of 3 instructions, so each can have a max of 3 CPU instructions) relative to the length of IO operations (5 ticks) means that the processes spend most of their time `WAITING` on IO, hence there is no opportunity for `IO_RUN_IMMEDIATE` to evict a currently running process.
