// Now write a short C program that computes the Fletcher checksum over an input
// file. Once again, test your program to see if it works.

// We implement Fletcher-16

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define FLETCHERS_MODULUS 255

void usage_error()
{
  printf("Usage: check_fletcher file_to_checksum [read_buffer_size]\n");
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

  unsigned int checksum_low_byte = 0;
  unsigned int checksum_high_byte = 0;
  ssize_t bytes_read;
  while ((bytes_read = read(file_descriptor, buffer, buffer_size)) > 0) {
    for (int buffer_idx = 0; buffer_idx < bytes_read; buffer_idx++) {
      checksum_low_byte += buffer[buffer_idx];
      while (checksum_low_byte >= FLETCHERS_MODULUS) {
        checksum_low_byte -= FLETCHERS_MODULUS;
      }
      checksum_high_byte += checksum_low_byte;
      while (checksum_high_byte >= FLETCHERS_MODULUS) {
        checksum_high_byte -= FLETCHERS_MODULUS;
      }
    }
  }
  if (bytes_read < 0) {
    read_error(file_path);
  }

  unsigned int fletcher_16_checksum = (checksum_high_byte << 8) + checksum_low_byte;
  printf("Fletcher-16 checksum for file: %u\n", fletcher_16_checksum);

  free(buffer);

  return 0;
}
