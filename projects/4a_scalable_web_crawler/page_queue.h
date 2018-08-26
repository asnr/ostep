#ifndef __PAGE_QUEUE_H
#define __PAGE_QUEUE_H

struct page {
  char *url;
  char *contents;
};

struct page_queue {
  int size;
  struct page page;
};

void page_queue_init(struct page_queue *queue);

void page_queue_enqueue(struct page_queue *queue,
                        char *page_url,
                        char *page_contents);

struct page *page_queue_dequeue(struct page_queue *queue);

#endif
