#include <stdio.h>
#include <assert.h>
#include "mem.h"

int
main()
{
  printf(">> User Test >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

  void *region_start = Mem_Init(4096);
  printf("Mem_Init() = %p\n", region_start);
  assert(region_start > (void *)0x100000);

  void *too_big_request = Mem_Alloc(8000);
  printf("Mem_Alloc(8000) = %p\n", too_big_request);
  assert(too_big_request == (void *) 0);

  void *first_request = Mem_Alloc(16);
  printf("Mem_Alloc(16) = %p\n", first_request);
  assert(first_request > 0);

  void *second_request = Mem_Alloc(16);
  printf("Mem_Alloc(16) = %p\n", second_request);
  assert(second_request > 0);

  int mem_free_rc = Mem_Free(first_request);
  printf("Mem_Free(first_request) = %d\n", mem_free_rc);
  assert(mem_free_rc == MEM_FREE_SUCCEEDED);

  int repeated_mem_free_rc = Mem_Free(first_request);
  printf("Mem_Free(first_request) = %d\n", repeated_mem_free_rc);
  assert(repeated_mem_free_rc == MEM_FREE_FAILED);

  void *request_after_free = Mem_Alloc(16);
  printf("Mem_Alloc(16) = %p\n", request_after_free);
  assert(request_after_free == first_request);

  void *second_request_after_free = Mem_Alloc(64);
  printf("Mem_Alloc(64) = %p\n", second_request_after_free);
  assert(request_after_free < second_request_after_free);

  int second_mem_free_rc = Mem_Free(second_request_after_free);
  printf("Mem_Free(second_request_after_free) = %d\n", second_mem_free_rc);
  assert(second_mem_free_rc == MEM_FREE_SUCCEEDED);

  void *request_part_of_freed_block = Mem_Alloc(16);
  printf("Mem_Alloc(16) = %p\n", request_part_of_freed_block);
  assert(request_part_of_freed_block > 0);

  void *request_more_from_freed_block = Mem_Alloc(16);
  printf("Mem_Alloc(16) = %p\n", request_more_from_freed_block);
  assert(request_more_from_freed_block > 0);

  void *request_spilling_out_of_freed_block = Mem_Alloc(16);
  printf("Mem_Alloc(16) = %p\n", request_spilling_out_of_freed_block);
  assert(request_spilling_out_of_freed_block > 0);

  printf("\nUser test passed!\n");

  return 0;
}
