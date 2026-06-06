#include "visual_mode.h"
#include "../utils/configs.h"
#include "gtk/gtkshortcut.h"
#include <stdio.h>
#include <webkit/webkit.h>

typedef struct {
    int x;
    int y;
} Coordinates;

static Coordinates visual_mode_cursor = {.x = 0, .y = 0};
static Coordinates visual_mode_anchor = {.x = 0, .y = 0};
static bool visual_mode_anchor_set = false;

bool get_visual_mode_anchored() { return visual_mode_anchor_set; }
void set_visual_mode_anchored(bool state) { visual_mode_anchor_set = state; }

void visual_mode_anchor_cursor() { visual_mode_anchor = visual_mode_cursor; }

void change_visual_mode_cursor(int dx, int dy) {
    visual_mode_cursor.x += dx;
    visual_mode_cursor.y += dy;
}

void change_visual_mode_anchor(int dx, int dy) {
    if (dx > 0)
        visual_mode_anchor.x += dx;
    if (dy > 0)
        visual_mode_anchor.y += dy;
}

void start_visual_mode(GtkWidget *web_view) {
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

void stop_visual_mode(GtkWidget *web_view) {
    const char *js = "if (window.vordVisualOverlay){"
                     "window.vordVisualOverlay.remove();"
                     "window.vordVisualOverlay = null;}"
                     "window.getSelection().removeAllRanges()";
    webkit_web_view_evaluate_javascript(WEBKIT_WEB_VIEW(web_view), js, -1, NULL,
                                        NULL, NULL, NULL, NULL);
    visual_mode_cursor.x = 0;
    visual_mode_cursor.y = 0;
    visual_mode_anchor_set = false;
}

void update_visual_mode_cursor(GtkWidget *web_view) {
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

void copy_to_clipboard(char *text) {
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

void yank_visual_selection(GtkWidget *web_view) {
    const char *js = "(function() {"
                     "const sel = window.getSelection();"
                     "if (!sel.rangeCount) return;"
                     "const range = sel.getRangeAt(0);"
                     "const rects = range.getClientRects();"
                     "for (const rect of rects) {"
                     "const div = document.createElement('div');"
                     "div.style.position = 'fixed';"
                     "div.style.left = rect.left + 'px';"
                     "div.style.top = rect.top + 'px';"
                     "div.style.width = rect.width + 'px';"
                     "div.style.height = rect.height + 'px';"
                     "div.style.background = 'rgba(255,255,0,0.6)';"
                     "div.style.pointerEvents = 'none';"
                     "div.style.zIndex = 999999;"
                     "div.style.transition = 'opacity 0.5s ease';"
                     "document.body.appendChild(div);"
                     "setTimeout(() => {"
                     "div.style.opacity = '0';"
                     "setTimeout(() => div.remove(), 500);"
                     "}, 300);"
                     "}"
                     "return sel.toString()"
                     "})();";

    webkit_web_view_evaluate_javascript(WEBKIT_WEB_VIEW(web_view), js, -1, NULL,
                                        NULL, NULL, on_js_evaluated, NULL);
}
