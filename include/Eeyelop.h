#ifndef EEYELOP_H
#define EEYELOP_H

#include <ArrayList.h>
#include <Egl.h>
#include <stdbool.h>

typedef struct {
  struct wl_compositor *compositor;
  struct zwlr_layer_shell_v1 *layer_shell;
  struct zxdg_output_manager_v1 *output_manager;
  ArrayList outputs;
  Egl egl;
  bool exit;
} __attribute__((aligned(128))) Eeyelop;

Eeyelop eeyelop_init(struct wl_display *display);

void eeyelop_deinit(Eeyelop *eeyelop);

#endif // EEYELOP_H
