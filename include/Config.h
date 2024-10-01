#ifndef CONFIG_H
#define CONFIG_H

#include <Output.h>

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
  int top;
  int left;
  int right;
  int bottom;
} Margin;

typedef struct {
  float color[4];
  float size;
} Border;

typedef struct {
  const char *name;
  int size;
  float color[4];
} Font;

typedef struct {
  int width;
  int height;
  Margin margin;
  char *output;
  enum Anchor anchor;
  enum Layer layer;
  Border border;
  Font font;
  Background background;
} Config;

Config config_init(void);

#endif // CONFIG_H
