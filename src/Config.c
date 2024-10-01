#include "Config.h"

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
      .output = "",
      .anchor = top_right,
      .layer = overlay,
      .background =
          {
              .color = {0, 1, 0, 1},
          },
      .border =
          {
              .size = 5,
              .color = {1, 0, 1, 1},
          },
      .font =
          {
              .name = "JetBrains Mono",
              .size = 16,
              .color = {1, 1, 1, 1},
          },
  };

  return config;
}
