/**
 * remove.c - code for removing nodes
 *
 *  Copyright 2011 Andreas Rönnquist
 *
 * This file is part of SciteProj
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
 */
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include <locale.h>

#include "clicked_node.h"

#include "gui.h"

#include "tree_manipulation.h"

#include "remove.h"


/**
 *		Actually remove selected nodes
 */
void remove_selected_items ( GtkTreeView *treeview )
{
	GtkTreeSelection *selection = gtk_tree_view_get_selection ( treeview );
	GtkTreeModel *model;
	GtkTreeIter iter;

	//GtkTreeStore *store;
	
	GError *error=NULL;
   
	if (gtk_tree_selection_count_selected_rows(selection) == 0)
		return;
	
	model=gtk_tree_view_get_model(treeview);

	GList *list = gtk_tree_selection_get_selected_rows( selection, &model );

	int nRemoved = 0;
	
	// Begin at the end and go to the start
	
	list=g_list_last(list);
	while(list) {
		GString *fixed_path = g_string_new("");
		g_string_printf(fixed_path, "%s", gtk_tree_path_to_string((GtkTreePath*)list->data)/*ipath*/);

		GtkTreePath *path = gtk_tree_path_new_from_string(fixed_path->str);
		g_string_free(fixed_path, TRUE);

		if (path) {
			if ( gtk_tree_model_get_iter ( model, &iter, path) ) { // get iter from specified path
					
				remove_tree_node(&iter,&error);
				nRemoved++;   
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
}


/**
 *
 */
void do_remove_node(gboolean ignore_clicked_node)
{
	GError *err = NULL;
	GtkWidget *dialog = NULL;
	gint dialogResponse;
	gchar *nodename = NULL;
	
	gint selected_rows=0;
	
	gboolean multiple_selected=FALSE;
	
	// Make sure a node has been selected
	if (!ignore_clicked_node) {
		if (!clicked_node.valid) {
			goto EXITPOINT;
		}
	}
	
	GtkTreeSelection *treeSelect;
	
	treeSelect=gtk_tree_view_get_selection(GTK_TREE_VIEW(projectTreeView));
	
	selected_rows=gtk_tree_selection_count_selected_rows(treeSelect);
	if (selected_rows>1) {
		multiple_selected=TRUE;
	}
	
	if (!ignore_clicked_node) {
		// Figure out the node name
		nodename = strrchr(clicked_node.name, '/');
		
		if (nodename != NULL) {
			++nodename;
		}
		else {
			nodename = clicked_node.name;
		}
	} else {
		// if there is only one selected, get its name
		nodename=NULL;
		multiple_selected=TRUE;
	}
		
	// Confirm removal from project
	
	if (multiple_selected==TRUE) {
		
		dialog=gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK_CANCEL, "Remove all selected items?" /*, nodename*/);
		
		dialogResponse = gtk_dialog_run(GTK_DIALOG (dialog));
		
		if (dialog_response_is_exit(dialogResponse)) {
			goto EXITPOINT;
		}
		
		// remove them!
		remove_selected_items(GTK_TREE_VIEW(projectTreeView));

	} else {
		
		if (clicked_node.type == ITEMTYPE_FILE) {
			if (nodename!=NULL) {
				dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK_CANCEL, "Remove file '%s' from project?", nodename);
			} else {
				dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK_CANCEL, "Remove file from project?");
			}
		}
		else {
			if (nodename!=NULL) {
				dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK_CANCEL, "Remove group '%s' and any contained files from project?", nodename);
			} else {
				dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK_CANCEL, "Remove group and any contained files from project?");
			}
		}
		
		dialogResponse = gtk_dialog_run(GTK_DIALOG (dialog));
		if (dialog_response_is_exit(dialogResponse)) {
			goto EXITPOINT;
		}
		
		
		// Remove the node
		
		if (!remove_tree_node(&(clicked_node.iter), &err)) {
			GtkWidget *errDialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Could not remove the selected node: \n\n%s", err->message);
			
			gtk_dialog_run(GTK_DIALOG (errDialog));
			
			gtk_widget_destroy(errDialog);	
		}
	}
	
EXITPOINT:
	
	if (err) g_error_free(err);
	if (dialog) gtk_widget_destroy(dialog);	
}


/**
 * Remove the selected file/group.
 */
void popup_remove_node_cb()
{
	do_remove_node(FALSE);
}


/**
 * Callback for Remove Item(s) menu item
 */
void removeitem_menu_cb()
{
	do_remove_node(TRUE);
}



