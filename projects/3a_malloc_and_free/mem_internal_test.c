#include <stdio.h>
#include <assert.h>
#include "mem.h"

const int CANARY_SIZE_IN_BYTES = 256;

void initialise_canary(char *canary_start, int size);
void assert_canary_untouched(char *canary_start, int size);

int
main()
{
  int region_size = 64;
  void *region = Mem_Init(region_size);
  char *canary_start = ((char *) region) + region_size;

  initialise_canary(canary_start, CANARY_SIZE_IN_BYTES);

  Mem_Alloc(32);

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
