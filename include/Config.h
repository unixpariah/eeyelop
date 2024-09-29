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
  int top;
  int left;
  int right;
  int bottom;
} Margin;

typedef struct {
  float background_color[4];
  int width;
  int height;
  Margin margin;
  char *output;
  enum Anchor anchor;
  enum Layer layer;
  struct {
    const char *name;
    int size;
  } font;
} Config;

Config config_init(void);

void config_update(Config *, Output *);

#endif // CONFIG_H
