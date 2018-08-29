#include <stdlib.h>
#include "url_queue.h"

void url_queue_init(struct url_queue *queue, int queue_size)
{
  queue->size = queue_size;
  queue->urls = (char **) calloc(queue_size, sizeof(char*));

  if (queue->urls == NULL) {
    exit(1);
  }
}

void url_queue_enqueue(struct url_queue *queue, char *url)
{
  (queue->urls)[0] = url;
}

char *url_queue_dequeue(struct url_queue *queue)
{
  return (queue->urls)[0];
}
