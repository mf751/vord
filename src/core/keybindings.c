
#include "../ui/visual_mode.h"
#include "../utils/configs.h"
#include "app.h"
#include "command_line.h"
#include "glib.h"
#include "glibconfig.h"
#include "gtk/gtk.h"
#include "gtk/gtkshortcut.h"
#include "tabs.h"
#include "window.h"

gboolean on_key_press(GtkEventControllerKey *controller, guint keyval,
                      guint keycode, GdkModifierType state,
                      gpointer user_data) {
    App *app = (App *)user_data;
    bool visual_mode_anchored = get_visual_mode_anchored();
    bool command_line_showen = get_command_line_showen();
    GtkWidget *window =
        gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(controller));
    if (keyval == GDK_KEY_Escape) {
        set_mode(app, NORMAL_MODE);
        if (app->current_mode == VISUAL_MODE)
            stop_visual_mode(app->current_tab->web_view);
    }

    gboolean ctrl_pressed = (state & GDK_CONTROL_MASK) != 0;
    gboolean shift_pressed = (state & GDK_SHIFT_MASK) != 0;

    if (app->current_mode == NORMAL_MODE) {
        switch (keyval) {
        case GDK_KEY_t:
            if (ctrl_pressed) {
                switch_to_next_tab(app);
                break;
            }
            Tab *tab = new_tab(app, "https://www.google.com");
            switch_to_tab(app, tab);
            break;
        case GDK_KEY_w:
            if (ctrl_pressed) {
                close_current_tab(app);
                break;
            }
        case GDK_KEY_i:
            set_mode(app, INSERT_MODE);
            break;
        case GDK_KEY_v:
            set_mode(app, VISUAL_MODE);
            break;
        case GDK_KEY_h:
            scroll_webview(app, -NORMAL_MODE_SCROLL_DISTANCE, 0);
            break;
        case GDK_KEY_l:
            scroll_webview(app, NORMAL_MODE_SCROLL_DISTANCE, 0);
            break;
        case GDK_KEY_k:
            scroll_webview(app, 0, -NORMAL_MODE_SCROLL_DISTANCE);
            break;
        case GDK_KEY_j:
            scroll_webview(app, 0, NORMAL_MODE_SCROLL_DISTANCE);
            break;
        case GDK_KEY_colon:
            show_command_line();
            break;
        }
    } else if (app->current_mode == VISUAL_MODE) {
        switch (keyval) {
        case GDK_KEY_h:
            if (ctrl_pressed) {
                scroll_webview(app, -NORMAL_MODE_SCROLL_DISTANCE, 0);
                break;
            }
            change_visual_mode_cursor(-VISUAL_MODE_CURSOR_STEP_X, 0);
            break;
        case GDK_KEY_l:
            if (ctrl_pressed) {
                scroll_webview(app, NORMAL_MODE_SCROLL_DISTANCE, 0);
                break;
            }
            change_visual_mode_cursor(VISUAL_MODE_CURSOR_STEP_X, 0);
            break;
        case GDK_KEY_k:
            if (ctrl_pressed) {
                scroll_webview(app, 0, -NORMAL_MODE_SCROLL_DISTANCE);
                break;
            }
            change_visual_mode_cursor(0, -VISUAL_MODE_CURSOR_STEP_Y);
            break;
        case GDK_KEY_j:
            if (ctrl_pressed) {
                scroll_webview(app, 0, NORMAL_MODE_SCROLL_DISTANCE);
                break;
            }
            change_visual_mode_cursor(0, VISUAL_MODE_CURSOR_STEP_Y);
            break;
        case GDK_KEY_v:
            if (visual_mode_anchored) {
                set_visual_mode_anchored(FALSE);
                break;
            }
            set_visual_mode_anchored(TRUE);
            visual_mode_anchor_cursor();
            break;
        case GDK_KEY_y:
            yank_visual_selection(app->current_tab->web_view);
            break;
        }

        update_visual_mode_cursor(app->current_tab->web_view);
    }
    return 0;
}
