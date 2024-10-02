#ifndef CONFIG_H
#define CONFIG_H

#include "stdfloat.h"
#include <Output.h>
#include <stdint.h>

enum Anchor {
  top_right,
  top_center,
  top_left,
  bottom_right,
  bottom_center,
  bottom_left,
  center_right,
  center_left,
  center,
};

enum Layer {
  background,
  bottom,
  top,
  overlay,
};

typedef struct {
  float color[4];
} Background;

typedef struct {
  float32_t top;
  float32_t left;
  float32_t right;
  float32_t bottom;
} Margin;

typedef struct {
  float32_t color[4];
  float32_t size;
} Border;

typedef struct {
  const uint8_t *name;
  float32_t size;
  float32_t color[4];
} Font;

typedef struct {
  float32_t width;
  float32_t height;
  Margin margin;
  uint8_t *output;
  enum Anchor anchor;
  enum Layer layer;
  Border border;
  Font font;
  Background background;
} Config;

Config config_init(void);

#endif // CONFIG_H
