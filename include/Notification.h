#include "Config.h"
#include "Eeyelop.h"
#include "stdfloat.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  float32_t x;
  float32_t y;
  float32_t width;
  float32_t height;
  uint8_t *text;
  int32_t tfd;
} Notification;

typedef struct {
  Notification *notification;
  Eeyelop *eeyelop;
} NotificationCallbackData;

Notification notification_init(Config *config, uint8_t *text, uint32_t index);

void eeyelop_notification_render(Eeyelop *eeyelop, Notification *notification);

bool notification_contains_coords(Notification *notification, float32_t x,
                                  float32_t y);

void notification_callback(void *data);

void eeyelop_notification_remove(Eeyelop *eeyelop, uint32_t index);
