/**
 * recent_files.c - list of recently opened files
 *
 *  Copyright 2011 Andreas Rönnquist
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


#include "recent_files.h"
#include "prefs.h"
#include "graphics.h"
#include "tree_manipulation.h"
#include "string_utils.h"
#include "clicked_node.h"
#include "file_utils.h"
#include "statusbar.h"
#include "scite_utils.h"
#include "clipboard.h"
#include "properties_dialog.h"

#define APP_SCITEPROJ_ERROR g_quark_from_static_string("APP_GUI_ERROR")


/**
 *
 */
GtkCellRenderer *recentPixbuffCellRenderer = NULL;

GtkWidget *recentTreeView=NULL;

GtkTreeStore *recentTreeStore = NULL;

ClickedNode recent_clicked_node;

GtkWidget *recentPopupMenu=NULL;


/**
 * forward declarations
 */
static gboolean recent_mouse_button_pressed_cb(GtkWidget *treeView, GdkEventButton *event, gpointer userData);
static void recent_tree_row_activated_cb(GtkTreeView *treeView, GtkTreePath *path, GtkTreeViewColumn *column, gpointer userData);

/**
 * Get the GTKTreeStore (evil, but necessary for setup_gui).
 *
 * @return the GtkTreeStore* for the project or NULL if a GtkTreeStore could not be allocate
 *
 * @param err returns any error information
 */
GtkTreeStore* create_treestore_recent(GError **err)
{
	GtkTreeStore *result=NULL;
		result= gtk_tree_store_new(COLUMN_EOL, TYPE_ITEMTYPE, TYPE_FILEPATH, TYPE_FILENAME, TYPE_FILESIZE, TYPE_FONTWEIGHT, TYPE_FONTWEIGHTSET, TYPE_ICON, TYPE_EXPANDED);
		
	if (result == NULL) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: Could not create GtkTreeStore, gtk_tree_store_new() = NULL", __func__);
	}
	
	
	return result;
}



/**
 *
 */
GtkWidget *init_recent_files(GError **err)
{

	GtkCellRenderer *recentCellRenderer = NULL;
	GtkTreeViewColumn *recentColumn1 = NULL;
	GtkWidget *recentScrolledWindow = NULL;
	
	GError *tempErr=NULL;

	
	// add a scrolledwindow for recent files
	if (!(recentScrolledWindow=gtk_scrolled_window_new(NULL,NULL))) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: Could not create recent scrolled window, gtk_scrolled_window_new() = NULL", __func__);
		
		goto EXITPOINT;
	} 
	
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(recentScrolledWindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

	
	if ((recentTreeStore=create_treestore_recent(&tempErr))==NULL) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1 ,"%s: Could not create the recent treestore", tempErr->message);
		goto EXITPOINT;
	}
	
	
	if (!(recentTreeView=gtk_tree_view_new_with_model(GTK_TREE_MODEL(recentTreeStore)))) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: Could not create GtkTreeView, gtk_tree_view_new_with_model() = NULL", __func__);
		
		goto EXITPOINT;
	}
	
	gtk_widget_show(recentScrolledWindow);

	gtk_container_add(GTK_CONTAINER(recentScrolledWindow), recentTreeView);

	if (!(recentCellRenderer = gtk_cell_renderer_text_new())) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: Could not create GtkCellRenderer, gtk_cell_renderer_text_new() = NULL", __func__);
		
		goto EXITPOINT;
	}
	
		
	if (!(recentColumn1 = gtk_tree_view_column_new())) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: Could not create GtkTreeViewColumn, gtk_tree_view_column_new() = NULL", __func__);
		
		goto EXITPOINT;
	}
	
	g_object_set(recentColumn1,"title","Recently opened files:",NULL);
	
	if (!(recentPixbuffCellRenderer = gtk_cell_renderer_pixbuf_new())) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: Could not create GtkCellRenderer, gtk_cell_renderer_pixbuf_new() = NULL", __func__);
		
		goto EXITPOINT;
	}
	
		
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(recentTreeView), TRUE);
	
	gtk_tree_view_column_set_resizable(recentColumn1, TRUE);
	gtk_tree_view_column_set_min_width(recentColumn1, (int)(gPrefs.width*.75));

	
	gtk_tree_view_column_pack_start(recentColumn1, recentPixbuffCellRenderer , FALSE);
	gtk_tree_view_column_add_attribute(recentColumn1, recentPixbuffCellRenderer , "pixbuf", COLUMN_ICON);
	
	
	gtk_tree_view_column_pack_start(recentColumn1, recentCellRenderer, TRUE);
	gtk_tree_view_column_add_attribute(recentColumn1, recentCellRenderer, "text", COLUMN_FILENAME);
	gtk_tree_view_column_add_attribute(recentColumn1, recentCellRenderer, "weight", COLUMN_FONTWEIGHT);
	gtk_tree_view_column_add_attribute(recentColumn1, recentCellRenderer, "weight-set", COLUMN_FONTWEIGHTSET);
	

	g_signal_connect(G_OBJECT(recentTreeView), "button-press-event", G_CALLBACK(recent_mouse_button_pressed_cb), recentTreeView);
	
	g_signal_connect(G_OBJECT(recentTreeView), "row-activated", G_CALLBACK(recent_tree_row_activated_cb), NULL);

	gtk_tree_view_append_column(GTK_TREE_VIEW(recentTreeView), recentColumn1);
	
	gtk_widget_show(recentTreeView);

