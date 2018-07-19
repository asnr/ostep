#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int *x = 0;
  int y = *x;
  char *c = 0;
  printf(1, "As int: %d\n", y);
  printf(1, "As string: %s\n", c);

  exit();
}
