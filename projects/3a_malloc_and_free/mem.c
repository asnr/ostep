#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include "mem.h"

struct header {
  int size;
  int used;
  struct header *next;
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
  region_start->next = NULL;
  return ptr;
}

static void
*alloc_block(int size, struct header *block_header)
{
  if (block_header->size < size || block_header->used) {
    return block_header->next == NULL ?
      NULL :
      alloc_block(size, block_header->next);
  }

  int room_for_next_header = block_header->size - size;

  // TODO: handle case where not enough room for next header
  // TODO: handle where next header has already been built
  char *after_header = (char *) (address_after_header(block_header));
  struct header *next_header = (struct header *)(after_header + size);
  next_header->size = room_for_next_header - sizeof(struct header);
  next_header->used = 0;
  next_header->next = NULL;

  block_header->next = next_header;
  block_header->used = 1;
  block_header->size = size;

  return address_after_header(block_header);
}

void
*Mem_Alloc(int size)
{
  return alloc_block(size, region_start);
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
