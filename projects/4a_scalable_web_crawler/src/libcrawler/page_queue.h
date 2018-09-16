#ifndef __PAGE_QUEUE_H
#define __PAGE_QUEUE_H

#include <stdbool.h>
#include <pthread.h>

struct page {
  char *url;
  char *contents;
  bool no_more_pages;
};

bool no_more_pages(struct page *page);

struct page_list {
  struct page page;
  struct page_list *next;
};

struct page_queue {
  pthread_cond_t has_elements_cond;
  pthread_mutex_t lock;
  struct page_list *page_list;
};

void page_queue_init(struct page_queue *queue);

void page_queue_enqueue(struct page_queue *queue,
                        char *page_url,
                        char *page_contents);

void page_queue_enqueue_no_more_pages(struct page_queue *queue);

struct page *page_queue_dequeue(struct page_queue *queue);

#endif
