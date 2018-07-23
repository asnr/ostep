#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include "mem.h"


struct header {
  int size;
};

static struct header* region_start;

static void *
address_after_header(struct header *header)
{
  return (void *) (header + 1);
}

void*
Mem_Init(int sizeOfRegion)
{
  off_t no_offset = 0;
  int no_mach_vm_flags = -1;

  int pagesize = getpagesize();
  size_t pages_for_region = (sizeOfRegion + pagesize - 1) / pagesize;
  size_t mmap_size = pages_for_region * pagesize;
  printf("Requesting %zu bytes from mmap\n", mmap_size);


  void *ptr = mmap(NULL,
                   mmap_size,
                   PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS,
                   no_mach_vm_flags,
                   no_offset);
  if (ptr == MAP_FAILED) { perror("mmap"); exit(1); }

  region_start = (struct header *) ptr;
  printf("region_start = %p\n", region_start);
  region_start->size = mmap_size - sizeof(struct header);
  return ptr;
}

void
*Mem_Alloc(int size)
{
  printf("Mem_Alloc region_start = %p\n", region_start);
  if (region_start->size < size) return NULL;

  return address_after_header(region_start);
}

int
Mem_Free(void *ptr)
{
  return 3;
}

void
Mem_Dump()
{
  return;
}