EXITPOINT:

	return recentScrolledWindow;
}


/**
 * GtkTreeModelForeachFunc
 */
gboolean tree_for_each(GtkTreeModel *model,GtkTreePath *path,GtkTreeIter *iter,gpointer data)
{
	
	gchar *filepath,*filename;
	gchar *inFile=(gchar*)(data);
	
	gtk_tree_model_get(model, iter, COLUMN_FILEPATH, &filename, -1);
	
	filepath= get_filename_from_full_path((gchar*)filename);
	
	if (g_strcmp0(filepath,inFile)==0) {
		//savedIter=iter;
		
		gtk_tree_store_remove(recentTreeStore,iter);
		return TRUE;
	}
	
	
	return FALSE;
}


/**
 *
 */
void remove_if_already_exists(const gchar *filepath)
{
	gtk_tree_model_foreach(gtk_tree_view_get_model(GTK_TREE_VIEW(recentTreeView)),tree_for_each,(gpointer)(filepath));
}


/**
 * add_file
 *	@returns TRUE if added the file successfully, FALSE otherwise
 */
gboolean add_file_to_recent(gchar *filepath,GError **err)
{
	g_assert(recentTreeStore != NULL);
	g_assert(filepath != NULL);
	//g_assert(position == ADD_BEFORE || position == ADD_AFTER || position == ADD_CHILD);
	
	gboolean finalResult = FALSE;
	GtkTreeIter iter;
	const gchar* fileName = NULL;
	gchar *relFilename = NULL;
	
	gchar *fileExt=NULL;
		
	relFilename = NULL; //g_strdup(filepath);
	
	/*
	if (!makeRelative) {
		relFilename = g_strdup(filepath);
	}
	else
		*/
	if (!abs_path_to_relative_path(filepath, &relFilename, get_project_directory(), err)) {
		printf("--- abs_path_to_relative_path FAILED!\n");
		goto EXITPOINT;
	}
	
	
	// Extract filename from filepath
	fileName = get_filename_from_full_path((gchar*)filepath);
	
	remove_if_already_exists(fileName);
	
	// Append to root, or before/after/within an existing node?
	
	//if (currentIter == NULL) {
	if (!gPrefs.recent_add_to_bottom) {
		gtk_tree_store_insert_after(recentTreeStore, &iter, NULL, NULL);
	} else {
		// get the iter of the last item
		gtk_tree_store_append(recentTreeStore,&iter,NULL);
	}
	//}
	/*
	else if (position == ADD_BEFORE) {
		gtk_tree_store_insert_before(recentTreeStore, &iter, NULL, currentIter);
	}
	else if (position == ADD_AFTER) {
		gtk_tree_store_insert_after(recentTreeStore, &iter, NULL, currentIter);
	}
	else if (position == ADD_CHILD) {
		gtk_tree_store_insert(recentTreeStore,&iter,currentIter,1000);
	}
	*/
	
	fileExt=strrchr(fileName,'.');
	
	if (fileExt!=NULL) {
		++fileExt;
	}
	
	if (fileExt == NULL || strlen(fileExt) <= 0) {
		fileExt=(gchar*)fileName;
	}	
	
	
	gtk_tree_store_set(recentTreeStore, &iter, COLUMN_ITEMTYPE, ITEMTYPE_FILE, -1);
	gtk_tree_store_set(recentTreeStore, &iter, COLUMN_FILEPATH, relFilename, -1);
	gtk_tree_store_set(recentTreeStore, &iter, COLUMN_FILENAME, fileName, -1);
	
	gtk_tree_store_set(recentTreeStore, &iter, COLUMN_EXPANDED, FALSE, -1);

	
	if (
		(strcmp(fileExt,"cc")==0) ||
		(strcmp(fileExt,"c++")==0) ||
		(strcmp(fileExt,"c")==0) ||
		(strcmp(fileExt,"cpp")==0)
		) {
		gtk_tree_store_set(recentTreeStore, &iter, COLUMN_ICON, cpp_file_pixbuf, -1);
	} else if (
		(strcmp(fileExt,"hh")==0) ||
		(strcmp(fileExt,"h++")==0) ||
		(strcmp(fileExt,"h")==0) ||
		(strcmp(fileExt,"hpp")==0)
	) {
		gtk_tree_store_set(recentTreeStore, &iter, COLUMN_ICON, header_file_pixbuf, -1);
	} else {
		gtk_tree_store_set(recentTreeStore, &iter, COLUMN_ICON, txt_file_pixbuf, -1);
	}
	
	GtkTreePath *path;
	if (!gPrefs.recent_add_to_bottom) {
		path=gtk_tree_path_new_from_string("0");
		gtk_tree_view_set_cursor_on_cell(GTK_TREE_VIEW(recentTreeView),path,NULL,NULL,FALSE);
	} else {
		GtkTreeModel *treeModel = gtk_tree_view_get_model(GTK_TREE_VIEW(recentTreeView));
		path=gtk_tree_model_get_path(treeModel,&iter);
		gtk_tree_view_set_cursor_on_cell(GTK_TREE_VIEW(recentTreeView),path,NULL,NULL,FALSE);
	}
	
	finalResult = TRUE;
	
EXITPOINT:
	
	if (relFilename) g_free(relFilename);
	
	return finalResult;
}



