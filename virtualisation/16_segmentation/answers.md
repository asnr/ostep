###### 1. First let’s use a tiny address space to translate some addresses. Use the parameters `-a 128 -p 512 -b 0 -l 20 -B 512 -L 20` and the seeds `-s 0`, `-s 1`, `-s 2` to generate virtual addresses. Translate them.

**Seed 0**. Address space size 128, phys mem size 512. Segment register information:
* Segment 0 base (grows positive): 0x00000000 (decimal 0), limit: 20
* Segment 1 base (grows negative): 0x00000200 (decimal 512), limit: 20

Virtual Address Trace:
* VA 0: 0x6c (decimal: 108) --> (seg. 1) 492
* VA 1: 0x61 (decimal: 97) --> (seg. 1) segmentation violation
* VA 2: 0x35 (decimal: 53) --> (seg. 0) segmentation violation
* VA 3: 0x21 (decimal: 33) --> (seg. 0) segmentation violation
* VA 4: 0x41 (decimal: 65) --> (seg. 1) segmentation violation

**Seed 1**. Address space size 128, phys mem size 512. Segment register information:
* Segment 0 base (grows positive): 0x00000000 (decimal 0), limit: 20
* Segment 1 base (grows negative): 0x00000200 (decimal 512), limit: 20

Virtual Address Trace:
* VA 0: 0x11 (decimal: 17) --> (seg. 0) 17
* VA 1: 0x6c (decimal: 108) --> (seg. 1) 492
* VA 2: 0x61 (decimal: 97) --> (seg. 1) segmentation violation
* VA 3: 0x20 (decimal: 32) --> (seg. 0) segmentation violation
* VA 4: 0x3f (decimal: 63) --> (seg. 0) segmentation violation


###### 2. Now, let’s see if we understand this tiny address space we’ve constructed (using the parameters from the question above). What is the highest legal virtual address in segment 0? What about the lowest legal virtual address in segment 1? What are the lowest and highest illegal addresses in this entire address space? Finally, how would you run `segmentation.py` with the `-A` flag to test if you are right?

* Highest legal virtual address in segment 0: 0x13 = 19
* Lowest legal virtual address in segment 1: 0x6c = 108
* Lowest illegal virtual address: 0x14 = 20
* Highest illegal virtual address: 0x6b = 107

These were verified using `./segmentation.py -a 128 -p 512 -b 0 -l 20 -B 512 -L 20 -A19,20,107,108 -c`.
