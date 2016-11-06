###### 1. Compute the response time and turnaround time when running three jobs of length 200 with the SJF and FIFO schedulers.

Shortest Job First: avg response time 200, avg turnaround time 400.
First In First Out: avg response time 200, avg turnaround time 400.


###### 2. Now do the same but with jobs of different lengths: 300, 200, and 100.

Shortest Job First: avg response time 133 + 1/3, avg turnaround time 333 + 1/3.
First In First Out: avg response time 266 + 2/3, avg turnaround time 466 + 2/3.


###### 3. Now do the same, but also with the RR scheduler and a time-slice of 1.

3 jobs of length 200: avg response time 1, avg turnaround time 599
Jobs of length 300, 200, 100: avg response time 1, avg turnaround time 466 + 2/3


###### 4. For what types of workloads does SJF deliver the same turnaround times as FIFO?

Workloads where the short jobs are the first ones to come in.


###### 5. For what types of workloads and quantum lengths does SJF deliver the same response times as RR?

Workloads where there is only ever one job to be run at a time (i.e. no job arrives before another is still running) or workloads where all the jobs (but the longest one) take at most one quantum length to complete.


###### 6. What happens to response time with SJF as job lengths increase? Can you use the simulator to demonstrate the trend?

Response times increase linearly with the length of the jobs. This can be demonstrated using a quick bash script and the simulator. Running the script

```sh
START_LEN=100
LEN_INCREMENT=100
END_LEN=1000
for l in $(seq $START_LEN $LEN_INCREMENT $END_LEN); do
    AVG_RESP=$(./scheduler.py -p SJF -l $l,$l,$l -c | \
               grep --o "Average -- Response: [0-9.]*" | \
               grep --o [0-9.]*$);
    echo "Average response time for 3 jobs of length $l: $AVG_RESP"
done
```

returns

```
Average response time for 3 jobs of length 100: 100.00
Average response time for 3 jobs of length 200: 200.00
Average response time for 3 jobs of length 300: 300.00
Average response time for 3 jobs of length 400: 400.00
Average response time for 3 jobs of length 500: 500.00
Average response time for 3 jobs of length 600: 600.00
Average response time for 3 jobs of length 700: 700.00
Average response time for 3 jobs of length 800: 800.00
Average response time for 3 jobs of length 900: 900.00
Average response time for 3 jobs of length 1000: 1000.00
```


###### 7. What happens to response time with RR as quantum lengths increase? Can you write an equation that gives the worst-case response time, given N jobs?

The response times increase linearly with the quantum length. In particular, if the quantum length is `Q` and the jobs each take at least `Q` time to finish, then the average response time is given by `Q(N-1)/2` and the time it takes for the last job to be scheduled is given by `Q(N-1)`.
