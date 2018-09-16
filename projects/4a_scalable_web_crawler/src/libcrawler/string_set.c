#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include "string_set.h"

const size_t INITIAL_CAPACITY = 1009;
const size_t MAX_SIZE = 900;

void string_set_init(struct string_set *set)
{
  set->size = 0;
  set->capacity = INITIAL_CAPACITY;
  set->hash_table = calloc(set->capacity, sizeof(*(set->hash_table)));
  assert(set->hash_table != NULL);

  int lock_init_rc = pthread_mutex_init(&(set->lock), NULL);
  assert(lock_init_rc == 0);
}

// Larson's hash
static size_t hash(char *s)
{
  size_t hash = 0;
  while (*s) {
    hash = hash * 101 + (size_t) *s++;
  }
  return hash;
}

static size_t index_for_element(struct string_set *set, char *s)
{
  size_t index = hash(s) % set->capacity;
  while((set->hash_table)[index] != NULL &&
        strcmp((set->hash_table)[index], s) != 0) {
    index++;
    if (index >= set->capacity) { index -= set->capacity; }
  }
  return index;
}

void string_set_add(struct string_set *set, char *s)
{
  int lock_rc = pthread_mutex_lock(&(set->lock));
  assert(lock_rc == 0);

  assert(set->size <= MAX_SIZE);

  size_t index = index_for_element(set, s);
  if ((set->hash_table)[index] == NULL) {
    (set->hash_table)[index] = s;
    set->size += 1;
  }

  lock_rc = pthread_mutex_unlock(&(set->lock));
  assert(lock_rc == 0);
}

bool string_set_contains(struct string_set *set, char *s)
{
  int lock_rc = pthread_mutex_lock(&(set->lock));
  assert(lock_rc == 0);

  size_t index = index_for_element(set, s);
  bool set_contains_string = (set->hash_table)[index] != NULL;

  lock_rc = pthread_mutex_unlock(&(set->lock));
  assert(lock_rc == 0);

  return set_contains_string;
}
