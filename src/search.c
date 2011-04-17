/**
 * search.c - Search dialog for searching the files in the SciteProj project
 *
 *  Copyright 2011 Andreas Ronnquist
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
	FILE_CONTENTS,
	COLUMNS
};


typedef struct _Data
{
	GAsyncQueue *queue;
	
	GList *search_list;
	
	GtkListStore *store;
	GtkWidget *search_button;
	GtkWidget *result_label;
	
	GdkCursor *busy_cursor;
	
	GtkWidget *search_string_entry;
	
	gchar *text_to_search_for;
	
	gboolean search_give_scite_focus;
	
	gboolean match_case;
	gboolean match_whole_words;
	
	gchar *error;
	
	sciteproj_prefs prefs;
	
	int number_of_results;
	
} Data;

typedef struct _Message
{
	gchar *filename;
	glong line_number;
	gchar *file_contents;
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
G_MODULE_EXPORT void close_button_pressed_cb(GtkWidget *widget,gpointer data);

static void search_button_clicked_cb(GtkButton *button,gpointer data);

static gpointer thread_func(Data *data);

static void cancel_search(gpointer data);


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
	
	data->prefs=gPrefs;

	// Make the dialog
	window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(window), 8);
	
	gtk_window_set_title(GTK_WINDOW(window),"Search project");
	
	// Set up size and position from prefs
	gtk_window_resize(GTK_WINDOW(window), gPrefs.search_width, gPrefs.search_height);
	
	if (gPrefs.search_xpos!=-1) {
		gtk_window_move(GTK_WINDOW(window),gPrefs.search_xpos,gPrefs.search_ypos);
	}

	
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
	
	data->number_of_results=0;
	
	data->search_button=gtk_button_new_from_stock(GTK_STOCK_FIND);
	
	gtk_box_pack_start(GTK_BOX(hbox),data->search_button,FALSE,FALSE,5);
	
	vbox=gtk_vbox_new(FALSE,8);
	
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
		
	
	check_button_box=gtk_hbox_new(FALSE,8);
	
	match_case_checkbutton=gtk_check_button_new_with_label("Match case");
	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(match_case_checkbutton),gPrefs.search_match_case);
	
	//gtk_table_attach(GTK_TABLE(table),match_case_checkbutton,0,2,1,2, GTK_SHRINK, GTK_SHRINK, 0,0);
	gtk_box_pack_start(GTK_BOX(check_button_box),match_case_checkbutton,FALSE,TRUE,5);
	
	
	match_whole_words_only_checkbutton=gtk_check_button_new_with_label("Match whole word only");
	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(match_whole_words_only_checkbutton),gPrefs.search_match_whole_words);
	
	//gtk_table_attach(GTK_TABLE(table),match_whole_words_only_checkbutton,2,4,1,2, GTK_SHRINK,GTK_SHRINK, 0,0);
	gtk_box_pack_start(GTK_BOX(check_button_box),match_whole_words_only_checkbutton,FALSE,TRUE,5);
	
	
	gtk_box_pack_start(GTK_BOX(vbox),check_button_box,FALSE,FALSE,0);
	
	
	
	treeview=gtk_tree_view_new();
	setup_tree_view(treeview);
	
	// init the list store
	data->store=gtk_list_store_new(COLUMNS,G_TYPE_STRING,G_TYPE_INT,G_TYPE_STRING);
	
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview),GTK_TREE_MODEL(data->store));
	g_object_unref(data->store);
	
	data->search_give_scite_focus=gPrefs.search_give_scite_focus;

	g_signal_connect(G_OBJECT(treeview), "row-activated", G_CALLBACK(tree_row_activated_cb), data);

	
	scrolled_win=gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	
	gtk_container_add(GTK_CONTAINER(scrolled_win),treeview);
	
	gtk_box_pack_start(GTK_BOX(vbox),scrolled_win,TRUE,TRUE,0);
	
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_win),
					    GTK_SHADOW_IN);
	
	close_button=gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	
	close_hbox=gtk_hbox_new(FALSE,8);
	
	data->result_label=gtk_label_new(NULL);
	
	gtk_box_pack_start(GTK_BOX(close_hbox),data->result_label,FALSE,FALSE,0);
	
	gtk_box_pack_end(GTK_BOX(close_hbox),close_button,FALSE,FALSE,5);
	
	gtk_box_pack_start(GTK_BOX(vbox),close_hbox,FALSE,FALSE,0);
	
	// add the table to the window
	gtk_container_add(GTK_CONTAINER(window),vbox);
			
	g_signal_connect(G_OBJECT(window), "destroy",G_CALLBACK (destroy_search_dialog_cb), &window);
	g_signal_connect(G_OBJECT(close_button),"clicked",G_CALLBACK(close_button_pressed_cb),data);
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
	
	msg=(Message*)g_async_queue_pop(data->queue);
	gtk_list_store_append(data->store,&iter);
		
	gtk_list_store_set(data->store,&iter,0,msg->filename,1,msg->line_number,2,msg->file_contents,-1);
	g_free(msg->filename);
	g_slice_free(Message,msg);
	
	return FALSE;
}

/**
 *
 */
