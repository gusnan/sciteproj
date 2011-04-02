/**
 * about.c - about dialog for SciteProj
 *
 *	 Copyright 2008-2011 Andreas Ronnquist
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

#include <glib.h> 
#include <gtk/gtk.h>

#include "about.h"

#include "graphics.h"


static gchar *sLicense =	(gchar*)"SciteProj is free software: you can redistribute it and/or modify\n"
											  "it under the terms of the GNU General Public License as published by\n"
											  "the Free Software Foundation, either version 3 of the License, or\n"
											  "(at your option) any later version.\n"
											  "\n"
											  "SciteProj is distributed in the hope that it will be useful,\n"
											  "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
											  "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
											  "GNU General Public License for more details.\n"
											  "\n"
											  "You should have received a copy of the GNU General Public License\n"
											  "along with SciteProj.  If not, see <http://www.gnu.org/licenses/>.\n";


gchar *homepage_string=(gchar*)"http://sciteproj.gusnan.se";

gchar *sVersion = (gchar*)"0.4.01";

static GtkWidget *window;

void create_about_dialog();

/**
 * show_about_dialog
 * Shows the about dialog, and if it isnt already made, creates it
 */
void show_about_dialog()
{
	if (!window) 
		create_about_dialog();
	else
		gtk_window_present(GTK_WINDOW(window));
}

/**
 *
 */
void create_about_dialog()
{
	GtkWidget *vbox;
	GtkWidget *textview;
	GtkWidget *logo_image;
	GtkWidget *linkbutton;
	GtkWidget *ok_button;
	GtkTextBuffer *textbuffer;
	GtkTextIter iter;
	
	gchar *copyrightstring;
	
	GtkWidget *label;
	
	
	// Make the dialog
	window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(window), 8);
	
	gtk_widget_set_size_request(window,500,400);
	
	// Make a container
	vbox=gtk_vbox_new(FALSE,5);
	
	gtk_container_add(GTK_CONTAINER(window),vbox);
	
	logo_image=gtk_image_new_from_pixbuf(program_icon_pixbuf);
	
	gtk_box_pack_start(GTK_BOX(vbox), logo_image, FALSE, FALSE, 0);
	
	label=gtk_label_new(NULL);
	gtk_label_set_selectable(GTK_LABEL(label),FALSE);
	gtk_label_set_markup(GTK_LABEL(label),"<big><b>SciteProj</b></big>");
		
	gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);
	
	// Show version of SciteProj
	
	gchar *version_string;
	
#ifdef _DEBUG
	version_string=g_strdup_printf("version %s DEBUG",sVersion);
#else
	version_string=g_strdup_printf("version %s",sVersion);
#endif
	
	label=gtk_label_new(version_string);
	gtk_label_set_selectable(GTK_LABEL(label),FALSE);
	gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);
	
	// Show SciteProj copyrights
	copyrightstring=g_strdup_printf("Copyright (C) 2008-2011 Andreas Rönnquist <gusnan@gusnan.se>");
	
	label=gtk_label_new(copyrightstring);
	gtk_label_set_selectable(GTK_LABEL(label),FALSE);
	gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);
	
	// show GTK versions
	gchar *gtk_string=g_strdup_printf("GTK+ %d.%d.%d / GLib %d.%d.%d",
		   //"Operating System: unknown",
		   gtk_major_version, gtk_minor_version, gtk_micro_version,
		   glib_major_version, glib_minor_version, glib_micro_version);
			
	label=gtk_label_new(gtk_string);
	
	gtk_label_set_selectable(GTK_LABEL(label),FALSE);
	gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);
	
	// Show a link to the SciteProj homepage
	linkbutton=gtk_link_button_new_with_label(homepage_string,"http://www.gusnan.se/sciteproj");
	
	
	GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	linkbutton=gtk_link_button_new_with_label(homepage_string,"http://www.gusnan.se/sciteproj");
	gtk_box_pack_start(GTK_BOX(hbox), linkbutton, TRUE, FALSE, 0);

	// New textbuffer - and we get the beginning of the textbuffer
	textbuffer=gtk_text_buffer_new(NULL);
	gtk_text_buffer_get_start_iter(textbuffer,&iter);

	gchar *about_text2=g_strdup_printf("----------------------------------------------------------\n"
											"based on ScitePM by\n"
											"Roy Wood<roy.wood@gmail.com> and\n"
											"Martin Andrews<ScitePM@PLATFORMedia.com>\n\n"
											"Thanks to\n"
											"Mattias Wecksten <wecksten@gmail.com>\n"
											"Frank Wunderlich\n"
											"----------------------------------------------------------\n\n");
	
	gtk_text_buffer_insert(textbuffer,&iter,about_text2,-1);
	
	gtk_text_buffer_get_end_iter(textbuffer,&iter);
	
	
	gtk_text_buffer_insert(textbuffer,&iter,sLicense,-1);
	
	// Setup the textview and windows
	textview=gtk_text_view_new_with_buffer(textbuffer);
	
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(textview),TRUE);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(textview),FALSE);
	
	GtkWidget *scrolled_window;
	
	scrolled_window=gtk_scrolled_window_new(NULL,NULL);

	// Never show horisontal scrollbar, always show vertical
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),GTK_POLICY_NEVER,GTK_POLICY_ALWAYS);

	gtk_container_add(GTK_CONTAINER(scrolled_window),textview);
		
	gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);
	
	gtk_text_buffer_place_cursor(textbuffer,&iter);
	gtk_text_buffer_select_range (textbuffer,&iter,&iter);

	hbox=gtk_hbox_new(FALSE,0);
	
	ok_button=gtk_button_new_from_stock(GTK_STOCK_OK);
	
	gtk_box_pack_end(GTK_BOX(hbox),ok_button,FALSE,FALSE,0);
	
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	
	gtk_widget_grab_focus(ok_button);
		
	gtk_text_buffer_get_start_iter(textbuffer, &iter);
	gtk_text_buffer_place_cursor(textbuffer, &iter);
	
	g_signal_connect_closure
		(G_OBJECT(ok_button), "clicked",
		 g_cclosure_new_swap(G_CALLBACK(gtk_widget_hide_on_delete),
				     window, NULL), FALSE);
					  
	
	gtk_widget_show_all(window);

}

