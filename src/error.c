/**
 * error.c - allocate and de-allocate error messages used in SciteProj
 *
 *  Copyright 2012 Andreas Rönnquist
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
#include <string.h>
#include <glib/gi18n.h>

#include <locale.h>


#include "error.h"

/**
 *
 */
gchar *error_init_recent_scrolled_window=NULL;
gchar *error_init_recent_treestore=NULL;
gchar *error_init_gtk_tree_view=NULL;
gchar *error_init_gtk_cell_renderer=NULL;
gchar *error_init_gtk_tree_view_column=NULL;
gchar *error_init_gtk_cell_renderer_pixbuf=NULL;
gchar *error_init_main_window=NULL;
gchar *error_init_gtk_action_group=NULL;
gchar *error_init_gtk_ui_manager=NULL;
gchar *error_init_menu_from_xml=NULL;
gchar *error_init_main_scrolled_window=NULL;
gchar *error_init_treestore=NULL;
gchar *error_init_recent_grid=NULL;
gchar *error_init_main_vbox=NULL;
gchar *error_init_main_hbox=NULL;
gchar *error_init_full_vbox=NULL;
gchar *error_init_statusbar_vbox=NULL;
gchar *error_init_statusbar=NULL;
gchar *error_formatting_scite_command=NULL;
gchar *error_calling_g_io_channel=NULL;


/**
 *
 */
gboolean init_error_strings()
{
	error_init_recent_scrolled_window=g_strdup(_("Could not create recent scrolled window"));
	error_init_recent_treestore=g_strdup(_("Couldn't init recent treestore"));
	error_init_gtk_tree_view=g_strdup(_("Couldn't init gtk treeview"));
	error_init_gtk_cell_renderer=g_strdup(_("Could not create GtkCellRenderer"));
	error_init_gtk_tree_view_column=g_strdup(_("Could not create GtkTreeViewColumn"));
	error_init_gtk_cell_renderer_pixbuf=g_strdup(_("Couldn't create GtkCellRendererPixbuf"));
	error_init_main_window=g_strdup(_("Couldn't create main Window"));
	error_init_gtk_action_group=g_strdup(_("Couldn't create GtkActionGroup"));
	error_init_gtk_ui_manager=g_strdup(_("Couldn't create GtkUIManager"));
	error_init_menu_from_xml=g_strdup(_("Couldn't create menus from XML"));
	error_init_main_scrolled_window=g_strdup(_("Could not create main scrolled window"));
	error_init_treestore=g_strdup(_("Could not create the treestore"));
	error_init_recent_grid=g_strdup(_("Could not create recentGrid"));
	error_init_main_vbox=g_strdup(_("Could not create main vbox"));
	error_init_main_hbox=g_strdup(_("Could not create main hbox"));
	error_init_full_vbox=g_strdup(_("Could not create full vbox"));
	error_init_statusbar_vbox=g_strdup(_("Error initing statusbar"));
	error_init_statusbar=g_strdup(_("Error initing statusbar"));
	error_formatting_scite_command=g_strdup(_("Error formatting SciTE director command"));
	error_calling_g_io_channel=g_strdup(_("Error calling g_io_channel_read_chars()"));
	
	return TRUE;
}


/**
 *
 */
void done_error_strings()
{
	g_free(error_init_recent_scrolled_window);
	
}
