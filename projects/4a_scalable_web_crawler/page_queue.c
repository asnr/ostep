#include <stddef.h>
#include "page_queue.h"

void page_queue_init(struct page_queue *queue)
{
  queue->size = 0;
}

void page_queue_enqueue(struct page_queue *queue,
                        char *page_url,
                        char *page_contents)
{
  (queue->page).url = page_url;
  (queue->page).contents = page_contents;
  queue->size += 1;
}

struct page *page_queue_dequeue(struct page_queue *queue)
{
  if (queue->size == 0) {
    return NULL;
  }

  queue->size -= 1;
  return &(queue->page);
}
