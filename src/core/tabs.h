#ifndef TABS_H
#define TABS_H

#include "app.h"

Tab *new_tab(App *app, char *uri);
void switch_to_tab(App *app, Tab *tab);
void switch_to_next_tab(App *app);
void close_tab(App *app, Tab *tab);
void close_current_tab(App *app);

#endif // !TABS_H
