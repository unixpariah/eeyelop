#include <stdint.h>
#define GL_GLEXT_PROTOTYPES 1

#include "GL/gl.h"
#include "GL/glext.h"
#include "math.h"
#include <Config.h>
#include <Text.h>
#include <fontconfig/fontconfig.h>
#include <freetype/freetype.h>
#include <stdfloat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freetype/ftimage.h"

int character_init(Character *character, FT_Face face, int key) {

  if (FT_Load_Char(face, key, FT_LOAD_RENDER)) {
    return -1;
  }

  glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, key,
                  (GLsizei)face->glyph->bitmap.width,
                  (GLsizei)face->glyph->bitmap.rows, 1, GL_RED,
                  GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  character->key = key;
  character->texture_id = key;
  character->size[0] = (float32_t)face->glyph->bitmap.width;
  character->size[1] = (float32_t)face->glyph->bitmap.rows;
  character->bearing[0] = (float32_t)face->glyph->bitmap_left;
  character->bearing[1] = (float32_t)face->glyph->bitmap_top;
  FT_Pos advance[2] = {face->glyph->advance.x >> 6,
                       face->glyph->advance.y >> 6};
  character->advance[0] = (float32_t)advance[0];
  character->advance[1] = (float32_t)advance[1];

  return 0;
}

int get_font_path(uint8_t **font_path_ptr, const uint8_t *font_name) {
  if (!FcInit()) {
    printf("Failed to initialize fontconfig.\n");
    return -1;
  }

  FcConfig *config = FcInitLoadConfigAndFonts();
  if (config == NULL) {
    printf("Failed to load fontconfig configuration.\n");
    return -1;
  }

  FcPattern *pattern = FcNameParse((const FcChar8 *)font_name);
  if (pattern == NULL) {
    printf("Failed to parse font name '%s'.\n", font_name);
    return -1;
  }

  FcDefaultSubstitute(pattern);
  if (!FcConfigSubstitute(config, pattern, FcMatchPattern)) {
    printf("Failed to substitute pattern.\n");
    return -1;
  }

  FcResult result = {0};
  FcPattern *match = FcFontMatch(config, pattern, &result);
  if (match == NULL) {
    printf("No matching font found for '%s'.\n", font_name);
    return -1;
  }

  uint8_t *font_path = NULL;
  if (FcPatternGetString(match, FC_FILE, 0, &font_path) != FcResultMatch) {
    printf("Failed to retrieve font path.\n");
    return -1;
  }

  if (font_path == NULL) {
    printf("Font path is NULL.\n");
    return -1;
  }

  *font_path_ptr = malloc(strlen((char *)font_path) + 1);
  if (*font_path_ptr == NULL) {
    printf("OOM\n");
    return -1;
  }

  strcpy((char *)*font_path_ptr, (const char *)font_path);

  FcPatternDestroy(match);
  FcPatternDestroy(pattern);
  FcConfigDestroy(config);

  FcFini();

  return 0;
}

int text_init(Text *text, Font *font) {
  FT_Library ft = NULL;
  if (FT_Init_FreeType(&ft)) {
    printf("Failed to initialize freetype2\n");
    return -1;
  }

  FT_Face face = NULL;

  uint8_t *font_path = NULL;
  if (get_font_path(&font_path, font->name) == -1) {
    return -1;
  };

  if (FT_New_Face(ft, (char *)font_path, 0, &face)) {
    printf("Failed to create new freetype face.\n");
    free(font_path);
    return -1;
  }

  free(font_path);

  if (FT_Set_Pixel_Sizes(face, 256, 256)) {
    printf("Failed to set face pixel size\n");
    return -1;
  }

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  GLuint texture_array = 0;
  glGenTextures(1, &texture_array);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D_ARRAY, texture_array);
  glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R8, 256, 256, CHAR_INFO_LEN, 0,
               GL_RED, GL_UNSIGNED_BYTE, NULL);

  for (int i = 0; i < CHAR_INFO_LEN; i++) {
    if (character_init(&text->char_info[i], face, i) == -1) {
      char str[2];
      str[0] = (char)i;
      str[1] = '\0';
      printf("Failed to initialize character %s\n", str);
    }
  }

  glBindTexture(GL_TEXTURE_2D_ARRAY, texture_array);

  FT_Done_Face(face);
  FT_Done_FreeType(ft);

  text->index = 0;
  text->scale = font->size / 256;
  text->font = font;
  text->texture = texture_array;

  return 0;
}

void text_render_call(Text *text_s, GLuint shader_program) {
  if (text_s->index == 0) {
    return;
  }

  GLint color_location = glGetUniformLocation(shader_program, "color");
  GLint transform_location = glGetUniformLocation(shader_program, "transform");
  GLint letterMap_location = glGetUniformLocation(shader_program, "letterMap");

  glUniform4fv(color_location, 1, &text_s->font->color[0]);
  glUniformMatrix4fv(transform_location, text_s->index, GL_FALSE,
                     &text_s->transform[0][0][0]);
  glUniform1iv(letterMap_location, text_s->index, &text_s->letter_map[0]);
  glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, text_s->index);

  text_s->index = 0;
}

void text_place(Text *text_s, const uint8_t *text, float32_t x, float32_t y,
                GLuint shader_program) {
  int text_len = (int)strlen((char *)text);
  if (text_len == 0) {
    return;
  }

  float32_t initial_x = x;

  float32_t move = 0;
  for (int i = 0; i < text_len; i++) {
    uint8_t ch = text[i];
    Character character = text_s->char_info[(int)text[i]];

    if (ch == '\n') {
      y += character.size[1] * text_s->scale;
      x = initial_x;
      continue;
    }

    if (ch == ' ') {
      x += character.advance[0] * text_s->scale;
      continue;
    }

    float32_t x_pos = x + character.bearing[0] * text_s->scale + move;
    float32_t y_pos = y + character.bearing[1] * text_s->scale;

    mat4_transform(&text_s->transform[text_s->index], 16, x_pos, y_pos);
    text_s->letter_map[text_s->index] = character.texture_id;

    move += character.advance[0] * text_s->scale;
    text_s->index++;
    if (text_s->index == LENGTH) {
      text_render_call(text_s, shader_program);
    }
  }
}

void text_deinit(Text *text) { glDeleteTextures(1, &text->texture); }
