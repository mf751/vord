// Copyright (c) 2026 mf751. All Rights Reserved.

#include "gdk/gdk.h"
#include "gdk/gdkkeysyms.h"
#include "glib-object.h"
#include "glib.h"
#include "glibconfig.h"
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
static int VISUAL_MODE_CURSOR_STEP_X = 5;
static int VISUAL_MODE_CURSOR_STEP_Y = 10;
static int VISUAL_MODE_CURSOR_WIDTH = 8;
static int VISUAL_MODE_CURSOR_HEIGHT = 18;
static char *VISUAL_MODE_CURSOR_BACKGROUND_COLOR = "rgba(38, 107, 255, .3)";
static Coordinates visual_mode_cursor = {.x = 0, .y = 0};
static Coordinates visual_mode_anchor = {.x = 0, .y = 0};
static bool visual_mode_anchor_set = false;
static int NORMAL_MODE_SCROLL_DISTANCE = 32;

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
           "window.vordVisualOverlay.style.position = 'fixed';"
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
                   "window.vordVisualOverlay = null;}"
                   "window.getSelection().removeAllRanges()";
  webkit_web_view_evaluate_javascript(WEBKIT_WEB_VIEW(web_view), js, -1, NULL,
                                      NULL, NULL, NULL, NULL);
  visual_mode_cursor.x = 0;
  visual_mode_cursor.y = 0;
}

static void update_visual_mode_cursor() {
  char js[1028];
  snprintf(
      js, sizeof(js),
      "(function() {"
      "if (!window.vordVisualOverlay) return; "
      "if (%d) {"
      "const pt1X = 0.5 * window.innerWidth + %d;"
      "const pt1y = 0.5 * window.innerHeight + %d;"
      "const pt2X = 0.5 * window.innerWidth + %d;"
      "const pt2y = 0.5 * window.innerHeight + %d;"
      "const sel = window.getSelection();"
      "sel.removeAllRanges();"
      "const start = document.caretRangeFromPoint(pt1X, pt1y);"
      "const end = document.caretRangeFromPoint(pt2X, pt2y);"
      "if (start && end){"
      "const range = document.createRange();"
      // check wihch comes first in the page
      "if (start.startContainer.compareDocumentPosition(end.startContainer) & "
      "Node.DOCUMENT_POSITION_FOLLOWING || (start.startContainer === "
      "end.startContainer && start.startOffset <= end.startOffset)){"
      "range.setStart(start.startContainer, start.startOffset);"
      "range.setEnd(end.startContainer, end.startOffset);"
      "} else {"
      "range.setStart(end.startContainer, end.startOffset);"
      "range.setEnd(start.startContainer, start.startOffset);"
      "}"

      "sel.addRange(range);}"
      "};"
      "window.vordVisualOverlay.style.left = 'calc(50vw + %dpx)';"
      "window.vordVisualOverlay.style.top = 'calc(50vh + %dpx)';"
      "})();",
      visual_mode_anchor_set, visual_mode_anchor.x, visual_mode_anchor.y,
      visual_mode_cursor.x, visual_mode_cursor.y, visual_mode_cursor.x,
      visual_mode_cursor.y);
  webkit_web_view_evaluate_javascript(WEBKIT_WEB_VIEW(web_view), js, -1, NULL,
                                      NULL, NULL, NULL, NULL);
}

static void copy_to_clipboard(char *text) {
  GdkDisplay *display = gdk_display_get_default();
  GdkClipboard *clipboard = gdk_display_get_clipboard(display);
  gdk_clipboard_set_text(clipboard, text);
}

static void on_js_evaluated(GObject *source_object, GAsyncResult *res,
                            gpointer user_data) {
  WebKitWebView *web_view = WEBKIT_WEB_VIEW(source_object);
  GError *error = NULL;

  JSCValue *js_value =
      webkit_web_view_evaluate_javascript_finish(web_view, res, &error);

  if (error) {
    g_warning("JS Evaluation Error: %s\n", error->message);
    g_error_free(error);
    return;
  }

  if (jsc_value_is_string(js_value)) {
    gchar *str_value = jsc_value_to_string(js_value);
    copy_to_clipboard(str_value);
    g_free(str_value);
  }

  g_object_unref(js_value);
}

