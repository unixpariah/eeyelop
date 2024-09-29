#include "GL/gl.h"
#include "GL/glext.h"
#include "math.h"
#include <Config.h>
#include <Text.h>
#include <fontconfig/fontconfig.h>
#include <freetype/freetype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int character_init(Character *character, FT_Face face, int key) {

  if (FT_Load_Char(face, key, FT_LOAD_DEFAULT)) {
    return -1;
  }

  if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL)) {
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
  character->size[0] = (int)face->glyph->bitmap.width;
  character->size[1] = (int)face->glyph->bitmap.rows;
  character->bearing[0] = (int)face->glyph->bitmap_left;
  character->bearing[1] = (int)face->glyph->bitmap_top;
  character->advance[0] = (int)face->glyph->advance.x >> 6;
  character->advance[1] = (int)face->glyph->advance.y >> 6;

  return 0;
}

int get_font_path(char **font_path_ptr, const char *font_name) {
  if (!FcInit()) {
    printf("Failed to initialize fontconfig\n");
    return -1;
  }

  FcConfig *config = FcInitLoadConfigAndFonts();
  if (config == NULL) {
    printf("Failed to initialize fontconfig\n");
    return -1;
  }

  FcPattern *pattern = FcNameParse((const FcChar8 *)font_name);
  if (pattern == NULL) {
    printf("Failed to parse name %s\n", font_name);
    return -1;
  }

  FcDefaultSubstitute(pattern);
  if (!FcConfigSubstitute(config, pattern, FcMatchPattern)) {
    printf("Failed to do something with substitute\n");
    return -1;
  };

  FcResult result = {0};
  FcPattern *match = FcFontMatch(config, pattern, &result);
  if (match == NULL) {
    printf("Failed to find a font match\n");
    return -1;
  }

  FcChar8 *font_path = NULL;
  if (FcPatternGetString(match, FC_FILE, 0, &font_path) != FcResultMatch) {
    printf("Match not found\n");
    return -1;
  }

  if (font_path == NULL) {
    return -1;
  }

  *font_path_ptr = (char *)malloc(strlen((const char *)font_path) + 1);
  if (font_path_ptr == NULL) {
    printf("Out of memory\n");
    return -1;
  }

  strcpy(*font_path_ptr, (const char *)font_path);

  FcPatternDestroy(match);
  FcPatternDestroy(pattern);
  FcConfigDestroy(config);

  FcFini();

  return 0;
}

Text text_init(Config *config) {
  Text text = {
      .letter_map = {0},
      .transform = {0},
      .char_info = {0},
      .index = 0,
      .scale = config->font.size / 256,
      .scale_mat = {0},
  };
  math_scale(&text.scale_mat, (float)config->font.size,
             (float)config->font.size, 0);

  FT_Library ft = NULL;
  if (FT_Init_FreeType(&ft)) {
    printf("Failed to initialize freetype2\n");
  }

  FT_Face face = NULL;

  char *font_path = NULL;
  if (get_font_path(&font_path, config->font.name) == -1) {
    printf("\n");
    exit(1);
  };

  if (FT_New_Face(ft, font_path, 0, &face)) {
    printf("\n");
    exit(1);
  }

  free(font_path);

  if (FT_Set_Pixel_Sizes(face, 256, 256)) {
    printf("\n");
    exit(1);
  }

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  GLuint texture_array = 0;
  glGenTextures(1, &texture_array);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D_ARRAY, texture_array);
  glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R8, 256, 256,
               256, // Char len
               0, GL_RED, GL_UNSIGNED_BYTE, NULL);

  for (int i = 0; i < 256; i++) {
    if (character_init(&text.char_info[i], face, i) == -1) {
      printf("Failed to initialize character\n");
    }
  }

  glBindTexture(GL_TEXTURE_2D_ARRAY, texture_array);

  FT_Done_Face(face);
  FT_Done_FreeType(ft);

  return text;
}
