###### 1. Compute the solutions for simulations with 3 jobs and random seeds of 1, 2, and 3.

```
$ ./lottery.py --jobs=3 --seed=1
ARG jlist
ARG jobs 3
ARG maxlen 10
ARG maxticket 100
ARG quantum 1
ARG seed 1

Here is the job list, with the run time of each job:
  Job 0 ( length = 1, tickets = 84 )
  Job 1 ( length = 7, tickets = 25 )
  Job 2 ( length = 4, tickets = 44 )


Here is the set of random numbers you will need (at most):
Random 651593  # mod 153 = 119
Random 788724  # 9
Random 93859   # mod 69 = 19
Random 28347   # 57
Random 835765  # 37
Random 432767  # 68
Random 762280  # 37
Random 2106    # 36
Random 445387  # 61
Random 721540  # 7
Random 228762  # 27
Random 945271  # 40
```

The jobs will get scheduled as follows:

* 2
* 0 (done, tick 1)
* 1
* 2
* 2
* 2 (done, tick 5)
* 1
* 1
* 1
* 1
* 1
* 1 (done, tick 11)

The problems with seeds 2 and 3 can be done similarly.


###### 2. Now run with two specific jobs: each of length 10, but one (job 0) with just 1 ticket and the other (job 1) with 100 (e.g., -l 10:1,10:100). What happens when the number of tickets is so imbalanced? Will job 0 ever run before job 1 completes? How often? In general, what does such a ticket imbalance do to the behavior of lottery scheduling?

The probability that job 0 will run before job 1 completes is 1 - 0.99^10 ~ 0.10, or 10%. Ticket imbalances mean that there a reasonable chance that processes with a small number of tickets will be starved of CPU for long stretches.


###### 3. When running with two jobs of length 100 and equal ticket allocations of 100 (-l 100:100,100:100), how unfair is the scheduler? Run with some different random seeds to determine the (probabilistic) answer; let unfairness be determined by how much earlier one job finishes than the other.

Running the simulation 100 times (with seeds from 1 to 100) yields an average unfairness value of 0.94, minimum value 0.85 and maximum value 0.995.


###### 4. How does your answer to the previous question change as the quantum size (-q) gets larger?

The scheduling becomes more and more unfair.

Quantum 2:  avg 0.93, min 0.79, max 0.99
Quantum 4:  avg 0.90, min 0.68, max 0.98
Quantum 8:  avg 0.85, min 0.62, max 0.96
Quantum 16: avg 0.79, min 0.50, max 0.93

These numbers were generated using the script

```sh
quantum=1
for seed in $(seq 1 100); do
    times=$(./lottery.py -q $quantum -l 100:100,100:100 -s $seed -c | \
            egrep DONE | \
            egrep -o [0-9]+$)
    fst=$(echo $times | cut -f 1 -d ' ')
    snd=$(echo $times | cut -f 2 -d ' ')
    bc <<< "scale=3; $fst/$snd"
done
```


###### 5. Can you make a version of the graph that is found in the chapter? What else would be worth exploring? How would the graph look with a stride scheduler?

Yes.

Response times, maximum time a job is not scheduled.

The graph would look very close to a flat line at 1 (i.e. perfectly fair).
