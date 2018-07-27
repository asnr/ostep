#include "mem.h"

int
main()
{
  Mem_Init(64);
  Mem_Alloc(32);
  Mem_Dump();
  return 0;
}
