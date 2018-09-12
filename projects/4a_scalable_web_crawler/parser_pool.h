#ifndef __PARSER_THREADS_H
#define __PARSER_THREADS_H

#include <pthread.h>
#include "job_counter.h"

struct parser_thread_args {
  struct parser_pool *parser_pool;
  int num_download_workers;
  struct page_queue *page_queue;
  void (*_edge_fn)(char *from, char *to);
  struct url_queue *url_queue;
};

struct parser_pool {
  int num_threads;
  int num_download_workers;
  pthread_t *threads;
  struct parser_thread_args thread_args;
  struct job_counter *job_counter;
};

void parser_pool_init(struct parser_pool *pool,
                      int num_parser_threads,
                      int num_download_workers);

void parser_pool_start(struct parser_pool *pool,
                       struct job_counter *job_counter,
                       struct page_queue *page_queue,
                       void (*_edge_fn)(char *from, char *to),
                       struct url_queue *url_queue);

void parser_pool_join(struct parser_pool *pool);

#endif
