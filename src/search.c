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
#include <unistd.h>
#include <string.h>

#include "search.h"
#include "tree_manipulation.h"
#include "graphics.h"
#include "string_utils.h"
#include "scite_utils.h"
#include "statusbar.h"
#include "prefs.h"

#include "filelist.h"

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


typedef struct _Data
{
	GAsyncQueue *queue;
	
	GList *search_list;
	
	GtkListStore *store;
	GtkWidget *search_button;
	
	GdkCursor *busy_cursor;
	
	GtkWidget *search_string_entry;
	
	gchar *text_to_search_for;
	
	gboolean search_give_scite_focus;
	
} Data;

typedef struct _Message
{
	gchar *filename;
	glong line_number;
} Message;

/**
 *
 */

GtkWidget *window;
gboolean is_searching=FALSE;

GtkWidget *treeview;
GtkWidget *match_case_checkbutton=NULL;
GtkWidget *match_whole_words_only_checkbutton=NULL;


GdkCursor *standard_cursor;

static GThread *thread=NULL;

static gint threaded_flag;

static gboolean search_dialog_open=FALSE;

/**
 *
 */
static void setup_tree_view(GtkWidget *treeview);
static void tree_row_activated_cb(GtkTreeView *treeView, GtkTreePath *path, GtkTreeViewColumn *column, gpointer userData);

void dialog_response(GtkDialog *dialog,gint response_id,gpointer user_data);

gboolean search_key_press_cb(GtkWidget *widget, GdkEventKey *event, gpointer userData);

static void destroy_search_dialog_cb(GtkWidget *widget,GdkEvent *event,gpointer data);
static void close_button_pressed_cb(GtkWidget *widget,gpointer data);

static void search_button_clicked_cb(GtkButton *button,gpointer data);

static gpointer thread_func(Data *data);

void cancel_search(gpointer data);


/**
 *
 */
void search_dialog_cb()
{
	if (!search_dialog_open) {
		search_dialog();
		search_dialog_open=TRUE;
	} else {
		gtk_window_present(GTK_WINDOW(window));
	}
}

/**
 *
 */
void search_dialog()
{
	GtkWidget *find_label;
	
	GtkWidget *close_button;
	GtkWidget *close_hbox;
	
	GtkWidget *check_button_box;

	GtkWidget *scrolled_win;
	
	GtkWidget *hbox;
	GtkWidget *vbox;
	
	Data *data;
	
	data=g_slice_new(Data);
	
	data->search_list=list;

	// Make the dialog
	window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(window), 8);
	
	gtk_window_set_title(GTK_WINDOW(window),"Search project");
	
	gtk_widget_set_size_request(window,500,400);
	
	// Create the busy cursor
	GdkDisplay *display=gdk_display_get_default();
	data->busy_cursor=gdk_cursor_new_for_display(display,GDK_WATCH);
	
	hbox=gtk_hbox_new(FALSE,8);
	
	find_label=gtk_label_new("Find What:");
	gtk_box_pack_start(GTK_BOX(hbox),find_label,FALSE,TRUE,5);
		
	data->search_string_entry=gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(data->search_string_entry),TRUE);
	
	gtk_box_pack_start(GTK_BOX(hbox),data->search_string_entry,TRUE,TRUE,5);
	
	/*
	GtkBox *hbox=gtk_hbox_new(TRUE,0);
	
	gtk_box_pack_end(hbox,search_text_entry,TRUE,TRUE,0);
	*/
	
	data->search_button=gtk_button_new_from_stock(GTK_STOCK_FIND);
	
	gtk_box_pack_start(GTK_BOX(hbox),data->search_button,FALSE,FALSE,5);
	
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
	data->store=gtk_list_store_new(COLUMNS,G_TYPE_STRING,G_TYPE_INT);
	
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview),GTK_TREE_MODEL(data->store));
	g_object_unref(data->store);
	
	data->search_give_scite_focus=gPrefs.search_give_scite_focus;

	g_signal_connect(G_OBJECT(treeview), "row-activated", G_CALLBACK(tree_row_activated_cb), data);

	
	scrolled_win=gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	
	gtk_container_add(GTK_CONTAINER(scrolled_win),treeview);
	
	gtk_box_pack_start(GTK_BOX(vbox),scrolled_win,TRUE,TRUE,0);
	
	
	close_button=gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	
	close_hbox=gtk_hbox_new(FALSE,8);
	gtk_box_pack_end(GTK_BOX(close_hbox),close_button,FALSE,FALSE,5);
	
	gtk_box_pack_start(GTK_BOX(vbox),close_hbox,FALSE,FALSE,0);
	
	// add the table to the window
	gtk_container_add(GTK_CONTAINER(window),vbox);
			
	g_signal_connect(G_OBJECT(window), "destroy",G_CALLBACK (destroy_search_dialog_cb), &window);
	g_signal_connect(G_OBJECT(close_button),"clicked",G_CALLBACK(close_button_pressed_cb),NULL);
	g_signal_connect(G_OBJECT(data->search_button),"clicked",G_CALLBACK(search_button_clicked_cb),data);
	g_signal_connect(G_OBJECT(window),"key-press-event",G_CALLBACK(search_key_press_cb),data);
	
	gtk_widget_show_all(window);
}

