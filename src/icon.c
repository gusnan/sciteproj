/*
 * icon.c - Icon helper functions
 *
 * Copyright (C) 2006 - Jesse van den Kieboom <jesse@icecrew.nl>
 *           (C) 2012-2017 - Andreas Rönnquist
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
  This is a modified and stripped version of gedit-file-bookmarks-utils.c of
  the gedit project
 */

#include <gtk/gtk.h>

/**
 *
 */


/**
 *
 */
GdkPixbuf *
get_pixbuf_from_icon(GIcon *icon, GtkIconSize size)
{
	GdkPixbuf *result = NULL;
	GtkIconTheme *theme;
	GtkIconInfo *info;
	gint width;

	if (!icon)
		return NULL;

	theme = gtk_icon_theme_get_default();
	gtk_icon_size_lookup(size, &width, NULL);

	info = gtk_icon_theme_lookup_by_gicon(theme,
	                                    icon,
	                                    width,
	                                    GTK_ICON_LOOKUP_USE_BUILTIN);

	if (!info)
		return NULL;

	result = gtk_icon_info_load_icon(info, NULL);

	return result;
}


/**
 *
 */
GdkPixbuf *
get_pixbuf_from_file(GFile *file, GtkIconSize size)
{
	GIcon *icon;
	GFileInfo *info;

	GdkPixbuf *result = NULL;

	info = g_file_query_info(file,
	                       G_FILE_ATTRIBUTE_STANDARD_ICON,
	                       G_FILE_QUERY_INFO_NONE,
	                       NULL,
	                       NULL);

	if (!info)
		return NULL;

	icon = g_file_info_get_icon(info);

	if (icon != NULL) {
		result = get_pixbuf_from_icon(icon, size);
	}

	g_object_unref(info);

	return result;
}


/**
 *
 */
GdkPixbuf *
get_pixbuf_from_filename(gchar *filename, GtkIconSize size)
{
	GFile *tempfile = g_file_new_for_path(filename);

	GdkPixbuf *result = get_pixbuf_from_file(tempfile, size);

	return result;
}
