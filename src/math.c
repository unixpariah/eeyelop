#include <math.h>

void math_orthographic_projection(Mat4 *mat4, float left, float right,
                                  float top, float bottom) {
  // Directly assign values to the mat4 matrix
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
  (*mat4)[3][2] = (1 + 0) / -1;
  (*mat4)[3][3] = 1;
}
