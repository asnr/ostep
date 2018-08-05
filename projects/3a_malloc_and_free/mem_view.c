#include <stdio.h>
#include "mem.h"

int
main()
{
  printf(">> View >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
  Mem_Init(64);
  Mem_Alloc(32);
  Mem_Dump();
  return 0;
}
