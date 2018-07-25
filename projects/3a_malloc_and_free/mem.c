#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include "mem.h"

struct header {
  int size;
  int used;
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
  region_start->used = 0;
  return ptr;
}

void
*Mem_Alloc(int size)
{
  if (region_start->size < size || region_start->used) return NULL;

  region_start->used = 1;
  return address_after_header(region_start);
}

int
Mem_Free(void *ptr)
{
  struct header *block_header = ((struct header *) ptr) - 1;
  printf("[Mem_Free] header to free: %p\n", block_header);
  if (!block_header->used) return MEM_FREE_FAILED;

  block_header->used = 0;
  return MEM_FREE_SUCCEEDED;
}

void
Mem_Dump()
{
  return;
}
