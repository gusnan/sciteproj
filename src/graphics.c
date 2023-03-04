/**
 * graphics.c - graphics code for SciteProj
 *
 *  Copyright 2009-2023 Andreas Rönnquist
 *
 * This file is part of SciteProj.
 *
 * SciteProj is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SciteProj is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SciteProj.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <gtk/gtk.h>

#include "string_utils.h"
#include "graphics.h"

#include "about.h"

#include "icons/icons_resources.h"

#include "icon.h"

#include "prefs.h"



GdkPixbuf *header_file_pixbuf = NULL;
GdkPixbuf *cpp_file_pixbuf = NULL;
GdkPixbuf *txt_file_pixbuf = NULL;
GdkPixbuf *java_file_pixbuf = NULL;
GdkPixbuf *lua_file_pixbuf = NULL;

GdkPixbuf *directory_closed_pixbuf = NULL;
GdkPixbuf *directory_open_pixbuf = NULL;

GdkPixbuf *program_icon_pixbuf = NULL;


GdkCursor *standard_cursor = NULL;
GdkCursor *busy_cursor = NULL;

#define APP_SCITEPROJ_ERROR g_quark_from_static_string("APP_GRAPHICS_ERROR")


/**
 *	Loads all graphics required by the program
 * @return TRUE on success, FALSE on failure
 * @param err returns any errors
 * @param widget - we need a widget for the gtk_widget_render_icon function
 *
 */
gboolean load_graphics(GtkWidget *widget, GError **err)
{
   GtkIconTheme *icon_theme;

   icon_theme = gtk_icon_theme_get_default();

   gchar *program_icon_resource;
   program_icon_resource = g_build_filename(SP_RESOURCE_PATH_ICONS,
                                            PIXBUF_INLINE_PROGRAM_ICON
                                            ".png", NULL);

   program_icon_pixbuf = gdk_pixbuf_new_from_resource(program_icon_resource, NULL);
   g_free(program_icon_resource);

   if (prefs.use_stock_folder_icon) {
      printf("Use stock folder icon!\n");
      // use GTK_STOCK_DIRECTORY
      directory_closed_pixbuf = gtk_icon_theme_load_icon(icon_theme, "folder", 14, 0, NULL);
      directory_open_pixbuf = gtk_icon_theme_load_icon(icon_theme, "folder", 14, 0, NULL);

   } else {

      gchar *resource_path_directory_open;

      resource_path_directory_open = g_build_filename(SP_RESOURCE_PATH_ICONS,
                                                      PIXBUF_INLINE_DIRECTORY_OPEN
                                                      ".png", NULL);

      directory_open_pixbuf = gdk_pixbuf_new_from_resource(resource_path_directory_open, NULL);
      g_free(resource_path_directory_open);

      gchar *resource_path_directory_closed;

      resource_path_directory_closed = g_build_filename(SP_RESOURCE_PATH_ICONS,
                                                        PIXBUF_INLINE_DIRECTORY_CLOSED
                                                        ".png", NULL);

      directory_closed_pixbuf = gdk_pixbuf_new_from_resource(resource_path_directory_closed, NULL);
      g_free(resource_path_directory_closed);
   }

   GdkDisplay *default_display;

   default_display = gdk_display_get_default();

   standard_cursor = gdk_cursor_new_for_display(default_display, GDK_X_CURSOR);
   busy_cursor = gdk_cursor_new_for_display(default_display, GDK_WATCH);

   return TRUE;
}


/**
 *
 */
void unload_graphics()
{
   if (program_icon_pixbuf != NULL) g_object_unref(program_icon_pixbuf);

   if (directory_closed_pixbuf != NULL) g_object_unref(directory_closed_pixbuf);
   if (directory_open_pixbuf != NULL) g_object_unref(directory_open_pixbuf);
}
