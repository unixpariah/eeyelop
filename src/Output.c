#include "math.h"
#define GL_GLEXT_PROTOTYPES 1

#include "Config.h"
#include "Egl.h"
#include "GL/gl.h"
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
                   struct wl_output *wl_output, unsigned int id) {

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
      .egl = egl_surface,
  };

  return output;
}

void output_surface_resize(Output *output, int width, int height) {
  zwlr_layer_surface_v1_set_size(output->layer_surface, width, height);
  wl_egl_window_resize(output->egl.window, width, height, 0, 0);

  wl_surface_commit(output->surface);

  Mat4 mat4;
  math_orthographic_projection(&mat4, 0, (float)width, 0, (float)height);

  glUseProgram(*output->egl.main_shader_program);
  GLint location =
      glGetUniformLocation(*output->egl.main_shader_program, "projection");
  glUniformMatrix4fv(location, 1, GL_FALSE, (const GLfloat *)mat4);
}

void output_deinit(Output *output) {
  wl_surface_destroy(output->surface);
  zwlr_layer_surface_v1_destroy(output->layer_surface);
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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-prototypes"

static void noop() {}

#pragma GCC diagnostic pop

const struct wl_output_listener output_listener = {
    .name = output_handle_name,
    .scale = noop,
    .done = noop,
    .description = noop,
    .mode = noop,
    .geometry = noop,
};

void layer_surface_handle_configure(void *data,
                                    struct zwlr_layer_surface_v1 *layer_surface,
                                    uint32_t serial, uint32_t width,
                                    uint32_t height) {
  (void)width, (void)height;
  Eeyelop *eeyelop = data;

  for (int i = 0; i < eeyelop->outputs.len; i++) {
    Output *output = (Output *)eeyelop->outputs.items[i];
    if (output->layer_surface == layer_surface) {
      config_update(&eeyelop->config, output);
      zwlr_layer_surface_v1_ack_configure(output->layer_surface, serial);
    }
  }
}

const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
    .configure = layer_surface_handle_configure,
    .closed = noop,
};
