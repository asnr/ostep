#include <assert.h>
#include "job_counter.h"

void job_counter_init(struct job_counter *counter)
{
  counter->jobs_remaining = 0;
  counter->finish_token = false;
  int lock_init_rc = pthread_mutex_init(&(counter->lock), NULL);
  assert(lock_init_rc == 0);
}

bool keep_working(struct job_counter *counter)
{
  int lock_rc = pthread_mutex_lock(&(counter->lock));
  assert(lock_rc == 0);

  bool there_are_more_jobs = 0 < counter->jobs_remaining;

  bool keep_working;
  if (there_are_more_jobs) {
    keep_working = true;
  } else {
    keep_working = counter->finish_token;
    counter->finish_token = true;
  }

  lock_rc = pthread_mutex_unlock(&(counter->lock));
  assert(lock_rc == 0);

  return keep_working;
}

void finished_a_job(struct job_counter *counter)
{
  int lock_rc = pthread_mutex_lock(&(counter->lock));
  assert(lock_rc == 0);

  counter->jobs_remaining -= 1;

  lock_rc = pthread_mutex_unlock(&(counter->lock));
  assert(lock_rc == 0);
}

void add_a_job(struct job_counter *counter)
{
  int lock_rc = pthread_mutex_lock(&(counter->lock));
  assert(lock_rc == 0);

  counter->jobs_remaining += 1;

  lock_rc = pthread_mutex_unlock(&(counter->lock));
  assert(lock_rc == 0);
}
