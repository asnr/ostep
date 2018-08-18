#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/mman.h>
#include "mem.h"

static void *
address_after_region_header(struct region *header)
{
  return (void *) (header + 1);
}

static struct region* region_start;

// We depend on 1 never being a valid (userland) memory address
static struct block* FREE_LIST_END = (struct block *) 1;

bool
is_block_free(struct block *block)
{
  return block->next_free_block != NULL;
}

bool
is_last_block_in_free_list(struct block *block)
{
  return block->next_free_block == FREE_LIST_END;
}

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
  first_block->next_free_block = FREE_LIST_END;
  region_start->first_free_block = first_block;

  int mutex_init_rc = pthread_mutex_init(&(region_start->allocator_lock), NULL);
  if (mutex_init_rc != 0) { perror("pthread_mutex_init"); exit(1); }

  return ptr;
}

static void *
alloc_block(int size, struct block **free_block_list)
{
  struct block *block = *free_block_list;

  if (block->size < size) {
    return is_last_block_in_free_list(block) ?
      NULL :
      alloc_block(size, &(block->next_free_block));
  }

  int room_for_next_block = block->size - size;

  if (sizeof(struct block) <= room_for_next_block) {
    char *after_this_header = (char *) (address_after_block_header(block));
    struct block *next_block = (struct block *)(after_this_header + size);
    next_block->size = room_for_next_block - sizeof(struct block);
    next_block->next_free_block = block->next_free_block;
    *free_block_list = next_block;
  } else {
    *free_block_list = block->next_free_block;
  }
  block->size = size;
  block->next_free_block = NULL;

  return address_after_block_header(block);
}

void *
Mem_Alloc(int size)
{
  pthread_mutex_lock(&(region_start->allocator_lock));
  void *allocation = alloc_block(size, &(region_start->first_free_block));
  pthread_mutex_unlock(&(region_start->allocator_lock));

  return allocation;
}

int
Mem_Free(void *ptr)
{
  pthread_mutex_lock(&(region_start->allocator_lock));

  int return_code;
  struct block *block = ((struct block *) ptr) - 1;
  printf("[Mem_Free] header to free: %p\n", block);
  if (is_block_free(block)) {
    return_code = MEM_FREE_FAILED;
  } else {
    block->next_free_block = region_start->first_free_block;
    region_start->first_free_block = block;
    return_code = MEM_FREE_SUCCEEDED;
  }

  pthread_mutex_unlock(&(region_start->allocator_lock));

  return return_code;
}

void
Mem_Dump()
{
  pthread_mutex_lock(&(region_start->allocator_lock));

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

  pthread_mutex_unlock(&(region_start->allocator_lock));
}
