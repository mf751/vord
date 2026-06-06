
#include "command_line.h"
#include "gtk/gtk.h"
#include <stdbool.h>

GtkWidget *cmd_overlay;
GtkWidget *cmd_entry;
static bool command_line = false;

GtkWidget *new_cmd_overlay(GtkWidget *web_view) {
    cmd_overlay = gtk_overlay_new();
    gtk_overlay_set_child(GTK_OVERLAY(cmd_overlay), web_view);

    cmd_entry = gtk_entry_new();
    gtk_widget_add_css_class(cmd_entry, "cmd-line");
    gtk_entry_set_overwrite_mode(GTK_ENTRY(cmd_entry), TRUE);

    gtk_overlay_add_overlay(GTK_OVERLAY(cmd_overlay), cmd_entry);

    gtk_widget_set_halign(cmd_entry, GTK_ALIGN_FILL);
    gtk_widget_set_valign(cmd_entry, GTK_ALIGN_END);

    gtk_widget_set_size_request(cmd_entry, -1, 32);

    gtk_widget_set_visible(cmd_entry, FALSE);

    return cmd_overlay;
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
