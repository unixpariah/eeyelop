#include "Config.h"
#include "stdfloat.h"
#include <stdint.h>

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
      .output = (uint8_t *)"",
      .anchor = top_right,
      .layer = overlay,
      .background =
          {
              .color = {(float32_t)0.09, (float32_t)0.055, (float32_t)0.122, 1},
          },
      .border =
          {
              .size = 2,
              .color = {(float32_t)0.788, (float32_t)0.796, 1},
          },
      .font =
          {
              .name = (uint8_t *)"JetBrainsMono Nerd Font",
              .size = 16,
              .color = {1, 1, 1, 1},
          },
      .default_timeout = 1000,
  };

  return config;
}
