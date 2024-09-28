#include "wayland-client-protocol.h"
#include "wayland-util.h"
#include <Seat.h>
#include <stdint.h>
#include <stdio.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-prototypes"

static void noop() {}

#pragma GCC diagnostic pop

Seat seat_init(void) {
  Seat seat = {
      .pointer = NULL,
      .seat = NULL,
  };

  return seat;
}

void seat_deinit(Seat *seat) {
  wl_pointer_release(seat->pointer);
  wl_seat_release(seat->seat);
}

void pointer_handle_enter(void *data, struct wl_pointer *pointer,
                          uint32_t serial, struct wl_surface *surface,
                          wl_fixed_t surface_x, wl_fixed_t surface_y) {
  printf("Entered\n");
}

void pointer_handle_leave(void *data, struct wl_pointer *pointer,
                          uint32_t serial, struct wl_surface *surface) {
  printf("Left :(\n");
}

void pointer_handle_motion(void *data, struct wl_pointer *pointer,
                           uint32_t serial, wl_fixed_t surface_x,
                           wl_fixed_t surface_y) {
  printf("Motion! %d %d\n", surface_y, surface_x);
}

void pointer_handle_button(void *data, struct wl_pointer *pointer,
                           uint32_t serial, uint32_t time, uint32_t button,
                           uint32_t button_state) {
  printf("New button state %d\n", button_state);
}

const struct wl_pointer_listener pointer_listener = {
    .axis = noop,
    .enter = pointer_handle_enter,
    .leave = pointer_handle_leave,
    .motion = pointer_handle_motion,
    .button = pointer_handle_button,
};

void seat_handle_capabilities(void *data, struct wl_seat *wl_seat,
                              unsigned int capabilities) {
  Seat *seat = data;

  if (seat->pointer != NULL) {
    wl_pointer_release(seat->pointer);
    seat->pointer = NULL;
  }
  if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
    seat->pointer = wl_seat_get_pointer(wl_seat);
    wl_pointer_add_listener(seat->pointer, &pointer_listener, seat);
  }
}

const struct wl_seat_listener seat_listener = {
    .capabilities = seat_handle_capabilities,
    .name = noop,
};
