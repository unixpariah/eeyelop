#define GL_GLEXT_PROTOTYPES 1
#define _POSIX_C_SOURCE 199309L

#include "Config.h"
#include "EGL/egl.h"
#include "EGL/eglplatform.h"
#include "Eeyelop.h"
#include "EventLoop.h"
#include "GL/glext.h"
#include "bits/time.h"
#include "bits/types.h"
#include "bits/types/struct_itimerspec.h"
#include "hiv/ArrayList.h"
#include "stdfloat.h"
#include "wayland-client-protocol.h"
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include <GL/gl.h>
#include <Notification.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>

int eeyelop_notification_init(Eeyelop *eeyelop, EventLoop *event_loop,
                              uint8_t *text) {
  Notification *notification = malloc(sizeof(Notification));
  if (!notification) {
    perror("Out of memory\nFailed to allocate space for notification\n");
    return -1;
  }

  float32_t index = (float32_t)eeyelop->notifications.len;

  if (index == 0) {
    if (eeyelop_surface_init(eeyelop) == -1) {
      free(notification);
      return -1;
    }
  }

  float32_t y =
      eeyelop->config.height * index + eeyelop->config.margin.top * (index + 1);

  if (index > 0) {
    y += eeyelop->config.margin.bottom * index;
  }

  int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  struct itimerspec value = {
      .it_value =
          {
              .tv_sec = eeyelop->config.default_timeout / 1000,
              .tv_nsec =
                  (__syscall_slong_t)(eeyelop->config.default_timeout % 1000) *
                  1000000,
          },
      .it_interval =
          {
              .tv_sec = 0,
              .tv_nsec = 0,
          },
  };
  timerfd_settime(fd, 0, &value, 0);

  notification->x = eeyelop->config.margin.left, notification->y = y;
  notification->width = eeyelop->config.width;
  notification->height = eeyelop->config.height, notification->text = text;
  notification->tfd = fd;

  NotificationCallbackData *noti_cb_data =
      malloc(sizeof(NotificationCallbackData));
  if (!noti_cb_data) {
    perror("Failed to allocate space for callback data\n");
    free(notification);
    return -1;
  }

  noti_cb_data->notification = notification;
  noti_cb_data->eeyelop = eeyelop;

  event_loop_insert_source(event_loop, notification->tfd, notification_callback,
                           noti_cb_data, 1);

  if (array_list_append(&eeyelop->notifications, notification) ==
      ARRAY_LIST_OOM) {
    perror("Out of memory\nFailed to append notification to list\n");
    free(noti_cb_data);
    free(notification);
  };

  float32_t total_width = eeyelop->config.width + eeyelop->config.margin.left +
                          eeyelop->config.margin.right;

  float32_t total_height =
      (eeyelop->config.height + eeyelop->config.margin.top +
       eeyelop->config.margin.bottom) *
      (float32_t)eeyelop->notifications.len;

  zwlr_layer_surface_v1_set_size(eeyelop->surface.layer, (uint32_t)total_width,
                                 (uint32_t)total_height);

  return 0;
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

void notification_callback(void *data) {
  NotificationCallbackData *noti_cb_data = data;

  uint64_t repeats = 0;
  read(noti_cb_data->notification->tfd, &repeats, sizeof(repeats));

  for (uint32_t i = 0; i < noti_cb_data->eeyelop->notifications.len; i++) {
    if (noti_cb_data->notification ==
        noti_cb_data->eeyelop->notifications.items[i]) {
      eeyelop_notification_deinit(noti_cb_data->eeyelop, i);
      break;
    }
  }

  if (eeyelop_render(noti_cb_data->eeyelop) == -1) {
    EGLint error = eglGetError();
    printf("Failed to render with error: 0x%x\n", error);
  };
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

void eeyelop_notification_deinit(Eeyelop *eeyelop, uint32_t index) {
  array_list_ordered_remove(&eeyelop->notifications, index);

  if (eeyelop->notifications.len == 0) {
    zwlr_layer_surface_v1_destroy(eeyelop->surface.layer);
    wl_surface_destroy(eeyelop->surface.wl_surface);
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
