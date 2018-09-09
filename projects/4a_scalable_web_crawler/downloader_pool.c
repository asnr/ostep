#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include "url_queue.h"
#include "page_queue.h"
#include "downloader_pool.h"

static void *download_loop_entry(void *thread_args);

void downloader_pool_init(struct downloader_pool *pool, int num_downloader_threads)
{
  pool->num_threads = num_downloader_threads;
  pool->threads = calloc(pool->num_threads, sizeof(pthread_t));

  if (pool->threads == NULL) { exit(1); }
}

void downloader_pool_start(struct downloader_pool *pool,
                           struct url_queue *url_queue,
                           char * (*_fetch_fn)(char *url),
                           struct page_queue *page_queue)
{
  (pool->thread_args).url_queue = url_queue;
  (pool->thread_args)._fetch_fn = _fetch_fn;
  (pool->thread_args).page_queue = page_queue;

  for (int i = 0; i < pool->num_threads; i++) {
    int create_rc = pthread_create(&((pool->threads)[i]),
                                   NULL,
                                   download_loop_entry,
                                   &(pool->thread_args));
    if (create_rc != 0) { exit(1); }
  }
}

void downloader_pool_join(struct downloader_pool *pool)
{
  for (int i = 0; i < pool->num_threads; i++) {
    int join_rc = pthread_join((pool->threads)[i], NULL);
    if (join_rc != 0) { exit(1); }
  }
}

void download_loop(struct url_queue *url_queue,
                   char * (*_fetch_fn)(char *url),
                   struct page_queue *page_queue)
{
  char *url = url_queue_dequeue(url_queue);
  char *contents = _fetch_fn(url);
  printf(">>> Contents of URL %s\n", url);
  printf("%s\n", contents);
  page_queue_enqueue(page_queue, url, contents);

  for (int i = 0; i < 2; i ++) {
    url = url_queue_dequeue(url_queue);
    contents = _fetch_fn(url);
    printf(">>> Contents of URL %s\n", url);
    printf("%s\n", contents);
  }
}

static void *download_loop_entry(void *thread_args)
{
  struct downloader_thread_args *args = (struct downloader_thread_args *) thread_args;
  download_loop(args->url_queue, args->_fetch_fn, args->page_queue);
  return NULL;
}
