#define GL_GLEXT_PROTOTYPES 1

#include "Config.h"
#include "Egl.h"
#include "GL/glext.h"
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
      .info = output_info,
      .surface = surface,
      .layer_surface = layer_surface,
      .xdg_output = xdg_output,
      .egl = egl_surface,
  };

  return output;
}

int output_is_configured(Output *output) {
  return output->info.width > 0 && output->info.height > 0;
}

void output_deinit(Output *output) {
  wl_surface_destroy(output->surface);
  zwlr_layer_surface_v1_destroy(output->layer_surface);
  zxdg_output_v1_destroy(output->xdg_output);
  output_info_deinit(&output->info);
  wl_output_release(output->wl_output);
  egl_surface_deinit(&output->egl);
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

void output_handle_name(void *data, struct zxdg_output_v1 *xdg_output,
                        const char *name) {
  Output *output = data;

  output->info.name = malloc(strlen(name) + 1);
  strncpy(output->info.name, name, strlen(name));
}

void output_handle_description(void *data, struct zxdg_output_v1 *xdg_output,
                               const char *description) {
  Output *output = data;

  output->info.description = malloc(strlen(description) + 1);
  strncpy(output->info.description, description, strlen(description));
}

void output_handle_logical_size(void *data, struct zxdg_output_v1 *xdg_output,
                                int32_t width, int32_t height) {
  Output *output = data;

  output->info.width = width;
  output->info.height = height;
}

void output_handle_logical_position(void *data,
                                    struct zxdg_output_v1 *xdg_output,
                                    int32_t x, int32_t y) {
  Output *output = data;

  output->info.x = x;
  output->info.y = y;
}

void output_handle_done(void *data, struct zxdg_output_v1 *xdg_output) {}

const struct zxdg_output_v1_listener output_listener = {
    .name = output_handle_name,
    .description = output_handle_description,
    .logical_size = output_handle_logical_size,
    .logical_position = output_handle_logical_position,
    .done = output_handle_done,
};

void layer_surface_handle_configure(void *data,
                                    struct zwlr_layer_surface_v1 *layer_surface,
                                    uint32_t serial, uint32_t width,
                                    uint32_t height) {
  Eeyelop *eeyelop = data;

  for (int i = 0; i < eeyelop->outputs.len; i++) {
    Output *output = (Output *)eeyelop->outputs.items[i];
    if (output->layer_surface == layer_surface) {
      config_update(&eeyelop->config, output);
      zwlr_layer_surface_v1_ack_configure(output->layer_surface, serial);

      // clang-format off
      int vertices[8] = {
          0,                     0,
          eeyelop->config.width, 0,
          0,                     eeyelop->config.height,
          eeyelop->config.width, eeyelop->config.height,
      };
      // clang-format on

      glBindBuffer(GL_ARRAY_BUFFER, eeyelop->egl.VBO);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices,
                   GL_STATIC_DRAW);
    }
  }
}

void layer_surface_handle_closed(void *data,
                                 struct zwlr_layer_surface_v1 *layer_surface) {}

const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
    .configure = layer_surface_handle_configure,
    .closed = layer_surface_handle_closed,
};
