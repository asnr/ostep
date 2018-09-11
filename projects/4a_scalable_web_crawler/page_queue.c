#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "page_queue.h"

void page_queue_init(struct page_queue *queue)
{
  int cond_init_rc = pthread_cond_init(&(queue->has_elements_cond), NULL);
  assert(cond_init_rc == 0);
  int mutex_init_rc = pthread_mutex_init(&(queue->lock), NULL);
  assert(mutex_init_rc == 0);
  queue->page_list = NULL;
}

void _page_queue_enqueue(struct page_queue *queue,
                         char *page_url,
                         char *page_contents)
{
  struct page_list *end_node = malloc(sizeof(struct page_list));
  assert(end_node != NULL);

  (end_node->page).url = page_url;
  (end_node->page).contents = page_contents;
  end_node->next = NULL;

  struct page_list **list = &(queue->page_list);
  while (*list != NULL) {
    list = &((*list)->next);
  }

  *list = end_node;
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
  struct page *page = &(queue->page_list->page);
  queue->page_list = queue->page_list->next;
  return page;
}

bool page_queue_is_empty(struct page_queue *queue)
{
  return queue->page_list == NULL;
}

struct page *page_queue_dequeue(struct page_queue *queue)
{
  int lock_rc = pthread_mutex_lock(&(queue->lock));
  assert(lock_rc == 0);

  while (page_queue_is_empty(queue)) {
    pthread_cond_wait(&(queue->has_elements_cond), &(queue->lock));
  }

  struct page *page = _page_queue_dequeue(queue);

  lock_rc = pthread_mutex_unlock(&(queue->lock));
  assert(lock_rc == 0);

  return page;
}
