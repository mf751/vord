// Copyright (c) 2026 mf751. All Rights Reserved.

#include "gdk/gdkkeysyms.h"
#include "glib.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <webkit/webkit.h>

const char NORMAL_MODE = 0;
const char INSERT_MODE = 1;
const char ADRESS_MODE = 2;
const char VISUAL_MODE = 3;

static char mode = 0;

static GtkWidget *url_entry;
static GtkWidget *web_view;

static void scroll_webview(int dx, int dy) {
  char js[120];
  snprintf(js, sizeof(js),
           "window.scrollBy({left: %d, top: %d, behavior: 'smooth'});", dx, dy);
  webkit_web_view_evaluate_javascript(WEBKIT_WEB_VIEW(web_view), js, -1, NULL,
                                      NULL, NULL, NULL, NULL);
}

static gboolean on_key_press(GtkEventControllerKey *controller, guint keyval,
                             guint keycode, GdkModifierType state,
                             gpointer user_data) {
  GtkWidget *window =
      gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(controller));
  if (keyval == GDK_KEY_Escape) {
    mode = 0;
    return 1;
  }

  if (mode == NORMAL_MODE) {
    switch (keyval) {
    case GDK_KEY_i:
      mode = 1;
    case GDK_KEY_h:
      scroll_webview(-120, 0);
      return TRUE;

    case GDK_KEY_l:
      scroll_webview(120, 0);
      return TRUE;

    case GDK_KEY_k:
      scroll_webview(0, -120);
      return TRUE;

    case GDK_KEY_j:
      scroll_webview(0, 120);
      return TRUE;
    }
  }
  return 0;
}

static void on_load_changed(WebKitWebView *wv, WebKitLoadEvent load_event,
                            gpointer user_data) {
  if (load_event == WEBKIT_LOAD_FINISHED) {
    const char *uri = webkit_web_view_get_uri(wv);
    if (uri)
      gtk_editable_set_text(GTK_EDITABLE(url_entry), uri);
  }
}

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

static void go_back() {
  if (webkit_web_view_can_go_back(WEBKIT_WEB_VIEW(web_view)))
    webkit_web_view_go_back(WEBKIT_WEB_VIEW(web_view));
}
static void go_forward() {
  if (webkit_web_view_can_go_forward(WEBKIT_WEB_VIEW(web_view)))
    webkit_web_view_go_forward(WEBKIT_WEB_VIEW(web_view));
}
static void reload() { webkit_web_view_reload(WEBKIT_WEB_VIEW(web_view)); }

static void activate(GtkApplication *app, gpointer user_data) {
  GtkWidget *window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Vordian");
  gtk_window_set_default_size(GTK_WINDOW(window), 1920, 1080);

  GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

  GtkWidget *top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_widget_set_margin_end(top_bar, 8);
  gtk_widget_set_margin_start(top_bar, 8);
  gtk_widget_set_margin_top(top_bar, 8);
  gtk_widget_set_margin_bottom(top_bar, 8);

  GtkWidget *back_btn = gtk_button_new_with_label("←");
  g_signal_connect(back_btn, "clicked", G_CALLBACK(go_back), NULL);

  GtkWidget *forward_btn = gtk_button_new_with_label("→");
  g_signal_connect(forward_btn, "clicked", G_CALLBACK(go_forward), NULL);

  GtkWidget *reload_btn = gtk_button_new_with_label("↻");
  g_signal_connect(reload_btn, "clicked", G_CALLBACK(reload), NULL);

  url_entry = gtk_entry_new();
  gtk_widget_set_hexpand(url_entry, TRUE);
  g_signal_connect(url_entry, "activate", G_CALLBACK(on_url_activate), NULL);

  gtk_box_append(GTK_BOX(top_bar), back_btn);
  gtk_box_append(GTK_BOX(top_bar), forward_btn);
  gtk_box_append(GTK_BOX(top_bar), reload_btn);
  gtk_box_append(GTK_BOX(top_bar), url_entry);

  web_view = webkit_web_view_new();
  gtk_widget_set_vexpand(web_view, TRUE);
  gtk_widget_set_hexpand(web_view, TRUE);
  g_signal_connect(web_view, "load-changed", G_CALLBACK(on_load_changed), NULL);

  gtk_box_append(GTK_BOX(main_box), top_bar);
  gtk_box_append(GTK_BOX(main_box), web_view);

  gtk_window_set_child(GTK_WINDOW(window), main_box);
  gtk_window_present(GTK_WINDOW(window));

  GtkEventController *key_controller = gtk_event_controller_key_new();
  g_signal_connect(key_controller, "key-pressed", G_CALLBACK(on_key_press),
                   NULL);
  gtk_widget_add_controller(window, key_controller);

  webkit_web_view_load_uri(WEBKIT_WEB_VIEW(web_view), "https://www.google.com");

  gtk_widget_queue_draw(window);
}

int main(int argc, char **argv) {
  GtkApplication *app = gtk_application_new("com.github.mf751.vordian",
                                            G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}
