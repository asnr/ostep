###### 2.  What would you predict `num_full` to be with different buffer sizes (e.g., `-m 10`) and different numbers of produced items (e.g., `-l 100`), when you change the consumer sleep string from default (no sleep) to `-C 0,0,0,0,0,0,1`?

The consumers would grab one item each and then immediately fall asleep, leaving the producers to fill the buffer. The producers would then sleep until the consumers woke and cycle repeated. The buffer would thus spend most of its time full, i.e. `num_full` would always be equal to or close to the buffer size.

This scenario simulates consumers that take much longer to consume items than it takes for producers to produce items.

###### 8. Now let’s look at `main-one-cv-while.c`. Can you configure a sleep string, assuming a single producer, one consumer, and a buffer of size 1, to cause a problem with this code?

No. One condition variable is sufficient when there is only one producer and one consumer.

###### 9. Now change the number of consumers to two. Can you construct sleep strings for the producer and the consumers so as to cause a problem in the code?

No.

It is possible in theory: we want to end up with both a consumer and the producer waiting on the condition variable, with the consumer being the first in line to wake up. Given how the code is written, the producer can't sleep at all, because it needs attempt to produce one item and then immediately loop around and wait on the condition variable before a consumer has entered the critical section and consumes that item. That doesn't leave us with enough flexibility to play the consumers to reach our desired state.

It doesn't help that most of the sleep controls are irrelevant: sleeping inside the critical section just causes all the threads to hang, it isn't an effective way to change the execution state.

For reference, below is the basic setup. (It's possible for this to cause a hang, given some lucky scheduling, but macOS doesn't play ball.)

```
./main-one-cv-while -c 2 -p 1 -l 4 -v -C 0,0,0,0,0,0,0:0,0,0,0,0,0,0 -P 0,0,0,0,0,0,0
```
