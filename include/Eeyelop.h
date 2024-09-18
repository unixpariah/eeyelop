#ifndef EEYELOP_H
#define EEYELOP_H

#include "./ArrayList.h"
#include "./wlr-layer-shell-unstable-v1-client-protocol.h"
#include "./xdg-output-client-protocol.h"
#include <wayland-client-protocol.h>

typedef struct {
  struct wl_compositor *compositor;
  struct zwlr_layer_shell_v1 *layer_shell;
  struct zxdg_output_manager_v1 *output_manager;
  struct wl_seat *seat;
  ArrayList outputs;
  bool exit;
} Eeyelop;

#endif // EEYELOP_H
