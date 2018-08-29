#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <pthread.h>
#include <stdbool.h>
#include "page_queue.h"
#include "url_queue.h"
#include "parser_pool.h"

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

  struct parser_pool parser_pool;
  parser_pool_init(&parser_pool, 1);

  url_queue_enqueue(&url_queue, start_url);
  char *url = url_queue_dequeue(&url_queue);
  char *contents = _fetch_fn(url);
  printf("%s", contents);
  page_queue_enqueue(&page_queue, start_url, contents);

  parser_pool_start(&parser_pool, &page_queue, _edge_fn);

  parser_pool_join(&parser_pool);

  return -1;
}
