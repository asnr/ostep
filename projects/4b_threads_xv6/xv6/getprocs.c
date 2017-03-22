#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  if (argc > 1) {
    printf(2, "Usage: getprocs\n");
    exit();
  }

  printf(1, "%d\n", getprocs());

  exit();
}
