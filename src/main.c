#include "EGL/eglplatform.h"
#include "Eeyelop.h"
#include "EventLoop.h"
#include "Output.h"
#include "Seat.h"
#include "wayland-client-core.h"
#include "wayland-client-protocol.h"
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/gl.h>
#include <Notification.h>
#include <hiv/ArrayList.h>
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

    Output *output = malloc(sizeof(Output));
    if (!output) {
      perror("Out of memory\nFailed to allocate space for output\n");
      return;
    }
    *output = output_init(wl_output, name);
    if (array_list_append(&eeyelop->outputs, output) == ARRAY_LIST_OOM) {
      perror("Out of memory\nFailed to add output to the list\n");
      return;
    };

    wl_output_add_listener(wl_output, &output_listener,
                           eeyelop->outputs.items[eeyelop->outputs.len - 1]);
  }
}

void handle_global_remove(void *data, struct wl_registry *registry,
                          uint32_t name) {
  (void)registry;

  Eeyelop *eeyelop = data;
  for (uint32_t i = 0; i < eeyelop->outputs.len; i++) {
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

void wl_display_callback(void *data) {
  struct wl_display *display = data;
  wl_display_dispatch(display);
}

int main(void) {
  struct wl_display *display = wl_display_connect(NULL);
  if (!display) {
    perror("Failed to create display\n");
    return EXIT_FAILURE;
  }

  Eeyelop eeyelop = eeyelop_init();

  EventLoop event_loop = {0};
  event_loop_init(&event_loop);
  int fd = wl_display_get_fd(display);
  event_loop_insert_source(&event_loop, fd, wl_display_callback, display, -1);

  struct wl_registry *registry = wl_display_get_registry(display);
  wl_registry_add_listener(registry, &registry_listener, &eeyelop);

  wl_display_dispatch(display);
  wl_display_roundtrip(display);

  if (!eeyelop.layer_shell) {
    perror("wlr_layer_shell protocol unsupported\n");
    return EXIT_FAILURE;
  }

  if (eeyelop_surface_init(&eeyelop) == -1) {
    return EXIT_FAILURE;
  }

  if (eeyelop_egl_init(&eeyelop, display) == -1) {
    EGLint error = eglGetError();
    printf("Failed to initialize egl with error: 0x%x\n", error);
    return EXIT_FAILURE;
  };

  for (int i = 0; i < 6; i++) {
    Notification *notification = malloc(sizeof(Notification));
    if (!notification) {
      perror("Out of memory\nFailed to allocate space for notification\n");
      continue;
    }

    *notification =
        notification_init(&eeyelop.config, (uint8_t *)"test\ntest2? test3!", i);

    NotificationCallbackData *noti_cb_data =
        malloc(sizeof(NotificationCallbackData));
    noti_cb_data->notification = notification;
    noti_cb_data->eeyelop = &eeyelop;

    event_loop_insert_source(&event_loop, notification->tfd,
                             notification_callback, noti_cb_data, 1);

    if (array_list_append(&eeyelop.notifications, notification) ==
        ARRAY_LIST_OOM) {
      perror("Out of memory\nFailed to append notification to list\n");
      free(notification);
      continue;
    };
  }

  eeyelop_config_apply(&eeyelop);

  if (text_init(&eeyelop.text, &eeyelop.config.font) == -1) {
    return EXIT_FAILURE;
  }

  wl_surface_commit(eeyelop.surface.wl_surface);
  wl_display_roundtrip(display);

  if (eeyelop_render(&eeyelop) == -1) {
    EGLint error = eglGetError();
    printf("Failed to render with error: 0x%x\n", error);
    return EXIT_FAILURE;
  };

  if (eeyelop_render(&eeyelop) == -1) {
    EGLint error = eglGetError();
    printf("Failed to render with error: 0x%x\n", error);
    return EXIT_FAILURE;
  };

  while (event_loop_poll(&event_loop) == EVENT_LOOP_OK) {
  }

  event_loop_deinit(&event_loop);
  eeyelop_deinit(&eeyelop);
  wl_registry_destroy(registry);
  wl_display_disconnect(display);

  return 0;
}
