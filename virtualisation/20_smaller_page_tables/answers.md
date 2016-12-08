###### 1. With a linear page table, you need a single register to locate the page table, assuming that hardware does the lookup upon a TLB miss. How many registers do you need to locate a two-level page table? A three-level table?

Only one register is needed to locate a two-level or three-level page table: only the location of the root page directory is needed.


###### 2. Use the simulator to perform translations given random seed 0 and check your answers using the `-c` flag. How many memory references are needed to perform each lookup?

`--addresses=5 --seed=0`: 
* 611c -> 0 11000 (=24) 01000 (=8) 11100 (=28) -> PDE: a1 -> PFN 0010 0001 (=33) -> PTE: b5 -> PFN 0011 0101 (=53) -> value: 08 (after three memory references)
* 3da8 -> 0 01111 (=15) 01101 (=13) 01000 (=8) -> PDE: d6 -> PFN 0101 0110 (=86) -> PTE: 7f -> Invalid address (after two memory references)
* 17f5 -> 0 00101 (=5) 11111 (=31) 10101 (=21) -> PDE: d4 -> PFN 0101 0100 (=84) -> PTE: ce -> PFN 0100 1110 (=78) -> value: 1c (after three memory references)
* 7f6c -> 0 11111 (=31) 11011 (=27) 01100 (=12) -> PDE: ff -> PFN 0111 1111 (=127) -> PTE: 7f -> Invalid address (after two memory references)
* 0bad -> 0 00010 (=2) 11101 (=29) 01101 (=13) -> PDE: fe -> PFN 0111 1110 (=126) -> PTE: 7f -> Invalid address (after two memory references)


###### 3. Given your understanding of how cache memory works, how do you think memory references to the page table will behave in the cache? Will they lead to lots of cache hits (and thus fast accesses?) Or lots of misses (and thus slow accesses)?

I don't know, it's complicated! Modern x86 processors can have anywhere between 4-way to 16-way cache associativity, a two-level TLB heirarchy (including a first level TLB split into data and instruction parts, and a second level TLB that can't hold translations for huge pages) with 4-way associativity, and pseudo-LRU cache and TLB eviction strategies.

**Assumptions**
* No TLB
* Fully associative cache
* Cache line eviction LRU
* We're going to ignore all but the last level of cache
* cache can't be addressed using virtual addresses: need to resolve to physical address first

In the abscence of a TLB, every memory reference will be immediately preceeded by a reference to the page table (or references in the case of a multi-level page table). If the cache evicts LRU lines, then at most one cache line of data will be held without its corresponding page table cache lines also in cache.

**Now assume we also have a TLB and TLB hits will not count as a cache read for the purposes of cache eviction**

There are some situations where data could be in cache but the address translation has been evicted and from cache and TLB. For example if a cache line is read repeatedly while hitting the TLB, then the data cache line will stay young but the translation cache lines will grow old and potentially be evicted. A couple of hundred memory accesses across different pages followed by reading the original data again could trigger a page table read from RAM while the data is still in cache.

I'm not going any further down this path because it's getting hairy.

We can make some very broad observations for silly sized TLB and caches and processes that exhibit a high degree of spatial locality in memory accesses. Define TLB coverage as the sum of the size of all the pages that are addressed by TLB entries.

1. Cache >> TLB coverage: the cache in excess of TLB coverage will *always* trigger a TLB miss before it can be accessed (regardless of TLB-cache efficiency). However, it is possible that the corresponding page table lines are also in the cache, in which case the latency penalty will be be on the order of small integer multiples (x2, x3) rather than several orders of magnitude.

2. Cache << TLB coverage: a process with a high degree of spatial locality in memory accesses could find itself only using a subset of TLB entries while not incurring TLB misses and still having a working set size greater than the cache. In other words, the bottleneck for this process is cache size, not TLB.