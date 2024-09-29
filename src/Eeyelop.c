#define GL_GLEXT_PROTOTYPES 1

#include "EGL/egl.h"
#include "EGL/eglplatform.h"
#include <ArrayList.h>
#include <Config.h>
#include <Eeyelop.h>
#include <Egl.h>
#include <Notification.h>
#include <Output.h>
#include <Seat.h>
#include <stdio.h>
#include <stdlib.h>
#include <wayland-client-protocol.h>
#include <wlr-layer-shell-unstable-v1-client-protocol.h>

int eeyelop_init(Eeyelop *eeyelop, struct wl_display *display) {
  eeyelop->compositor = NULL;
  eeyelop->layer_shell = NULL;
  eeyelop->outputs = array_list_init(sizeof(Output));
  eeyelop->notifications = array_list_init(sizeof(Notification));
  eeyelop->config = config_init();
  eeyelop->seat = seat_init();

  if (egl_init(&eeyelop->egl, display) == -1) {
    EGLint error = eglGetError();
    printf("Failed to initialize egl with error: 0x%x\n", error);
    return -1;
  };

  if (text_init(&eeyelop->text, &eeyelop->config) == -1) {
    return -1;
  }

  return 0;
}

void eeyelop_deinit(Eeyelop *eeyelop) {
  wl_compositor_destroy(eeyelop->compositor);
  zwlr_layer_shell_v1_destroy(eeyelop->layer_shell);
  seat_deinit(&eeyelop->seat);

  for (int i = 0; i < eeyelop->outputs.len; i++) {
    Output *output = (Output *)eeyelop->outputs.items[i];
    output_deinit(output);
  }

  array_list_deinit(&eeyelop->outputs);
  egl_deinit(&eeyelop->egl);
}
