#ifndef WINDOW_H
#define WINDOW_H

#include "gtk/gtk.h"
#include "gtk/gtkshortcut.h"

void activate(GtkApplication *app, gpointer user_data);

void update_mode_indicator(char *text);

void scroll_webview(int dx, int dy);

void activate(GtkApplication *app, gpointer user_data);

GtkWidget *get_web_view();

#endif
