
#include "command_line.h"
#include "gtk/gtk.h"
#include <stdbool.h>

GtkWidget *cmd_overlay;
static bool command_line = false;

GtkWidget *new_cmd_overlay() {
  cmd_overlay = gtk_overlay_new();
  gtk_widget_set_visible(cmd_overlay, FALSE);
  return cmd_overlay;
}

void show_command_line() {
  gtk_widget_set_visible(cmd_overlay, TRUE);
  command_line = TRUE;
}

void hide_command_line() {
  gtk_widget_set_visible(cmd_overlay, FALSE);
  command_line = FALSE;
}

bool get_command_line_showen() { return command_line; }
