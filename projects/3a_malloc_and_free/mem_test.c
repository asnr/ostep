#include <stdio.h>
#include "mem.h"

int
main ()
{
  void *ptr = Mem_Init(409);
  printf("Mem_Init() = %p\n", ptr);
  printf("Mem_Alloc(8000) = %p\n", Mem_Alloc(8000));
  void *memory = Mem_Alloc(20);
  printf("Mem_Alloc(20) = %p\n", memory);
  printf("Mem_Alloc(20) = %p\n", Mem_Alloc(20));
  printf("Mem_Free(memory) = %d\n", Mem_Free(memory));
  printf("Mem_Free(memory) = %d\n", Mem_Free(memory));
  printf("Mem_Alloc(20) = %p\n", Mem_Alloc(20));
  return 0;
}
