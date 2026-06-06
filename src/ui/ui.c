// Copyright(c) 2026 MF751.All Rights Reserved.

#include "ui.h"
#include "../core/app.h"
#include "gtk/gtk.h"
#include "gtk/gtkcssprovider.h"
#include "gtk/gtkshortcut.h"
#include "pango/pango-layout.h"
#include "visual_mode.h"

void load_css(const char *filepath) {
    GtkCssProvider *provider = gtk_css_provider_new();
    GError *error = NULL;

    gtk_css_provider_load_from_path(provider, filepath);

    gtk_style_context_add_provider_for_display(gdk_display_get_default(),
                                               GTK_STYLE_PROVIDER(provider),
                                               GTK_STYLE_PROVIDER_PRIORITY_USER);

    g_object_unref(provider);
}

char *start_page_uri() {
    char *path = realpath("./src/ui/start.html", NULL);
    return g_filename_to_uri(path, NULL, NULL);
}

void *add_tab_widget(Tab *tab, char *title, int number) {
    GtkWidget *tab_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_add_css_class(tab_container, "tab-container");
    gtk_widget_add_css_class(tab_container, "active");
    GtkWidget *tab_number = gtk_label_new_with_mnemonic("[0]");
    gtk_widget_add_css_class(tab_number, "tab-number");
    GtkWidget *tab_title = gtk_label_new_with_mnemonic("mf751/vord: web browser vim");
    gtk_widget_set_size_request(tab_title, 212, -1);
    gtk_label_set_ellipsize(GTK_LABEL(tab_title), PANGO_ELLIPSIZE_END);
    gtk_widget_add_css_class(tab_title, "tab-title");
    gtk_label_set_max_width_chars(GTK_LABEL(tab_title), 1);
    gtk_box_append(GTK_BOX(tab_container), tab_number);
    gtk_box_append(GTK_BOX(tab_container), tab_title);

    GtkWidget *web_view = webkit_web_view_new();
    gtk_widget_set_vexpand(web_view, TRUE);
    gtk_widget_set_hexpand(web_view, TRUE);
    gtk_widget_set_visible(web_view, FALSE);

    tab->container = tab_container;
    tab->web_view = web_view;
    tab->number = tab_number;
    tab->title = tab_title;
}