/**
 *
 */
static gboolean update_tree(Data *data)
{
	GtkTreeIter iter;
	Message *msg;
	
	if (threaded_flag!=0) {
		msg=(Message*)g_async_queue_pop(data->queue);
		gtk_list_store_append(data->store,&iter);
		
		gtk_list_store_set(data->store,&iter,0,msg->filename,1,msg->line_number,-1);
		g_free(msg->filename);
		g_slice_free(Message,msg);
	}
	return FALSE;
}

/**
 *
 */
static gpointer thread_func(Data *data)
{
	gboolean do_work=TRUE;
	
	Message *msg;
	GError *err;
	
	gchar *absFilePath;
	
	gint counter=0;
	int line_number;
	
	char line[256];
	int co;
	
	GList *list=data->search_list;
	GList *saved_list=list;
	
	g_async_queue_ref(data->queue);
	
	while(g_atomic_int_get(&threaded_flag) && do_work && list!=NULL) {
		msg=g_slice_new(Message);
		
		msg->filename=NULL;
		if (list!=NULL) {
			
			if ((gchar*)(list->data)!=NULL) {
				
				File *file=(File*)(list->data);
				
				gchar *filename=(gchar*)(file->full_path);
				
				//msg->filename=g_strdup_printf("File: %d %s - '%s'",counter,(gchar*)(file->filename),(gchar*)(data->text_to_search_for));
				
				// do your stuff for the file
				
				// convert to a full path
				if (!relative_path_to_abs_path(filename, &absFilePath, get_project_directory(), &err)) {
					goto EXITPOINT;
				}
				
				// Create a list to store the found elements.
				GList *result_list=NULL;
				
				// Open the file
				FILE *file_read;
				file_read=fopen(absFilePath,"rt");
				
				line_number=0;
				
				while(fgets(line,256,file_read)!=NULL) {
					
					char *tempString=line;
					line_number++;

					for (co=0;co<strlen(line);co++) {
						int length=strlen(data->text_to_search_for);
						
						if (strncmp(tempString,data->text_to_search_for,length)==0) {
							// text found!
							
							Message *tempMessage=g_slice_new(Message);
							
							tempMessage->line_number=line_number;
							tempMessage->filename=g_strdup_printf(filename);
							
							result_list=g_list_append(result_list,(gpointer)(tempMessage));
						
						}
						tempString++;
					}
				}
				
				fclose(file_read);
				
				// Now that we have the results for the current file in the list, go
				// through that list, and add the results to the tree
				GList *iter=result_list;
	
				while(iter) {
		
					Message *msg=(Message*)iter->data;
		
					//printf("N:%s compared to %s\n",data->full_path,full_path);
		
					//printf("%s%ld\n",msg->filename,msg->line_number);
					if (msg->filename!=NULL) {
				
						g_async_queue_push(data->queue,msg);
						g_idle_add((GSourceFunc)update_tree,data);
				
						sleep(0);
					}	
		
					iter=iter->next;
				}
				
				msg->line_number=counter;
				counter++;
				
				// go to the next element in the list
				list=g_list_next(list);
				
				// final element?
				if (!list) do_work=FALSE;
					
			}
		} 
	}
	
	g_async_queue_unref(data->queue);
	
	if (g_atomic_int_get(&threaded_flag)) {
		g_idle_add((GSourceFunc)cancel_search,data);
	}
	
	data->search_list=saved_list;
	
EXITPOINT:	
	
	return NULL;
}

/**
 *
 */
static void search_button_clicked_cb(GtkButton *button,gpointer user_data)
{
	Data *data=(Data*)user_data;
	
	// clear the list
	gtk_list_store_clear(data->store);
	
	if (!is_searching) {
		
		if (gtk_entry_get_text_length(GTK_ENTRY(data->search_string_entry))!=0) {
		
			
			// get the text from the search string entry
			data->text_to_search_for=(gchar*)(gtk_entry_get_text(GTK_ENTRY(data->search_string_entry)));
					
			gtk_widget_set_sensitive(GTK_WIDGET(data->search_button),FALSE);
			
			g_atomic_int_set(&threaded_flag,1);
			
			data->queue=g_async_queue_new();
			thread=g_thread_create((GThreadFunc)thread_func,data,TRUE,NULL);
			
			is_searching=TRUE;
			
			// set the mouse cursor
			gdk_window_set_cursor(gtk_widget_get_window(window),data->busy_cursor);
		} else {
			// string to search for has a length of zero.
			GtkWidget *warningDialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, 
				"Please enter a text to search for!\n"
				);
			gtk_dialog_run(GTK_DIALOG(warningDialog));
			gtk_widget_destroy(warningDialog);
			
		}
	}
}

/**
 *
 */
