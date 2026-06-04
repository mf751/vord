
#include "../ui/visual_mode.h"
#include "../utils/configs.h"
#include "command_line.h"
#include "glib.h"
#include "glibconfig.h"
#include "gtk/gtk.h"
#include "gtk/gtkshortcut.h"
#include "window.h"

gboolean on_key_press(GtkEventControllerKey *controller, guint keyval,
                      guint keycode, GdkModifierType state,
                      gpointer user_data) {
  EditorMode current_mode = get_mode();
  GtkWidget *web_view = get_web_view();
  bool visual_mode_anchored = get_visual_mode_anchored();
  bool command_line_showen = get_command_line_showen();
  GtkWidget *window =
      gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(controller));
  if (keyval == GDK_KEY_Escape) {
    set_mode(NORMAL_MODE);
    if (current_mode == VISUAL_MODE)
      stop_visual_mode(web_view);
    if (command_line_showen)
      hide_command_line();
    return 1;
  }

  gboolean ctrl_pressed = (state & GDK_CONTROL_MASK) != 0;
  gboolean shift_pressed = (state & GDK_SHIFT_MASK) != 0;

  if (current_mode == NORMAL_MODE) {
    switch (keyval) {
    case GDK_KEY_i:
      set_mode(INSERT_MODE);
      return TRUE;
    case GDK_KEY_v:
      set_mode(VISUAL_MODE);
      return TRUE;
    case GDK_KEY_h:
      scroll_webview(-NORMAL_MODE_SCROLL_DISTANCE, 0);
      return TRUE;

    case GDK_KEY_l:
      scroll_webview(NORMAL_MODE_SCROLL_DISTANCE, 0);
      return TRUE;

    case GDK_KEY_k:
      scroll_webview(0, -NORMAL_MODE_SCROLL_DISTANCE);
      return TRUE;

    case GDK_KEY_j:
      scroll_webview(0, NORMAL_MODE_SCROLL_DISTANCE);
      return TRUE;
    case GDK_KEY_colon:
      show_command_line();
      return TRUE;
    }
  } else if (current_mode == VISUAL_MODE) {
    switch (keyval) {
    case GDK_KEY_h:
      if (ctrl_pressed) {
        scroll_webview(-NORMAL_MODE_SCROLL_DISTANCE, 0);
        update_visual_mode_cursor(web_view);
        return 1;
      }
      change_visual_mode_cursor(-VISUAL_MODE_CURSOR_STEP_X, 0);
      update_visual_mode_cursor(web_view);
      return 1;
    case GDK_KEY_l:
      if (ctrl_pressed) {
        update_visual_mode_cursor(web_view);
        scroll_webview(NORMAL_MODE_SCROLL_DISTANCE, 0);
        return 1;
      }
      change_visual_mode_cursor(VISUAL_MODE_CURSOR_STEP_X, 0);
      update_visual_mode_cursor(web_view);
      return 1;
    case GDK_KEY_k:
      if (ctrl_pressed) {
        scroll_webview(0, -NORMAL_MODE_SCROLL_DISTANCE);
        update_visual_mode_cursor(web_view);
        return 1;
      }
      change_visual_mode_cursor(0, -VISUAL_MODE_CURSOR_STEP_Y);
      update_visual_mode_cursor(web_view);
      return 1;
    case GDK_KEY_j:
      if (ctrl_pressed) {
        scroll_webview(0, NORMAL_MODE_SCROLL_DISTANCE);
        update_visual_mode_cursor(web_view);
        return 1;
      }
      change_visual_mode_cursor(0, VISUAL_MODE_CURSOR_STEP_Y);
      update_visual_mode_cursor(web_view);
      return 1;
    case GDK_KEY_v:
      if (visual_mode_anchored) {
        set_visual_mode_anchored(FALSE);
        update_visual_mode_cursor(web_view);
        return 1;
      }
      set_visual_mode_anchored(TRUE);
      visual_mode_anchor_cursor();
      update_visual_mode_cursor(web_view);
      return 1;
    case GDK_KEY_y:
      yank_visual_selection(web_view);
      update_visual_mode_cursor(web_view);
      return 1;
    }
  }
  return 0;
}
