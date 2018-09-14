#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "string_set.h"

const size_t INITIAL_CAPACITY = 1009;
const size_t MAX_SIZE = 900;

void string_set_init(struct string_set *set)
{
  set->capacity = INITIAL_CAPACITY;
  set->hash_table = calloc(set->capacity, sizeof(*(set->hash_table)));
  assert(set->hash_table != NULL);
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
  size_t index = index_for_element(set, s);
  if ((set->hash_table)[index] == NULL) {
    (set->hash_table)[index] = s;
  }
}

bool string_set_contains(struct string_set *set, char *s)
{
  size_t index = index_for_element(set, s);
  return (set->hash_table)[index] != NULL;
}
