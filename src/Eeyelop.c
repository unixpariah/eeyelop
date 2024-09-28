#include "ArrayList.h"
#include "EGL/egl.h"
#include "EGL/eglplatform.h"
#include "wayland-client-protocol.h"
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include "xdg-output-client-protocol.h"
#include <Config.h>
#include <Eeyelop.h>
#include <Egl.h>
#include <Output.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

Eeyelop eeyelop_init(struct wl_display *display) {
  Eeyelop eeyelop = {
      .compositor = NULL,
      .layer_shell = NULL,
      .output_manager = NULL,
      .outputs = array_list_init(sizeof(Output)),
      .exit = false,
      .config = config_init(),
      .seat = seat_init(),
  };

  if (egl_init(&eeyelop.egl, display) == -1) {
    EGLint error = eglGetError();
    printf("Failed to initialize egl with error: 0x%x\n", error);
    exit(1);
  };

  return eeyelop;
}

void eeyelop_deinit(Eeyelop *eeyelop) {
  wl_compositor_destroy(eeyelop->compositor);
  zwlr_layer_shell_v1_destroy(eeyelop->layer_shell);
  zxdg_output_manager_v1_destroy(eeyelop->output_manager);
  seat_deinit(&eeyelop->seat);

  for (int i = 0; i < eeyelop->outputs.len; i++) {
    Output *output = (Output *)eeyelop->outputs.items[i];
    output_deinit(output);
  }

  array_list_deinit(&eeyelop->outputs);
  egl_deinit(&eeyelop->egl);
}
