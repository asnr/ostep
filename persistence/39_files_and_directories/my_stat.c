#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Error: my_stat expects 1 argument");
    exit(1);
  }

  char *path = argv[1];

  struct stat file_status;
  int rc = stat(path, &file_status);

  if (rc != 0) {
    printf("Error: stat() failed");
    exit(1);
  }

  printf("File size: %lld, number of blocks allocated: %lld, reference count: %u\n",
         file_status.st_size, file_status.st_blocks, file_status.st_nlink);
  return 0;
}
