#ifndef EGL_H
#define EGL_H

#include "GL/gl.h"
#include "wayland-egl-core.h"
#include <EGL/egl.h>

typedef struct {
  GLuint VBO;
  GLuint UBO;
  GLuint EBO;
  GLuint main_shader_program;
  EGLDisplay display;
  EGLConfig config;
  EGLContext context;
} Egl;

int egl_init(Egl *, struct wl_display *);

int egl_deinit(Egl *);

typedef struct {
  struct wl_egl_window *window;
  EGLSurface surface;
  EGLDisplay *display;
  EGLConfig *config;
  EGLContext *context;
  GLuint *main_shader_program;
} EglSurface;

EglSurface egl_surface_init(Egl *, struct wl_surface *, int[2]);

void egl_surface_deinit(EglSurface *);

#endif // EGL_H
