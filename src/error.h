/**
 * error.h - allocate and de-allocate error messages used in SciteProj
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
#ifndef __HEADER_ERROR_
#define __HEADER_ERROR_

/**
 *
 */
gchar *error_init_recent_scrolled_window;
gchar *error_init_recent_treestore;
gchar *error_init_gtk_tree_view;
gchar *error_init_gtk_cell_renderer;
gchar *error_init_gtk_tree_view_column;
gchar *error_init_gtk_cell_renderer_pixbuf;
gchar *error_init_main_window;
gchar *error_init_gtk_action_group;
gchar *error_init_gtk_ui_manager;
gchar *error_init_menu_from_xml;
gchar *error_init_main_scrolled_window;
gchar *error_init_treestore;
gchar *error_init_recent_grid;
gchar *error_init_main_vbox;
gchar *error_init_main_hbox;
gchar *error_init_full_vbox;
gchar *error_init_statusbar_vbox;
gchar *error_init_statusbar;
gchar *error_formatting_scite_command;
gchar *error_calling_g_io_channel;


/**
 *
 */
gboolean init_error_strings();
void done_error_strings();


#endif /*__HEADER_ERROR_*/
