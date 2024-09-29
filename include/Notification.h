#include "Config.h"

typedef struct {
  int x;
  int y;
  int width;
  int height;
} Notification;

Notification notification_init(Config *config, int index);

void notification_render(Notification *notification, Egl *egl);
