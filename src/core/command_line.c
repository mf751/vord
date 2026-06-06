
#include "command_line.h"
#include "gtk/gtk.h"
#include "gtk/gtkshortcut.h"
#include <stdbool.h>

GtkWidget *cmd_entry;
static bool command_line = false;

void add_cmd_to_overlay(GtkWidget *overlay) {
    cmd_entry = gtk_entry_new();
    gtk_widget_add_css_class(cmd_entry, "cmd-line");
    gtk_entry_set_overwrite_mode(GTK_ENTRY(cmd_entry), TRUE);
    gtk_widget_set_halign(cmd_entry, GTK_ALIGN_FILL);
    gtk_widget_set_valign(cmd_entry, GTK_ALIGN_END);

    gtk_widget_set_size_request(cmd_entry, -1, 32);

    gtk_widget_set_visible(cmd_entry, FALSE);

    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), cmd_entry);
}

void show_command_line() {
    gtk_widget_set_visible(cmd_entry, TRUE);
    command_line = TRUE;
}

void hide_command_line() {
    gtk_widget_set_visible(cmd_entry, FALSE);
    command_line = FALSE;
}

bool get_command_line_showen() { return command_line; }
