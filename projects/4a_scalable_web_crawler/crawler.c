#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <pthread.h>
#include <stdbool.h>
#include <ctype.h>

const char LINK_PREFIX[] = "link:";

const size_t MAX_URL_SIZE = 1028;
char url_buffer[MAX_URL_SIZE];

void parse_page(char *url, char *contents, void (*_edge_fn)(char *from, char *to));

int crawl(char *start_url,
	  int download_workers,
	  int parse_workers,
	  int queue_size,
	  char * (*_fetch_fn)(char *url),
	  void (*_edge_fn)(char *from, char *to)) {

  char *contents = _fetch_fn(start_url);
  printf("%s", contents);

  parse_page(start_url, contents, _edge_fn);

  return -1;
}

bool copy_url_to_buffer(char *link_prefix, char *buffer, size_t buffer_size)
{
  size_t prefix_length = sizeof(LINK_PREFIX) - 1;
  char *url_start = link_prefix + prefix_length;
  if (isspace(*url_start)) { return false; }

  for (size_t i = 0; i < buffer_size - 1; i++) {
    char curr_char = *(url_start + i);
    if (isspace(curr_char)) {
      *(buffer + i) = '\0';
      return true;
    }

    *(buffer + i) = curr_char;
  }

  return false;
}

void parse_page(char *url, char *contents, void (*_edge_fn)(char *from, char *to))
{
  char *link_prefix = strstr(contents, LINK_PREFIX);
  if (copy_url_to_buffer(link_prefix, url_buffer, MAX_URL_SIZE)) {
    /* printf(">>>>>>>>> found URL: %s", url_buffer); */
    _edge_fn(url, url_buffer);
  } else {
    // link too long or empty, do something?
  }
}
