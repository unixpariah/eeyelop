#define GL_GLEXT_PROTOTYPES 1

#include "ArrayList.h"
#include "EGL/eglplatform.h"
#include "Eeyelop.h"
#include "Egl.h"
#include "GL/glext.h"
#include "Output.h"
#include "Seat.h"
#include "wayland-client-core.h"
#include "wayland-client-protocol.h"
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include "xdg-output-client-protocol.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/gl.h>
#include <Notification.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void handle_global(void *data, struct wl_registry *registry, uint32_t name,
                   const char *interface, uint32_t version) {
  Eeyelop *eeyelop = data;
  if (strcmp(interface, wl_compositor_interface.name) == 0) {
    eeyelop->compositor =
        wl_registry_bind(registry, name, &wl_compositor_interface, version);
  } else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
    eeyelop->layer_shell = wl_registry_bind(
        registry, name, &zwlr_layer_shell_v1_interface, version);
  } else if (strcmp(interface, wl_seat_interface.name) == 0) {
    eeyelop->seat.seat =
        wl_registry_bind(registry, name, &wl_seat_interface, 3);
    wl_seat_add_listener(eeyelop->seat.seat, &seat_listener, eeyelop);
  } else if (strcmp(interface, wl_output_interface.name) == 0) {
    struct wl_output *wl_output =
        wl_registry_bind(registry, name, &wl_output_interface, version);

    struct wl_surface *surface =
        wl_compositor_create_surface(eeyelop->compositor);

    struct zwlr_layer_surface_v1 *layer_surface =
        zwlr_layer_shell_v1_get_layer_surface(eeyelop->layer_shell, surface,
                                              wl_output, 3, "eeyelop");

    zwlr_layer_surface_v1_add_listener(layer_surface, &layer_surface_listener,
                                       eeyelop);

    zwlr_layer_surface_v1_set_anchor(layer_surface,
                                     ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
                                         ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
                                         ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
                                         ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
    zwlr_layer_surface_v1_set_exclusive_zone(layer_surface, -1);
    zwlr_layer_surface_v1_set_keyboard_interactivity(
        layer_surface, ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_NONE);

    EglSurface egl_surface =
        egl_surface_init(&eeyelop->egl, surface, (int[2]){1, 1});

    Output output =
        output_init(egl_surface, surface, layer_surface, wl_output, name);

    wl_surface_commit(surface);

    if (array_list_append(&eeyelop->outputs, &output) == -1) {
      printf("Out of memory\n");
      exit(1);
    };

    wl_output_add_listener(wl_output, &output_listener,
                           eeyelop->outputs.items[eeyelop->outputs.len - 1]);
  }
}

void handle_global_remove(void *data, struct wl_registry *registry,
                          uint32_t name) {
  Eeyelop *eeyelop = data;
  for (int i = 0; i < eeyelop->outputs.len; i++) {
    Output *output = (Output *)eeyelop->outputs.items[i];
    if (output->info.id == name) {
      Output *output = (Output *)array_list_swap_remove(&eeyelop->outputs, i);
      output_deinit(output);
    }
  }
}

static const struct wl_registry_listener registry_listener = {
    .global = handle_global,
    .global_remove = handle_global_remove,
};

void render_pane(Eeyelop *eeyelop, int index) {
  int x = eeyelop->config.margin.left;
  int y =
      eeyelop->config.height * index + eeyelop->config.margin.top * (index + 1);

  if (index > 0) {
    y += eeyelop->config.margin.bottom * index;
  }

  // clang-format off
  int vertices[8] = {
      x,                                                    y,
      eeyelop->config.width - eeyelop->config.margin.right, y,
      x,                                                    y + eeyelop->config.height,
      eeyelop->config.width - eeyelop->config.margin.right, y + eeyelop->config.height,
  };
  // clang-format on

  glBindBuffer(GL_ARRAY_BUFFER, eeyelop->egl.VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, eeyelop->egl.VBO);
  glVertexAttribPointer(0, 2, GL_INT, GL_FALSE, 0, NULL);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

int render(Eeyelop *eeyelop) {
  for (int i = 0; i < eeyelop->outputs.len; i++) {
    Output *output = (Output *)eeyelop->outputs.items[i];
    if (strcmp(output->info.name, eeyelop->config.output) != 0) {
      continue;
    }

    if (!eglMakeCurrent(*output->egl.display, output->egl.surface,
                        output->egl.surface, *output->egl.context)) {
      return -1;
    };

    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0, 0, 0, 0);

    glUseProgram(eeyelop->egl.main_shader_program);
    for (int i = 0; i < eeyelop->notifications.len; i++) {
      Notification *notification =
          (Notification *)eeyelop->notifications.items[i];
      notification_render(notification, &eeyelop->egl);
    }

    if (!eglSwapBuffers(*output->egl.display, output->egl.surface)) {
      return -1;
    };
  }

  return 0;
}

int main(void) {
  struct wl_display *display = wl_display_connect(NULL);
  if (!display) {
    printf("Failed to create display\n");
    return EXIT_FAILURE;
  }

  Eeyelop eeyelop = eeyelop_init(display);

  struct wl_registry *registry = wl_display_get_registry(display);

  wl_registry_add_listener(registry, &registry_listener, &eeyelop);
  wl_display_dispatch(display);
  wl_display_roundtrip(display);

  if (!eeyelop.layer_shell) {
    printf("wlr_layer_shell protocol unsupported\n");
    return EXIT_FAILURE;
  }

  for (int i = 0; i < 5; i++) {
    Notification notification = notification_init(&eeyelop.config, i);
    array_list_append(&eeyelop.notifications, &notification);
  }

  for (int i = 0; i < eeyelop.outputs.len; i++) {
    Output *output = (Output *)eeyelop.outputs.items[i];
    if (strcmp(output->info.name, eeyelop.config.output) != 0) {
      continue;
    }

    int total_width = eeyelop.config.width + eeyelop.config.margin.left +
                      eeyelop.config.margin.right;

    int total_height = (eeyelop.config.height + eeyelop.config.margin.top +
                        eeyelop.config.margin.bottom) *
                       eeyelop.notifications.len;

    output_surface_resize(output, total_width, total_height);
  }

  if (render(&eeyelop) == -1) {
    EGLint error = eglGetError();
    printf("Failed to render with error: 0x%x\n", error);
    return EXIT_FAILURE;
  };

  while (wl_display_dispatch(display) != -1) {
    if (render(&eeyelop) == -1) {
      EGLint error = eglGetError();
      printf("Failed to render with error: 0x%x\n", error);
      return EXIT_FAILURE;
    };
  }

  eeyelop_deinit(&eeyelop);
  wl_registry_destroy(registry);
  wl_display_disconnect(display);

  return 0;
}
