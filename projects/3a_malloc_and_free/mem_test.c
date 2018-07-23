#include <stdio.h>
#include "mem.h"

int
main ()
{
  void *ptr = Mem_Init(409);
  printf("Mem_Init() = %p\n", ptr);
  printf("Mem_Alloc(8000) = %p\n", Mem_Alloc(8000));
  printf("Mem_Alloc(20) = %p\n", Mem_Alloc(20));
  return 0;
}
