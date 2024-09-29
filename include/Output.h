#ifndef OUTPUT_H
#define OUTPUT_H

#include "Egl.h"
#include "wayland-client-protocol.h"

typedef struct {
  uint32_t id;
  int height;
  int width;
  int x;
  int y;
  char *name;
  char *description;
} OutputInfo;

void output_info_deinit(OutputInfo *);

typedef struct {
  OutputInfo info;
  EglSurface egl;
  struct wl_surface *surface;
  struct zwlr_layer_surface_v1 *layer_surface;
  struct wl_output *wl_output;
} Output;

Output output_init(EglSurface, struct wl_surface *,
                   struct zwlr_layer_surface_v1 *, struct wl_output *,
                   unsigned int);

void output_surface_resize(Output *, int, int);

void output_deinit(Output *);

extern const struct wl_output_listener output_listener;

extern const struct zwlr_layer_surface_v1_listener layer_surface_listener;

#endif // OUTPUT_H
