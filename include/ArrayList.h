#ifndef ARRAY_LIST_H
#define ARRAY_LIST_H

#include <stdio.h>

typedef struct {
  size_t elem_size;
  int capacity;
  int len;
  void **items;
} ArrayList;

void array_list_init(ArrayList *array_list, size_t elem_size);

int array_list_init_capacity(ArrayList *array_list, size_t elem_size,
                             int capacity);

void array_list_deinit(ArrayList *array_list);

int array_list_append(ArrayList *array_list, void *item);

void *array_list_swap_remove(ArrayList *array_list, int index);

void *array_list_ordered_remove(ArrayList *array_list, int index);

void *array_list_pop(ArrayList *array_list);

void *array_list_pop_or_null(ArrayList *array_list);

#endif // ARRAY_LIST_H
