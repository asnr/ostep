// Tail: Write a program that prints out the last few lines of a file.
// The program should be efficient, in that it seeks to near the end of the
// file, reads in a block of data, and then goes backwards until it finds the
// requested number of lines; at this point, it should print out those lines
// from beginning to the end of the file. To invoke the program, one should
// type: 'mytail -n file', where n is the number of lines at the end of the file
// to print. Useful interfaces: stat(), lseek(), open(), read(), close().

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

typedef int bool;
const bool TRUE = 1;
const bool FALSE = 0;

const int BUFFER_SIZE = 4096;

bool is_valid_number_argument(char *arg) {
  if (*arg == '-' || *(arg + 1) != '\0') {
    return TRUE;
  } else {
    return FALSE;
  }
}

void usage_error()
{
  dprintf(2, "Usage: ./my_tail -n file, where n is number of lines to print.\n");
  exit(1);
}

void stat_error(char *file_path)
{
  dprintf(2, "Error: stat() on file '%s' failed.\n", file_path);
  exit(1);
}

void file_open_error(char *file_path)
{
  dprintf(2, "Error: read() on file '%s' failed.\n", file_path);
  exit(1);
}

void buffer_too_small_error(int buffer_size)
{
  dprintf(2,
          "Internal buffer too small, needs to read more than %d bytes",
          buffer_size);
}

void read_error()
{
  dprintf(2, "Error: read() failed.\n");
  exit(1);
}

void write_error()
{
  dprintf(2, "Error: write() failed.\n");
  exit(1);
}

int main(int argc, char *argv[])
{
  long number_of_lines_to_print;
  char *file_path;

  if (argc != 3 || !is_valid_number_argument(argv[1])) {
    usage_error();
  }

  char *parse_end;
  const int base_10 = 10;
  number_of_lines_to_print = strtol(argv[1] + 1, &parse_end, base_10);
  if (*parse_end != '\0' || number_of_lines_to_print <= 0) {
    usage_error();
  }

  file_path = argv[2];

  struct stat file_status;
  int rc = stat(file_path, &file_status);
  if (rc != 0) {
    stat_error(file_path);
  }

  int descriptor_to_tail = open(file_path, O_RDONLY);
  if (descriptor_to_tail == -1) {
    file_open_error(file_path);
  }

  char buffer[BUFFER_SIZE];
  int number_of_lines_seen = 0;
  size_t read_offset_from_end = 0;
  size_t chunk_size = file_status.st_blksize;
  size_t file_size = file_status.st_size;
  char *print_start;
  while (read_offset_from_end < file_size &&
         number_of_lines_seen < number_of_lines_to_print) {
    read_offset_from_end += chunk_size;
    if (file_size < read_offset_from_end) {
      chunk_size -= read_offset_from_end - file_size;
      read_offset_from_end = file_size;
    }
    if (BUFFER_SIZE < read_offset_from_end) {
      buffer_too_small_error(BUFFER_SIZE);
    }

    char *chunk_start_in_buffer = buffer + BUFFER_SIZE - read_offset_from_end;
    char *chunk_end_in_buffer = chunk_start_in_buffer + chunk_size - 1;
    lseek(descriptor_to_tail, -1 * read_offset_from_end, SEEK_END);
    if (read(descriptor_to_tail, chunk_start_in_buffer, chunk_size) == -1) {
      read_error();
    }
    for (print_start = chunk_end_in_buffer;
         chunk_start_in_buffer <= print_start;
         print_start--) {
      if (*print_start == '\n') {
        number_of_lines_seen++;
        if (number_of_lines_seen == number_of_lines_to_print) {
          print_start++;  // move forward to the line start
          break;
        }
      }
    }
  }

  int stdout = 0;
  size_t chars_to_print = (buffer + BUFFER_SIZE) - print_start;
  if (write(stdout, print_start, chars_to_print) == -1) {
    write_error();
  }

  return 0;
}
