#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "page_queue.h"
#include "parser_pool.h"

const char LINK_PREFIX[] = "link:";

const size_t MAX_URL_SIZE = 1028;
const size_t MAX_URL_SIZE_WITH_NUL = 1028 + 1;
char url_buffer[MAX_URL_SIZE_WITH_NUL];

static void *parse_loop_entry(void *thread_args);

void parser_pool_init(struct parser_pool *pool, int num_parser_threads)
{
  pool->num_threads = num_parser_threads;
  pool->threads = calloc(pool->num_threads, sizeof(pthread_t));

  if (pool->threads == NULL) {
    exit(1);
  }
}

void parser_pool_start(struct parser_pool *pool,
                       struct page_queue *page_queue,
                       void (*_edge_fn)(char *from, char *to))
{
  (pool->thread_args).page_queue = page_queue;
  (pool->thread_args)._edge_fn = _edge_fn;

  for (int i = 0; i < pool->num_threads; i++) {
    int create_rc =
      pthread_create(&((pool->threads)[i]), NULL, parse_loop_entry, &(pool->thread_args));

    if (create_rc != 0) { exit(1); }
  }
}

void parser_pool_join(struct parser_pool *pool)
{
  for (int i = 0; i < pool->num_threads; i++) {
    int join_rc = pthread_join((pool->threads)[i], NULL);

    if (join_rc != 0) { exit(1); }
  }
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
      exit(1);
    }

    next_link_prefix = strstr(position, LINK_PREFIX);
  }
}

void parse_loop(struct page_queue *page_queue, void (*_edge_fn)(char *, char *))
{
  struct page *page = NULL;
  page = page_queue_dequeue(page_queue);
  // Keep parsing until there is no more
  while (page) {
    parse_page(page, _edge_fn);
    page = page_queue_dequeue(page_queue);
  }
}

static void *parse_loop_entry(void *thread_args)
{
  struct parser_thread_args *args = (struct parser_thread_args *) thread_args;
  parse_loop(args->page_queue, args->_edge_fn);
  return NULL;
}
