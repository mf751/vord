#ifndef UI_H
#define UI_H

#include "../core/app.h"
#include "gtk/gtk.h"
void load_css(const char *filepath);

void *add_tab_widget(Tab *tab, char *title, int number);

char *start_page_uri();

#endif
