/**
 * gui.h - GUI code for SciteProj
 *
 *  Copyright 2006 Roy Wood, 2009-2012 Andreas Rönnquist
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
#ifndef __HEADER_GUI_
#define __HEADER_GUI_
	
/**
 * Variables
 */
extern gchar *window_saved_title;
extern GtkWidget *projectTreeView;
extern GtkWidget *recentTreeView;
extern GtkCellRenderer *textCellRenderer;
extern ClickedNode clicked_node;

extern GtkTreeViewColumn *column1;


// Initialize the GUI
gboolean setup_gui(GError **err);

void gui_close();

// Enable/Disable the "Save Project" button
void set_save_button_sensitivity(gboolean enabled);

// Set the window title
void set_window_title(const gchar *newName);

// Is a given row expanded?
gboolean tree_row_is_expanded(GtkTreePath *path);

// Expand a row
void expand_tree_row(GtkTreePath *path, gboolean expandChildren);
void collapse_tree_row(GtkTreePath *path);

void get_dimensions(gint *left, gint *top, gint *width, gint *height);

void update_project_is_dirty(gboolean dirty);

gboolean dialog_response_is_exit(gint test);


#endif /*__HEADER_GUI_*/