/**
 * Respond to a Gtk "button-press-event" message.
 *
 * @param treeView is the GTKTreeView widget in which the mouse-button event occurred
 * @param event is the GdkEventButton event object
 * @param userData is not currently used
 */
static gboolean recent_mouse_button_pressed_cb(GtkWidget *treeView, GdkEventButton *event, gpointer userData)
{
	gboolean eventHandled = FALSE;
	GtkTreePath *path = NULL;
	GtkTreeModel *treeModel = NULL;
	gchar *nodeName = NULL;
	gint nodeItemType;
	GtkTreeIter iter;
	
	g_assert(treeView != NULL);
	g_assert(event != NULL);
	
	// Until we know for sure, assume that the user has not clicked on a node
	
	recent_clicked_node.valid=FALSE;
	
	
	// If it is not a right-click, then ignore it
	
	if (event->type != GDK_BUTTON_PRESS || event->button != 3) {
		goto EXITPOINT;
	}
	
	
	// Find if the user has clicked on a node
	
	if (!gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeView), (gint) event->x, (gint) event->y, &path, NULL, NULL, NULL)) {
		// Nope-- user clicked in the GtkTreeView, but not on a node
		
		//gtk_menu_popup(GTK_MENU(sGeneralPopupMenu), NULL, NULL, NULL, NULL, event->button, gdk_event_get_time((GdkEvent*) event));
		
		goto EXITPOINT;
	}
	
	
	// User clicked on a node, so retrieve the particulars
	
	treeModel = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));
	
	if (!gtk_tree_model_get_iter(treeModel, &iter, path)) {
		goto EXITPOINT;
	}
	
	gtk_tree_model_get(treeModel, &iter, COLUMN_ITEMTYPE, &nodeItemType, COLUMN_FILEPATH, &nodeName, -1);
	
	// Save the node info for use by the popup menu callbacks
	
	if (recent_clicked_node.name) g_free(recent_clicked_node.name);
	recent_clicked_node.name=NULL;
	
	recent_clicked_node.valid=TRUE;
	recent_clicked_node.iter=iter;
	recent_clicked_node.type=nodeItemType;
	recent_clicked_node.name=nodeName;
	nodeName = NULL;
	
	// Check if something is selected
	GtkTreeSelection *tree_selection=NULL;
	
	tree_selection=gtk_tree_view_get_selection(GTK_TREE_VIEW(treeView));
	
	if (tree_selection!=NULL) {
		// Check if clicked on something in the selection, otherwise make the clicked one the selection.
			
		if (gtk_tree_selection_path_is_selected(tree_selection,path)==FALSE) {
			// clear selection and make current line selected
				
			gtk_tree_selection_unselect_all(tree_selection);				
				
			gtk_tree_selection_select_path (tree_selection,path);
		}
	}
	
	// Pop up the appropriate menu for the node type
	
	//if (nodeItemType == ITEMTYPE_FILE) {
		gtk_menu_popup(GTK_MENU(recentPopupMenu), NULL, NULL, NULL, NULL, event->button, gdk_event_get_time((GdkEvent*) event));
	/*
	}
	else if (nodeItemType == ITEMTYPE_GROUP) {
		gtk_menu_popup(GTK_MENU(sGroupPopupMenu), NULL, NULL, NULL, NULL, event->button, gdk_event_get_time((GdkEvent*) event));
	}
	*/
	
	// We took care of the event, so no need to propogate it
	
	eventHandled = TRUE;
	
	
