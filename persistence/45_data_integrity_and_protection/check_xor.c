// 1. Write a short C program that computes an XOR-based checksum over an input
// file, and prints the checksum as output. Use a 8-bit unsigned char to store
// the (one byte) checksum. Make some test files to see if it works as expected.

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

void usage_error()
{
  printf("Usage: check_xor file_to_checksum [read_buffer_size]\n");
  exit(1);
}

void malloc_error(size_t buffer_size)
{
  printf("Error: malloc() could not allocate buffer of size %ld bytes\n",
         buffer_size);
  exit(1);
}

void open_error(char *file_path)
{
  printf("Error: could not open() file %s\n", file_path);
  exit(1);
}

void read_error(char *file_path)
{
  printf("Error: could not read() file %s\n", file_path);
  exit(1);
}

int main(int argc, char *argv[])
{
  if (!(argc == 2 || argc == 3)) {
    usage_error();
  }

  char *file_path = argv[1];

  size_t buffer_size;
  if (argc == 3) {
    char *number_start = argv[2];
    char *number_end;
    int base_10 = 10;
    buffer_size = strtol(number_start, &number_end, base_10);
    if (number_start == number_end) {
      usage_error();
    }
  } else {
    buffer_size = 1;
  }

  unsigned char *buffer;
  if ((buffer = malloc(buffer_size)) == NULL) {
    malloc_error(buffer_size);
  }

  int file_descriptor;
  if ((file_descriptor = open(file_path, O_RDONLY)) < 0) {
    open_error(file_path);
  }

  unsigned char xor_checksum = 0;
  ssize_t bytes_read;
  while ((bytes_read = read(file_descriptor, buffer, buffer_size)) > 0) {
    for (int buffer_idx = 0; buffer_idx < bytes_read; buffer_idx++) {
      xor_checksum = xor_checksum ^ buffer[buffer_idx];
    }
  }
  if (bytes_read < 0) {
    read_error(file_path);
  }

  printf("XOR checksum for file: %u\n", (unsigned int) xor_checksum);

  free(buffer);

  return 0;
}
