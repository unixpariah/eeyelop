#define GL_GLEXT_PROTOTYPES 1

#include "EGL/egl.h"
#include "EGL/eglplatform.h"
#include "math.h"
#include <ArrayList.h>
#include <Config.h>
#include <Eeyelop.h>
#include <Egl.h>
#include <Output.h>
#include <Seat.h>
#include <stdio.h>
#include <stdlib.h>
#include <wayland-client-protocol.h>
#include <wlr-layer-shell-unstable-v1-client-protocol.h>
#include <xdg-output-client-protocol.h>

#include "GL/glext.h"

Eeyelop eeyelop_init(struct wl_display *display) {
  Eeyelop eeyelop = {
      .compositor = NULL,
      .layer_shell = NULL,
      .outputs = array_list_init(sizeof(Output)),
      .config = config_init(),
      .seat = seat_init(),
      .surface_count = 0,
  };

  if (egl_init(&eeyelop.egl, display) == 1) {
    EGLint error = eglGetError();
    printf("Failed to initialize egl with error: 0x%x\n", error);
    exit(1);
  };

  struct {
    Mat4 projection;
  } uniform_object;

  math_orthographic_projection(&uniform_object.projection, 0,
                               (float)eeyelop.config.width, 0,
                               (float)eeyelop.config.height);

  glBindBuffer(GL_UNIFORM_BUFFER, eeyelop.egl.UBO);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(uniform_object), &uniform_object,
               GL_STATIC_DRAW);

  glBindBufferBase(GL_UNIFORM_BUFFER, 0, eeyelop.egl.UBO);
  glUniformBlockBinding(
      eeyelop.egl.main_shader_program,
      glGetUniformBlockIndex(eeyelop.egl.main_shader_program, "UniformBlock"),
      0);

  return eeyelop;
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
