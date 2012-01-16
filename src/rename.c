/**
 * rename.c - code for renaming a group for SciteProj
 *
 *  Copyright 2011-2012 Andreas Rönnquist
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
#include <glib/gi18n.h>

#include <locale.h>

#include "tree_manipulation.h"

#include "clicked_node.h"
#include "gui.h"

#include "rename.h"


/**
 *
 */


/**
 *
 */
void rename_cb(gchar *new_text, gchar *path_string,gpointer user_data)
{
	gchar *new_folder_name=(gchar*)user_data;
	GError *err = NULL;
	GtkTreePath *path=gtk_tree_path_new_from_string(path_string);
	GtkTreeModel *model=gtk_tree_view_get_model(GTK_TREE_VIEW(projectTreeView));
	GtkTreeIter iter;
	
	if ( !gtk_tree_model_get_iter ( model, &iter, path) ) { // get iter from specified path
		GtkWidget *errDialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "An error occurred in gtk_tree_model_get_iter");
		gtk_dialog_run(GTK_DIALOG(errDialog));
		gtk_widget_destroy(errDialog);
	}
	
	if (!set_tree_node_name(&iter,new_folder_name,&err)) {	
	
		GtkWidget *errDialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "An error occurred while renaming the group: %s", err->message);
		gtk_dialog_run(GTK_DIALOG(errDialog));
		gtk_widget_destroy(errDialog);
	}
	
	if (err) g_error_free(err);
	
}


/**
 *
 */
void do_rename_iter(GtkTreeIter iter)
{
	GtkTreeModel *model=gtk_tree_view_get_model(GTK_TREE_VIEW(projectTreeView));
	
	int nodeItemType;
	
	//gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, COLUMN_ITEMTYPE, &nodeItemType, -1);
	
	// We can only rename groups
	if (nodeItemType==ITEMTYPE_GROUP) {
	
		// Set the node as editable
		g_object_set(G_OBJECT(textCellRenderer), "editable",TRUE, NULL);
		
		GtkTreePath *path;
		path=gtk_tree_model_get_path(model,&iter);
		
		// set the cursor on the cell that is to be edited, and activate the editing
		gtk_tree_view_set_cursor_on_cell(GTK_TREE_VIEW(projectTreeView),path,column1,NULL,TRUE);

		// Set the node as not editable after we are done
		g_object_set(G_OBJECT(textCellRenderer), "editable",FALSE, NULL);
	}
}


/**
 *
 */
void do_rename_node(gboolean ignore_clicked_node)
{
	GError *err = NULL;
	GtkWidget *dialog = NULL;
	
	// if there is only one selected, get its name
	GtkTreeSelection *selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW(projectTreeView) );
	GtkTreeModel *model;
	GtkTreeIter iter;
	
	if (gtk_tree_selection_count_selected_rows(selection) == 0)
		return;
	
	model=gtk_tree_view_get_model(GTK_TREE_VIEW(projectTreeView));

	GList *list = gtk_tree_selection_get_selected_rows( selection, &model );
	
	// Begin at the end and go to the start
	
	list=g_list_last(list);
	while(list) {
		GString *fixed_path = g_string_new("");
		g_string_printf(fixed_path, "%s", gtk_tree_path_to_string((GtkTreePath*)list->data)/*ipath*/);

		GtkTreePath *path = gtk_tree_path_new_from_string(fixed_path->str);
		g_string_free(fixed_path, TRUE);

		if (path) {
			if ( gtk_tree_model_get_iter ( model, &iter, path) ) { // get iter from specified path
				
				do_rename_iter(iter);
			}
			else { // invalid path
				g_error(_("Error!!!\n"));
			}
			gtk_tree_path_free (path);
		}
		else {
			g_error(_("Error!!!\n"));
		}
		list=list->prev;
	}
	
	g_list_foreach (list, (GFunc)gtk_tree_path_free, NULL);
	g_list_free (list);
	
	if (err) g_error_free(err);
	if (dialog) gtk_widget_destroy(dialog);	
}


/**
 * Rename a group
 */
void popup_rename_group_cb()
{
	GError *err = NULL;
	// We can only rename groups!
	
	if (!clicked_node.valid || clicked_node.type != ITEMTYPE_GROUP) {
		goto EXITPOINT;
	}
	
	do_rename_iter(clicked_node.iter);
	
EXITPOINT:
	
	// Destroying the dialog should also destroy the table, label, and entry widgets
	
	if (err) g_error_free(err);
	
	// Do NOT free newGroupName, since that is owned by the GtkEntry widget!
}


