#ifndef ARRAY_LIST_H
#define ARRAY_LIST_H

#include <stdio.h>

typedef struct {
  size_t elem_size;
  int capacity;
  int len;
  void **items;
} ArrayList;

ArrayList array_list_init(size_t);

void array_list_deinit(ArrayList *);

int array_list_append(ArrayList *, void *);

void *array_list_swap_remove(ArrayList *, int);

void *array_list_ordered_remove(ArrayList *, int);

void *array_list_pop(ArrayList *);

#endif // ARRAY_LIST_H
