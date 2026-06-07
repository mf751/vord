#include "tabs.h"
#include "../ui/ui.h"
#include "app.h"
#include "glib.h"
#include "glibconfig.h"
#include "gtk/gtk.h"
#include "webkit/webkit.h"
#include "window.h"

Tab *new_tab(App *app, char *uri) {
    Tab *tab = g_new0(Tab, 1);

    add_tab_widget(tab, "", 0);
    add_tab(tab);
    app->tabs = g_list_append(app->tabs, tab);

    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(tab->web_view), uri);

    g_signal_connect(tab->web_view, "load-changed", G_CALLBACK(on_load_changed), app);
    return tab;
}

void switch_to_tab(App *app, Tab *tab) {
    if (!tab || tab == app->current_tab)
        return;

    if (app->current_tab) {
        gtk_widget_remove_css_class(app->current_tab->container, "active");
        gtk_widget_set_visible(app->current_tab->web_view, FALSE);
    }

    app->current_tab = tab;
    gtk_widget_add_css_class(app->current_tab->container, "active");
    gtk_widget_set_visible(app->current_tab->web_view, TRUE);
}

void switch_to_next_tab(App *app) {
    if (!app->tabs || !app->current_tab)
        return;

    GList *current_node = g_list_find(app->tabs, app->current_tab);
    if (!current_node)
        return;

    GList *next_node = current_node->next;
    if (next_node) {
        Tab *next_tab = (Tab *)next_node->data;
        switch_to_tab(app, next_tab);
        return;
    }
    Tab *first_tab = (Tab *)app->tabs->data;
    switch_to_tab(app, first_tab);
}

void close_tab(App *app, Tab *tab) {
    if (g_list_length(app->tabs) == 1) {
        Tab *start_tab = new_tab(app, start_page_uri());
        add_tab(start_tab);
        switch_to_tab(app, start_tab);
    } else {
        switch_to_next_tab(app);
    }

    remove_tab(tab);
    app->tabs = g_list_remove(app->tabs, tab);
}

void close_current_tab(App *app) {
    close_tab(app, app->current_tab);
}
