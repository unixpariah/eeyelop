#define GL_GLEXT_PROTOTYPES 1

#include "shaders/main.h"
#include "wayland-egl-core.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>
#include <Egl.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>

#include "GL/glext.h"

void gl_message_callback(unsigned int source, unsigned int err_type,
                         unsigned int id, unsigned int severity, int length,
                         const char *message, const void *data) {
  printf("%d %d %d %d %d %s\n", source, err_type, id, severity, length,
         message);
}

int compile_shader(const char *shader_source, GLuint shader,
                   GLuint shader_program) {
  glShaderSource(shader, 1, &shader_source, NULL);
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

int egl_init(Egl *egl, struct wl_display *display) {
  if (eglBindAPI(EGL_OPENGL_API) != EGL_TRUE) {
    return -1;
  }
  egl->display = eglGetPlatformDisplay(EGL_PLATFORM_WAYLAND_EXT, display, NULL);
  if (egl->display == EGL_NO_DISPLAY) {
    return -1;
  }

  int minor = 0;
  int major = 0;
  if (eglInitialize(egl->display, &major, &minor) != EGL_TRUE) {
    return -1;
  }

  EGLint count = 0;
  eglGetConfigs(egl->display, NULL, 0, &count);

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
  EGLConfig *configs = calloc(count, sizeof(EGLConfig));
  eglChooseConfig(egl->display, config_attribs, configs, count, &n);
  if (n == 0) {
    return -1;
  }
  egl->config = configs[0];

  // clang-format off
  EGLint context_attribs[] = {
      EGL_CONTEXT_MAJOR_VERSION,       major,
      EGL_CONTEXT_MINOR_VERSION,       minor,
      EGL_CONTEXT_OPENGL_DEBUG,        EGL_TRUE,
      EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
      EGL_NONE
  };
  // clang-format on

  egl->context = eglCreateContext(egl->display, egl->config, EGL_NO_CONTEXT,
                                  context_attribs);
  if (egl->context == EGL_NO_CONTEXT) {
    return -1;
  }

  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(gl_message_callback, NULL);

  if (!eglMakeCurrent(egl->display, EGL_NO_SURFACE, EGL_NO_SURFACE,
                      egl->context)) {
    return -1;
  }

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  GLuint main_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  GLuint main_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

  egl->main_shader_program = glCreateProgram();

  if (compile_shader(vertex_shader_source, main_vertex_shader,
                     egl->main_shader_program) == -1) {
    return -1;
  }

  if (compile_shader(fragment_shader_source, main_fragment_shader,
                     egl->main_shader_program) == -1) {
    return -1;
  }

  glLinkProgram(egl->main_shader_program);

  int link_success = 0;
  glGetProgramiv(egl->main_shader_program, GL_LINK_STATUS, &link_success);
  if (link_success != GL_TRUE) {
    GLchar info_log[512];
    glGetShaderInfoLog(egl->main_shader_program, 512, NULL, info_log);
    printf("%s\n", info_log);
    return -1;
  }

  glDeleteShader(main_vertex_shader);
  glDeleteShader(main_fragment_shader);

  glGenBuffers(1, &egl->VBO);

  return 0;
}

int egl_deinit(Egl *egl) {
  if (egl->context != EGL_NO_CONTEXT) {
    eglDestroyContext(egl->display, egl->context);
  }

  if (egl->display != EGL_NO_DISPLAY) {
    eglTerminate(egl->display);
    egl->display = EGL_NO_DISPLAY;
  }

  if (egl->main_shader_program != 0) {
    glDeleteProgram(egl->main_shader_program);
    egl->main_shader_program = 0;
  }

  return 0;
}

EglSurface egl_surface_init(Egl *egl, struct wl_surface *wl_surface,
                            int size[2]) {
  struct wl_egl_window *egl_window =
      wl_egl_window_create(wl_surface, size[0], size[1]);

  EGLSurface surface = eglCreatePlatformWindowSurface(egl->display, egl->config,
                                                      egl_window, NULL);
  EglSurface egl_surface = {
      .window = egl_window,
      .surface = surface,
      .display = &egl->display,
      .config = &egl->config,
      .context = &egl->context,
      .main_shader_program = &egl->main_shader_program,
  };

  if (egl_surface.window == NULL || egl_surface.surface == EGL_NO_SURFACE) {
    printf("Failed to create egl window or surface");
    exit(1);
  }

  return egl_surface;
}

void egl_surface_deinit(EglSurface *egl_surface) {
  wl_egl_window_destroy(egl_surface->window);
  eglDestroySurface(*egl_surface->display, egl_surface->surface);
  egl_surface->display = NULL;
  egl_surface->main_shader_program = NULL;
  egl_surface->window = NULL;
  egl_surface->config = NULL;
  egl_surface->context = NULL;
}
