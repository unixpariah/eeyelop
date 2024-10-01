#include <math.h>

void math_transform(Mat4 *mat4, float font_size, float x, float y) {
  (*mat4)[0][0] = (*mat4)[1][1] = font_size;
  (*mat4)[0][1] = (*mat4)[0][2] = (*mat4)[0][3] = 0;
  (*mat4)[1][0] = (*mat4)[1][2] = (*mat4)[1][3] = 0;
  (*mat4)[2][0] = (*mat4)[2][1] = (*mat4)[2][2] = (*mat4)[2][2] =
      (*mat4)[2][3] = 0;
  (*mat4)[3][0] = x;
  (*mat4)[3][1] = y;
  (*mat4)[3][2] = 0;
  (*mat4)[3][3] = 1;
}

void math_orthographic_projection(Mat4 *mat4, float left, float right,
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

typedef float Mat4[4][4];

void math_scale(Mat4 *mat4, float x, float y, float z) {
  (*mat4)[0][0] = x;
  (*mat4)[1][1] = y;
  (*mat4)[2][2] = z;
  (*mat4)[3][3] = 1;

  (*mat4)[0][1] = (*mat4)[0][2] = (*mat4)[0][3] = 0;
  (*mat4)[1][0] = (*mat4)[1][2] = (*mat4)[1][3] = 0;
  (*mat4)[2][0] = (*mat4)[2][1] = (*mat4)[2][3] = 0;
  (*mat4)[3][0] = (*mat4)[3][1] = (*mat4)[3][2] = 0;
}

void math_translate(Mat4 *mat4, float x, float y, float z) {
  (*mat4[0][0]) = (*mat4[1][1]) = (*mat4[2][2]) = (*mat4[3][3]) = 1;

  (*mat4[3][0]) = x;
  (*mat4[3][1]) = y;
  (*mat4[3][2]) = z;

  (*mat4[0][1]) = (*mat4[0][2]) = (*mat4[0][3]) = 0;
  (*mat4[1][0]) = (*mat4[1][2]) = (*mat4[1][3]) = 0;
  (*mat4[2][0]) = (*mat4[2][1]) = (*mat4[2][3]) = 0;
}

void mat4_mul(const Mat4 m0, const Mat4 m1, Mat4 result) {
  for (int row = 0; row < 4; row++) {
    float vx[4] = {m0[row][0], m0[row][0], m0[row][0], m0[row][0]};
    float vy[4] = {m0[row][1], m0[row][1], m0[row][1], m0[row][1]};
    float vz[4] = {m0[row][2], m0[row][2], m0[row][2], m0[row][2]};
    float vw[4] = {m0[row][3], m0[row][3], m0[row][3], m0[row][3]};

    for (int i = 0; i < 4; i++) {
      vx[i] *= m1[0][i];
      vy[i] *= m1[1][i];
      vz[i] *= m1[2][i];
      vw[i] *= m1[3][i];
    }

    for (int i = 0; i < 4; i++) {
      vx[i] += vz[i];
      vy[i] += vw[i];
      vx[i] += vy[i];
      result[row][i] = vx[i];
    }
  }
}
