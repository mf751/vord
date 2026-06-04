#ifndef VISUAL_MODE_H
#define VISUAL_MODE_H

#include <webkit/webkit.h>

void start_visual_mode(GtkWidget *web_view);

void stop_visual_mode(GtkWidget *web_view);

void update_visual_mode_cursor(GtkWidget *web_view);

void copy_to_clipboard(char *text);

void yank_visual_selection(GtkWidget *web_view);

bool get_visual_mode_anchored();

void set_visual_mode_anchored(bool state);

void change_visual_mode_cursor(int dx, int dy);

void change_visual_mode_anchor(int dx, int dy);

void visual_mode_anchor_cursor();

#endif // !VISUAL_MODE_H
