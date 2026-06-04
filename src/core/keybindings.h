#ifndef KEYBINDINGS_H
#define KEYBINDINGS_H

#include "glib.h"
#include "gtk/gtk.h"

gboolean on_key_press(GtkEventControllerKey *controller, guint keyval,
                      guint keycode, GdkModifierType state, gpointer user_data);
#endif // !KEYBINDINGS_H
