#include <stdlib.h>
#include <assert.h>
#include "url_queue.h"

void url_queue_init(struct url_queue *queue, int queue_size)
{
  int cond_init_rc = pthread_cond_init(&(queue->has_elements_cond), NULL);
  assert(cond_init_rc == 0);
  cond_init_rc = pthread_cond_init(&(queue->has_space_cond), NULL);
  assert(cond_init_rc == 0);
  int lock_init_rc = pthread_mutex_init(&(queue->lock), NULL);
  assert(lock_init_rc == 0);

  queue->urls = (char **) calloc(queue_size, sizeof(char*));
  assert(queue->urls != NULL);

  queue->capacity = queue_size;
  queue->size = 0;
  queue->first_element_index = 0;
}

void _url_queue_enqueue(struct url_queue *queue, char *url)
{
  int new_element_index =
    (queue->first_element_index + queue->size) % queue->capacity;
  (queue->urls)[new_element_index] = url;
  queue->size += 1;
}

void url_queue_enqueue(struct url_queue *queue, char *url)
{
  int lock_rc = pthread_mutex_lock(&(queue->lock));
  assert(lock_rc == 0);

  while (queue->size == queue->capacity) {
    pthread_cond_wait(&(queue->has_space_cond), &(queue->lock));
  }

  _url_queue_enqueue(queue, url);

  pthread_cond_signal(&(queue->has_elements_cond));

  lock_rc = pthread_mutex_unlock(&(queue->lock));
  assert(lock_rc == 0);
}

char *_url_queue_dequeue(struct url_queue *queue)
{
  char *url = (queue->urls)[queue->first_element_index];
  (queue->urls)[queue->first_element_index] = NULL;
  queue->size -= 1;
  queue->first_element_index = (queue->first_element_index + 1) % queue->capacity;
  return url;
}

char *url_queue_dequeue(struct url_queue *queue)
{
  int lock_rc = pthread_mutex_lock(&(queue->lock));
  assert(lock_rc == 0);

  while (queue->size == 0) {
    pthread_cond_wait(&(queue->has_elements_cond), &(queue->lock));
  }

  char *url = _url_queue_dequeue(queue);

  pthread_cond_signal(&(queue->has_space_cond));

  lock_rc = pthread_mutex_unlock(&(queue->lock));
  assert(lock_rc == 0);

  return url;
}
