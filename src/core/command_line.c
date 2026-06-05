
#include "command_line.h"
#include "gtk/gtk.h"
#include <stdbool.h>

GtkWidget *cmd_overlay;
static bool command_line = false;

GtkWidget *new_cmd_overlay() {
  cmd_overlay = gtk_overlay_new();
  gtk_widget_set_visible(cmd_overlay, FALSE);
  GtkWidget *cmd_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_overlay_add_overlay(GTK_OVERLAY(cmd_overlay), cmd_box);

  GtkWidget *cmd_line = gtk_entry_new();
  gtk_widget_add_css_class(cmd_line, "cmd-line");
  gtk_entry_set_overwrite_mode(GTK_ENTRY(cmd_line), TRUE);

  gtk_overlay_add_overlay(GTK_OVERLAY(cmd_overlay), cmd_line);
  gtk_widget_set_halign(cmd_line, GTK_ALIGN_FILL);
  gtk_widget_set_valign(cmd_line, GTK_ALIGN_END);
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
