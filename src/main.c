#include "EGL/eglplatform.h"
#include "Seat.h"
#define GL_GLEXT_PROTOTYPES 1

#include "ArrayList.h"
#include "Eeyelop.h"
#include "Egl.h"
#include "Output.h"
#include "wayland-client-core.h"
#include "wayland-client-protocol.h"
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include "xdg-output-client-protocol.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/gl.h>
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
    wl_seat_add_listener(eeyelop->seat.seat, &seat_listener, &eeyelop->seat);
  } else if (strcmp(interface, zxdg_output_manager_v1_interface.name) == 0) {
    eeyelop->output_manager = wl_registry_bind(
        registry, name, &zxdg_output_manager_v1_interface, version);
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

    struct zxdg_output_v1 *xdg_output = zxdg_output_manager_v1_get_xdg_output(
        eeyelop->output_manager, wl_output);

    EglSurface egl_surface =
        egl_surface_init(&eeyelop->egl, surface, (int[2]){1, 1});

    Output output = output_init(egl_surface, surface, layer_surface, wl_output,
                                xdg_output, name);

    wl_surface_commit(surface);

    if (array_list_append(&eeyelop->outputs, &output) == -1) {
      printf("Out of memory\n");
      exit(1);
    };

    zxdg_output_v1_add_listener(
        xdg_output, &output_listener,
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

int render(Eeyelop *eeyelop) {
  for (int i = 0; i < eeyelop->outputs.len; i++) {
    Output *output = (Output *)eeyelop->outputs.items[i];
    if (!output_is_configured(output) ||
        strcmp(output->info.name, eeyelop->config.output) != 0) {
      continue;
    }

    if (!eglMakeCurrent(*output->egl.display, output->egl.surface,
                        output->egl.surface, *output->egl.context)) {
      return -1;
    };

    // glUseProgram(eeyelop->egl.main_shader_program);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(1, 0, 0, 1);

    // glBindBuffer(GL_ARRAY_BUFFER, eeyelop->egl.VBO);
    // glVertexAttribPointer(0, 2, GL_INT, GL_FALSE, 0, NULL);
    // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

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

  if (render(&eeyelop) == -1) {
    EGLint error = eglGetError();
    printf("Failed to render with error: 0x%x\n", error);
    return EXIT_FAILURE;
  };

  while (!eeyelop.exit && wl_display_dispatch(display) != -1) {
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
