#ifndef __PARSER_THREADS_H
#define __PARSER_THREADS_H

#include <pthread.h>

struct parser_thread_args {
  struct page_queue *page_queue;
  void (*_edge_fn)(char *from, char *to);
};

struct parser_pool {
  int num_threads;
  pthread_t *threads;
  struct parser_thread_args thread_args;
};


void parser_pool_init(struct parser_pool *pool, int num_parser_threads);

void parser_pool_start(struct parser_pool *pool,
                       struct page_queue *page_queue,
                       void (*_edge_fn)(char *from, char *to));

void parser_pool_join(struct parser_pool *pool);

#endif
