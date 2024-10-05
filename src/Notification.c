#define GL_GLEXT_PROTOTYPES 1

#include "Config.h"
#include "Eeyelop.h"
#include "GL/glext.h"
#include "hiv/ArrayList.h"
#include "stdfloat.h"
#include "wayland-client-protocol.h"
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include <GL/gl.h>
#include <Notification.h>
#include <cglm/mat4.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Notification notification_init(Config *config, uint8_t *text, uint32_t index) {
  float32_t y = config->height * (float32_t)index +
                config->margin.top * ((float32_t)index + 1);

  if (index > 0) {
    y += config->margin.bottom * (float32_t)index;
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
  float32_t vertices[8] = {
      notification->x,                       notification->y,
      notification->x + notification->width, notification->y,
      notification->x,                       notification->y + notification->height,
      notification->x + notification->width, notification->y + notification->height,
  };
  // clang-format on

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

void notification_render_border(Notification *notification, Border *border,
                                GLuint shader_program, GLuint VBO) {
  glLineWidth(border->size);
  GLint location = glGetUniformLocation(shader_program, "color");
  glUniform4fv(location, 1, (const GLfloat *)&border->color);

  // clang-format off
  float32_t vertices[8] = {
      notification->x,                       notification->y,
      notification->x + notification->width, notification->y,
      notification->x,                       notification->y + notification->height,
      notification->x + notification->width, notification->y + notification->height,
  };
  // clang-format on

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
  glDrawElements(GL_LINE_LOOP, 6, GL_UNSIGNED_INT, NULL);
}

void notification_render_text(Notification *notification, Text *text,
                              GLuint shader_program, GLuint VBO) {
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
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

void eeyelop_notification_remove(Eeyelop *eeyelop, uint32_t index) {
  array_list_ordered_remove(&eeyelop->notifications, index);

  if (eeyelop->notifications.len == 0) {
    zwlr_layer_surface_v1_destroy(eeyelop->surface.layer);
    return;
  }

  for (uint32_t i = index; i < eeyelop->notifications.len; i++) {
    Notification *notification =
        (Notification *)eeyelop->notifications.items[i];

    notification->y =
        (notification->height + eeyelop->config.margin.bottom) * (float32_t)i +
        eeyelop->config.margin.top * ((float32_t)i + 1);
  }

  float32_t total_width = eeyelop->config.width + eeyelop->config.margin.left +
                          eeyelop->config.margin.right;

  float32_t total_height =
      (eeyelop->config.height + eeyelop->config.margin.top +
       eeyelop->config.margin.bottom) *
      (float32_t)eeyelop->notifications.len;

  zwlr_layer_surface_v1_set_size(eeyelop->surface.layer, (uint32_t)total_width,
                                 (uint32_t)total_height);
  wl_surface_commit(eeyelop->surface.wl_surface);
}

bool notification_contains_coords(Notification *notification, float32_t x,
                                  float32_t y) {
  return x > notification->x && x < notification->x + notification->width &&
         y > notification->y && y < notification->y + notification->height;
}
