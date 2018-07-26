#include <stdio.h>
#include <assert.h>
#include "mem.h"

int
main ()
{
  void *region_start = Mem_Init(409);
  assert(region_start > (void *)0x100000);
  printf("Mem_Init() = %p\n", region_start);

  void *too_big_request = Mem_Alloc(8000);
  assert(too_big_request == (void *) 0);
  printf("Mem_Alloc(8000) = %p\n", too_big_request);

  void *first_request = Mem_Alloc(20);
  assert(first_request > 0);
  printf("Mem_Alloc(20) = %p\n", first_request);

  void *second_request = Mem_Alloc(20);
  assert(second_request > 0);
  printf("Mem_Alloc(20) = %p\n", second_request);

  int mem_free_rc = Mem_Free(first_request);
  assert(mem_free_rc == MEM_FREE_SUCCEEDED);
  printf("Mem_Free(first_request) = %d\n", mem_free_rc);

  int repeated_mem_free_rc = Mem_Free(first_request);
  assert(repeated_mem_free_rc == MEM_FREE_FAILED);
  printf("Mem_Free(first_request) = %d\n", repeated_mem_free_rc);

  void *request_after_free = Mem_Alloc(20);
  assert(request_after_free == first_request);
  printf("Mem_Alloc(20) = %p\n", request_after_free);

  return 0;
}
