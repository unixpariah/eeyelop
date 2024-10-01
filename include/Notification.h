#include "Config.h"
#include "Eeyelop.h"
#include <stdbool.h>

typedef struct {
  int x;
  int y;
  int width;
  int height;
  char *text;
} Notification;

Notification notification_init(Config *config, char *text, int index);

void eeyelop_notification_render(Eeyelop *eeyelop, Notification *notification);

bool notification_contains_coords(Notification *notification, int x, int y);

void eeyelop_notification_remove(Eeyelop *eeyelop, int index);
