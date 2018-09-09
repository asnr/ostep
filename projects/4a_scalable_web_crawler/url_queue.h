#ifndef __URL_QUEUE_H
#define __URL_QUEUE_H

#include <pthread.h>

extern char NO_MORE_URLS[];
extern size_t SIZE_OF_NO_MORE_URLS;

struct url_queue {
  pthread_cond_t has_elements_cond;
  pthread_cond_t has_space_cond;
  pthread_mutex_t lock;
  int size;
  int capacity;
  int first_element_index;
  char **urls;
};

void url_queue_init(struct url_queue *queue, int queue_size);

void url_queue_enqueue(struct url_queue *queue, char *url);

char *url_queue_dequeue(struct url_queue *queue);

#endif
