#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include "mem.h"

struct region {
  int total_size;
  struct block *first_free_block;
};

static void *
address_after_region_header(struct region *header)
{
  return (void *) (header + 1);
}

struct block {
  int size;
  int used;
  struct block *next_free_block;
};

static struct region* region_start;

static void *
address_after_block_header(struct block *block)
{
  return (void *) (block + 1);
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
  struct block *first_block =
    (struct block *) address_after_region_header(region_start);
  first_block->size = sizeOfRegion - sizeof(struct region) - sizeof(struct block);
  first_block->used = 0;
  first_block->next_free_block = NULL;
  region_start->first_free_block = first_block;
  return ptr;
}

static void
*alloc_block(int size, struct block **free_block_list)
{
  struct block *block = *free_block_list;
  /* printf("Block %p: size = %d, used = %d, next = %p\n", block, block->size, block->used, block->next_free_block); */

  if (block->size < size || block->used) {
    return block->next_free_block == NULL ?
      NULL :
      alloc_block(size, &(block->next_free_block));
  }

  int room_for_next_block = block->size - size;

  if (sizeof(struct block) <= room_for_next_block) {
    char *after_this_header = (char *) (address_after_block_header(block));
    struct block *next_block = (struct block *)(after_this_header + size);
    next_block->size = room_for_next_block - sizeof(struct block);
    next_block->used = 0;
    next_block->next_free_block = block->next_free_block;
    *free_block_list = next_block;
  } else {
    *free_block_list = block->next_free_block;
  }
  block->used = 1;
  block->size = size;
  block->next_free_block = NULL;

  return address_after_block_header(block);
}

void
*Mem_Alloc(int size)
{
  return alloc_block(size, &(region_start->first_free_block));
}

int
Mem_Free(void *ptr)
{
  struct block *block = ((struct block *) ptr) - 1;
  printf("[Mem_Free] header to free: %p\n", block);
  if (!block->used) return MEM_FREE_FAILED;

  block->used = 0;
  block->next_free_block = region_start->first_free_block;
  region_start->first_free_block = block;
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
