#ifndef __STRING_SET_H
#define __STRING_SET_H

#include <stddef.h>
#include <stdbool.h>

struct string_set {
  char **hash_table;
  size_t capacity;
};

void string_set_init(struct string_set *set);

void string_set_add(struct string_set *set, char *s);

bool string_set_contains(struct string_set *set, char *s);

#endif
