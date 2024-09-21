#include "Egl.h"
#include "wayland-egl-core.h"
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include "xdg-output-client-protocol.h"
#include <Eeyelop.h>
#include <Output.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client-protocol.h>

Output output_init(EglSurface egl_surface, struct wl_surface *surface,
                   struct zwlr_layer_surface_v1 *layer_surface,
                   struct wl_output *wl_output,
                   struct zxdg_output_v1 *xdg_output, unsigned int id) {

  OutputInfo output_info = {
      .id = id,
      .description = NULL,
      .name = NULL,
      .x = 0,
      .y = 0,
      .width = 0,
      .height = 0,
  };

  Output output = {
      .wl_output = wl_output,
      .output_info = output_info,
      .surface = surface,
      .layer_surface = layer_surface,
      .xdg_output = xdg_output,
      .egl_surface = egl_surface,

  };

  return output;
}

void output_deinit(Output *output) {
  wl_surface_destroy(output->surface);
  zwlr_layer_surface_v1_destroy(output->layer_surface);
  zxdg_output_v1_destroy(output->xdg_output);
  output_info_deinit(&output->output_info);
  wl_output_release(output->wl_output);
  egl_surface_deinit(&output->egl_surface);
}

void output_info_deinit(OutputInfo *output_info) {
  free(output_info->name);
  free(output_info->description);
  output_info->id = 0;
  output_info->x = 0;
  output_info->y = 0;
  output_info->width = 0;
  output_info->height = 0;
}

void xdg_output_handle_name(void *data, struct zxdg_output_v1 *xdg_output,
                            const char *name) {
  Eeyelop *eeyelop = data;

  for (int i = 0; i < eeyelop->outputs.len; i++) {
    Output *output = (Output *)eeyelop->outputs.items[i];

    if (output->xdg_output == xdg_output) {
      output->output_info.name = malloc(strlen(name) + 1);
      strncpy(output->output_info.name, name, strlen(name));
      return;
    }
  }

  printf("Output not found\n");
  exit(1);
}

void xdg_output_handle_description(void *data,
                                   struct zxdg_output_v1 *xdg_output,
                                   const char *description) {

  Eeyelop *eeyelop = data;

  for (int i = 0; i < eeyelop->outputs.len; i++) {
    Output *output = (Output *)eeyelop->outputs.items[i];

    if (output->xdg_output == xdg_output) {
      output->output_info.description = malloc(strlen(description) + 1);
      strncpy(output->output_info.description, description,
              strlen(description));
      return;
    }
  }

  printf("Output not found\n");
  exit(1);
}

void xdg_output_handle_logical_size(void *data,
                                    struct zxdg_output_v1 *xdg_output,
                                    int32_t width, int32_t height) {

  Eeyelop *eeyelop = data;
  for (int i = 0; i < eeyelop->outputs.len; i++) {
    Output *output = (Output *)eeyelop->outputs.items[i];

    if (output->xdg_output == xdg_output) {
      output->output_info.width = width;
      output->output_info.height = height;
      return;
    }
  }

  printf("Output not found\n");
  exit(1);
}

void xdg_output_handle_logical_position(void *data,
                                        struct zxdg_output_v1 *xdg_output,
                                        int32_t x, int32_t y) {

  Eeyelop *eeyelop = data;
  for (int i = 0; i < eeyelop->outputs.len; i++) {
    Output *output = (Output *)eeyelop->outputs.items[i];

    if (output->xdg_output == xdg_output) {
      output->output_info.x = x;
      output->output_info.y = y;
      return;
    }
  }

  printf("Output not found\n");
  exit(1);
}

void xdg_output_handle_done(void *data, struct zxdg_output_v1 *xdg_output) {}

const struct zxdg_output_v1_listener xdg_output_listener = {
    .name = xdg_output_handle_name,
    .description = xdg_output_handle_description,
    .logical_size = xdg_output_handle_logical_size,
    .logical_position = xdg_output_handle_logical_position,
    .done = xdg_output_handle_done,
};

void zwlr_layer_surface_handle_configure(
    void *data, struct zwlr_layer_surface_v1 *layer_surface, uint32_t serial,
    uint32_t width, uint32_t height) {
  Eeyelop *eeyelop = data;

  for (int i = 0; i < eeyelop->outputs.len; i++) {
    Output *output = (Output *)eeyelop->outputs.items[i];
    if (output->layer_surface == layer_surface) {
      zwlr_layer_surface_v1_set_size(output->layer_surface, width, height);
      zwlr_layer_surface_v1_ack_configure(output->layer_surface, serial);
      wl_egl_window_resize(output->egl_surface.window, (int)width, (int)height,
                           output->output_info.x, output->output_info.y);
    }
  }
}

void zwlr_layer_surface_handle_closed(
    void *data, struct zwlr_layer_surface_v1 *layer_surface) {}

const struct zwlr_layer_surface_v1_listener zwlr_layer_surface_listener = {
    .configure = zwlr_layer_surface_handle_configure,
    .closed = zwlr_layer_surface_handle_closed,
};
