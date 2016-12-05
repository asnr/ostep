###### 1. First run with the flags `--numOps=10 --headerSize=0 --policy=BEST --seed=0` to generate a few random allocations and frees. Can you predict what alloc()/free() will return? Can you guess the state of the free list after each request? What do you notice about the free list over time?

Defaults that apply: no alignment restriction, size 100, base address 1000, free list sorted by address, not coalesced.

The free list gets more and more fragmented over time. Searching for free space will take longer and longer.

0. ptr[0] = Alloc(3) returns 1000 after searching 1 element. Free list: [addr: 1003, sz: 97].

0. Free(ptr[0]) returns 0. Free list: [addr: 1000, sz: 3], [addr: 1003, sz: 97].

0. ptr[1] = Alloc(5) returns 1003 after searching 2 elements. Free list: [addr: 1000, sz: 3], [addr: 1008, sz: 92].

0. Free(ptr[1]) returns 0. Free list: [addr: 1000, sz: 3], [addr: 1003, sz: 5], [addr: 1008, sz: 92].

0. ptr[2] = Alloc(8) returns 1008 after searching 3 elements. Free list: [addr: 1000, sz: 3], [addr: 1003, sz: 5], [addr: 1016, sz: 84].

0. Free(ptr[2]) returns 0. Free list: [addr: 1000, sz: 3], [addr: 1003, sz: 5], [addr: 1008, sz: 8], [addr: 1016, sz: 84].

0. ptr[3] = Alloc(8) returns 1008 after searching 4 elements. Free list: [addr: 1000, sz: 3], [addr: 1003, sz: 5], [addr: 1016, sz: 84].

0. Free(ptr[3]) returns 0. Free list: [addr: 1000, sz: 3], [addr: 1003, sz: 5], [addr: 1008, sz: 8], [addr: 1016, sz: 84].

0. ptr[4] = Alloc(2) returns 1000 after searching 4 element. Free list: [addr: 1002, sz: 1], [addr: 1003, sz: 5], [addr: 1008, sz: 8], [addr: 1016, sz: 84].

0. ptr[5] = Alloc(7) returns 1008 after searching 4 elements. Free list: [addr: 1002, sz: 1], [addr: 1003, sz: 5], [addr: 1015, sz: 1], [addr: 1016, sz: 84].


###### 2. How are the results different when using a WORST fit policy to search the free list (`--policy=WORST`)? What changes?

There are more fragments, but they are larger in size (no 1 byte free fragments). This also slightly increases the search length.


###### 3. What about when using FIRST fit (`--p=FIRST`)? What speeds up when you use first fit?

Searching for free space speeds up: the whole list doesn't need to be searched before allocating memory.
