#ifndef COMMAND_LINE_H
#define COMMAND_LINE_H

#include "gtk/gtk.h"
#include <stdbool.h>

GtkWidget *new_cmd_overlay(GtkWidget *web_view);

void show_command_line();

void hide_command_line();

bool get_command_line_showen();

#endif // !COMMAND_LINE_H
