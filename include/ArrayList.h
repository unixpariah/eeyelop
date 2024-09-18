#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  void **items;
  size_t elem_size;
  int capacity;
  int len;
} ArrayList;

ArrayList array_list_init(size_t);

void array_list_deinit(ArrayList *);

int array_list_append(ArrayList *, void *);