static void yank_visual_selection() {
  const char *js = "(function() {"
                   "  const sel = window.getSelection();"
                   "  if (!sel.rangeCount) return;"
                   "  const range = sel.getRangeAt(0);"
                   "  const rects = range.getClientRects();"
                   "  for (const rect of rects) {"
                   "    const div = document.createElement('div');"
                   "    div.style.position = 'fixed';"
                   "    div.style.left = rect.left + 'px';"
                   "    div.style.top = rect.top + 'px';"
                   "    div.style.width = rect.width + 'px';"
                   "    div.style.height = rect.height + 'px';"
                   "    div.style.background = 'rgba(255,255,0,0.6)';"
                   "    div.style.pointerEvents = 'none';"
                   "    div.style.zIndex = 999999;"
                   "    div.style.transition = 'opacity 0.5s ease';"
                   "    document.body.appendChild(div);"
                   "    setTimeout(() => {"
                   "      div.style.opacity = '0';"
                   "      setTimeout(() => div.remove(), 500);"
                   "    }, 300);"
                   "  }"
                   "return sel.toString()"
                   "})();";

  webkit_web_view_evaluate_javascript(WEBKIT_WEB_VIEW(web_view), js, -1, NULL,
                                      NULL, NULL, on_js_evaluated, NULL);
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
  snprintf(js, sizeof(js), "window.scrollBy({left: %d, top: %d});", dx, dy);
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
    if (current_mode == VISUAL_MODE)
      stop_visual_mode();
    return 1;
  }

  gboolean ctrl_pressed = (state & GDK_CONTROL_MASK) != 0;
  gboolean shift_pressed = (state & GDK_SHIFT_MASK) != 0;

  if (current_mode == NORMAL_MODE) {
    switch (keyval) {
    case GDK_KEY_i:
      set_mode(INSERT_MODE);
      return TRUE;
    case GDK_KEY_v:
      set_mode(VISUAL_MODE);
      return TRUE;
    case GDK_KEY_h:
      scroll_webview(-NORMAL_MODE_SCROLL_DISTANCE, 0);
      return TRUE;

    case GDK_KEY_l:
      scroll_webview(NORMAL_MODE_SCROLL_DISTANCE, 0);
      return TRUE;

    case GDK_KEY_k:
      scroll_webview(0, -NORMAL_MODE_SCROLL_DISTANCE);
      return TRUE;

    case GDK_KEY_j:
      scroll_webview(0, NORMAL_MODE_SCROLL_DISTANCE);
      return TRUE;
    }
  } else if (current_mode == VISUAL_MODE) {
    switch (keyval) {
    case GDK_KEY_h:
      if (ctrl_pressed) {
        scroll_webview(-NORMAL_MODE_SCROLL_DISTANCE, 0);
        update_visual_mode_cursor();
        return 1;
      }
      visual_mode_cursor.x -= VISUAL_MODE_CURSOR_STEP_X;
      update_visual_mode_cursor();
      return 1;
    case GDK_KEY_l:
      if (ctrl_pressed) {
        update_visual_mode_cursor();
        scroll_webview(NORMAL_MODE_SCROLL_DISTANCE, 0);
        return 1;
      }
      visual_mode_cursor.x += VISUAL_MODE_CURSOR_STEP_X;
      update_visual_mode_cursor();
      return 1;
    case GDK_KEY_k:
      if (ctrl_pressed) {
        scroll_webview(0, -NORMAL_MODE_SCROLL_DISTANCE);
        update_visual_mode_cursor();
        return 1;
      }
      visual_mode_cursor.y -= VISUAL_MODE_CURSOR_STEP_Y;
      update_visual_mode_cursor();
      return 1;
    case GDK_KEY_j:
      if (ctrl_pressed) {
        scroll_webview(0, NORMAL_MODE_SCROLL_DISTANCE);
        update_visual_mode_cursor();
        return 1;
      }
      visual_mode_cursor.y += VISUAL_MODE_CURSOR_STEP_Y;
      update_visual_mode_cursor();
      return 1;
    case GDK_KEY_v:
      if (visual_mode_anchor_set) {
        visual_mode_anchor_set = !visual_mode_anchor_set;
        update_visual_mode_cursor();
        return 1;
      }
      visual_mode_anchor_set = TRUE;
      visual_mode_anchor = visual_mode_cursor;
      update_visual_mode_cursor();
      return 1;
    case GDK_KEY_y:
      yank_visual_selection();
      update_visual_mode_cursor();
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
  gtk_window_set_title(GTK_WINDOW(window), "Vord");
  gtk_window_set_default_size(GTK_WINDOW(window), 1920, 1080);

  GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

  GtkWidget *top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

  url_entry = gtk_entry_new();
  gtk_widget_set_hexpand(url_entry, true);
  gtk_widget_add_css_class(url_entry, "url-entry");

  mode_indicator = gtk_label_new_with_mnemonic(get_mode_name());
  gtk_widget_set_size_request(mode_indicator, 84, 8);
  gtk_widget_add_css_class(mode_indicator, "mode-label");

  const char *css = ".mode-label {"
                    "color: #fff;"
                    "font-size: 14px;"
                    "font-family: 'Sans Serif';"
                    "border-radius: 0px;"
                    "padding: 0;"
                    "margin: 0;"
                    "text-transform: uppercase;"
                    "font-weight: 600;"
                    "background-color: red;"
                    "}"
                    ".url-entry,"
                    ".url-entry text,"
                    ".url-entry image {"
                    " border-radius: 0;"
                    "color: #fff;"
                    "border: none;"
                    "box-shadow: none;"
                    "outline-width: 0px;"
                    "background-image: none;"
                    "background-color: transparent;"
                    "}";
  GtkCssProvider *provider = gtk_css_provider_new();
  gtk_css_provider_load_from_string(provider, css);
  gtk_style_context_add_provider_for_display(gdk_display_get_default(),
                                             GTK_STYLE_PROVIDER(provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_USER);
  g_object_unref(provider);

  gtk_box_append(GTK_BOX(top_bar), mode_indicator);
  gtk_box_append(GTK_BOX(top_bar), url_entry);
  gtk_box_set_spacing(GTK_BOX(top_bar), 0);

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

  WebKitSettings *settings =
      webkit_web_view_get_settings(WEBKIT_WEB_VIEW(web_view));

  char *path = realpath("./assets/start.html", NULL);
  char *uri = g_filename_to_uri(path, NULL, NULL);
  webkit_web_view_load_uri(WEBKIT_WEB_VIEW(web_view), uri);

  gtk_widget_queue_draw(window);
}

int main(int argc, char **argv) {
  GtkApplication *app =
      gtk_application_new("com.github.mf751.vord", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}
