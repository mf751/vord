// Copyright(c) 2026 MF751.All Rights Reserved.

#include "style.h"

#include "gtk/gtk.h"
#include "gtk/gtkcssprovider.h"

void load_css(const char *filepath) {
  GtkCssProvider *provider = gtk_css_provider_new();
  GError *error = NULL;

  gtk_css_provider_load_from_path(provider, filepath);

  gtk_style_context_add_provider_for_display(gdk_display_get_default(),
                                             GTK_STYLE_PROVIDER(provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_USER);

  g_object_unref(provider);
}
