/**
 * remove.c - code for removing nodes
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
 */
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include <locale.h>

#include "clicked_node.h"

#include "gui.h"

#include "tree_manipulation.h"

#include "remove.h"

#include "file_utils.h"


/**
 * delete_file
 * 	deletes a file (or directory) from the filesystem - directories needs to
 * be empty to be able to delete them.
 */
gboolean delete_file(gchar *filename,GError **error)
{
	gboolean result=FALSE;
	
	gchar *file_to_delete;
	
	if (g_file_test(filename, G_FILE_TEST_IS_DIR)) {
		file_to_delete=filename;
			
		// We have already checked if the folder is empty
	} else {
		
		if (!relative_path_to_abs_path(filename, &file_to_delete, get_project_directory(), error)) {
			result=FALSE;
			goto EXITPOINT;
		}
	}

	printf("%s\n", file_to_delete);
	result=TRUE;
	
EXITPOINT:
	return result;
}


/**
 *
 */
GList *get_list_of_selected_items_rows(GtkTreeView *treeview)
{
	GtkTreeSelection *selection = gtk_tree_view_get_selection(treeview);
	GtkTreeModel *model;
	
	if (gtk_tree_selection_count_selected_rows(selection) == 0)
		return NULL;
	
	model = gtk_tree_view_get_model(treeview);
	
	GList *result_list = gtk_tree_selection_get_selected_rows(selection, &model);
	
	return result_list;
}


/**
 *
 */
GList *get_list_of_selected_items_string_list(GtkTreeView *treeview)
{
	GtkTreeModel *model = gtk_tree_view_get_model(treeview);
	GList *row_list = get_list_of_selected_items_rows(treeview);
	
	GList *string_list = NULL;
	
	// Begin at the end, and go to the start
	row_list = g_list_last(row_list);
	
	GtkTreeIter iter;
	
	int count=0;
		
	while (row_list) {
		GtkTreePath *path = (GtkTreePath *)row_list->data;
		
		if (path) {
		
			gtk_tree_model_get_iter(model, &iter, path);
			
			gchar *path;
			
			gtk_tree_model_get(model, &iter, COLUMN_FILEPATH, &path, -1);
			
			string_list = g_list_prepend(string_list, path);
			
			count++;
			
		}
		
		row_list = row_list -> prev;
	}
	
	g_list_foreach (row_list, (GFunc)gtk_tree_path_free, NULL);
	g_list_free (row_list);
	
	return string_list;
}


/**
 * get_list_of_selected_items_strings
 * 	Will only give the 7 first values, orelse it probably wouldn't fit in a
 *		dialog
 */
gchar *get_list_of_selected_items_strings(GtkTreeView *treeview)
{
	GtkTreeModel *model = gtk_tree_view_get_model(treeview);
	GList *list = get_list_of_selected_items_rows(treeview);
	
	// Begin at the end, and go to the start
	list = g_list_last(list);
	
	GtkTreeIter iter;
	
	gchar *result_string = g_strdup("\n\n");
	
	int count=0;
		
	while (list) {
		GtkTreePath *path = (GtkTreePath *)list->data;
		
		if (path) {
		
			gtk_tree_model_get_iter(model, &iter, path);
			
			gchar *path;
			
			gtk_tree_model_get(model, &iter, 
								COLUMN_FILEPATH, &path, -1);
			
			if (count <7) {
				gchar *temp = g_strconcat(result_string, path, "\n", NULL);
				g_free(result_string);
				result_string=temp;
				
			} else if (count == 7) {
				gchar *temp = g_strconcat(result_string, "...\n", NULL);
				g_free(result_string);
				result_string=temp;
			}
			count++;
			
		}
		
		list = list -> prev;
	}
	
	g_list_foreach (list, (GFunc)gtk_tree_path_free, NULL);
	g_list_free (list);	
		
	return result_string;
}


/**
 *		Actually remove selected nodes
 */
