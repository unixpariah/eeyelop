#ifndef EEYELOP_H
#define EEYELOP_H

#include "EGL/egl.h"
#include "GL/gl.h"
#include <ArrayList.h>
#include <Config.h>
#include <Seat.h>
#include <Text.h>
#include <stdbool.h>

typedef struct {
  GLuint VAO;
  GLuint VBO;
  GLuint EBO;
  GLuint main_shader_program;
  GLuint text_shader_program;
  EGLDisplay display;
  EGLConfig config;
  EGLContext context;
} Egl;

typedef struct {
  struct wl_compositor *compositor;
  struct zwlr_layer_shell_v1 *layer_shell;
  struct {
    bool configured;
    int width;
    int height;
    struct wl_egl_window *egl_window;
    EGLSurface egl_surface;
    struct zwlr_layer_surface_v1 *layer;
    struct wl_surface *wl_surface;
  } surface;
  ArrayList outputs;
  ArrayList notifications;
  Egl egl;
  Config config;
  Seat seat;
  Text text;
} Eeyelop;

Eeyelop eeyelop_init(void);

int eeyelop_surface_init(Eeyelop *);

void eeyelop_config_update(Eeyelop *);

int eeyelop_egl_init(Eeyelop *eeyelop, struct wl_display *);

int eeyelop_egl_deinit(Egl *);

void eeyelop_deinit(Eeyelop *eeyelop);

#endif // EEYELOP_H
