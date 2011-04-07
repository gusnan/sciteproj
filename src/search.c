/**
 * search.c - Search dialog for searching the files in the SciteProj project
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
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "search.h"
#include "tree_manipulation.h"
#include "graphics.h"
#include "string_utils.h"
#include "scite_utils.h"
#include "statusbar.h"
#include "prefs.h"



#define APP_SCITEPROJ_ERROR g_quark_from_static_string("APP_SEARCH_ERROR")



/**
 *
 */
enum
{
	FILENAME=0,
	LINENUMBER,
	COLUMNS
};

/**
 *
 */
GtkWidget *window;
gboolean is_searching=FALSE;
GtkWidget *search_string_entry=NULL;
	
GtkListStore *store;
GtkWidget *treeview;
GtkWidget *search_button=NULL;
GtkWidget *match_case_checkbutton=NULL;
GtkWidget *match_whole_words_only_checkbutton=NULL;


static void setup_tree_view(GtkWidget *treeview);
static void search_button_clicked(GtkButton *button, gpointer data);
static void tree_row_activated_cb(GtkTreeView *treeView, GtkTreePath *path, GtkTreeViewColumn *column, gpointer userData);

void dialog_response(GtkDialog *dialog,gint response_id,gpointer user_data);

gboolean search_key_press_cb(GtkWidget *widget, GdkEventKey *event, gpointer userData);



/**
 *
 */
void search_dialog_cb()
{
	search_dialog();
}

/**
 *
 */
void search_dialog()
{
	GtkWidget *find_label;
	GtkWidget *search_text_entry;
	GtkWidget *search_button;
	
	GtkWidget *close_button;
	GtkWidget *close_hbox;
	
	GtkWidget *check_button_box;

	GtkWidget *scrolled_win;
	
	GtkWidget *hbox;
	GtkWidget *vbox;

	// Make the dialog
	window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(window), 8);
	
	gtk_widget_set_size_request(window,500,400);
	
	hbox=gtk_hbox_new(FALSE,8);
	
	find_label=gtk_label_new("Find What:");
	gtk_box_pack_start(GTK_BOX(hbox),find_label,FALSE,TRUE,5);
		
	search_text_entry=gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(search_text_entry),TRUE);
	
	gtk_box_pack_start(GTK_BOX(hbox),search_text_entry,TRUE,TRUE,5);
	
	/*
	GtkBox *hbox=gtk_hbox_new(TRUE,0);
	
	gtk_box_pack_end(hbox,search_text_entry,TRUE,TRUE,0);
	*/
	
	search_button=gtk_button_new_with_label("Search");
	
	gtk_box_pack_start(GTK_BOX(hbox),search_button,FALSE,FALSE,5);
	
	vbox=gtk_vbox_new(FALSE,8);
	
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
		
	
	check_button_box=gtk_hbox_new(FALSE,8);
	
	match_case_checkbutton=gtk_check_button_new_with_label("Match case");
	
	//gtk_table_attach(GTK_TABLE(table),match_case_checkbutton,0,2,1,2, GTK_SHRINK, GTK_SHRINK, 0,0);
	gtk_box_pack_start(GTK_BOX(check_button_box),match_case_checkbutton,FALSE,TRUE,5);
	
	match_whole_words_only_checkbutton=gtk_check_button_new_with_label("Match whole word");
	
	//gtk_table_attach(GTK_TABLE(table),match_whole_words_only_checkbutton,2,4,1,2, GTK_SHRINK,GTK_SHRINK, 0,0);
	gtk_box_pack_start(GTK_BOX(check_button_box),match_whole_words_only_checkbutton,FALSE,TRUE,5);
	
	gtk_box_pack_start(GTK_BOX(vbox),check_button_box,FALSE,FALSE,0);
	
	
	
	treeview=gtk_tree_view_new();
	setup_tree_view(treeview);
	
	// init the list store
	store=gtk_list_store_new(COLUMNS,G_TYPE_STRING,G_TYPE_INT);
	
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview),GTK_TREE_MODEL(store));
	g_object_unref(store);
	
	g_signal_connect(G_OBJECT(treeview), "row-activated", G_CALLBACK(tree_row_activated_cb), NULL);

	
	scrolled_win=gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	
	gtk_container_add(GTK_CONTAINER(scrolled_win),treeview);
	
	gtk_box_pack_start(GTK_BOX(vbox),scrolled_win,TRUE,TRUE,0);
	
	
	close_button=gtk_button_new_with_label("Close");
	
	close_hbox=gtk_hbox_new(FALSE,8);
	gtk_box_pack_end(GTK_BOX(close_hbox),close_button,FALSE,FALSE,5);
	
	gtk_box_pack_start(GTK_BOX(vbox),close_hbox,FALSE,FALSE,0);
	
	// add the table to the window
	gtk_container_add(GTK_CONTAINER(window),vbox);
	
	gtk_widget_show_all(window);
}

/**
 *
 */
static void search_button_clicked(GtkButton *button,gpointer data)
{
	printf("Search...\n");
}

/**
 *
 */
