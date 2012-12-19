/**
 * gui_callbacks.c - GUI callback code for SciteProj
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

#include <string.h>
#include <sys/stat.h>
#include <glib.h>
#include <gtk/gtk.h>

#include <gdk/gdkkeysyms.h>

#include <stdlib.h>
#include <glib/gi18n.h>

#include <locale.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "gui_callbacks.h"
#include "clicked_node.h"

#include "gui.h"
#include "tree_manipulation.h"
#include "scite_utils.h"
#include "string_utils.h"
#include "prefs.h"
#include "statusbar.h"
#include "graphics.h"
#include "about.h"
#include "properties_dialog.h"
#include "file_utils.h"

#include "addfiles.h"
#include "recent_files.h"
#include "remove.h"
#include "rename.h"
#include "filelist.h"
#include "sort.h"

#include "load_folder.h"

#include "gtk3_compat.h"

#include "script.h"


/**
 *		Expands all folders
 */
gboolean foreach_expand(GtkTreeModel *model,GtkTreePath *path,
                               GtkTreeIter *iter,gpointer data)
{
	expand_tree_row(path,TRUE);
	return FALSE;
}


/**
 *		Collapses all folders
 */
gboolean foreach_collapse(GtkTreeModel *model,GtkTreePath *path,
                          GtkTreeIter *iter,gpointer data)
{
	collapse_tree_row(path);
	return FALSE;
}




/**
 * Open the selected file.
 *	This is called when a file is rightclicked and open is selected in the menu
 */
void popup_open_file_cb()
{
	gchar *command = NULL;
	GError *err = NULL;
	GtkWidget *dialog = NULL;
	gchar *absFilePath = NULL;

	// several files in selection?

	// We can only open files

	if (!clicked_node.valid || clicked_node.type != ITEMTYPE_FILE) {
		goto EXITPOINT;
	}

	if (!open_filename(clicked_node.name,(gchar*)(get_project_directory()),&err)) {
		goto EXITPOINT;
	}

	add_file_to_recent(clicked_node.name,NULL);


EXITPOINT:

	if (err != NULL) {
		dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, 
		                                GTK_BUTTONS_OK,
		                                _("Could not open selected file: \n\n%s"), 
		                                err->message);

		gtk_dialog_run(GTK_DIALOG (dialog));
	}

	if (command) g_free(command);
	if (absFilePath) g_free(absFilePath);
	if (err) g_error_free(err);
	if (dialog) gtk_widget_destroy(dialog);
}





/**
 *
 */
void collapse_all_items_cb()
{
	gtk_tree_model_foreach(gtk_tree_view_get_model(GTK_TREE_VIEW(projectTreeView)),
	                       foreach_collapse,NULL);
}


/**
 *		edit_options_cb
 *			opens the user-specific options-file ($HOME/.sciteproj) in SciTE.
 */
void edit_options_cb()
{
	GError *err=NULL;
	gchar *command=NULL;

	if ((command = g_strdup_printf("open:%s\n", prefs_filename)) == NULL) {
		g_set_error(&err, APP_SCITEPROJ_ERROR, -1,
			"%s: %s, g_strdup_printf() = NULL",
			"Error formatting SciTE command",
			__func__);
	}
	else {
		if (send_scite_command(command, &err)) {
			// Try to activate SciTE; ignore errors

			activate_scite(NULL);

			if (prefs.give_scite_focus==TRUE) {
				send_scite_command((gchar*)"focus:0",NULL);
			}
		}
	}
}


/**
 *
 */
void expand_all_items_cb()
{
	gtk_tree_model_foreach(gtk_tree_view_get_model(GTK_TREE_VIEW(projectTreeView)),
	                       foreach_expand,NULL);
}


/**
 * step-through function for expand/collapse folder
 *
 * @param tree_view
 * @param newiter
 * @param tree_path
 */
static void fix_folders_step_through(GtkTreeView *tree_view, GtkTreeIter newiter,GtkTreePath *tree_path)
{
	GtkTreeModel *tree_model = gtk_tree_view_get_model(tree_view);

	gchar *relFilePath;

	GError *error;
	gint nodeItemType;

	GtkTreeIter iter=newiter;

	do {

		gtk_tree_model_get(tree_model, &iter, COLUMN_ITEMTYPE, &nodeItemType, -1);


		if (nodeItemType==ITEMTYPE_GROUP) {

			GtkTreePath *srcPath = gtk_tree_model_get_path(tree_model, &iter);
			gboolean groupIsExpanded = tree_row_is_expanded(srcPath);

			if (groupIsExpanded) {
				set_tree_node_icon(&iter,directory_open_pixbuf,&error);
			} else {
				set_tree_node_icon(&iter,directory_closed_pixbuf,&error);
			}
			
			set_tree_node_expanded(&iter, groupIsExpanded, NULL);

			gtk_tree_model_get(tree_model, &iter, COLUMN_FILEPATH, &relFilePath, -1);

			if (gtk_tree_model_iter_has_child(tree_model,&iter)) {

				GtkTreeIter newIter;
				gtk_tree_model_iter_children(tree_model,&newIter,&iter);
				fix_folders_step_through(tree_view,newIter,tree_path);
			}

			g_free(relFilePath);
			gtk_tree_path_free(srcPath);

		} else {

		}


	} while(gtk_tree_model_iter_next(tree_model,&iter));
}


