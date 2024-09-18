#ifndef OUTPUT_H
#define OUTPUT_H

#include "../include/Eeyelop.h"

typedef struct {
  uint32_t id;
  char *name;
  char *description;
  int height;
  int width;
  int x;
  int y;
} OutputInfo;

void output_info_deinit(OutputInfo *);

typedef struct {
  // TODO: Egl surface
  OutputInfo output_info;
  struct wl_surface *surface;
  struct zwlr_layer_surface_v1 *layer_surface;
  struct zxdg_output_v1 *xdg_output;
  struct wl_output *wl_output;
} Output;

void output_init(Output *, struct wl_surface *, struct zwlr_layer_surface_v1 *,
                 struct wl_output *, struct zxdg_output_v1 *, int);

void output_deinit(Output *);

extern const struct zxdg_output_v1_listener xdg_output_listener;

#endif // OUTPUT_H
