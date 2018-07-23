#include <stdio.h>
#include "mem.h"

int
main ()
{
  printf("Did run. Mem_Init() = %d, Mem_Free() = %d",
         Mem_Init(1, 1),
         Mem_Free((void *)0));
  return 0;
}
