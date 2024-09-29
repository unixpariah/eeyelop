#ifndef SEAT_H
#define SEAT_H

#include "wayland-client-protocol.h"

typedef struct {
  struct {
    struct wl_pointer *wl_pointer;
    int x;
    int y;
  } pointer;
  struct wl_seat *seat;
} Seat;

Seat seat_init(void);

void seat_deinit(Seat *);

extern const struct wl_seat_listener seat_listener;

#endif // SEAT_H
