#ifndef WINDOW_H
#define WINDOW_H

#include "gtk/gtk.h"
#include "tabs.h"
#include "webkit/webkit.h"

void add_tab_to_view(Tab *tab);

void activate(GtkApplication *app, gpointer user_data);

void on_load_changed(WebKitWebView *wv, WebKitLoadEvent load_event,
                     gpointer user_data);

void update_mode_indicator(char *text);

void scroll_webview(App *app, int dx, int dy);

void activate(GtkApplication *app, gpointer user_data);

#endif
