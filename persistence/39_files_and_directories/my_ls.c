#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

typedef int bool;
const bool TRUE = 1;
const bool FALSE = 0;

bool arg_is_include_details_option(const char *arg)
{
  if (strncmp("-l", arg, 3) == 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}

void join_path(const char *directory_path,
               const char *filename,
               char *buf,
               const size_t buf_size)
{
  size_t buf_size_without_slash_and_nul = buf_size - 2;
  char *directory_end = stpncpy(buf, directory_path, buf_size_without_slash_and_nul);

  if (*(directory_end - 1) != '/') {
    *directory_end = '/';
    directory_end++;
    *directory_end = '\0';
  }

  size_t directory_length = directory_end - buf - 1;
  size_t space_for_filename = buf_size - directory_length - 1;
  strncat(buf, filename, space_for_filename);
}

void argument_error() {
  printf("Usage: my_ls [-l] [directory_path]\n");
  exit(1);
}

void cannot_open_directory_error(const char *directory_path) {
  printf("Error: could not open directory: %s\n", directory_path);
  exit(1);
}

void stat_error(const char *file_path) {
  printf("Error: stat() failed on path: %s\n", file_path);
  exit(1);
}

int main(int argc, char *argv[])
{
  const size_t MAX_PATH_LENGTH = 4096;
  char path[MAX_PATH_LENGTH];
  bool include_details = FALSE;
  char* directory_path = ".";

  if (argc == 2) {
    if (arg_is_include_details_option(argv[1])) {
      include_details = TRUE;
    } else {
      directory_path = argv[1];
    }
  } else if (argc == 3) {
    if (!arg_is_include_details_option(argv[1])) {
      argument_error();
    }
    include_details = TRUE;
    directory_path = argv[2];
  } else if (argc != 1) {
    argument_error();
  }

  DIR *directory = opendir(directory_path);
  if (directory == NULL) {
    cannot_open_directory_error(directory_path);
  }

  struct dirent *directory_entry;
  while ((directory_entry = readdir(directory)) != NULL) {
    if (include_details) {
      join_path(directory_path, directory_entry->d_name, path, MAX_PATH_LENGTH);
      struct stat file_status;
      int rc = stat(path, &file_status);
      if (rc != 0) { stat_error(path); }

      printf("%s - mode: %u, owner: %d, group: %d, create time: %lld.%.9ld, size: %lld\n",
             path,
             file_status.st_mode,
             file_status.st_uid,
             file_status.st_gid,
             (long long) file_status.st_birthtimespec.tv_sec,
             file_status.st_birthtimespec.tv_nsec,
             file_status.st_size);
    } else {
      printf("%s\n", directory_entry->d_name);
    }
  }
  return 0;
}