/**
 *
 */
void load_tree_at_iter(GtkTreeView *tree_view, GtkTreeIter *iter)
{
	// We've got the folder - get the child
	GtkTreeIter child;
	
	GtkTreeModel *tree_model = gtk_tree_view_get_model(tree_view);

	if (iter) {
	
		if (gtk_tree_model_iter_children(tree_model, &child, iter)) {
			remove_tree_node(&child,NULL);
			
			gchar *folder_path;
			
			gtk_tree_model_get(tree_model, iter, COLUMN_FILEPATH, &folder_path, -1);
			
			// Load the wanted filter from the LUA config
			GSList *filter_list=load_filter_from_lua(folder_path);
			
			GSList *file_list; //=load_folder_to_list(folder_path, FALSE, 
			GSList *folder_list;
	
			file_list=load_folder_to_list(folder_path, FALSE, compare_strings_bigger /*file_sort_by_extension_bigger_func*/, filter_list);
			
			folder_list=load_folder_to_list(folder_path, TRUE, compare_strings_bigger, filter_list);
			
			// Here we should filter out the unwanted items
			
			add_tree_folderlist(iter, folder_list, folder_path);

			if (file_list) {
				file_list=g_slist_reverse(file_list);

				add_tree_filelist(iter, file_list, NULL);
			}
			
			set_tree_node_expanded(iter,TRUE, NULL);
			
			GtkTreePath *tree_path = gtk_tree_model_get_path(tree_model, iter);
			
			gtk_tree_view_expand_row(tree_view, tree_path, FALSE);
			
			gtk_tree_path_free(tree_path);
			
			g_slist_foreach(filter_list, (GFunc)g_free, NULL);
			g_slist_free(filter_list);
			
		}
	}
}


/**
 * Callback for expand/collapse event of GtkTreeView
 *
 * @param treeView is not used
 * @param arg1 is not used
 * @param arg2 is not used
 * @param user_data is not used
 */
void row_expand_or_collapse_cb(GtkTreeView *tree_view, GtkTreeIter *iter, 
                               GtkTreePath *tree_path, gpointer user_data)
{
	/* Switch the folder icon open/closed*/

	GtkTreeModel *tree_model = gtk_tree_view_get_model(tree_view);

	// make sure all icons the folder (and folders inside it) are set to a correct icon.
	fix_folders_step_through(tree_view,*iter,tree_path);
	
	gchar *temp;
	gboolean expanded;
	gboolean loaded;
	
	gtk_tree_model_get(tree_model, iter, COLUMN_FILEPATH, &temp, -1);
	gtk_tree_model_get(tree_model, iter, COLUMN_EXPANDED, &expanded, -1);
	gtk_tree_model_get(tree_model, iter, COLUMN_FOLDER_CONTENT_LOADED, &loaded, -1);

	//printf("%s : %d\n", temp, (int)expanded);
	
	if (!loaded) {
		set_tree_node_loaded(iter, TRUE, NULL);
		
		load_tree_at_iter(tree_view, iter);
		
		
	}
}



/**
 * Callback for "Quit" menu item
 */
void quit_menu_cb()
{
	gtk_main_quit();
}


/**
 * Callback for "About" menu item
 */
void about_menu_cb()
{
	show_about_dialog();
}


/**
 * Callback for "Create Group" menu item
 */
void creategroup_menu_cb()
{
	ask_name_add_group(NULL);
}


/**
 *
 */
