#include <wayland-client.h>

int main() {
  struct wl_display *display = wl_display_connect(NULL);

  return 0;
}
