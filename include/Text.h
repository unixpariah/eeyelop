#define CHAR_INFO_LEN 128
#define LENGTH 400

#include "Config.h"
#include "GL/gl.h"
#include "math.h"
#include "stdfloat.h"
#include <stdint.h>

typedef struct {
  uint16_t texture_id;
  uint8_t size[2];
  uint8_t bearing[2];
  uint8_t advance[2];
} Character;

typedef struct {
  int letter_map[LENGTH];
  Mat4 transform[LENGTH];
  int index;
  float32_t scale;
  Character char_info[CHAR_INFO_LEN];
  Font *font;
  GLuint texture;
} Text;

int text_init(Text *text, Font *font);

void text_place(Text *text_s, const uint8_t *text, float32_t x, float32_t y,
                GLuint shader_program);

void text_render_call(Text *text_s, GLuint shader_program);

void text_deinit(Text *text);
