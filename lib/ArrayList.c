#include "ArrayList.h"
#include "stdio.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

ArrayList array_list_init(size_t elem_size) {
  ArrayList array_list = {
      .capacity = 0,
      .len = 0,
      .items = NULL,
      .elem_size = elem_size,
  };
  return array_list;
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
  void **new_items =
      (void **)realloc(array_list->items, new_capacity * sizeof(void *));
  if (!new_items) {
    return -1;
  }

  for (int i = array_list->len; i < array_list->capacity; i++) {
    new_items[i] = 0;
  }

  array_list->capacity = new_capacity;
  array_list->items = new_items;

  return 0;
}

int array_list_add_one(ArrayList *array_list) {
  int new_len = array_list->len + 1;
  if (array_list->capacity < new_len) {
    int better_capacity = grow_capacity(array_list->capacity, new_len);
    if (array_list_ensure_total_capacity_precise(array_list, better_capacity) ==
        -1) {
      return -1;
    }
  }

  assert(array_list->len < array_list->capacity);
  array_list->len++;

  return 0;
}

int array_list_append(ArrayList *array_list, void *item) {
  if (array_list_add_one(array_list) != 0) {
    return -1;
  }

  void *new_item_ptr = malloc(array_list->elem_size);
  if (!new_item_ptr) {
    return -1;
  }

  memcpy(new_item_ptr, item, array_list->elem_size);

  array_list->items[array_list->len - 1] = new_item_ptr;

  return 0;
}

void *array_list_pop(ArrayList *array_list) {
  if (array_list->len == 0) {
    return NULL;
  }

  void *item = array_list->items[array_list->len - 1];
  array_list->items[array_list->len - 1] = 0;

  array_list->len--;

  return item;
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
  for (int i = 0; i < array_list->len; i++) {
    free(array_list->items[i]);
  }
  free((void *)array_list->items);
  array_list->items = NULL;
  array_list->capacity = 0;
  array_list->len = 0;
}
