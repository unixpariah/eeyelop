#ifndef EEYELOP_H
#define EEYELOP_H

#include <ArrayList.h>
#include <Config.h>
#include <Egl.h>
#include <Seat.h>
#include <stdbool.h>

typedef struct {
  struct wl_compositor *compositor;
  struct zwlr_layer_shell_v1 *layer_shell;
  ArrayList outputs;
  ArrayList notifications;
  Egl egl;
  Config config;
  Seat seat;
} Eeyelop;

Eeyelop eeyelop_init(struct wl_display *display);

void eeyelop_deinit(Eeyelop *eeyelop);

#endif // EEYELOP_H
