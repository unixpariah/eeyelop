#define GL_GLEXT_PROTOTYPES 1

#include "Config.h"
#include "Egl.h"
#include "GL/glext.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/gl.h>
#include <Notification.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Notification notification_init(Config *config, int index) {
  int y = config->height * index + config->margin.top * (index + 1);

  if (index > 0) {
    y += config->margin.bottom * index;
  }

  Notification notification = {
      .x = config->margin.left,
      .y = y,
      .width = config->width,
      .height = config->height,
  };

  return notification;
}

void notification_render(Notification *notification, Egl *egl) {
  // clang-format off
  int vertices[8] = {
      notification->x,                       notification->y,
      notification->x + notification->width, notification->y,
      notification->x,                       notification->y + notification->height,
      notification->x + notification->width, notification->y + notification->height,
  };
  // clang-format on

  glBindBuffer(GL_ARRAY_BUFFER, egl->VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, egl->VBO);
  glVertexAttribPointer(0, 2, GL_INT, GL_FALSE, 0, NULL);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}
