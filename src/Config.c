#include <Config.h>
#include <Output.h>
#include <stdint.h>
#include <wayland-client-protocol.h>
#include <wayland-egl-core.h>
#include <wlr-layer-shell-unstable-v1-client-protocol.h>

Config config_init(void) {
  Config config = {
      .height = 100,
      .width = 300,
      .margin =
          {
              .bottom = 10,
              .right = 10,
              .left = 10,
              .top = 10,
          },
      .output = "eDP-1",
      .anchor = top_right,
      .layer = overlay,
  };

  return config;
}

void config_update(Config *config, Output *output) {
  uint32_t layer = 0;
  switch (config->layer) {
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
  switch (config->anchor) {
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

  zwlr_layer_surface_v1_set_layer(output->layer_surface, layer);
  zwlr_layer_surface_v1_set_anchor(output->layer_surface, anchor);

  output_surface_resize(output, config->width, config->height);
}