void remove_selected_items ( GtkTreeView *treeview )
{
	GError *error=NULL;
	GtkTreeIter iter;
	GtkTreeModel *model=gtk_tree_view_get_model(treeview);
	GList *list = get_list_of_selected_items_rows(treeview); // gtk_tree_selection_get_selected_rows( selection, &model );

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
GtkWidget *do_question_dialog(const gchar *buffer)
{
	
	GtkWidget *dialog = gtk_message_dialog_new(NULL, 
															GTK_DIALOG_MODAL, 
															GTK_MESSAGE_QUESTION,
															GTK_BUTTONS_OK_CANCEL,
															"%s", buffer);
	return dialog;
}


/**
 *
 */
gboolean really_do_delete_question(const gchar *format, ...)
{
	GtkWidget *dialog = NULL;
	gint dialog_response;
	char buffer[256];
	gboolean result;
	
	va_list args;
	va_start(args, format);
	vsprintf(buffer, format, args);

	dialog = do_question_dialog(buffer);
	
	va_end(args);
	
	result=TRUE;

	dialog_response = gtk_dialog_run(GTK_DIALOG (dialog));
	if (dialog_response_is_exit(dialog_response)) {
		result=FALSE;
		goto EXITPOINT;
	}
	
	// Ask again to really make sure the user knows what he/she is doing.
	if (result) {
		
		gtk_widget_destroy(dialog);

		gchar *extended_question_string = g_strdup_printf(
								"Are you really really sure? \n"
								"This will actually delete the files\n"
								"from the filesystem.\n\n%s",buffer);

		dialog = do_question_dialog(extended_question_string);

		dialog_response=gtk_dialog_run(GTK_DIALOG(dialog));
		if (dialog_response_is_exit(dialog_response)) {
			result=FALSE;
			goto EXITPOINT;
		}
	}
	
EXITPOINT:
	
	gtk_widget_destroy(dialog);
	return result;
}


/**
 *
 */
void do_remove_node(gboolean ignore_clicked_node)
{
	GError *err = NULL;
	GtkWidget *dialog = NULL;
	gchar *nodename = NULL;

	gint selected_rows=0;

	gboolean multiple_selected=FALSE;

	// Make sure a node has been selected
	if (!ignore_clicked_node) {
		if (!clicked_node.valid) {
			goto EXITPOINT;
		}
	}

	GtkTreeSelection *tree_select;

	tree_select=gtk_tree_view_get_selection(GTK_TREE_VIEW(projectTreeView));

	selected_rows=gtk_tree_selection_count_selected_rows(tree_select);
	if (selected_rows>1) {
		multiple_selected=TRUE;
	}

	if (!ignore_clicked_node) {
		// Figure out the node name
		nodename = strrchr(clicked_node.name, '/');

		if (nodename) {
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

	if (multiple_selected) {
		
		gchar *file_list_string = get_list_of_selected_items_strings(GTK_TREE_VIEW(projectTreeView));
		
		gchar *question_string = g_strdup_printf("%s\n%s",_("Delete all selected items?"),file_list_string);

		if (really_do_delete_question(question_string)) {
			// remove them!
			
			GList *string_list = get_list_of_selected_items_string_list(GTK_TREE_VIEW(projectTreeView));
			
			while (string_list) {
				gchar *temp = (gchar *)string_list->data;
				
				if (!delete_file(temp, &err)) {
					goto EXITPOINT;
				}
				
				string_list = string_list->next;
			};
			
			remove_selected_items(GTK_TREE_VIEW(projectTreeView));
		}
				
		g_free(file_list_string);
		
		g_free(question_string);


	} else {
		
		gboolean really_do_delete=FALSE;

		if (clicked_node.type == ITEMTYPE_FILE) {
			if (nodename) {
				really_do_delete = really_do_delete_question(_("Delete file '%s'?"), nodename);
				
			} else {
				really_do_delete = really_do_delete_question(_("Delete file?"));
			}
		}
		else {
			
			// TODO: check if the folder is empty already here before "confirmation" questions
			
			gchar *filepath;
					
			GtkTreeModel *tree_model = gtk_tree_view_get_model(GTK_TREE_VIEW(projectTreeView));
		
			gtk_tree_model_get(tree_model, &clicked_node.iter, 
										COLUMN_FILEPATH, &filepath, -1);
				
			if (get_number_of_files_in_folder(filepath)>0) {
				
				GtkWidget *dialog = gtk_message_dialog_new(NULL,
																GTK_DIALOG_MODAL, 
																GTK_MESSAGE_INFO,
																GTK_BUTTONS_OK,
																"Folder need to be empty to be able to delete it!");
				gtk_dialog_run(GTK_DIALOG(dialog));
				
				gtk_widget_destroy(dialog);

				goto EXITPOINT;
			}

			
			if (nodename) {
				really_do_delete = really_do_delete_question(_("Delete folder '%s' and any contained files?"),nodename);
				
			} else {
				really_do_delete = really_do_delete_question(_("Delete folder and any contained files?"));
			}
		}

		if (!really_do_delete) {
			goto EXITPOINT;
		}

		// Delete the file

		gchar *filepath;
		
		GtkTreeModel *tree_model = gtk_tree_view_get_model(GTK_TREE_VIEW(projectTreeView));
		
		gtk_tree_model_get(tree_model, &clicked_node.iter, 
										COLUMN_FILEPATH, &filepath, -1);
		
		if (!delete_file(filepath, &err)) {
			goto EXITPOINT;
		}

		// Remove the node

		if (!remove_tree_node(&(clicked_node.iter), &err)) {
			GtkWidget *errDialog = gtk_message_dialog_new(NULL, 
												GTK_DIALOG_MODAL,
												GTK_MESSAGE_ERROR,
												GTK_BUTTONS_OK,
												_("Could not delete the selected node: \n\n%s"),
												err->message);

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