gboolean whole_word_helper(gchar before,gchar after)
{
	gboolean result=TRUE;
	
	if (is_word_character(after)) {
		result=FALSE;
	} else {
		if (is_word_character(before)) {
			result=FALSE;
		}
	}
	
	return result;
}


/**
 *
 */
gboolean check_match(gchar *full_line,int begin_temp_string,gchar *tempString,gchar *text_to_search_for,gint length,gpointer user_data)
{
	gboolean result=FALSE;;
	Data *data=(Data*)(user_data);		
	
	gboolean whole_word=TRUE;
	
	if (data->match_case) {
		//printf("%s\n",data->text_to_search_for);
		
		if (!data->match_whole_words) {
		
			// match case, but not whole words
			if (strncmp(tempString,text_to_search_for,length)==0) result=TRUE;
		} else {
			
			// match the whole word, and also case
			if (strncmp(tempString,text_to_search_for,length)==0) {
				result=TRUE;
			
				// check if we are at the beginning of the string (full_line points to same location as
				// tempString
							
				if (full_line!=tempString) {
					
					gchar before=*(tempString-1);
					
					if (begin_temp_string+length>=((int)strlen(full_line))) {
						// we are at the end (no more characters after)
					} else {
						// there are more characters after 
						gchar after=*(tempString+length);
						whole_word=whole_word_helper(before,after);
					}
				}
			}
			if (!whole_word) result=FALSE;
		}
		
	} else {
		
		gchar *indep1=g_utf8_casefold(tempString,length);
		gchar *indep2=g_utf8_casefold(text_to_search_for,length);
		
		if (!data->match_whole_words) {
		
			// match whole word, but don't match case
			if (strncmp(indep1,indep2,length)==0) result=TRUE;
		} else {
			
			// match the whole word only, and don't match case
			if (strncmp(indep1,indep2,length)==0) {
				result=TRUE;
			
				// check if we are at the beginning of the string (full_line points to same location as
				// tempString
							
				if (full_line!=tempString) {
					
					gchar before=*(tempString-1);
					
					if (begin_temp_string+length>=((int)strlen(full_line))) {
						// we are at the end (no more characters after)
					} else {
						
						// there are more characters after 
						gchar after=*(tempString+length);
						whole_word=whole_word_helper(before,after);
					}
				}
			}
			if (!whole_word) result=FALSE;
		}
	}
	
	return result;
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
	
	GList *file_error_list=NULL;
	
	gint counter=0;
	int line_number;
	
	char line[512];
	int co;
	
	GList *search_list=data->search_list;
	GList *saved_list=search_list;
	
	g_async_queue_ref(data->queue);
	
	while(g_atomic_int_get(&threaded_flag) && do_work && search_list!=NULL) {
		msg=g_slice_new(Message);
		
		msg->filename=NULL;
		msg->file_contents=NULL;
		
		if (search_list!=NULL) {
			
			if ((gchar*)(search_list->data)!=NULL) {
				
				File *file=(File*)(search_list->data);
				
				gchar *filename=(gchar*)(file->full_path);
				
				// convert to a full path
				if (!relative_path_to_abs_path(filename, &absFilePath, get_project_directory(), &err)) {
					goto EXITPOINT;
				}
				
				// Create a list to store the found elements.
				GList *result_list=NULL;
				// Open the file
				FILE *file_read;
				file_read=fopen((const char*)absFilePath,"r");
				
				if (file_read==NULL) {
					gchar *temp;
					temp=g_strdup_printf("%s",filename);
					file_error_list=g_list_prepend(file_error_list,(gpointer)(temp));
					
				} else {
					
					line_number=0;
					
					while(fgets(line,512,file_read)!=NULL) {
						
						char *tempString=line;
						line_number++;
						
						for (co=0;co<strlen(line);co++) {
							int length=strlen(data->text_to_search_for);
							
							gboolean text_found=check_match(line,co,tempString,data->text_to_search_for,length,data);
							
							// text found!
							if (text_found) {
								Message *tempMessage=g_slice_new(Message);
								
								tempMessage->line_number=line_number;
								tempMessage->filename=g_strdup_printf(filename);
								tempMessage->file_contents=g_strdup_printf("%s",remove_newline(line));
								
								//result_list=g_list_insert_sorted(result_list,(gpointer)(tempMessage),insert_sorted_func);
								result_list=g_list_append(result_list,(gpointer)tempMessage);
								
								data->number_of_results++;
								
								text_found=FALSE;
							
							}
							tempString++;
						}
					}
					
					fclose(file_read);
					
					// Now that we have the results for the current file in the list, go
					// through that list, and add the results to the tree
					GList *iter=result_list;
		
					while ((iter) && (g_atomic_int_get(&threaded_flag))) {
			
						Message *msg=(Message*)iter->data;
			
						//printf("N:%s compared to %s\n",data->full_path,full_path);
			
						//printf("%s%ld\n",msg->filename,msg->line_number);
						if (msg->filename!=NULL) {
					
							g_async_queue_push(data->queue,msg);
							g_idle_add((GSourceFunc)update_tree,data);
					
							sleep(0);
						}	
			
						iter=g_list_next(iter);
					}
					
					msg->line_number=counter;
					counter++;
					
				}
				// go to the next element in the list
				search_list=g_list_next(search_list);
				
				g_free(absFilePath);
				
				// final element?
				if (!search_list) do_work=FALSE;
			}
		} 
	}
	
	g_async_queue_unref(data->queue);
	
	if (g_atomic_int_get(&threaded_flag)) {
		g_idle_add((GSourceFunc)cancel_search,data);
	}
	
	data->search_list=saved_list;
	
	if (file_error_list!=NULL) {
		
		gchar *error_message=NULL;
		data->error=NULL;
		
		error_message=g_strdup_printf("There was problems opening the following files in the project:\n\n");
					
		while((file_error_list)) {
			
			gchar *temp=(gchar*)file_error_list->data;
			
			gchar *temp2=g_strconcat(error_message,temp,", \n",NULL);
			error_message=temp2;
			g_free(temp);
			
			file_error_list=g_list_next(file_error_list);
		}
		
		gchar *temp2=g_strconcat(error_message,"\nThey couldn't be opened for reading in the search.\n",NULL);
		
		g_free(error_message);
		error_message=temp2;
				
		if (error_message!=NULL) 
		{
			data->error=g_strdup(error_message);
			g_free(error_message);
		}
	}
	
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
		
			data->number_of_results=0;
			
			// get the text from the search string entry
			data->text_to_search_for=(gchar*)(gtk_entry_get_text(GTK_ENTRY(data->search_string_entry)));
			
			data->match_case=(gboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(match_case_checkbutton));
			data->match_whole_words=(gboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(match_whole_words_only_checkbutton));
					
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
		case GDK_Escape:
		{
			gtk_widget_destroy(GTK_WIDGET(window));
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
	
		
	// Get the data from the row that was activated
	// We need to do this before launching Scite - because if SciTE wasn't ran before, we need to 
	// provide the line number to SciTE in the launching command.
	gchar *temppath;
	
	treeModel = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
	gtk_tree_model_get_iter(treeModel, &iter, path);
	gtk_tree_model_get(treeModel, &iter, /*COLUMN_ITEMTYPE, &nodeItemType,*/ FILENAME, &temppath, LINENUMBER, &line_number, -1);
	
	// convert to a full path
	if (!relative_path_to_abs_path(temppath, &absFilePath, get_project_directory(), &err)) {
		printf("Error:%s\n",err->message);
		goto EXITPOINT;
	}

	
	// Launch scite if it isn't already launched
	if (!scite_ready()) {
		
		gchar *run_options;
		
		run_options=g_strdup_printf("%s -goto:%d",absFilePath,line_number);
		
		if (!launch_scite(run_options,&err)) {
			
			printf("Error:%s\n",err->message);
			
			//goto EXITPOINT;
			
			/*
			gchar *errorstring=g_strdup_printf("Couldn't launch scite: %s",newerr->message);
			
			GtkWidget *errDialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, errorstring);
		
			gtk_dialog_run(GTK_DIALOG(errDialog));
			
			g_free(errorstring);
		
			gtk_widget_destroy(errDialog);
			
			return;
			*/
			
			goto EXITPOINT;
			
		}
	}
	
	// It's a file, so try to open it
	
	if ((command = g_strdup_printf("open:%s\n", absFilePath)) == NULL) {
		g_set_error(&err, APP_SCITEPROJ_ERROR, -1, "%s: Error formatting Scite director command, g_strdup_printf() = NULL", __func__);
		goto EXITPOINT;
	}
	else {
		GError *err;
		if (send_scite_command(command, &err)) {
			// Try to activate SciTE; ignore errors
			
			activate_scite(NULL);
			
			if (data->search_give_scite_focus) {
				
				GError *err=NULL;
				send_scite_command((gchar*)"focus:1",&err);
			}
			
			//GError *err;
			
			gchar *statusbar_text=NULL;
			
			statusbar_text=g_strdup_printf("Opened %s",remove_newline(get_filename_from_full_path(command)));
			
			set_statusbar_text(statusbar_text);
			
			g_free(statusbar_text);
		} else {

			if (err) {
				printf("Error:%s\n",err->message);
			}
			
		}
	}
	
	debug_printf("Go to the line number...\n");
	
	// go to the right line number:
	if ((command = g_strdup_printf("goto:%d\n", line_number)) == NULL) {
		g_set_error(&err, APP_SCITEPROJ_ERROR, -1, "%s: Error formatting Scite director command, g_strdup_printf() = NULL", __func__);
	}
	else {
		
				
		if (send_scite_command(command, &err)) {
			// Try to activate SciTE; ignore errors
			
			debug_printf("Send the commend worked!\n");
			
			if (data->search_give_scite_focus!=FALSE) {
				send_scite_command((gchar*)"focus:0",NULL);
			}
						
			//activate_scite(NULL);
			
			//GError *err;
			
			gchar *statusbar_text=NULL;
			
			statusbar_text=g_strdup_printf("Opened %s",remove_newline(get_filename_from_full_path(command)));
			
			set_statusbar_text(statusbar_text);
			
			g_free(statusbar_text);
		} else {
			
			printf("goto didn't work...\n");
			if (err) {
				printf("Error:%s\n",err->message);
			}
			
		}
	}
	
	debug_printf("Set scite focus...\n");
	
	if (gPrefs.search_give_scite_focus) {
		send_scite_command((gchar*)"focus:1",NULL);
		/*
		GError *err=NULL;
		activate_scite(&err);
		
		if (err) {
			printf("focus\n");
			printf("Error:%s\n",err->message);
		}
		*/
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
	
	renderer=gtk_cell_renderer_text_new();
	column=gtk_tree_view_column_new_with_attributes("File contents",renderer,"text",FILE_CONTENTS,NULL);
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
 * stop_search
 */
static void stop_search(gpointer user_data)
{
	Data *data=(Data*)(user_data);
	
	if (is_searching==1) {
		is_searching=0;
		
		g_atomic_int_set(&threaded_flag,0);
		if (thread!=NULL) g_thread_join(thread);
		
		g_async_queue_unref(data->queue);
		
		gtk_widget_set_sensitive(GTK_WIDGET(data->search_button),TRUE);
		
		gdk_window_set_cursor(gtk_widget_get_window(window),NULL);
		
		// If we have error results from the search - open a dialog, and show it
		if (data->error!=NULL) {
			
			if (data->prefs.search_alert_file_warnings) {
			
				GtkWidget *warningDialog = gtk_message_dialog_new(NULL,
					GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,"%s",data->error);
				gtk_dialog_run(GTK_DIALOG(warningDialog));
				gtk_widget_destroy(warningDialog);
			}
		
		}

		gchar *string=g_strdup_printf("Found %d elements",data->number_of_results);
			
		gtk_label_set_text(GTK_LABEL(data->result_label),string);
		
		g_free(string);
		
		// Give a dialog if the search didn't find anything.
		if (data->number_of_results==0) {
			GtkWidget *dialog=gtk_message_dialog_new(NULL,
				GTK_DIALOG_MODAL,GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,"The search didn't give any results.");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
		}
	}
}


/**
 *
 */
static void cancel_search(gpointer data)
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
G_MODULE_EXPORT void close_button_pressed_cb(GtkWidget *widget,gpointer user_data)
{
	Data *data=(Data*)user_data;
	
	stop_search(data);
	
	// close the window (calls the destroy_search_dialog_cb function)
	gtk_widget_destroy(GTK_WIDGET(window));
}
