#ifndef __DOWNLOADER_POOL_H
#define __DOWNLOADER_POOL_H

#include <pthread.h>

struct downloader_thread_args {
  struct url_queue *url_queue;
  struct page_queue *page_queue;
  char * (*_fetch_fn)(char *url);
};

struct downloader_pool {
  int num_threads;
  pthread_t *threads;
  struct downloader_thread_args thread_args;
};

void downloader_pool_init(struct downloader_pool *pool, int num_downloader_threads);

void downloader_pool_start(struct downloader_pool *pool,
                           struct url_queue *url_queue,
                           char * (*_fetch_fn)(char *url),
                           struct page_queue *page_queue);

void downloader_pool_join(struct downloader_pool *pool);

#endif
