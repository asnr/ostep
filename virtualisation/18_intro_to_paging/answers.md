###### 1. Use the simulator to study how linear page tables change size given different parameters. Compute the size of linear page tables as different parameters change. How should page table size change as the address space grows? As the page size grows? Why shouldn’t we just use really big pages in general?

Page table size is governed by the following equations:

```
address space size = number of page table entries * page size
```

and 

```
bits necesssary to address a page = log( physical memory size / page size )
```

The number of page table entries increases linearly with address size and the number of bits necessary to address a page increases logarithmicly with address size:

`./paging-linear-translate.py --pagesize=1k --physmem=512m --numaddrs=0 -v`

* `--asize=1m`: 1024 entries
* `--asize=2m`: 2048 entries
* `--asize=4m`: 4096 entries

The number of page table entries is proportional to the inverse of page size: doubling the page size halves the the number of entries. The number of bits necessary to address a page is proportional to the logarithm of the inverse of the page size.

`./paging-linear-translate.py --asize=1m --physmem=512m -v --numaddrs=0`
* `--pagesize=1k`: 1024 entries
* `--pagesize=2k`: 512 entries
* `--pagesize=4k`: 256 entries

Using large pages isn't a good solution to page tables that take up too much space as it will increase the risk of space waste through internal fragmentation: processes are less likely to use larger pages as efficiently as small pages.


###### 2. Now let’s do some translations. What happens as you increase the percentage of pages that are allocated in each address space?

`./paging-linear-translate.py --pagesize=1k --asize=16k --physmem=32k -v`

`--used=0`: none (0%) of the page tables are in use by the process; resolving any address is a segmentation fault.

32kb: 2^15 = 0x8000 -> last physical address 7fff
 1kb: 2^10  000, 400, 800, c00, 10, 14, 18, 1c, 

`--used=50`:

```
  VA 0x1bc9 (decimal:  7113) -->  6 * 0x400 + 0x3c9 -> PTE  6: 0x8000001d -> phys address 29 * 0x400 + 0x3c9 = 0x77c9
  VA 0x2718 (decimal: 10008) -->  9 * 0x400 + 0x318 -> PTE  9: 0x00000000 -> invalid
  VA 0x3a6e (decimal: 14958) --> 14 * 0x400 + 0x26e -> PTE 14: 0x8000000f -> phys address 15 * 0x400 + 0x26e = 0x3e6e
  VA 0x3ddc (decimal: 15836) --> 15 * 0x400 + 0x1dc -> PTE 15: 0x00000000 -> invalid
  VA 0x1e87 (decimal:  7815) -->  7 * 0x400 + 0x287 -> PTE  7: 0x80000009 -> phys address 9 * 0x400 + 0x287 = 0x2687
```

As the percentage of pages that are allocated in each address space increases, the percentage of random virtual addresses that trigger page faults decreases proportionally.


###### 3. Consider the following address space parameters: `--pagesize=8 --asize=32 --physmem=1024`, `--pagesize=8k --asize=32k --physmem=1m`, `--pagesize=1m --asize=256m --physmem=512m`. Which of these parameter combinations are unrealistic? Why?

The first parameters are unrealistic because each page table would have to hold 128 entries, which could not possibly fit in a 32 bit address space.

The second set of parameters, while not impossible (each page table entry would have to hold 4 PTE's, and each PTE would have to be at least 7bits in size, easily fitting into a 32k address space), would only result in 4 pages per process. This risks wasting a lot of space due to internal fragmentation.

The third set of parameters are not as unrealistic as the first two: each page table would be composed of 256 entries and each entry would need to address one of 2^9 physical pages, yielding a minimum size for a flat page table of around 2k bits..
