#include "Eeyelop.h"

Eeyelop eeyelop_init(void) {
  Eeyelop eeyelop = {
      .compositor = NULL,
      .layer_shell = NULL,
      .seat = NULL,
      .output_manager = NULL,
      .outputs = array_list_init(sizeof(Output)),
      .exit = false,
  };

  return eeyelop;
}

void eeyelop_deinit(Eeyelop *eeyelop) {
  wl_seat_destroy(eeyelop->seat);
  wl_compositor_destroy(eeyelop->compositor);
  zwlr_layer_shell_v1_destroy(eeyelop->layer_shell);
  zxdg_output_manager_v1_destroy(eeyelop->output_manager);

  for (int i = 0; i < eeyelop->outputs.len; i++) {
    Output *output = (Output *)eeyelop->outputs.items[i];
    output_deinit(output);
  }

  array_list_deinit(&eeyelop->outputs);
}
