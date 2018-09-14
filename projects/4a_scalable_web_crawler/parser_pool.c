#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <assert.h>
#include "page_queue.h"
#include "url_queue.h"
#include "parser_pool.h"
#include "job_counter.h"
#include "string_set.h"

const char LINK_PREFIX[] = "link:";

static void *parse_loop_entry(void *thread_args);

void parser_pool_init(struct parser_pool *pool,
                      int num_parser_threads,
                      int num_download_workers)
{
  pool->num_threads = num_parser_threads;
  pool->num_download_workers = num_download_workers;
  pool->threads = calloc(pool->num_threads, sizeof(pthread_t));
  string_set_init(&(pool->visited_urls));
  assert(pool->threads != NULL);
}

void parser_pool_start(struct parser_pool *pool,
                       struct job_counter *job_counter,
                       struct page_queue *page_queue,
                       void (*_edge_fn)(char *from, char *to),
                       struct url_queue *url_queue)
{
  pool->job_counter = job_counter;

  (pool->thread_args).num_download_workers = pool->num_download_workers;
  (pool->thread_args).parser_pool = pool;
  (pool->thread_args).page_queue = page_queue;
  (pool->thread_args)._edge_fn = _edge_fn;
  (pool->thread_args).url_queue = url_queue;

  for (int i = 0; i < pool->num_threads; i++) {
    int create_rc =
      pthread_create(&((pool->threads)[i]),
                     NULL,
                     parse_loop_entry,
                     &(pool->thread_args));

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

char *copy_url_to_buffer(char *link_prefix, char **buffer)
{
  size_t prefix_length = sizeof(LINK_PREFIX) - 1;
  char *url_start = link_prefix + prefix_length;

  size_t length = url_length(url_start);
  *buffer = (char *) malloc(length + 1);
  assert(*buffer != NULL);

  memcpy(*buffer, url_start, length);
  *(*buffer + length) = '\0';

  return url_start + length;
}

void parse_page(struct page *page,
                struct parser_pool *parser_pool,
                void (*_edge_fn)(char *from, char *to),
                struct url_queue *url_queue)
{
  char *position = page->contents;
  char *next_link_prefix = strstr(position, LINK_PREFIX);
  char *url_buffer;
  while (next_link_prefix != NULL) {
    position = copy_url_to_buffer(next_link_prefix, &url_buffer);
    bool valid_url = *url_buffer != '\0';
    if (valid_url) {
      _edge_fn(page->url, url_buffer);
      bool new_url = !string_set_contains(&(parser_pool->visited_urls), url_buffer);
      if (new_url) {
        add_a_job(parser_pool->job_counter);
        string_set_add(&(parser_pool->visited_urls), url_buffer);
        url_queue_enqueue(url_queue, url_buffer);
      }
    } else {
      // TODO: link too long or empty, do something?
      exit(1);
    }

    next_link_prefix = strstr(position, LINK_PREFIX);
  }
}

void parse_loop(int num_download_workers,
                struct parser_pool *parser_pool,
                struct page_queue *page_queue,
                void (*_edge_fn)(char *, char *),
                struct url_queue *url_queue)
{
  while (there_are_more_jobs(parser_pool->job_counter)) {
    struct page *page = page_queue_dequeue(page_queue);
    parse_page(page, parser_pool, _edge_fn, url_queue);
    finished_a_job(parser_pool->job_counter);
  }

  for (int i = 0; i < num_download_workers; i++) {
    url_queue_enqueue(url_queue, NO_MORE_URLS);
  }
}

static void *parse_loop_entry(void *thread_args)
{
  struct parser_thread_args *args = (struct parser_thread_args *) thread_args;
  parse_loop(args->num_download_workers,
             args->parser_pool,
             args->page_queue,
             args->_edge_fn,
             args->url_queue);
  return NULL;
}
