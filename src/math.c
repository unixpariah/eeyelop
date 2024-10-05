#include <math.h>

void mat4_transform(Mat4 *mat4, float font_size, float x, float y) {
  (*mat4)[0][0] = font_size;
  (*mat4)[1][1] = font_size;

  (*mat4)[0][1] = (*mat4)[0][2] = (*mat4)[0][3] = 0;
  (*mat4)[1][0] = (*mat4)[1][2] = (*mat4)[1][3] = 0;
  (*mat4)[2][0] = (*mat4)[2][1] = (*mat4)[2][2] = 1;
  (*mat4)[2][3] = 0;

  (*mat4)[3][0] = x;
  (*mat4)[3][1] = y;
  (*mat4)[3][2] = 0;
  (*mat4)[3][3] = 1;
}

void mat4_orthographic_projection(Mat4 *mat4, float left, float right,
                                  float top, float bottom) {
  (*mat4)[0][0] = 2 / (right - left);
  (*mat4)[0][1] = 0;
  (*mat4)[0][2] = 0;
  (*mat4)[0][3] = 0;

  (*mat4)[1][0] = 0;
  (*mat4)[1][1] = 2 / (top - bottom);
  (*mat4)[1][2] = 0;
  (*mat4)[1][3] = 0;

  (*mat4)[2][0] = 0;
  (*mat4)[2][1] = 0;
  (*mat4)[2][2] = 2;
  (*mat4)[2][3] = 0;

  (*mat4)[3][0] = -(right + left) / (right - left);
  (*mat4)[3][1] = -(top + bottom) / (top - bottom);
  (*mat4)[3][2] = -1;
  (*mat4)[3][3] = 1;
}
