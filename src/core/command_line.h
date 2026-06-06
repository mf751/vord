#ifndef COMMAND_LINE_H
#define COMMAND_LINE_H

#include "gtk/gtk.h"
#include "gtk/gtkshortcut.h"
#include <stdbool.h>

void add_cmd_to_overlay(GtkWidget *overlay);

void show_command_line();

void hide_command_line();

bool get_command_line_showen();

#endif // !COMMAND_LINE_H
