#include "Config.h"
#define LENGTH 400

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
  Mat4 scale_mat;
  Character char_info[LENGTH];
} Text;

Text text_init(Config *config);
