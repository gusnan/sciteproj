/**
 * addfiles.c - Interface for adding files to the project
 *
 *  Copyright 2011-2012 Andreas Rönnquist
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
 *
 */

#include <gtk/gtk.h>
#include <glib-object.h>
#include <glib/gi18n.h>

#include <locale.h>

#include "clicked_node.h"

#include "gui.h"
#include "tree_manipulation.h"

#include "addfiles.h"


/**
 * Callback to handle "activation" message from a GtkEntry widget.  This 
 * code allows a return/enter keystroke to trigger dismissal of the dialog 
 * box containing the GtkEntry.
 *
 * @param entry is the GtkEntry widget
 * @param dialog is the dialog that contains the GtkEntry widget
 */
void entry_widget_activated_cb(GtkEntry *entry, gpointer dialog)
{
	if (dialog != NULL) {
		gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);
	}
}





/**
 * Callback for "Add File" menu item
 */
void addfile_menu_cb()
{
	GError *err = NULL;

	// Get the selction, and add to that group

	GtkTreeIter *parentIter=NULL;

	GtkTreeIter nodeIter;

	gboolean nodeValid=FALSE;

	GtkTreeSelection *treeSelection = gtk_tree_view_get_selection(GTK_TREE_VIEW(projectTreeView));

	GtkTreeModel *tree_model=gtk_tree_view_get_model(GTK_TREE_VIEW(projectTreeView)); 
	//GTK_TREE_MODEL(get_treestore(&err));

	GList *list = gtk_tree_selection_get_selected_rows(treeSelection, NULL);

	// is the list empty?
	if (list) {
		list=g_list_first(list);

		GtkTreePath *path=(GtkTreePath*)list->data;

		gtk_tree_model_get_iter (tree_model, &nodeIter, path);

		nodeValid=TRUE;

		g_list_foreach (list, (GFunc)(gtk_tree_path_free), NULL);
		g_list_free (list);

	}

	if (nodeValid) {

		int nodeType=-1;
		gtk_tree_model_get(tree_model, &nodeIter, COLUMN_ITEMTYPE, &nodeType, -1);

		if (nodeType==ITEMTYPE_GROUP) {
			parentIter=&nodeIter;
		} else {
			GtkTreeIter newIter;
			if (gtk_tree_model_iter_parent(tree_model,&newIter,&nodeIter)) {
				parentIter=&newIter;
			}

		}
	}


	if (parentIter!=NULL) {
		gchar *nodeContents=NULL;
		gtk_tree_model_get(tree_model, parentIter, COLUMN_FILENAME, &nodeContents, -1);

		g_free(nodeContents);
	} else {

	}

	if (!add_files_to_project(parentIter, &err)) {
		GtkWidget *dialog = gtk_message_dialog_new(NULL,
					GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
					"An error occurred while trying add files to the project: %s", err->message);

		if (dialog) {
			gtk_dialog_run(GTK_DIALOG(dialog));

			gtk_widget_destroy(dialog);
		}
	}

	if (err) g_error_free(err);
}



/**
 * Callback for menu manager to populate GUI widgets
 *
 * @param ui is the GtkUIManager
 * @param widget is the GtkWidget to add to the UI
 * @param container is the container to add widget to
 */
void menu_add_widget_cb(GtkUIManager *ui, GtkWidget *widget, GtkContainer *container)
{
	// use Grid instead of box packing on GTK3
#if GTK_MAJOR_VERSION>=3
	gtk_grid_attach(GTK_GRID(container),widget,0,0,1,1);
#else
	gtk_box_pack_start(GTK_BOX(container), widget, FALSE, FALSE, 0);
#endif
	gtk_widget_show(widget);
}


/**
 * Add files to a group or the root of the tree
 */
void popup_add_files_cb()
{
	GtkWidget *dialog = NULL;
	GError *err = NULL;
	GtkTreeIter *nodeIter = NULL;

	//debug_printf("%s!\n",__func__);

	// Files cannot be added to files!

	if (clicked_node.valid && clicked_node.type == ITEMTYPE_FILE) {
		goto EXITPOINT;
	}

	// Add to the root or to a group?

	if (clicked_node.valid) {
		nodeIter = &(clicked_node.iter);
	}

	if (!add_files_to_project(nodeIter, &err)) {
		dialog = gtk_message_dialog_new(NULL,
		                                GTK_DIALOG_MODAL,
		                                GTK_MESSAGE_ERROR,
		                                GTK_BUTTONS_OK,
		                                "An error occurred while trying add files to the project: %s",
		                                err->message);

		gtk_dialog_run(GTK_DIALOG(dialog));
	} else {

	}


EXITPOINT:

	if (err) g_error_free(err);
	if (dialog) gtk_widget_destroy(dialog);
}




/**
 * Add a group to the root of the tree, or to an existing group
 */
