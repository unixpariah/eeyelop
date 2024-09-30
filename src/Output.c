#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include "xdg-output-client-protocol.h"
#include <Output.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client-protocol.h>

Output output_init(struct wl_output *wl_output, unsigned int id) {

  OutputInfo output_info = {
      .id = id,
      .name = NULL,
      .scale = 1,
  };

  Output output = {
      .wl_output = wl_output,
      .info = output_info,
  };

  return output;
}

void output_deinit(Output *output) {
  output_info_deinit(&output->info);
  wl_output_release(output->wl_output);
}

void output_info_deinit(OutputInfo *output_info) {
  free(output_info->name);
  output_info->id = 0;
}

void output_handle_name(void *data, struct wl_output *wl_output,
                        const char *name) {
  (void)wl_output;

  Output *output = data;

  output->info.name = malloc(strlen(name) + 1);
  if (output->info.name == NULL) {
    printf("Out of memory\n");
    output->info.name = "<unkown>";
    return;
  }
  strncpy(output->info.name, name, strlen(name));
}

void output_handle_scale(void *data, struct wl_output *wl_output,
                         int32_t scale) {
  (void)wl_output;

  Output *output = data;
  output->info.scale = scale;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-prototypes"

static void noop() {}

#pragma GCC diagnostic pop

const struct wl_output_listener output_listener = {
    .name = output_handle_name,
    .scale = output_handle_scale,
    .done = noop,
    .description = noop,
    .mode = noop,
    .geometry = noop,
};