gboolean search_key_press_cb(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	Data *data=(Data*)(user_data);
	
	switch (event->keyval) 
	{
		// Check for both return and Keypad enter
		case GDK_Return:
		case GDK_KP_Enter:
		{
			//g_print((gchar*)"key_press_cb: keyval = %d = GDK_Return, hardware_keycode = %d\n", event->keyval, event->hardware_keycode);
			
			if (data->search_string_entry!=NULL) {
				
				data->text_to_search_for=(gchar*)gtk_entry_get_text(GTK_ENTRY(data->search_string_entry));
				
				search_button_clicked_cb(GTK_BUTTON(data->search_button),user_data);
			}
			
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
static void tree_row_activated_cb(GtkTreeView *treeView, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data)
{
	GtkTreeModel *treeModel = NULL;
	GtkTreeIter iter;
	gchar *relFilePath = NULL;
	gchar *absFilePath = NULL;
	gchar *command = NULL;
	GError *err = NULL;
	GtkWidget *dialog = NULL;
	//gint nodeItemType;
	
	Data *data=(Data*)(user_data);
	
	gint line_number=-1;
	
	printf("Tree_row..\n");
	
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
	
	gchar *temppath;
	
	treeModel = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
	gtk_tree_model_get_iter(treeModel, &iter, path);
	gtk_tree_model_get(treeModel, &iter, /*COLUMN_ITEMTYPE, &nodeItemType,*/ FILENAME, &temppath, LINENUMBER, &line_number, -1);
	
	// convert to a full path
	if (!relative_path_to_abs_path(temppath, &absFilePath, get_project_directory(), &err)) {
		goto EXITPOINT;
	}
	
	// It's a file, so try to open it
	
	if ((command = g_strdup_printf("open:%s\n", absFilePath)) == NULL) {
		g_set_error(&err, APP_SCITEPROJ_ERROR, -1, "%s: Error formatting Scite director command, g_strdup_printf() = NULL", __func__);
	}
	else {
		if (send_scite_command(command, &err)) {
			// Try to activate SciTE; ignore errors
			
			activate_scite(NULL);
			
			if (data->search_give_scite_focus) {
				
				GError *err;
				send_scite_command((gchar*)"focus:1",&err);
			}
			
			//GError *err;
			
			gchar *statusbar_text=NULL;
			
			statusbar_text=g_strdup_printf("Opened %s",remove_newline(get_filename_from_full_path(command)));
			
			set_statusbar_text(statusbar_text);
			
			g_free(statusbar_text);
		}
	}
	
	// go to the right line number:
	if ((command = g_strdup_printf("goto:%d\n", line_number)) == NULL) {
		g_set_error(&err, APP_SCITEPROJ_ERROR, -1, "%s: Error formatting Scite director command, g_strdup_printf() = NULL", __func__);
	}
	else {
		if (send_scite_command(command, &err)) {
			// Try to activate SciTE; ignore errors
			
			activate_scite(NULL);
			
			if (data->search_give_scite_focus!=FALSE) {
				send_scite_command((gchar*)"focus:0",NULL);
			}
			
			//GError *err;
			
			gchar *statusbar_text=NULL;
			
			statusbar_text=g_strdup_printf("Opened %s",remove_newline(get_filename_from_full_path(command)));
			
			set_statusbar_text(statusbar_text);
			
			g_free(statusbar_text);
		}
	}
	
	printf("krooth\n");
	if (gPrefs.search_give_scite_focus) {
		printf("Bah!\n");
		send_scite_command((gchar*)"focus:1",NULL);
		GError *err;
		activate_scite(&err);
	}
			

	
	if (err != NULL) {
		dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Could not open selected file: \n\n%s", err->message);
		
		gtk_dialog_run(GTK_DIALOG (dialog));
	}
EXITPOINT:
	
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
	column=gtk_tree_view_column_new_with_attributes("Line",renderer,"text",LINENUMBER,NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview),column);
}


/**
 *
 */
void dialog_response(GtkDialog *dialog, gint response_id,gpointer user_data)
{
	if (response_id==GTK_RESPONSE_CLOSE) {
		// Close all (threads and everything)
		
		gtk_widget_destroy(GTK_WIDGET(window));
	}
}

/**
 *
 */
void stop_search(gpointer user_data)
{
	Data *data=(Data*)(user_data);
	
	if (is_searching) {
		is_searching=FALSE;
		
		gtk_widget_set_sensitive(GTK_WIDGET(data->search_button),TRUE);
		
		gdk_window_set_cursor(gtk_widget_get_window(window),NULL);
	}
}

/**
 *
 */
void cancel_search(gpointer data)
{
	stop_search((Data*)data);
}

/**
 *
 */
static void destroy_search_dialog_cb(GtkWidget *widget,GdkEvent *event,gpointer data)
{
	stop_search(data);
	search_dialog_open=FALSE;
}

/**
 *
 */
static void close_button_pressed_cb(GtkWidget *widget,gpointer user_data)
{
	Data *data=(Data*)user_data;
	
	stop_search(data);
	
	// close the window (calls the destroy_search_dialog_cb function)
	gtk_widget_destroy(GTK_WIDGET(window));
}

