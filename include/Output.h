#ifndef OUTPUT_H
#define OUTPUT_H

#include "Egl.h"

typedef struct {
  uint32_t id;
  int height;
  int width;
  int x;
  int y;
  char *name;
  char *description;
} __attribute__((aligned(64))) OutputInfo;

void output_info_deinit(OutputInfo *);

typedef struct {
  OutputInfo output_info;
  EglSurface egl_surface;
  struct wl_surface *surface;
  struct zwlr_layer_surface_v1 *layer_surface;
  struct zxdg_output_v1 *xdg_output;
  struct wl_output *wl_output;
} __attribute__((aligned(128))) Output;

Output output_init(EglSurface, struct wl_surface *,
                   struct zwlr_layer_surface_v1 *, struct wl_output *,
                   struct zxdg_output_v1 *, unsigned int);

void output_deinit(Output *);

extern const struct zxdg_output_v1_listener xdg_output_listener;

extern const struct zwlr_layer_surface_v1_listener zwlr_layer_surface_listener;

#endif // OUTPUT_H
