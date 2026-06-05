#include "window.h"
#include "../ui/style.h"
#include "../ui/visual_mode.h"
#include "command_line.h"
#include "gtk/gtk.h"
#include "keybindings.h"
#include "mode.h"
#include <stdio.h>
#include <webkit/webkit.h>

static GtkWidget *web_view;
static GtkWidget *mode_indicator;
static GtkWidget *url_entry;

static void go_back() {
  if (webkit_web_view_can_go_back(WEBKIT_WEB_VIEW(web_view)))
    webkit_web_view_go_back(WEBKIT_WEB_VIEW(web_view));
}
static void go_forward() {
  if (webkit_web_view_can_go_forward(WEBKIT_WEB_VIEW(web_view)))
    webkit_web_view_go_forward(WEBKIT_WEB_VIEW(web_view));
}
static void reload() { webkit_web_view_reload(WEBKIT_WEB_VIEW(web_view)); }

static void on_url_activate(GtkEntry *entry, gpointer user_data) {
  const char *text = gtk_editable_get_text(GTK_EDITABLE(entry));
  if (g_str_has_prefix(text, "http://") || g_str_has_prefix(text, "https://")) {
    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(web_view), text);
  } else {
    char *url = g_strdup_printf("https://www.google.com/search?q=%s", text);
    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(web_view), url);
    g_free(url);
  }
}

static void on_load_changed(WebKitWebView *wv, WebKitLoadEvent load_event,
                            gpointer user_data) {
  if (load_event == WEBKIT_LOAD_FINISHED) {
    const char *uri = webkit_web_view_get_uri(wv);
    if (uri)
      gtk_editable_set_text(GTK_EDITABLE(url_entry), uri);
  }
}

void scroll_webview(int dx, int dy) {
  char js[120];
  snprintf(js, sizeof(js), "window.scrollBy({left: %d, top: %d});", dx, dy);
  webkit_web_view_evaluate_javascript(WEBKIT_WEB_VIEW(web_view), js, -1, NULL,
                                      NULL, NULL, NULL, NULL);
}

GtkWidget *get_web_view() { return web_view; }

void update_mode_indicator() {
  gtk_label_set_text(GTK_LABEL(mode_indicator), get_mode_name());
}

void activate(GtkApplication *app, gpointer user_data) {
  GtkWidget *window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Vord");
  gtk_window_set_default_size(GTK_WINDOW(window), 1920, 1080);

  GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

  GtkWidget *top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

  url_entry = gtk_entry_new();
  gtk_widget_set_hexpand(url_entry, true);
  gtk_widget_add_css_class(url_entry, "url-entry");
  gtk_entry_set_overwrite_mode(GTK_ENTRY(url_entry), TRUE);

  mode_indicator = gtk_label_new_with_mnemonic(get_mode_name());
  gtk_widget_add_css_class(mode_indicator, "mode-label");
  gtk_widget_set_size_request(mode_indicator, 137, -1);
  gtk_label_set_xalign(GTK_LABEL(mode_indicator), 0.0);
  gtk_widget_set_size_request(mode_indicator, 84, -1);
  gtk_widget_set_halign(mode_indicator, GTK_ALIGN_START);
  gtk_widget_set_hexpand(mode_indicator, FALSE);

  GtkWidget *cmd_overlay = new_cmd_overlay();

  gtk_box_append(GTK_BOX(top_bar), mode_indicator);
  gtk_box_append(GTK_BOX(top_bar), url_entry);
  gtk_box_set_spacing(GTK_BOX(top_bar), 0);

  web_view = webkit_web_view_new();
  gtk_widget_set_vexpand(web_view, TRUE);
  gtk_widget_set_hexpand(web_view, TRUE);
  g_signal_connect(web_view, "load-changed", G_CALLBACK(on_load_changed), NULL);

  gtk_box_append(GTK_BOX(main_box), top_bar);
  gtk_box_append(GTK_BOX(main_box), web_view);
  gtk_box_append(GTK_BOX(main_box), cmd_overlay);

  gtk_window_set_child(GTK_WINDOW(window), main_box);
  gtk_window_present(GTK_WINDOW(window));

  load_css("./src/ui/styles.css");

  GtkEventController *key_controller = gtk_event_controller_key_new();
  g_signal_connect(key_controller, "key-pressed", G_CALLBACK(on_key_press),
                   NULL);
  gtk_widget_add_controller(window, key_controller);

  char *path = realpath("./src/ui/start.html", NULL);
  char *uri = g_filename_to_uri(path, NULL, NULL);
  webkit_web_view_load_uri(WEBKIT_WEB_VIEW(web_view), uri);

  gtk_widget_queue_draw(window);
}
