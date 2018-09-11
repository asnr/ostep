#include "job_counter.h"

void job_counter_init(struct job_counter *counter)
{
  counter->jobs_remaining = 0;
}

bool there_are_more_jobs(struct job_counter *counter)
{
  return 0 < counter->jobs_remaining;
}

void finished_a_job(struct job_counter *counter)
{
  counter->jobs_remaining -= 1;
}

void add_a_job(struct job_counter *counter)
{
  counter->jobs_remaining += 1;
}
