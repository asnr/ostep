#include <stdio.h>
#include "mem.h"

int
main()
{
  printf(">> View >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
  int allocation_size = 32;
  int region_size =
    sizeof(struct region) + sizeof(struct block) + allocation_size + 32;
  Mem_Init(region_size);
  Mem_Alloc(allocation_size);
  Mem_Dump();
  return 0;
}
