#ifndef APP_H
#define APP_H

#include "glib.h"
#include "gtk/gtk.h"
#include "gtk/gtkshortcut.h"

typedef enum {
    NORMAL_MODE = 0,
    INSERT_MODE,
    VISUAL_MODE,
    ADDRESS_MODE,
} EditorMode;

typedef struct {
    GtkWidget *container;
    GtkWidget *number;
    GtkWidget *title;
    GtkWidget *web_view;
} Tab;

typedef struct {
    GtkApplication *gtk_app;
    GList *tabs;
    Tab *current_tab;

    EditorMode current_mode;
} App;

App *new_app(GtkApplication *gtk_app);

void free_app(App *app);

void set_mode(App *app, EditorMode new_mode);

char *get_mode_name(EditorMode mode);

#endif // ifndef APP_H
