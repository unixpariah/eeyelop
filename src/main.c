#include "../include/wlr-layer-shell-unstable-v1-client-protocol.h"
#include "../include/xdg-output-client-protocol.h"
#include "wayland-client-protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client.h>
#include <wayland-egl.h>

typedef struct {
  struct wl_compositor *compositor;
  struct zwlr_layer_shell_v1 *layer_shell;
  struct zxdg_output_manager_v1 *output_manager;
} Eeyelop;

void handle_global(void *data, struct wl_registry *registry, uint32_t name,
                   const char *interface, uint32_t version) {
  Eeyelop *eeyelop = data;
  if (strcmp(interface, wl_compositor_interface.name) == 0) {
    eeyelop->compositor =
        wl_registry_bind(registry, name, &wl_compositor_interface, version);
  } else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
    eeyelop->layer_shell = wl_registry_bind(
        registry, name, &zwlr_layer_shell_v1_interface, version);
  } else if (strcmp(interface, zxdg_output_manager_v1_interface.name) == 0) {
    eeyelop->output_manager = wl_registry_bind(
        registry, name, &zxdg_output_manager_v1_interface, version);
  }
}

void handle_global_remove(void *data, struct wl_registry *registry,
                          uint32_t name) {}

static const struct wl_registry_listener registry_listener = {
    .global = handle_global,
    .global_remove = handle_global_remove,
};

int main() {
  struct wl_display *display = wl_display_connect(NULL);
  struct wl_registry *registry = wl_display_get_registry(display);

  Eeyelop eeyelop = {
      .compositor = NULL,
      .layer_shell = NULL,
  };

  wl_registry_add_listener(registry, &registry_listener, &eeyelop);
  wl_display_roundtrip(display);

  if (eeyelop.compositor == NULL || eeyelop.layer_shell == NULL) {
    printf("Missing wl_compositor or zwlr_layer_shell protocol\n");
    exit(1);
  }

  wl_registry_destroy(registry);
  wl_display_disconnect(display);

  return 0;
}
