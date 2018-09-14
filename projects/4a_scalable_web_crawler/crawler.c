#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <pthread.h>
#include <stdbool.h>
#include "page_queue.h"
#include "url_queue.h"
#include "job_counter.h"
#include "parser_pool.h"
#include "downloader_pool.h"

int crawl(char *start_url,
	        int download_workers,
	        int parse_workers,
	        int queue_size,
	        char * (*_fetch_fn)(char *url),
	        void (*_edge_fn)(char *from, char *to)) {
  struct url_queue url_queue;
  url_queue_init(&url_queue, queue_size);

  struct page_queue page_queue;
  page_queue_init(&page_queue);

  struct job_counter job_counter;
  job_counter_init(&job_counter);

  download_workers = 2;
  struct downloader_pool downloader_pool;
  downloader_pool_init(&downloader_pool, download_workers);

  struct parser_pool parser_pool;
  parser_pool_init(&parser_pool, 1, download_workers, &job_counter, &url_queue);

  parser_pool_put_url_in_queue(&parser_pool, start_url);

  downloader_pool_start(&downloader_pool, &url_queue, _fetch_fn, &page_queue);
  parser_pool_start(&parser_pool, &page_queue, _edge_fn);

  downloader_pool_join(&downloader_pool);
  parser_pool_join(&parser_pool);

  return 0;
}
