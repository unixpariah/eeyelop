#define GL_GLEXT_PROTOTYPES 1

#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "EGL/eglplatform.h"
#include "GL/gl.h"
#include "GL/glext.h"
#include "math.h"
#include "shaders/main.h"
#include "shaders/text.h"
#include "stdfloat.h"
#include "wayland-egl-core.h"
#include <Config.h>
#include <Eeyelop.h>
#include <Notification.h>
#include <Output.h>
#include <Seat.h>
#include <hiv/ArrayList.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client-protocol.h>
#include <wlr-layer-shell-unstable-v1-client-protocol.h>

void layer_surface_handle_configure(void *data,
                                    struct zwlr_layer_surface_v1 *layer_surface,
                                    uint32_t serial, uint32_t width,
                                    uint32_t height) {
  (void)layer_surface;

  Eeyelop *eeyelop = data;

  if (eeyelop->surface.configured &&
      eeyelop->surface.width == (float32_t)width &&
      eeyelop->surface.height == (float32_t)height) {
    wl_surface_commit(eeyelop->surface.wl_surface);
    return;
  }

  eeyelop->surface.width = (float32_t)width;
  eeyelop->surface.height = (float32_t)height;
  eeyelop->surface.configured = true;

  wl_egl_window_resize(eeyelop->surface.egl_window, (int)width, (int)height, 0,
                       0);

  Mat4 mat4;
  mat4_orthographic_projection(&mat4, 0, (float)width, 0, (float)height);

  {
    glUseProgram(eeyelop->egl.main_shader_program);
    GLint location =
        glGetUniformLocation(eeyelop->egl.main_shader_program, "projection");
    glUniformMatrix4fv(location, 1, GL_FALSE, &mat4[0][0]);
  }

  {
    glUseProgram(eeyelop->egl.text_shader_program);
    GLint location =
        glGetUniformLocation(eeyelop->egl.text_shader_program, "projection");
    glUniformMatrix4fv(location, 1, GL_FALSE, &mat4[0][0]);
  }

  glViewport(0, 0, (int)width, (int)height);
  zwlr_layer_surface_v1_ack_configure(eeyelop->surface.layer, serial);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-prototypes"

static void noop() {}

#pragma GCC diagnostic pop

const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
    .configure = layer_surface_handle_configure,
    .closed = noop,
};

Eeyelop eeyelop_init(void) {
  Eeyelop eeyelop = {0};

  array_list_init(&eeyelop.outputs);
  array_list_init(&eeyelop.notifications);
  eeyelop.config = config_init();
  eeyelop.seat = seat_init();
  eeyelop.surface.configured = false;

  return eeyelop;
}

void eeyelop_config_apply(Eeyelop *eeyelop) {
  uint32_t layer = 0;
  switch (eeyelop->config.layer) {
  case background:
    break;
  case bottom:
    layer = 1;
    break;
  case top:
    layer = 2;
    break;
  case overlay:
    layer = 3;
    break;
  }

  unsigned int anchor = 0;
  switch (eeyelop->config.anchor) {
  case top_right:
    anchor =
        ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;
    break;
  case top_center:
    anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP;
    break;
  case top_left:
    anchor =
        ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP | ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT;
    break;
  case bottom_right:
    anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
             ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;
    break;
  case bottom_center:
    anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM;
    break;
  case bottom_left:
    anchor =
        ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM | ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT;
    break;
  case center_right:
    anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;
    break;
  case center_left:
    anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT;
    break;
  case center:
    break;
  }

  zwlr_layer_surface_v1_set_layer(eeyelop->surface.layer, layer);
  zwlr_layer_surface_v1_set_anchor(eeyelop->surface.layer, anchor);
}

struct zwlr_layer_surface_v1 *create_layer_surface(Eeyelop *eeyelop) {
  struct zwlr_layer_surface_v1 *layer_surface = NULL;

