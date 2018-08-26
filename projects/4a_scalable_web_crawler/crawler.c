#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <pthread.h>
#include <stdbool.h>
#include <ctype.h>
#include "page_queue.h"

const char LINK_PREFIX[] = "link:";

const size_t MAX_URL_SIZE = 1028;
const size_t MAX_URL_SIZE_WITH_NUL = 1028 + 1;
char url_buffer[MAX_URL_SIZE_WITH_NUL];

void parse_loop(struct page_queue *page_queue, void (*_edge_fn)(char *, char *));

int crawl(char *start_url,
	        int download_workers,
	        int parse_workers,
	        int queue_size,
	        char * (*_fetch_fn)(char *url),
	        void (*_edge_fn)(char *from, char *to)) {
  /* struct url_queue url_queue; */
  /* url_queue_init(&url_queue, queue_size); */
  struct page_queue page_queue;
  page_queue_init(&page_queue);

  char *contents = _fetch_fn(start_url);
  printf("%s", contents);
  page_queue_enqueue(&page_queue, start_url, contents);

  parse_loop(&page_queue, _edge_fn);

  return -1;
}

bool is_url_terminator(char c)
{
  return isspace(c) || c == '\0';
}

size_t url_length(char *url)
{
  size_t length = 0;
  while (!is_url_terminator(*(url+length))) {
    length++;
  }
  return length;
}

char *copy_url_to_buffer(char *link_prefix, char *buffer, size_t buffer_size)
{
  size_t prefix_length = sizeof(LINK_PREFIX) - 1;
  char *url_start = link_prefix + prefix_length;

  size_t length = url_length(url_start);
  if (0 < length && length < buffer_size) {
    memcpy(buffer, url_start, length);
    *(buffer + length) = '\0';
  } else {
    *buffer = '\0';
  }

  return url_start + length;
}

void parse_page(struct page *page, void (*_edge_fn)(char *from, char *to))
{
  char *position = page->contents;
  char *next_link_prefix = strstr(position, LINK_PREFIX);
  while (next_link_prefix != NULL) {
    position = copy_url_to_buffer(next_link_prefix, url_buffer, MAX_URL_SIZE);
    bool valid_url = *url_buffer != '\0';
    if (valid_url) {
      _edge_fn(page->url, url_buffer);
    } else {
      // TODO: link too long or empty, do something?
    }

    next_link_prefix = strstr(position, LINK_PREFIX);
  }
}

void parse_loop(struct page_queue *page_queue, void (*_edge_fn)(char *, char *))
{
  struct page *page = page_queue_dequeue(page_queue);
  while (page) {
    parse_page(page, _edge_fn);
    page = page_queue_dequeue(page_queue);
  }
}
