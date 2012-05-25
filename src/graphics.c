/**
 * graphics.c - graphics code for SciteProj
 *
 *  Copyright 2009-2012 Andreas Rönnquist
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

#include "../graphics/dir-close.xpm"
#include "../graphics/dir-open.xpm"
#include "../graphics/text-x-cpp.xpm"
#include "../graphics/text-x-h.xpm"
#include "../graphics/text-x-txt.xpm"
#include "../graphics/text-x-java.xpm"
#include "../graphics/text-x-lua.xpm"
#include "../graphics/sciteproj.xpm"



GdkPixbuf *header_file_pixbuf=NULL;
GdkPixbuf *cpp_file_pixbuf=NULL;
GdkPixbuf *txt_file_pixbuf=NULL;
GdkPixbuf *java_file_pixbuf=NULL;
GdkPixbuf *lua_file_pixbuf=NULL;

GdkPixbuf *directory_closed_pixbuf=NULL;
GdkPixbuf *directory_open_pixbuf=NULL;

GdkPixbuf *program_icon_pixbuf=NULL;


GdkCursor *standard_cursor=NULL;
GdkCursor *busy_cursor=NULL;

#define APP_SCITEPROJ_ERROR g_quark_from_static_string("APP_GRAPHICS_ERROR")


/**
 *	Loads all graphics required by the program
 * @return TRUE on success, FALSE on failure
 * @param err returns any errors
 */
gboolean load_graphics(GError **err)
{
	GdkPixbuf *temp=NULL;

	program_icon_pixbuf=gdk_pixbuf_new_from_xpm_data((const char **)sciteproj_xpm);

	directory_closed_pixbuf=gdk_pixbuf_new_from_xpm_data((const char**)dir_close_xpm);
	directory_open_pixbuf=gdk_pixbuf_new_from_xpm_data((const char **)dir_open_xpm);

	temp=gdk_pixbuf_new_from_xpm_data((const char **)text_x_cpp_xpm);
	cpp_file_pixbuf=gdk_pixbuf_scale_simple(temp,16,16,GDK_INTERP_HYPER);
	g_object_unref(temp);

	temp=gdk_pixbuf_new_from_xpm_data((const char **)text_x_h_xpm);
	header_file_pixbuf=gdk_pixbuf_scale_simple(temp,16,16,GDK_INTERP_HYPER);
	g_object_unref(temp);

	temp=gdk_pixbuf_new_from_xpm_data((const char **)text_x_txt_xpm);
	txt_file_pixbuf=gdk_pixbuf_scale_simple(temp,16,16,GDK_INTERP_HYPER);
	g_object_unref(temp);

	temp=gdk_pixbuf_new_from_xpm_data((const char **)text_x_java_xpm);
	java_file_pixbuf=gdk_pixbuf_scale_simple(temp,16,16,GDK_INTERP_HYPER);
	g_object_unref(temp);

	temp=gdk_pixbuf_new_from_xpm_data((const char **)text_x_lua_xpm);
	lua_file_pixbuf=gdk_pixbuf_scale_simple(temp,16,16,GDK_INTERP_HYPER);
	g_object_unref(temp);

	standard_cursor=gdk_cursor_new(GDK_X_CURSOR);
	busy_cursor=gdk_cursor_new(GDK_WATCH);

	return TRUE;
}


/**
 *
 */
void unload_graphics()
{
	if (program_icon_pixbuf!=NULL) g_object_unref(program_icon_pixbuf);


	if (cpp_file_pixbuf!=NULL) g_object_unref(cpp_file_pixbuf);
	if (header_file_pixbuf!=NULL) g_object_unref(header_file_pixbuf);
	if (txt_file_pixbuf!=NULL) g_object_unref(txt_file_pixbuf);
	if (java_file_pixbuf!=NULL) g_object_unref(java_file_pixbuf);
	if (lua_file_pixbuf!=NULL) g_object_unref(lua_file_pixbuf);

	if (directory_closed_pixbuf!=NULL) g_object_unref(directory_closed_pixbuf);
	if (directory_open_pixbuf!=NULL) g_object_unref(directory_open_pixbuf);
}
