// Recursive Search: write a program that prints out the names of each file and
// directory in the file system tree, starting at a given point in the tree.
// For example, when run without arguments, the program should start with the
// current working directory and print its contents, as well as the contents of
// any sub-directories, etc., until the entire tree, root at the CWD, is
// printed. If given a single argument (of a directory name), use that as the
// root of the tree instead. Refine your recursive search with more fun options,
// similar to the powerful find command line tool. Useful interfaces: you figure
// it out.

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>

#define PATH_MAXSIZE 4096

char current_working_directory[] = ".";

void usage_error()
{
  printf("Usage: my_find [directory]\n");
  exit(1);
}

void opendir_error(const char *directory_path)
{
  printf("Error: opendir() failed on %s\n", directory_path);
  exit(1);
}

void closedir_error(const char *directory_path)
{
  printf("Error: closedir() failed on %s\n", directory_path);
  exit(1);
}

void buffer_too_small_error(const char *directory_path, const char *filename)
{
  printf("Error: internal buffer to small to hold '%s/%s", directory_path, filename);
  exit(1);
}

void search(const char* directory_path)
{
  DIR *directory = opendir(directory_path);
  if (directory == NULL) opendir_error(directory_path);

  char current_path[PATH_MAXSIZE];
  char *current_filename;
  struct dirent *directory_entry;
  while ((directory_entry = readdir(directory)) != NULL) {
    current_filename = directory_entry->d_name;
    if (strncmp(current_filename, ".", 2) == 0 ||
        strncmp(current_filename, "..", 3) == 0) {
      continue;
    }
    switch(directory_entry->d_type) {
    case DT_DIR:
      if (strlcpy(current_path, directory_path, PATH_MAXSIZE) > PATH_MAXSIZE ||
          strlcat(current_path, "/", PATH_MAXSIZE) > PATH_MAXSIZE ||
          strlcat(current_path, current_filename, PATH_MAXSIZE) > PATH_MAXSIZE) {
        buffer_too_small_error(directory_path, current_filename);
      }
      printf("%s\n", current_path);
      search(current_path);
      break;
    default:
      printf("%s/%s\n", directory_path, current_filename);
      break;
    }
  }

  if (closedir(directory) == -1) closedir_error(directory_path);
}

int main(int argc, char *argv[])
{
  char *starting_directory;
  if (argc == 1) {
    starting_directory = current_working_directory;
  } else if (argc == 2) {
    starting_directory = argv[1];
  } else {
    usage_error();
  }

  search(starting_directory);

  return 0;
}

