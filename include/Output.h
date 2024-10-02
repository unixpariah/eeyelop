#ifndef OUTPUT_H
#define OUTPUT_H

#include "wayland-client-protocol.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint32_t id;
  uint8_t *name;
  int32_t scale;
} OutputInfo;

void output_info_deinit(OutputInfo *);

typedef struct {
  OutputInfo info;
  struct wl_output *wl_output;
} Output;

Output output_init(struct wl_output *, unsigned int);

void output_deinit(Output *);

extern const struct wl_output_listener output_listener;

#endif // OUTPUT_H
