#define CHAR_INFO_LEN 128
#define LENGTH 400

#include "Config.h"
#include "GL/gl.h"
#include "math.h"

typedef struct {
  int texture_id;
  int key;
  int size[2];
  int bearing[2];
  int advance[2];
} Character;

typedef struct {
  int letter_map[LENGTH];
  Mat4 transform[LENGTH];
  int index;
  int scale;
  Character char_info[CHAR_INFO_LEN];
  Font *font;
  GLuint texture;
} Text;

int text_init(Text *text, Font *font);

void text_place(Text *text_s, const char *text, int x, int y,
                GLuint shader_program);

void text_render_call(Text *text_s, GLuint shader_program);

void text_deinit(Text *text);