gboolean search_key_press_cb(GtkWidget *widget, GdkEventKey *event, gpointer userData)
{
	switch (event->keyval) 
	{
		case GDK_Return:
		{
			//g_print((gchar*)"key_press_cb: keyval = %d = GDK_Return, hardware_keycode = %d\n", event->keyval, event->hardware_keycode);
			
			search_button_clicked(GTK_BUTTON(search_button),NULL);
			break;
		}
	}

/*	
	if (event->state & GDK_SHIFT_MASK) debug_printf(", GDK_SHIFT_MASK");
	if (event->state & GDK_CONTROL_MASK) debug_printf(", GDK_CONTROL_MASK");
	if (event->state & GDK_MOD1_MASK) debug_printf(", GDK_MOD1_MASK");
	if (event->state & GDK_MOD2_MASK) debug_printf(", GDK_MOD2_MASK");
	if (event->state & GDK_MOD3_MASK) debug_printf(", GDK_MOD3_MASK");
	if (event->state & GDK_MOD4_MASK) debug_printf(", GDK_MOD4_MASK");
	if (event->state & GDK_MOD5_MASK) debug_printf(", GDK_MOD5_MASK");
	
	debug_printf("\n");
	*/
	return FALSE;
}


/**
 * Callback handler for Gtk "row-activated" event.
 *
 * @param treeView is the GtkTreeView
 * @param path is the GtkTreePath of the activated row
 * @param column is not used
 * @param userData is not used
 */
static void tree_row_activated_cb(GtkTreeView *treeView, GtkTreePath *path, GtkTreeViewColumn *column, gpointer userData)
{
	GtkTreeModel *treeModel = NULL;
	GtkTreeIter iter;
	gchar *relFilePath = NULL;
	gchar *absFilePath = NULL;
	gchar *command = NULL;
	GError *err = NULL;
	GtkWidget *dialog = NULL;
	//gint nodeItemType;
	
	gint line_number=-1;
	
	// Launch scite if it isn't already launched
	if (!scite_ready()) {
		if (!launch_scite("",&err)) {
			
			//goto EXITPOINT;
			
			/*
			gchar *errorstring=g_strdup_printf("Couldn't launch scite: %s",newerr->message);
			
			GtkWidget *errDialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, errorstring);
		
			gtk_dialog_run(GTK_DIALOG(errDialog));
			
			g_free(errorstring);
		
			gtk_widget_destroy(errDialog);
			
			return;
			*/
			
		}
	}
	
	// Get the data from the row that was activated
	
	treeModel = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
	gtk_tree_model_get_iter(treeModel, &iter, path);
	gtk_tree_model_get(treeModel, &iter, /*COLUMN_ITEMTYPE, &nodeItemType,*/ FILENAME, &absFilePath, LINENUMBER, &line_number, -1);
	
	debug_printf("Path:%s, line:%d\n",absFilePath,line_number);
	
	
	
	// It's a file, so try to open it
	
	if ((command = g_strdup_printf("open:%s\n", absFilePath)) == NULL) {
		g_set_error(&err, APP_SCITEPROJ_ERROR, -1, "%s: Error formatting Scite director command, g_strdup_printf() = NULL", __func__);
	}
	else {
		if (send_scite_command(command, &err)) {
			// Try to activate SciTE; ignore errors
			
			activate_scite(NULL);
			
			if (gPrefs.give_scite_focus==TRUE) {
				send_scite_command((gchar*)"focus:0",NULL);
			}
			
			//GError *err;
			
			gchar *statusbar_text=NULL;
			
			statusbar_text=g_strdup_printf("Opened %s",remove_newline(get_filename_from_full_path(command)));
			
			set_statusbar_text(statusbar_text);
			
			g_free(statusbar_text);
		}
	}
	
	debug_printf("Next?");
	
	// go to the right line number:
	if ((command = g_strdup_printf("goto:%d\n", line_number)) == NULL) {
		g_set_error(&err, APP_SCITEPROJ_ERROR, -1, "%s: Error formatting Scite director command, g_strdup_printf() = NULL", __func__);
	}
	else {
		if (send_scite_command(command, &err)) {
			// Try to activate SciTE; ignore errors
			
			activate_scite(NULL);
			
			//if (gPrefs.give_scite_focus==TRUE) {
				send_scite_command((gchar*)"focus:0",NULL);
			//}
			
			//GError *err;
			
			gchar *statusbar_text=NULL;
			
			statusbar_text=g_strdup_printf("Opened %s",remove_newline(get_filename_from_full_path(command)));
			
			set_statusbar_text(statusbar_text);
			
			g_free(statusbar_text);
		}
	}
	
	if (err != NULL) {
		dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Could not open selected file: \n\n%s", err->message);
		
		gtk_dialog_run(GTK_DIALOG (dialog));
	}
	
	if (relFilePath) g_free(relFilePath);
	if (absFilePath) g_free(absFilePath);
	if (command) g_free(command);
	if (err) g_error_free(err);
	if (dialog) gtk_widget_destroy(dialog);
}


/**
 *
 */
static void setup_tree_view(GtkWidget *treeview)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	
	renderer=gtk_cell_renderer_text_new();
	column=gtk_tree_view_column_new_with_attributes("Filename",renderer,"text",FILENAME,NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview),column);
	
	
	renderer=gtk_cell_renderer_text_new();
	column=gtk_tree_view_column_new_with_attributes("Line Number",renderer,"text",LINENUMBER,NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview),column);
}


/**
 *
 */
void dialog_response(GtkDialog *dialog, gint response_id,gpointer user_data)
{
	if (response_id==GTK_RESPONSE_CLOSE) {
		// Close all (threads and everything)
		
		gtk_widget_destroy(GTK_WIDGET(dialog));
	}
	
	//debug_printf("Response:%d\n",response_id);
}

