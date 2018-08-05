#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "mem.h"

#define TRUE 1
#define FALSE 0
typedef int bool;

const int NUM_ALLOCS_FOR_TEST = 40000;
const int MAX_NUM_ALLOCS = 100000;
const int SIZE_PER_ALLOC = 16;
const int BLOCK_HEADER_SIZE = 16;
const int REGION_HEADER_SIZE = 32;

// Running commit f39a34c against this measurement script with
// num_allocs = 40000 takes ~8 seconds. We want to be much faster.
const double MAX_SECONDS_ALLOWED = 1.0;

void *allocs[MAX_NUM_ALLOCS];

int
main(int argc, char *argv[])
{
  int num_allocs;
  bool fail_if_too_slow;
  if (argc == 2) {
    printf(">> Perf measurement >>>>>>>>>\n");
    char *endptr;
    num_allocs = (int) strtol(argv[1], &endptr, 10);
    fail_if_too_slow = FALSE;
  } else if (argc == 1) {
    printf(">> Perf test >>>>>>>>>>>>>>>>\n");
    num_allocs = NUM_ALLOCS_FOR_TEST;
    fail_if_too_slow = TRUE;
  } else {
    printf("Usage: ./mem_perf_test [NUM_ALLOCS]\n");
    exit(1);
  }

  int region_size =
    REGION_HEADER_SIZE + num_allocs * (SIZE_PER_ALLOC + BLOCK_HEADER_SIZE);

  clock_t start = clock();

  Mem_Init(region_size);

  for (int i = 0; i < num_allocs; i++) {
    allocs[i] = Mem_Alloc(SIZE_PER_ALLOC);
    assert(allocs[i] > 0);
  }

  clock_t end = clock();

  double cpu_time_taken = ((double) end - start) / CLOCKS_PER_SEC;

  if (fail_if_too_slow) {
    assert(cpu_time_taken < MAX_SECONDS_ALLOWED);
  }

  printf("Took %f seconds\n", cpu_time_taken);

  return 0;
}