  if (strcmp((char *)eeyelop->config.output, "") != 0) {
    for (uint32_t i = 0; i < eeyelop->outputs.len; i++) {
      Output *output = (Output *)eeyelop->outputs.items[i];

      if (strcmp((char *)eeyelop->config.output, (char *)output->info.name) ==
          0) {
        layer_surface = zwlr_layer_shell_v1_get_layer_surface(
            eeyelop->layer_shell, eeyelop->surface.wl_surface,
            output->wl_output, 3, "eeyelop");
        break;
      }
    }

    printf("Output %s doesn't exist, spawning layer on focused output\n",
           eeyelop->config.output);
  } else {
    layer_surface = zwlr_layer_shell_v1_get_layer_surface(
        eeyelop->layer_shell, eeyelop->surface.wl_surface, NULL, 3, "eeyelop");
  }

  zwlr_layer_surface_v1_add_listener(layer_surface, &layer_surface_listener,
                                     eeyelop);

  zwlr_layer_surface_v1_set_exclusive_zone(layer_surface, -1);
  zwlr_layer_surface_v1_set_keyboard_interactivity(
      layer_surface, ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_NONE);

  return layer_surface;
}

int eeyelop_surface_init(Eeyelop *eeyelop) {
  eeyelop->surface.layer = create_layer_surface(eeyelop);
  eeyelop_config_apply(eeyelop);

  EGLSurface egl_surface =
      eglCreatePlatformWindowSurface(eeyelop->egl.display, eeyelop->egl.config,
                                     eeyelop->surface.egl_window, NULL);

  if (!egl_surface) {
    printf("Failed to create egl window or surface\n");
    return -1;
  }

  eeyelop->surface.egl_surface = egl_surface;

  if (!eglMakeCurrent(eeyelop->egl.display, eeyelop->surface.egl_surface,
                      eeyelop->surface.egl_surface, eeyelop->egl.context)) {
    return -1;
  }

  return 0;
}

void eeyelop_surface_deinit(Eeyelop *eeyelop) {
  wl_egl_window_destroy(eeyelop->surface.egl_window);
  eglDestroySurface(eeyelop->egl.display, eeyelop->surface.egl_surface);
}

void gl_message_callback(unsigned int source, unsigned int err_type,
                         unsigned int id, unsigned int severity, int length,
                         const char *message, const void *data) {
  (void)data;
  printf("%d %d %d %d %d %s\n", source, err_type, id, severity, length,
         message);
}

int compile_shader(const uint8_t *shader_source, GLuint shader,
                   GLuint shader_program) {
  glShaderSource(shader, 1, (const char **)&shader_source, NULL);
  glCompileShader(shader);

  int success = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (success != GL_TRUE) {
    GLchar info_log[512];
    glGetShaderInfoLog(shader, 512, NULL, info_log);
    printf("%s\n", info_log);
    return -1;
  }

  glAttachShader(shader_program, shader);
  return 0;
}

int create_shader_program(GLuint *shader_program, const uint8_t *vertex_source,
                          const uint8_t *fragment_source) {
  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

  *shader_program = glCreateProgram();

  if (compile_shader(vertex_source, vertex_shader, *shader_program) == -1) {
    return -1;
  }

  if (compile_shader(fragment_source, fragment_shader, *shader_program) == -1) {
    return -1;
  }

  glLinkProgram(*shader_program);

  {
    int link_success = 0;
    glGetProgramiv(*shader_program, GL_LINK_STATUS, &link_success);
    if (link_success != GL_TRUE) {
      GLchar info_log[512];
      glGetShaderInfoLog(*shader_program, 512, NULL, info_log);
      printf("%s\n", info_log);
      return -1;
    }
  }

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  return 0;
}