/**
 *	show_usage_dialog
 */
void show_usage_dialog()
{
	GtkWidget *dialog,/**table,*/*textview; //,scrolled_window;
	GtkTextBuffer *textbuffer;
	GtkTextIter iter;
	GtkWidget *table;
	
	gchar* usage_text=g_strdup_printf(	
			"To open or save projects, click the 'Open Project' or 'Save Project' "
			"buttons.  Project files are simple XML files, and may be easily "
			"modified using a text editor or other tool.\n\n"
			"Click the 'Add Files' or 'Add Group' buttons to add items to the "
			"project tree, then reorganize them via drag-and-drop.  Additional "
			"file and group operations are available via right-clicking.\n\n"
			"To open a file in SciTE, double-click the file item in the project "
			"tree, or highlight it and press return.");
	
	gint result;
	
	dialog=gtk_dialog_new_with_buttons("Usage",NULL,GTK_DIALOG_MODAL,
													GTK_STOCK_OK,GTK_RESPONSE_OK,NULL);
	
	gtk_dialog_set_default_response(GTK_DIALOG(dialog),GTK_RESPONSE_OK);
	
	gtk_widget_set_size_request(dialog,500,400);
	
	table=gtk_table_new(15,15,TRUE);
	
	textbuffer=gtk_text_buffer_new(NULL);
	
	gtk_text_buffer_get_start_iter(textbuffer,&iter);
	gtk_text_buffer_insert(textbuffer,&iter,usage_text,-1);
	
	textview=gtk_text_view_new_with_buffer(textbuffer);
	
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview),GTK_WRAP_WORD_CHAR);
	
	//gtk_text_buffer_get_start_iter(textbuffer,&iter);
	gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(textview),&iter,0,0);
	gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(textview),&iter,0.0f,FALSE,0.0f,0.0f);
	
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(textview),FALSE);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(textview),FALSE);
	
	GtkScrolledWindow *scrolled_window;
	
	scrolled_window=GTK_SCROLLED_WINDOW(gtk_scrolled_window_new(NULL,NULL));
	
	gtk_scrolled_window_set_policy(scrolled_window,GTK_POLICY_NEVER,GTK_POLICY_ALWAYS);
	
	gtk_container_add(GTK_CONTAINER(scrolled_window),textview);

		
	GtkWidget *icon_image=gtk_image_new_from_pixbuf(program_icon_pixbuf);

	gtk_table_attach_defaults(GTK_TABLE(table),icon_image,0,2,0,3);
	
	gtk_table_attach_defaults(GTK_TABLE(table),GTK_WIDGET(scrolled_window),2,15,0,15);
	
	gtk_table_set_row_spacings(GTK_TABLE(table),0);
	gtk_table_set_col_spacings(GTK_TABLE(table),0);
	
	gtk_container_set_border_width(GTK_CONTAINER(table),0);

	GtkWidget *container_vbox=gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	gtk_box_pack_start(GTK_BOX(container_vbox),table,TRUE,TRUE,0);

	gtk_widget_show_all(dialog);
	
	result=gtk_dialog_run(GTK_DIALOG(dialog));
	
	gtk_widget_destroy(dialog);
	
	g_free(usage_text);
}


/**
 * Show version
 */
void show_version()
{
	g_print("SciteProj version %s\n", sVersion);
}
