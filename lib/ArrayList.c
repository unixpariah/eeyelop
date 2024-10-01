#include "ArrayList.h"
#include "stdio.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void array_list_init(ArrayList *array_list, size_t elem_size) {
  array_list->capacity = 0;
  array_list->len = 0;
  array_list->items = NULL;
  array_list->elem_size = elem_size;
}

int array_list_init_capacity(ArrayList *array_list, size_t elem_size,
                             int capacity) {
  void *items = malloc(sizeof(void *));
  if (items == NULL) {
    return 1;
  }

  array_list->capacity = capacity;
  array_list->len = 0;
  array_list->items = items;
  array_list->elem_size = elem_size;

  return 0;
}

int grow_capacity(int current, int minimum) {
  int new_capacity = current;
  while (new_capacity < minimum) {
    new_capacity += new_capacity / 2 + 8;
  }
  return new_capacity;
}

int array_list_ensure_total_capacity_precise(ArrayList *array_list,
                                             int new_capacity) {
  if (array_list->capacity >= new_capacity) {
    return 0;
  }

  void **new_items = (void **)realloc((void *)array_list->items,
                                      new_capacity * sizeof(void *));
  if (!new_items) {
    return -1;
  }

  array_list->capacity = new_capacity;
  array_list->items = new_items;

  return 0;
}

int array_list_ensure_total_capacity(ArrayList *array_list, int new_capacity) {
  if (array_list->capacity >= new_capacity) {
    return 0;
  }

  int better_capacity = grow_capacity(array_list->capacity, new_capacity);
  return array_list_ensure_total_capacity_precise(array_list, better_capacity);
}

void **array_list_add_one_assume_capacity(ArrayList *array_list) {
  assert(array_list->len < array_list->capacity);
  array_list->len++;
  return &array_list->items[array_list->len - 1];
}

void **array_list_add_one(ArrayList *array_list) {
  int new_len = array_list->len + 1;
  if (array_list_ensure_total_capacity(array_list, new_len) == -1) {
    return NULL;
  };
  return array_list_add_one_assume_capacity(array_list);
}

int array_list_append(ArrayList *array_list, void *item) {
  void **new_item_ptr = array_list_add_one(array_list);
  if (new_item_ptr == NULL) {
    return -1;
  }

  *new_item_ptr = item;

  return 0;
}

void *array_list_pop(ArrayList *array_list) {
  void *item = array_list->items[array_list->len - 1];
  array_list->items[array_list->len - 1] = 0;

  array_list->len--;

  return item;
}

void *array_list_pop_or_null(ArrayList *array_list) {
  if (array_list->len == 0) {
    return NULL;
  }

  return array_list_pop(array_list);
}

void *array_list_swap_remove(ArrayList *array_list, int index) {
  if (index < 0 || index >= array_list->len) {
    return NULL;
  }

  if (array_list->len - 1 == index) {
    return array_list_pop(array_list);
  }

  void *old_item = array_list->items[index];
  array_list->items[index] = array_list_pop(array_list);

  return old_item;
}

void *array_list_ordered_remove(ArrayList *array_list, int index) {
  if (index < 0 || index >= array_list->len) {
    return NULL;
  }

  if (array_list->len - 1 == index) {
    return array_list_pop(array_list);
  }

  void *old_item = array_list->items[index];
  for (int i = index; i < array_list->len - 1; i++) {
    array_list->items[i] = array_list->items[i + 1];
  }

  array_list->len--;

  return old_item;
}

void array_list_deinit(ArrayList *array_list) {
  free((void *)array_list->items);
  array_list->items = NULL;
  array_list->capacity = 0;
  array_list->len = 0;
}
