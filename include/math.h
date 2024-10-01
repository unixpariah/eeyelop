#ifndef MATH_H
#define MATH_H

typedef float Mat4[4][4];

void math_orthographic_projection(Mat4 *mat4, float left, float right,
                                  float top, float bottom);

void math_transform(Mat4 *mat4, float font_size, float x, float y);

#endif // MATH_H
