#include "types.h"
#include "user.h"
#include "pstat.h"

int
main(int argc, char *argv[])
{
  if (argc > 1) {
    printf(2, "Usage: ps\n");
    exit();
  }

  struct pstat stats;
  if (getpinfo(&stats) < 0) {
    printf(2, "Error: getpinfo() failed.\n");
    exit();
  }

  int pidx;
  for (pidx = 0; pidx < NPROC; pidx++) {
    if (stats.inuse[pidx]) {
        printf(1, "PID: %d, lticks: %d, hticks: %d\n",
               stats.pid[pidx], stats.lticks[pidx], stats.hticks[pidx]);
    }
  }

  exit();
}