EXITPOINT:
	
	if (path) gtk_tree_path_free(path);
	if (nodeName) g_free(nodeName);
	
	return eventHandled;
}


/**
 * Open the selected file.
 *	This is called when a file is rightclicked and open is selected in the menu
 */
void popup_open_recent_file_cb()
{
	gchar *command = NULL;
	GError *err = NULL;
	GtkWidget *dialog = NULL;
	gchar *absFilePath = NULL;
		
	// We can only open files
	
	if (!recent_clicked_node.valid || recent_clicked_node.type != ITEMTYPE_FILE) {
		goto EXITPOINT;
	}
	
	//debug_printf("name:%s\n",recent_clicked_node.name);
	
	if (!open_filename(recent_clicked_node.name,(gchar*)get_project_directory(),&err)) {
		goto EXITPOINT;
	}
	
	
EXITPOINT:
	
	if (err != NULL) {
		dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Could not open selected file: \n\n%s", err->message);
		
		gtk_dialog_run(GTK_DIALOG (dialog));
	}
	
	if (command) g_free(command);
	if (absFilePath) g_free(absFilePath);
	if (err) g_error_free(err);
	if (dialog) gtk_widget_destroy(dialog);	
}



/**
 * Callback handler for Gtk "row-activated" event.
 *
 * @param treeView is the GtkTreeView
 * @param path is the GtkTreePath of the activated row
 * @param column is not used
 * @param userData is not used
 */
