#include "window.h"
#include "../ui/ui.h"
#include "../ui/visual_mode.h"
#include "app.h"
#include "command_line.h"
#include "glib.h"
#include "glibconfig.h"
#include "gtk/gtk.h"
#include "gtk/gtkshortcut.h"
#include "keybindings.h"
#include "tabs.h"
#include <stdio.h>
#include <webkit/webkit.h>

static GtkWidget *main_box;
static GtkWidget *mode_indicator;
static GtkWidget *url_entry;
static GtkWidget *tab_bar;
static GtkWidget *tab_stack;

#include <gtk/gtk.h>
#include <stdio.h>

void add_tab(Tab *tab) {
    gtk_box_append(GTK_BOX(tab_bar), tab->container);
    gtk_stack_add_child(GTK_STACK(tab_stack), tab->web_view);
}

void remove_tab(Tab *tab) {
    gtk_box_remove(GTK_BOX(tab_bar), tab->container);
    gtk_stack_remove(GTK_STACK(tab_stack), tab->web_view);
}

// static void go_back() {
//     if (webkit_web_view_can_go_back(WEBKIT_WEB_VIEW(web_view)))
//         webkit_web_view_go_back(WEBKIT_WEB_VIEW(web_view));
// }
// static void go_forward() {
//     if (webkit_web_view_can_go_forward(WEBKIT_WEB_VIEW(web_view)))
//         webkit_web_view_go_forward(WEBKIT_WEB_VIEW(web_view));
// }
// static void reload() { webkit_web_view_reload(WEBKIT_WEB_VIEW(web_view)); }
//
// static void on_url_activate(GtkEntry *entry, gpointer user_data) {
//     const char *text = gtk_editable_get_text(GTK_EDITABLE(entry));
//     if (g_str_has_prefix(text, "http://") || g_str_has_prefix(text, "https://")) {
//         webkit_web_view_load_uri(WEBKIT_WEB_VIEW(web_view), text);
//     } else {
//         char *url = g_strdup_printf("https://www.google.com/search?q=%s", text);
//         webkit_web_view_load_uri(WEBKIT_WEB_VIEW(web_view), url);
//         g_free(url);
//     }
// }

void on_load_changed(WebKitWebView *wv, WebKitLoadEvent load_event,
                     gpointer user_data) {
    if (load_event == WEBKIT_LOAD_FINISHED) {
        const char *uri = webkit_web_view_get_uri(wv);
        if (uri)
            gtk_editable_set_text(GTK_EDITABLE(url_entry), uri);
    }
}

void scroll_webview(App *app, int dx, int dy) {
    char js[120];
    snprintf(js, sizeof(js), "window.scrollBy({left: %d, top: %d});", dx, dy);
    webkit_web_view_evaluate_javascript(WEBKIT_WEB_VIEW(app->current_tab->web_view), js, -1, NULL,
                                        NULL, NULL, NULL, NULL);
}

void update_mode_indicator(char *text) {
    gtk_label_set_text(GTK_LABEL(mode_indicator), text);
}

void activate(GtkApplication *gtk_app, gpointer user_data) {
    App *app = new_app(gtk_app);

    GtkWidget *window = gtk_application_window_new(gtk_app);
    gtk_window_set_title(GTK_WINDOW(window), "Vord");
    gtk_window_set_default_size(GTK_WINDOW(window), 1920, 1080);

    main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    tab_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_add_css_class(tab_bar, "tab-bar");

    GtkWidget *scrollable_container = gtk_scrolled_window_new();
    gtk_widget_set_hexpand(scrollable_container, TRUE);
    gtk_widget_set_halign(scrollable_container, GTK_ALIGN_FILL);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrollable_container), tab_bar);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollable_container), GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);

    GtkWidget *bar_separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_add_css_class(bar_separator, "bar-separator");

    GtkWidget *top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_add_css_class(url_entry, "top-bar-container");

    url_entry = gtk_entry_new();
    gtk_widget_set_hexpand(url_entry, true);
    gtk_widget_add_css_class(url_entry, "url-entry");
    gtk_entry_set_overwrite_mode(GTK_ENTRY(url_entry), TRUE);
    gtk_widget_set_valign(url_entry, GTK_ALIGN_START);

    mode_indicator = gtk_label_new_with_mnemonic(get_mode_name(app->current_mode));
    gtk_widget_add_css_class(mode_indicator, "mode-label");
    gtk_label_set_xalign(GTK_LABEL(mode_indicator), 0.0);
    gtk_widget_set_size_request(mode_indicator, 84, -1);
    gtk_widget_set_halign(mode_indicator, GTK_ALIGN_START);
    gtk_widget_set_hexpand(mode_indicator, FALSE);

    gtk_box_append(GTK_BOX(top_bar), mode_indicator);
    gtk_box_append(GTK_BOX(top_bar), url_entry);
    gtk_box_set_spacing(GTK_BOX(top_bar), 0);

    gtk_box_append(GTK_BOX(main_box), scrollable_container);
    gtk_box_append(GTK_BOX(main_box), top_bar);
    gtk_box_append(GTK_BOX(main_box), bar_separator);

    GtkWidget *overlay = gtk_overlay_new();
    add_cmd_to_overlay(overlay);
    tab_stack = gtk_stack_new();
    gtk_widget_set_hexpand(tab_stack, TRUE);
    gtk_widget_set_vexpand(tab_stack, TRUE);
    gtk_overlay_set_child(GTK_OVERLAY(overlay), tab_stack);

    gtk_box_append(GTK_BOX(main_box), overlay);

    gtk_window_set_child(GTK_WINDOW(window), main_box);
    gtk_window_present(GTK_WINDOW(window));

    load_css("./src/ui/styles.css");

    Tab *tab = new_tab(app, start_page_uri());
    switch_to_tab(app, tab);

    GtkEventController *key_controller = gtk_event_controller_key_new();
    g_signal_connect(key_controller, "key-pressed", G_CALLBACK(on_key_press),
                     app);
    gtk_widget_add_controller(window, key_controller);

    gtk_widget_queue_draw(window);
}
