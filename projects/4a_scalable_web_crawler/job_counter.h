#ifndef __WORK_COUNTER_H
#define __WORK_COUNTER_H

#include <stddef.h>
#include <stdbool.h>

struct job_counter {
  size_t jobs_remaining;
};

void job_counter_init(struct job_counter *counter);

bool there_are_more_jobs(struct job_counter *counter);

void finished_a_job(struct job_counter *counter);

void add_a_job(struct job_counter *counter);

#endif