static void recent_tree_row_activated_cb(GtkTreeView *treeView, GtkTreePath *path, GtkTreeViewColumn *column, gpointer userData)
{
	GtkTreeModel *treeModel = NULL;
	GtkTreeIter iter;
	gchar *relFilePath = NULL;
	gchar *absFilePath = NULL;
	gchar *command = NULL;
	GError *err = NULL;
	GtkWidget *dialog = NULL;
	gint nodeItemType;
	gchar *fixed=NULL;
	
	
	// Get the data from the row that was activated
	
	treeModel = gtk_tree_view_get_model(treeView);
	gtk_tree_model_get_iter(treeModel, &iter, path);
	gtk_tree_model_get(treeModel, &iter, COLUMN_ITEMTYPE, &nodeItemType, COLUMN_FILEPATH, &relFilePath, -1);
	
	absFilePath=fix_path((gchar*)get_project_directory(),relFilePath);

	fixed=fix_path((gchar*)get_project_directory(),relFilePath);
	
	if ((command = g_strdup_printf("open:%s\n", fixed)) == NULL) {
		g_set_error(&err, APP_SCITEPROJ_ERROR, -1, "%s: Error formatting Scite director command, g_strdup_printf() = NULL", __func__);
	}
	else {
		if (send_scite_command(command, &err)) {
			// Try to activate SciTE; ignore errors
			
			activate_scite(NULL);
			
			if (gPrefs.give_scite_focus==TRUE) {
				send_scite_command((gchar*)"focus:0",NULL);
			}
			
			add_file_to_recent(fixed,NULL);
			
			gchar *statusbar_text=NULL;
			
			statusbar_text=g_strdup_printf("Opened %s",remove_newline(get_filename_from_full_path(command)));
			
			set_statusbar_text(statusbar_text);
			
			g_free(statusbar_text);
		}
	}
	
//EXITPOINT:
	
	if (err != NULL) {
		dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Could not open selected file: \n\n%s", err->message);
		
		gtk_dialog_run(GTK_DIALOG (dialog));
	}
	
	if (relFilePath) g_free(relFilePath);
	if (absFilePath) g_free(absFilePath);
	if (command) g_free(command);
	if (err) g_error_free(err);
	if (dialog) gtk_widget_destroy(dialog);
	if (fixed) g_free(fixed);
}


/**
 *
 */
void popup_remove_recent_file_cb()
{
	gchar *command = NULL;
	GError *err = NULL;
	GtkWidget *dialog = NULL;
	gchar *absFilePath = NULL;
		
	// There are only files in this list
	
	if (!recent_clicked_node.valid || recent_clicked_node.type != ITEMTYPE_FILE) {
		goto EXITPOINT;
	}
	
	gchar *fileName = get_filename_from_full_path((gchar*)recent_clicked_node.name);
		
	remove_if_already_exists(fileName);
	
EXITPOINT:
	
	if (err != NULL) {
		dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Could not open selected file: \n\n%s", err->message);
		
		gtk_dialog_run(GTK_DIALOG (dialog));
	}
	
	if (command) g_free(command);
	if (absFilePath) g_free(absFilePath);
	if (err) g_error_free(err);
	if (dialog) gtk_widget_destroy(dialog);	
	
}


/**
 * Callback for the menu item
 */
void copy_recent_filename_to_clipboard_cb()
{
	gint selected_rows=0;
	gboolean multiple_selected=FALSE;
	
	GtkTreeSelection *treeSelect;
	
	treeSelect=gtk_tree_view_get_selection(GTK_TREE_VIEW(recentTreeView));
	
	selected_rows=gtk_tree_selection_count_selected_rows(treeSelect);
	if (selected_rows>1) {
		multiple_selected=TRUE;
	}
		
	if (!recent_clicked_node.valid || recent_clicked_node.type != ITEMTYPE_FILE) {
		//goto EXITPOINT;
	} else {
		copy_filename_to_clipboard(gtk_tree_view_get_model(GTK_TREE_VIEW(recentTreeView)),&(recent_clicked_node.iter));
		
	}
}


/**
 *
 */
void properties_recent_file_cb()
{
	gint selected_rows=0;
	gboolean multiple_selected=FALSE;
	
	GtkTreeSelection *treeSelect;
	
	treeSelect=gtk_tree_view_get_selection(GTK_TREE_VIEW(recentTreeView));
	
	selected_rows=gtk_tree_selection_count_selected_rows(treeSelect);
	if (selected_rows>1) {
		multiple_selected=TRUE;
	}
		
	if (!recent_clicked_node.valid || recent_clicked_node.type != ITEMTYPE_FILE) {
		//goto EXITPOINT;
	} else {
		file_properties_gui(gtk_tree_view_get_model(GTK_TREE_VIEW(recentTreeView)),&(recent_clicked_node.iter));
		
	}
}