void popup_add_group_cb()
{
	GtkTreeIter *nodeIter = NULL;

	// Groups cannot be added to files!
	if (clicked_node.valid && clicked_node.type == ITEMTYPE_FILE) {
		goto EXITPOINT;
	}

	// Are we adding this to the root of the tree, or to an existing group?
	if (clicked_node.valid) {
		nodeIter = &(clicked_node.iter);
	}

	ask_name_add_group(nodeIter);


EXITPOINT:

	return;
}




/**
 * Ask the user for the name of a group and add it to the tree
 *
 * @param nodeIter is the node the group will be added to as a child; 
 * if NULL, then the group is added to the root of the tree
 */
void ask_name_add_group(GtkTreeIter *nodeIter)
{
	GError *err = NULL;

#if GTK_MAJOR_VERSION<3
	GtkAttachOptions options = (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL);
#endif
	const gchar *groupName = NULL;
	gint dialogResponse;


	// Create a dialog box with a nicely-centered text entry widget
	GtkWidget *dialog = gtk_dialog_new_with_buttons(_("Create Group"),
	                                                NULL,
	                                                GTK_DIALOG_MODAL,
	                                                GTK_STOCK_OK,
	                                                GTK_RESPONSE_ACCEPT,
	                                                GTK_STOCK_CANCEL,
	                                                GTK_RESPONSE_REJECT,
	                                                NULL);

	g_signal_connect(dialog, "response",  G_CALLBACK(gtk_widget_hide), dialog);

	gtk_container_set_border_width(GTK_CONTAINER(dialog), 0);
#if GTK_MAJOR_VERSION>=3
	GtkWidget *table=gtk_grid_new();
	gtk_grid_set_row_spacing (GTK_GRID (table), 6);
	gtk_grid_set_column_spacing(GTK_GRID(table),6);

#else
	GtkWidget *table = gtk_table_new(1, 2, FALSE);
#endif

	GtkWidget *gtkLabel = gtk_label_new(_("Enter name of new group:"));

#if GTK_MAJOR_VERSION>=3
	gtk_grid_attach(GTK_GRID(table),gtkLabel,0,0,3,1);
#else
	gtk_table_attach(GTK_TABLE(table), gtkLabel, 0, 1, 0, 1, options, options, 5, 5);
#endif

	GtkWidget *gtkEntry = gtk_entry_new();

	g_signal_connect(G_OBJECT(gtkEntry), "activate", G_CALLBACK(entry_widget_activated_cb), dialog);

#if GTK_MAJOR_VERSION>=3
	gtk_grid_attach_next_to(GTK_GRID(table),gtkEntry,gtkLabel,GTK_POS_RIGHT,3,1);
#else
	gtk_table_attach(GTK_TABLE(table), gtkEntry, 1, 2, 0, 1, options, options, 5, 5);
#endif


	GtkWidget *container_vbox=gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	gtk_container_add(GTK_CONTAINER(container_vbox), table);

	gtk_widget_show_all(dialog);


	// Let the user enter a name or cancel the whole thing

	dialogResponse=gtk_dialog_run(GTK_DIALOG(dialog));
	if (dialog_response_is_exit(dialogResponse)) {
		goto EXITPOINT;
	}

	groupName = gtk_entry_get_text(GTK_ENTRY(gtkEntry));

	if (groupName == NULL || *groupName == '\0') {
		GtkWidget *errDialog = gtk_message_dialog_new(NULL,
		                                              GTK_DIALOG_MODAL,
		                                              GTK_MESSAGE_ERROR,
		                                              GTK_BUTTONS_OK,
		                                              "Invalid group name");

		gtk_dialog_run(GTK_DIALOG(errDialog));

		gtk_widget_destroy(errDialog);

		goto EXITPOINT;
	}


	// Add the group

	if (!add_tree_group(nodeIter, ADD_CHILD, groupName, groupName, TRUE, NULL, &err)) {
		GtkWidget *errDialog = gtk_message_dialog_new(NULL,
		                                              GTK_DIALOG_MODAL,
		                                              GTK_MESSAGE_ERROR,
		                                              GTK_BUTTONS_OK,
		                                              "An error occurred while adding the group: %s",
		                                              err->message);

		gtk_dialog_run(GTK_DIALOG(errDialog));

		gtk_widget_destroy(errDialog);
	}


EXITPOINT:

	// Destroying the dialog should also destroy the table, label, and entry widgets
	if (gtkLabel) gtk_widget_destroy(GTK_WIDGET(gtkLabel));
	if (table) gtk_widget_destroy(GTK_WIDGET(table));

	if (dialog) gtk_widget_destroy(GTK_WIDGET(dialog));

	if (err) g_error_free(err);

	// Do NOT free groupName, since that is owned by the GtkEntry widget!
}

