#ifndef WINDOW_H
#define WINDOW_H

#include "gtk/gtk.h"
#include "gtk/gtkshortcut.h"
typedef enum {
  NORMAL_MODE = 0,
  INSERT_MODE,
  ADDRESS_MODE,
  VISUAL_MODE,
} EditorMode;

char *get_mode_name();

EditorMode get_mode();

void set_mode(EditorMode new_mode);

void activate(GtkApplication *app, gpointer user_data);

void scroll_webview(int dx, int dy);

void activate(GtkApplication *app, gpointer user_data);

GtkWidget *get_web_view();

#endif