int eeyelop_egl_init(Eeyelop *eeyelop, struct wl_display *display) {
  if (eglBindAPI(EGL_OPENGL_API) != EGL_TRUE) {
    return -1;
  }
  EGLDisplay egl_display =
      eglGetPlatformDisplay(EGL_PLATFORM_WAYLAND_EXT, display, NULL);
  if (egl_display == EGL_NO_DISPLAY) {
    return -1;
  }

  int minor = 0;
  int major = 0;
  if (eglInitialize(egl_display, &major, &minor) != EGL_TRUE) {
    return -1;
  }

  EGLint count = 0;
  eglGetConfigs(egl_display, NULL, 0, &count);

  // clang-format off
  EGLint config_attribs[] = {
      EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
      EGL_RED_SIZE,        8,
      EGL_GREEN_SIZE,      8,
      EGL_BLUE_SIZE,       8,
      EGL_ALPHA_SIZE,      8,
      EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
      EGL_NONE
  };
  // clang-format on
  EGLint n = 0;
  EGLConfig *configs = (void **)calloc(count, sizeof(EGLConfig));
  eglChooseConfig(egl_display, config_attribs, configs, count, &n);
  if (n == 0) {
    return -1;
  }
  EGLConfig egl_config = configs[0];

  // clang-format off
  EGLint context_attribs[] = {
      EGL_CONTEXT_MAJOR_VERSION,       major,
      EGL_CONTEXT_MINOR_VERSION,       minor,
      EGL_CONTEXT_OPENGL_DEBUG,        EGL_TRUE,
      EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
      EGL_NONE
  };
  // clang-format on

  EGLContext egl_context = eglCreateContext(egl_display, egl_config,
                                            EGL_NO_CONTEXT, context_attribs);
  if (egl_context == EGL_NO_CONTEXT) {
    return -1;
  }

  if (!eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE,
                      egl_context)) {
    return -1;
  }

  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(gl_message_callback, NULL);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  GLuint main_shader_program = 0;
  if (create_shader_program(&main_shader_program, main_vertex_source,
                            main_fragment_source) == -1) {
    return -1;
  };

  glValidateProgram(main_shader_program);

  GLuint text_shader_program = 0;
  if (create_shader_program(&text_shader_program, text_vertex_source,
                            text_fragment_source) == -1) {
    return -1;
  };

  glValidateProgram(text_shader_program);

  GLuint VAO = 0;
  GLuint VBO[2] = {0};
  GLuint EBO = 0;

  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);
  glEnableVertexAttribArray(0);

  glGenBuffers(2, VBO);
  glGenBuffers(1, &EBO);

  // clang-format off
  float vertices [8] = { // Text VBO
      0, 0,
      1, 0,
      0, 1,
      1, 1,
  };
  // clang-format on

  glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

  // clang-format off
  int indices[6] = {
      0, 1, 3,
      3, 2, 0,
  };
  // clang-format on

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices,
               GL_STATIC_DRAW);

  eeyelop->egl.display = egl_display;
  eeyelop->egl.config = egl_config;
  eeyelop->egl.context = egl_context;
  eeyelop->egl.main_shader_program = main_shader_program;
  eeyelop->egl.text_shader_program = text_shader_program;
  eeyelop->egl.VAO = VAO;
  eeyelop->egl.VBO[0] = VBO[0];
  eeyelop->egl.VBO[1] = VBO[1];
  eeyelop->egl.EBO = EBO;

  return 0;
}

int eeyelop_render(Eeyelop *eeyelop) {
  glClear(GL_COLOR_BUFFER_BIT);
  glClearColor(0, 0, 0, 0);

  for (uint32_t i = 0; i < eeyelop->notifications.len; i++) {
    Notification *notification =
        (Notification *)eeyelop->notifications.items[i];
    eeyelop_notification_render(eeyelop, notification);
  }

  if (!eglSwapBuffers(eeyelop->egl.display, eeyelop->surface.egl_surface)) {
    return -1;
  };

  return 0;
}

int egl_deinit(Egl *egl) {
  eglDestroyContext(egl->display, egl->context);
  eglTerminate(egl->display);
  glDeleteProgram(egl->main_shader_program);
  glDeleteProgram(egl->text_shader_program);
  glDeleteBuffers(1, &egl->VAO);
  glDeleteBuffers(2, &egl->VBO[0]);
  glDeleteBuffers(1, &egl->EBO);

  return 0;
}

void eeyelop_deinit(Eeyelop *eeyelop) {
  wl_compositor_destroy(eeyelop->compositor);
  zwlr_layer_shell_v1_destroy(eeyelop->layer_shell);
  seat_deinit(&eeyelop->seat);

  for (uint32_t i = 0; i < eeyelop->outputs.len; i++) {
    Output *output = (Output *)eeyelop->outputs.items[i];
    output_deinit(output);
  }

  eeyelop_surface_deinit(eeyelop);
  array_list_deinit(&eeyelop->outputs);
  egl_deinit(&eeyelop->egl);
  text_deinit(&eeyelop->text);
  zwlr_layer_surface_v1_destroy(eeyelop->surface.layer);
}
