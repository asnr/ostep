#include <stddef.h>
#include <assert.h>
#include "page_queue.h"

void page_queue_init(struct page_queue *queue)
{
  int cond_init_rc = pthread_cond_init(&(queue->has_elements_cond), NULL);
  assert(cond_init_rc == 0);
  int mutex_init_rc = pthread_mutex_init(&(queue->lock), NULL);
  assert(mutex_init_rc == 0);
  queue->size = 0;
}

void _page_queue_enqueue(struct page_queue *queue,
                         char *page_url,
                         char *page_contents)
{
  (queue->page).url = page_url;
  (queue->page).contents = page_contents;
  queue->size += 1;
}

void page_queue_enqueue(struct page_queue *queue,
                        char *page_url,
                        char *page_contents)
{
  int lock_rc = pthread_mutex_lock(&(queue->lock));
  assert(lock_rc == 0);

  _page_queue_enqueue(queue, page_url, page_contents);

  pthread_cond_signal(&(queue->has_elements_cond));

  lock_rc = pthread_mutex_unlock(&(queue->lock));
  assert(lock_rc == 0);
}

struct page *_page_queue_dequeue(struct page_queue *queue)
{
  struct page *page = &(queue->page);
  queue->size -= 1;
  return page;
}

struct page *page_queue_dequeue(struct page_queue *queue)
{
  int lock_rc = pthread_mutex_lock(&(queue->lock));
  assert(lock_rc == 0);

  while (queue->size == 0) {
    pthread_cond_wait(&(queue->has_elements_cond), &(queue->lock));
  }

  struct page *page = _page_queue_dequeue(queue);

  lock_rc = pthread_mutex_unlock(&(queue->lock));
  assert(lock_rc == 0);

  return page;
}
