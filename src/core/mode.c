#include "mode.h"
#include "../ui/visual_mode.h"
#include "gtk/gtkshortcut.h"
#include "window.h"

static EditorMode current_mode = NORMAL_MODE;

EditorMode get_mode() { return current_mode; }

char *get_mode_name() {
  switch (current_mode) {
  case NORMAL_MODE:
    return "Normal";
  case INSERT_MODE:
    return "Insert";
  case ADDRESS_MODE:
    return "Address";
  case VISUAL_MODE:
    return "VISUAL";
  default:
    return "Unkown";
  }
}

void set_mode(EditorMode new_mode) {
  GtkWidget *web_view = get_web_view();
  if (current_mode == VISUAL_MODE && new_mode == NORMAL_MODE)
    stop_visual_mode(web_view);
  current_mode = new_mode;
  update_mode_indicator();
  if (new_mode == VISUAL_MODE)
    start_visual_mode(web_view);
}
