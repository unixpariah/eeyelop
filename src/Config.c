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
      .background_color = {0, 0, 0, 1},
      .font =
          {
              .name = "JetBrains Mono",
              .size = 16,
          },
  };

  return config;
}
