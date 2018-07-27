#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include "mem.h"

struct region {
  int total_size;
  struct header *next;
};

static void *
address_after_region_header(struct region *header)
{
  return (void *) (header + 1);
}

struct header {
  int size;
  int used;
  struct header *next;
};

static struct region* region_start;

static void *
address_after_block_header(struct header *header)
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
  printf("[Mem_Init] Requesting %zu bytes from mmap\n", mmap_size);

  void *ptr = mmap(NULL,
                   mmap_size,
                   PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS,
                   no_mach_vm_flags,
                   no_offset);
  if (ptr == MAP_FAILED) { perror("mmap"); exit(1); }

  region_start = (struct region *) ptr;
  printf("[Mem_Init] region_start = %p\n", region_start);
  region_start->total_size = sizeOfRegion;
  struct header *first_block =
    (struct header *) address_after_region_header(region_start);
  first_block->size = sizeOfRegion - sizeof(struct region) - sizeof(struct header);
  first_block->used = 0;
  first_block->next = NULL;
  region_start->next = first_block;
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

  int room_for_next_block = block_header->size - size;

  // TODO: handle where next header has already been built
  if (room_for_next_block < sizeof(struct header)) {
    block_header->next = NULL;
  } else {
    char *after_this_header = (char *) (address_after_block_header(block_header));
    struct header *next_header = (struct header *)(after_this_header + size);
    next_header->size = room_for_next_block - sizeof(struct header);
    next_header->used = 0;
    next_header->next = NULL;

    block_header->next = next_header;
  }
  block_header->used = 1;
  block_header->size = size;

  return address_after_block_header(block_header);
}

void
*Mem_Alloc(int size)
{
  return alloc_block(size, region_start->next);
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
  char *region = (char *) region_start;
  for (int idx = 0; idx < region_start->total_size; idx++) {
    if (idx > 0) {
      if (idx % 16 == 0) {
        putchar('\n');
      } else {
        putchar(' ');
      }
    }
    int byte = ((unsigned int) region[idx]) & 0xff;
    printf("%02x", byte);
  }
  return;
}
