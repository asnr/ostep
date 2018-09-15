#ifndef __PARSER_THREADS_H
#define __PARSER_THREADS_H

#include <pthread.h>
#include "url_queue.h"
#include "job_counter.h"
#include "string_set.h"

struct parser_thread_args {
  struct parser_pool *parser_pool;
  int num_download_workers;
  struct page_queue *page_queue;
  void (*_edge_fn)(char *from, char *to);
};

struct parser_pool {
  int num_threads;
  int num_download_workers;
  pthread_t *threads;
  struct parser_thread_args thread_args;
  struct job_counter job_counter;
  struct string_set visited_urls;
  struct url_queue *url_queue;
};

void parser_pool_init(struct parser_pool *pool,
                      int num_parser_threads,
                      int num_download_workers,
                      struct url_queue *url_queue);

void parser_pool_put_url_in_queue(struct parser_pool *pool, char *url);

void parser_pool_start(struct parser_pool *pool,
                       struct page_queue *page_queue,
                       void (*_edge_fn)(char *from, char *to));

void parser_pool_join(struct parser_pool *pool);

#endif
