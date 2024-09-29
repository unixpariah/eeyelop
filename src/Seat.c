#include "ArrayList.h"
#include "Eeyelop.h"
#include "wayland-client-protocol.h"
#include "wayland-util.h"
#include <Notification.h>
#include <Seat.h>
#include <stdint.h>
#include <stdio.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-prototypes"

static void noop() {}

#pragma GCC diagnostic pop

Seat seat_init(void) {
  Seat seat = {
      .pointer = {},
      .seat = NULL,
  };

  return seat;
}

void seat_deinit(Seat *seat) {
  wl_pointer_release(seat->pointer.wl_pointer);
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
  Eeyelop *eeyelop = data;
  eeyelop->seat.pointer.x = wl_fixed_to_int(surface_x);
  eeyelop->seat.pointer.y = wl_fixed_to_int(surface_y);

  for (int i = 0; i < eeyelop->notifications.len; i++) {
  }
}

void pointer_handle_button(void *data, struct wl_pointer *pointer,
                           uint32_t serial, uint32_t time, uint32_t button,
                           uint32_t button_state) {
  Eeyelop *eeyelop = data;
  for (int i = 0; i < eeyelop->notifications.len; i++) {
    Notification *notification =
        (Notification *)eeyelop->notifications.items[i];

    if (button_state == 0 && eeyelop->seat.pointer.x > notification->x &&
        eeyelop->seat.pointer.x < notification->x + notification->width &&
        eeyelop->seat.pointer.y > notification->y &&
        eeyelop->seat.pointer.y < notification->y + notification->height) {
      array_list_ordered_remove(&eeyelop->notifications, i);
    }
  }
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
  Eeyelop *eeyelop = data;

  if (eeyelop->seat.pointer.wl_pointer != NULL) {
    wl_pointer_release(eeyelop->seat.pointer.wl_pointer);
    eeyelop->seat.pointer.wl_pointer = NULL;
  }
  if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
    eeyelop->seat.pointer.wl_pointer = wl_seat_get_pointer(wl_seat);
    wl_pointer_add_listener(eeyelop->seat.pointer.wl_pointer, &pointer_listener,
                            eeyelop);
  }
}

const struct wl_seat_listener seat_listener = {
    .capabilities = seat_handle_capabilities,
    .name = noop,
};
