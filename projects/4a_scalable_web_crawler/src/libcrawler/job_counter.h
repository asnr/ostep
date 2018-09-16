#ifndef __WORK_COUNTER_H
#define __WORK_COUNTER_H

#include <stddef.h>
#include <stdbool.h>
#include <pthread.h>

struct job_counter {
  size_t jobs_remaining;
  bool finish_token;
  pthread_mutex_t lock;
};

void job_counter_init(struct job_counter *counter);

bool keep_working(struct job_counter *counter);

void finished_a_job(struct job_counter *counter);

void add_a_job(struct job_counter *counter);

#endif