gboolean key_press_cb(GtkWidget *widget, GdkEventKey *event, gpointer userData)
{
	switch (event->keyval)
	{
		case GDK_KEY_BackSpace:
		{
			debug_printf((gchar*)"key_press_cb: keyval = %d = GDK_BackSpace, hardware_keycode = %d\n", 
			             event->keyval, event->hardware_keycode);
			break;
		}

		case GDK_KEY_Delete:
		{
			do_remove_node(TRUE);
			break;
		}
		case GDK_KEY_Insert:
		{
			break;
		}
		case GDK_KEY_F2:
		{
			do_rename_node(TRUE);
			return TRUE;
		}
		case GDK_KEY_F5:
		{
			print_filelist();
			break;
		}
		default:
		{
			debug_printf("key_press_cb: keyval = %d = '%c', hardware_keycode = %d\n", 
			             event->keyval, (char) event->keyval, event->hardware_keycode);
			return FALSE;
		}
	}

	if (event->state & GDK_SHIFT_MASK) debug_printf(", GDK_SHIFT_MASK");
	if (event->state & GDK_CONTROL_MASK) debug_printf(", GDK_CONTROL_MASK");
	if (event->state & GDK_MOD1_MASK) debug_printf(", GDK_MOD1_MASK");
	if (event->state & GDK_MOD2_MASK) debug_printf(", GDK_MOD2_MASK");
	if (event->state & GDK_MOD3_MASK) debug_printf(", GDK_MOD3_MASK");
	if (event->state & GDK_MOD4_MASK) debug_printf(", GDK_MOD4_MASK");
	if (event->state & GDK_MOD5_MASK) debug_printf(", GDK_MOD5_MASK");

	debug_printf("\n");

	return FALSE;
}


/**
 *		search function for the gtk_tree_view_set_search_equal_func
 *		@return TRUE when rows DONT match, FALSE when rows match
 */
gboolean tree_view_search_equal_func(GtkTreeModel *model,gint column,
                                     const gchar *key,GtkTreeIter *iter,
                                     gpointer search_data)
{
	gchar *filename;
	// For some reason this should return TRUE if the row DONT match
	gboolean res=TRUE;

	gtk_tree_model_get(model, iter, COLUMN_FILENAME, &filename, -1);

	// zero when matches, which means we should return FALSE
	if (g_ascii_strncasecmp(key,filename,strlen(key))==0) res=FALSE;

	g_free(filename);

	return res;
}


/**
 *
 */
void refresh_folder_cb()
{
	if (!clicked_node.valid || clicked_node.type != ITEMTYPE_GROUP) {
		return;
	}
	
	gchar *folder_name;
	GtkTreeModel *tree_model=gtk_tree_view_get_model(GTK_TREE_VIEW(projectTreeView));
	GtkTreeIter iter=clicked_node.iter;
	
	GtkTreeIter *stored_iter=gtk_tree_iter_copy(&iter);
	
	gboolean expanded;
	
	gtk_tree_model_get(tree_model, &iter, COLUMN_FILENAME, &folder_name,
													  COLUMN_EXPANDED, &expanded, 
													  -1);
	
	// If the folder is expanded
	if (expanded) {
			
		// add all rows below to a list of GtkTreePath
	
		
		GtkTreeIter child;
		
		GList *list_of_items=NULL;

		// First, store all GtkTreePath in a linked list
		
		if (gtk_tree_model_iter_children(tree_model, &child, &iter)) {
			int co=0;
			GtkTreePath *tree_path;

			GtkTreeIter *temp_iter=&child;
			do {

				gchar *temp;
				gtk_tree_model_get(tree_model, temp_iter, COLUMN_FILENAME, &temp, -1);

				tree_path=gtk_tree_model_get_path(tree_model, temp_iter);
				GtkTreeRowReference *row_reference=gtk_tree_row_reference_new(tree_model, tree_path);

				list_of_items=g_list_append(list_of_items, row_reference);

				gtk_tree_path_free(tree_path);
				co++;

			} while(gtk_tree_model_iter_next(tree_model, temp_iter));
			
			// go through the list of row-references
			
			GList *node;
			for (node = list_of_items; node != NULL; node = node -> next) {
				tree_path=gtk_tree_row_reference_get_path((GtkTreeRowReference*)node->data);
				
				if (tree_path) {
					GtkTreeIter iter;
					if (gtk_tree_model_get_iter(tree_model, &iter, tree_path))
						remove_tree_node(&iter,NULL);
				}
			}
			
			g_list_foreach(list_of_items, (GFunc) gtk_tree_row_reference_free, NULL);
		}

		GtkTreeIter *temp_iter=gtk_tree_iter_copy(stored_iter);

		gchar *folder;
		gtk_tree_model_get(tree_model, temp_iter, 
									COLUMN_FILEPATH, &folder, 
									-1);
		
		GtkTreeIter new_iter;
		if (get_number_of_files_in_folder(folder)>0) {
			add_tree_file(temp_iter, ADD_CHILD, "<loading...>", &new_iter, FALSE, NULL);
		}

		load_tree_at_iter(GTK_TREE_VIEW(projectTreeView), temp_iter);
		//set_tree_node_loaded(temp_iter, TRUE, NULL);	
		
		sort_children(stored_iter, NULL, compare_strings_smaller);
		
	} else {
	}
	
}
