#include "app.h"
#include "../ui/visual_mode.h"
#include "gtk/gtk.h"
#include "window.h"

App *new_app(GtkApplication *gtk_app) {
    App *app = g_new0(App, 1);
    app->gtk_app = gtk_app;
    app->current_mode = NORMAL_MODE;
    return app;
}

void free_app(App *app) {
    if (app) {
        g_free(app);
    }
}

char *get_mode_name(EditorMode mode) {
    switch (mode) {
    case NORMAL_MODE:
        return "Normal";
    case INSERT_MODE:
        return "Insert";
    case ADDRESS_MODE:
        return "Address";
    case VISUAL_MODE:
        return "VISUAL";
    default:
        return "Unkown";
    }
}

void set_mode(App *app, EditorMode new_mode) {
    GtkWidget *web_view = app->web_view;
    if (app->current_mode == VISUAL_MODE && new_mode == NORMAL_MODE)
        stop_visual_mode(web_view);
    app->current_mode = new_mode;
    update_mode_indicator(get_mode_name(app->current_mode));
    if (new_mode == VISUAL_MODE)
        start_visual_mode(web_view);
}
