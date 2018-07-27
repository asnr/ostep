#include <stdio.h>
#include <assert.h>
#include "mem.h"

const int CANARY_SIZE_IN_BYTES = 256;

void initialise_canary(char *canary_start, int size);
void assert_canary_untouched(char *canary_start, int size);
void fill_memory_with_ones(char *block, int size);

int
main()
{
  int region_size = 64;
  void *region = Mem_Init(region_size);
  char *canary_start = ((char *) region) + region_size;

  initialise_canary(canary_start, CANARY_SIZE_IN_BYTES);

  int block_size = 32;
  char *block = (char *) Mem_Alloc(block_size);
  fill_memory_with_ones(block, block_size);
  // Fail test if the block size needs to be increased because of changes in
  // internal header sizes.
  assert(*(canary_start - 1) == (char) 0xff);

  assert_canary_untouched(canary_start, CANARY_SIZE_IN_BYTES);

  printf("Internal test passed!\n");
  return 0;
}

void initialise_canary(char *canary_start, int size)
{
  for (int idx = 0; idx < size; idx++) {
    canary_start[idx] = 0;
  }
}

void assert_canary_untouched(char *canary_start, int size)
{
  for (int idx = 0; idx < size; idx++) {
    assert(canary_start[idx] == 0);
  }
}

void fill_memory_with_ones(char *block, int size)
{
  for (int idx = 0; idx < size; idx++) {
    block[idx] = (char) 0xff;
  }
}
