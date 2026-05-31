// Copyright (c) 2026 mf751. All Rights Reserved.

#include "gdk/gdk.h"
#include "gdk/gdkkeysyms.h"
#include "glib-object.h"
#include "glib.h"
#include "gtk/gtkcssprovider.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <webkit/webkit.h>

typedef enum {
  NORMAL_MODE = 0,
  INSERT_MODE,
  ADDRESS_MODE,
  VISUAL_MODE,
} EditorMode;

typedef struct {
  int x;
  int y;
} Coordinates;

static EditorMode current_mode = NORMAL_MODE;
static GtkWidget *url_entry;
static GtkWidget *web_view;
static GtkWidget *mode_indicator;
static int VISUAL_MODE_CURSOR_STEP = 7;
static int VISUAL_MODE_CURSOR_WIDTH = 8;
static int VISUAL_MODE_CURSOR_HEIGHT = 18;
static char *VISUAL_MODE_CURSOR_BACKGROUND_COLOR = "rgba(38, 107, 255, .3)";
static Coordinates visual_mode_cursor_coordinates = {.x = 0, .y = 0};

static char *get_mode_name() {
  switch (current_mode) {
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
static void start_visual_mode() {
  char js[1028];
  snprintf(js, sizeof(js),
           "if (!window.vordVisualOverlay) {"
           "window.vordVisualOverlay = document.createElement('div');"
           "window.vordVisualOverlay.style.position = 'absolute';"
           "window.vordVisualOverlay.style.top = 'calc(50vh)';"
           "window.vordVisualOverlay.style.left = 'calc(50vw)';"
           "window.vordVisualOverlay.style.width = '%dpx';"
           "window.vordVisualOverlay.style.height = '%dpx';"
           "window.vordVisualOverlay.style.pointerEvents = 'none';"
           "window.vordVisualOverlay.style.zIndex = '999999';"
           "window.vordVisualOverlay.style.background = '%s';"
           "document.body.appendChild(window.vordVisualOverlay);"
           "window.vordVisualOverlay"
           "}",
           VISUAL_MODE_CURSOR_WIDTH, VISUAL_MODE_CURSOR_HEIGHT,
           VISUAL_MODE_CURSOR_BACKGROUND_COLOR);

  webkit_web_view_evaluate_javascript(WEBKIT_WEB_VIEW(web_view), js, -1, NULL,
                                      NULL, NULL, NULL, NULL);
}

static void stop_visual_mode() {
  const char *js = "if (window.vordVisualOverlay){"
                   "window.vordVisualOverlay.remove();"
                   "window.vordVisualOverlay = null;}";
  webkit_web_view_evaluate_javascript(WEBKIT_WEB_VIEW(web_view), js, -1, NULL,
                                      NULL, NULL, NULL, NULL);
  visual_mode_cursor_coordinates.x = 0;
  visual_mode_cursor_coordinates.y = 0;
}

static void update_visual_mode_cursor() {
  char js[512];
  snprintf(js, sizeof(js),
           "if (window.vordVisualOverlay) {"
           "  window.vordVisualOverlay.style.top = 'calc(50vh + %dpx)';"
           "  window.vordVisualOverlay.style.left = 'calc(50vw + %dpx)';"
           "}",
           visual_mode_cursor_coordinates.y, visual_mode_cursor_coordinates.x);
  webkit_web_view_evaluate_javascript(WEBKIT_WEB_VIEW(web_view), js, -1, NULL,
                                      NULL, NULL, NULL, NULL);
}

static void set_mode(EditorMode new_mode) {
  if (current_mode == VISUAL_MODE && new_mode == NORMAL_MODE)
    stop_visual_mode();
  current_mode = new_mode;
  gtk_label_set_text(GTK_LABEL(mode_indicator), get_mode_name());
  if (new_mode == VISUAL_MODE)
    start_visual_mode();
}

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
    set_mode(NORMAL_MODE);
    return 1;
  }

  if (current_mode == NORMAL_MODE) {
    switch (keyval) {
    case GDK_KEY_i:
      set_mode(INSERT_MODE);
      return TRUE;
    case GDK_KEY_v:
      set_mode(VISUAL_MODE);
      return TRUE;
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
  } else if (current_mode == VISUAL_MODE) {
    switch (keyval) {
    case GDK_KEY_h:
      visual_mode_cursor_coordinates.x -= VISUAL_MODE_CURSOR_STEP;
      update_visual_mode_cursor();
      return 1;
    case GDK_KEY_l:
      visual_mode_cursor_coordinates.x += VISUAL_MODE_CURSOR_STEP;
      update_visual_mode_cursor();
      return 1;
    case GDK_KEY_j:
      visual_mode_cursor_coordinates.y += VISUAL_MODE_CURSOR_STEP;
      update_visual_mode_cursor();
      return 1;
    case GDK_KEY_k:
      visual_mode_cursor_coordinates.y -= VISUAL_MODE_CURSOR_STEP;
      update_visual_mode_cursor();
      return 1;
    case GDK_KEY_v:
      set_mode(NORMAL_MODE);
      stop_visual_mode();
      return 1;
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

  mode_indicator = gtk_label_new_with_mnemonic(get_mode_name());
  gtk_widget_set_size_request(mode_indicator, 84, -1);
  gtk_widget_add_css_class(mode_indicator, "mode-label");
  GtkCssProvider *provider = gtk_css_provider_new();
  const char *css = ".mode-label {"
                    "  color: #ddd;"
                    "  font-size: 12px;"
                    "  font-weight: 800;"
                    "  font-family: sans-serif;"
                    "  padding: 4px 10px;"
                    "  border-radius: 0px;"
                    "  text-transform: uppercase;"
                    "}";
  gtk_css_provider_load_from_string(provider, css);
  gtk_style_context_add_provider_for_display(gdk_display_get_default(),
                                             GTK_STYLE_PROVIDER(provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_USER);
  g_object_unref(provider);

  gtk_box_append(GTK_BOX(top_bar), back_btn);
  gtk_box_append(GTK_BOX(top_bar), forward_btn);
  gtk_box_append(GTK_BOX(top_bar), reload_btn);
  gtk_box_append(GTK_BOX(top_bar), url_entry);
  gtk_box_append(GTK_BOX(top_bar), mode_indicator);

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
