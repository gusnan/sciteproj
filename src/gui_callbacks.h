/**
 * gui_callbacks.h - GUI callback code for SciteProj
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
#ifndef __HEADER_GUI_CALLBACKS_
#define __HEADER_GUI_CALLBACKS_

#define APP_SCITEPROJ_ERROR g_quark_from_static_string("APP_GUI_ERROR")


//gint window_delete_event_cb(GtkWidget *widget, GdkEvent *event, gpointer data);
//void tree_row_activated_cb(GtkTreeView *treeView, GtkTreePath *path, GtkTreeViewColumn *column, gpointer userData);
//gboolean mouse_button_pressed_cb(GtkWidget *treeView, GdkEventButton *event, gpointer userData);

//static void ask_name_add_group(GtkTreeIter *nodeIter);

void row_expand_or_collapse_cb(GtkTreeView *treeview, GtkTreeIter *arg1, GtkTreePath *arg2, gpointer user_data);

//static void menu_add_widget_cb(GtkUIManager *ui, GtkWidget *widget, GtkContainer *container);

void quit_menu_cb();
void about_menu_cb();
void saveproject_menu_cb();
void saveproject_as_menu_cb();
void openproject_menu_cb();
void creategroup_menu_cb();

void popup_open_file_cb();

void expand_all_items_cb();
void collapse_all_items_cb();

void sort_ascending_cb();
void sort_descending_cb();

void edit_options_cb();


#endif /*__HEADER_GUI_CALLBACKS_*/
