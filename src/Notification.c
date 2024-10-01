#define GL_GLEXT_PROTOTYPES 1

#include "Config.h"
#include "Eeyelop.h"
#include "GL/glext.h"
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include <GL/gl.h>
#include <Notification.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Notification notification_init(Config *config, char *text, int index) {
  int y = config->height * index + config->margin.top * (index + 1);

  if (index > 0) {
    y += config->margin.bottom * index;
  }

  Notification notification = {
      .x = config->margin.left,
      .y = y,
      .width = config->width,
      .height = config->height,
      .text = text,
  };

  return notification;
}

void notification_render_background(Notification *notification,
                                    Background *background,
                                    GLuint shader_program, GLuint VBO) {

  GLint location = glGetUniformLocation(shader_program, "color");
  glUniform4fv(location, 1, (const GLfloat *)&background->color);

  // clang-format off
  int vertices[8] = {
      notification->x,                       notification->y,
      notification->x + notification->width, notification->y,
      notification->x,                       notification->y + notification->height,
      notification->x + notification->width, notification->y + notification->height,
  };
  // clang-format on

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_INT, GL_FALSE, 0, NULL);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

void notification_render_border(Notification *notification, Border *border,
                                GLuint shader_program, GLuint VBO) {
  glLineWidth(border->size);
  GLint location = glGetUniformLocation(shader_program, "color");
  glUniform4fv(location, 1, (const GLfloat *)&border->color);

  // clang-format off
  int vertices[8] = {
      notification->x,                       notification->y,
      notification->x + notification->width, notification->y,
      notification->x,                       notification->y + notification->height,
      notification->x + notification->width, notification->y + notification->height,
  };
  // clang-format on

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_INT, GL_FALSE, 0, NULL);
  glDrawElements(GL_LINE_LOOP, 6, GL_UNSIGNED_INT, NULL);
}

void notification_render_text(Notification *notification, Text *text,
                              GLuint shader_program, GLuint VBO) {
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  text_place(text, notification->text, notification->x, notification->y,
             shader_program);
  text_render_call(text, shader_program);
}

void eeyelop_notification_render(Eeyelop *eeyelop, Notification *notification) {
  glUseProgram(eeyelop->egl.main_shader_program);

  notification_render_background(notification, &eeyelop->config.background,
                                 eeyelop->egl.main_shader_program,
                                 eeyelop->egl.VBO[0]);

  notification_render_border(notification, &eeyelop->config.border,
                             eeyelop->egl.main_shader_program,
                             eeyelop->egl.VBO[0]);

  glUseProgram(eeyelop->egl.text_shader_program);
  notification_render_text(notification, &eeyelop->text,
                           eeyelop->egl.text_shader_program,
                           eeyelop->egl.VBO[1]);
}

void eeyelop_notification_remove(Eeyelop *eeyelop, int index) {
  array_list_ordered_remove(&eeyelop->notifications, index);

  if (eeyelop->notifications.len == 0) {
    zwlr_layer_surface_v1_destroy(eeyelop->surface.layer);
    return;
  }

  for (int i = index; i < eeyelop->notifications.len; i++) {
    Notification *notification =
        (Notification *)eeyelop->notifications.items[i];

    notification->y =
        (notification->height + eeyelop->config.margin.bottom) * i +
        eeyelop->config.margin.top * (i + 1);
  }

  int total_width = eeyelop->config.width + eeyelop->config.margin.left +
                    eeyelop->config.margin.right;

  int total_height = (eeyelop->config.height + eeyelop->config.margin.top +
                      eeyelop->config.margin.bottom) *
                     eeyelop->notifications.len;

  zwlr_layer_surface_v1_set_size(eeyelop->surface.layer, total_width,
                                 total_height);
  wl_surface_commit(eeyelop->surface.wl_surface);
}

bool notification_contains_coords(Notification *notification, int x, int y) {
  return x > notification->x && x < notification->x + notification->width &&
         y > notification->y && y < notification->y + notification->height;
}
