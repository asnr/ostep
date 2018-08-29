#ifndef __URL_QUEUE_H
#define __URL_QUEUE_H

struct url_queue {
  int size;
  char **urls;
};

void url_queue_init(struct url_queue *queue, int queue_size);

void url_queue_enqueue(struct url_queue *queue, char *url);

char *url_queue_dequeue(struct url_queue *queue);

#endif
