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

/**
 *	show_about_dialog
 */
void show_about_dialog()
{
	GtkWidget *dialog,/**table,*/*textview; //,scrolled_window;
	GtkTextBuffer *textbuffer;
	GtkTextIter iter;
	gchar *about_text,*about_text2;
	GtkWidget *table;
	GtkWidget *linkbutton;
	
		
	gint result;
	// Create a new dialog
	dialog=gtk_dialog_new_with_buttons("About SciteProj",NULL,GTK_DIALOG_MODAL,
													GTK_STOCK_OK,GTK_RESPONSE_OK,NULL);
	
	gtk_dialog_set_default_response(GTK_DIALOG(dialog),GTK_RESPONSE_OK);
	
	gtk_widget_set_size_request(dialog,500,400);
	
	table=gtk_table_new(15,15,TRUE);
	
	// New textbuffer - and we get the beginning of the textbuffer
	textbuffer=gtk_text_buffer_new(NULL);
	gtk_text_buffer_get_start_iter(textbuffer,&iter);
	
	// Write out program title with nice text attributes
	GtkTextTag *tag;
	
	tag=gtk_text_buffer_create_tag(textbuffer,NULL,
													"foreground","blue",
													"size-points",15.0,
													"weight",700,
													NULL);
	
	gtk_text_buffer_insert_with_tags(textbuffer,&iter,"SciteProj",-1,tag,NULL);
	
	
	
	gtk_text_buffer_get_end_iter(textbuffer,&iter);
	
	
	// Version and credits
	about_text=g_strdup_printf("\n"
											"version %s\n"
											"by Andreas Ronnquist <gusnan@gusnan.se>\n",sVersion);
	
	gtk_text_buffer_insert(textbuffer,&iter,about_text,-1);
	
	gtk_text_buffer_get_end_iter(textbuffer,&iter);

	// fix a proper link of the homepage
	
	GtkTextChildAnchor *anchor;
	
	anchor=gtk_text_buffer_create_child_anchor(textbuffer,&iter);
	
	gtk_text_buffer_insert(textbuffer,&iter,"\n",-1);
	
	
	// Print is we have been built with Debug information or not
#ifdef _DEBUG
	GtkTextTag *debug_tag;
	
	debug_tag=gtk_text_buffer_create_tag(textbuffer,NULL, "foreground", "red", NULL);
	
	gtk_text_buffer_insert_with_tags(textbuffer,&iter,"Built with DEBUG information enabled\n",-1,debug_tag,NULL);
	
	gtk_text_buffer_get_end_iter(textbuffer,&iter);
	
#else
	// Just write an empty line if we are running the release executable
	gtk_text_buffer_insert(textbuffer,&iter,"\n",-1);
	
	gtk_text_buffer_get_end_iter(textbuffer,&iter);
#endif
	
	// What are we based on?
	about_text2=g_strdup_printf("----------------------------------------------------------\n"
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
	
	gtk_text_buffer_insert(textbuffer,&iter,"----------------------------------------------------------\n",-1);
	
	// GTK versions
	gchar *gtk_string=g_strdup_printf("GTK+ %d.%d.%d / GLib %d.%d.%d\n",
		   //"Operating System: unknown",
		   gtk_major_version, gtk_minor_version, gtk_micro_version,
		   glib_major_version, glib_minor_version, glib_micro_version);
	
	gtk_text_buffer_get_end_iter(textbuffer,&iter);
	gtk_text_buffer_insert(textbuffer,&iter,gtk_string,-1);
	
	gtk_text_buffer_get_end_iter(textbuffer,&iter);


	
	gtk_text_buffer_insert(textbuffer,&iter,"\n",-1);
	gtk_text_buffer_get_end_iter(textbuffer,&iter);
	
	// Setup the textview and windows
	textview=gtk_text_view_new_with_buffer(textbuffer);
	
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(textview),FALSE);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(textview),FALSE);
	
	
	// add a "button" for the link at the anchor we created prevoiusly
	
	gchar *buf_homepage=g_strdup_printf("%s\n",homepage_string);

	linkbutton=gtk_link_button_new_with_label(homepage_string,"http://www.gusnan.se/sciteproj");
	gtk_text_view_add_child_at_anchor(GTK_TEXT_VIEW(textview),linkbutton,anchor);
	
	// create the scrolledwindow
	
	GtkScrolledWindow *scrolled_window;
	
	scrolled_window=GTK_SCROLLED_WINDOW(gtk_scrolled_window_new(NULL,NULL));

	// Never show horisontal scrollbar, always show vertical
	gtk_scrolled_window_set_policy(scrolled_window,GTK_POLICY_NEVER,GTK_POLICY_ALWAYS);

	gtk_container_add(GTK_CONTAINER(scrolled_window),textview);

	// Nice big icon
	GtkWidget *icon_image=gtk_image_new_from_pixbuf(program_icon_pixbuf);

	// ---
	gtk_table_attach_defaults(GTK_TABLE(table),icon_image,0,2,0,3);
	
	gtk_table_attach_defaults(GTK_TABLE(table),GTK_WIDGET(scrolled_window),2,15,0,15);
	
	gtk_table_set_row_spacings(GTK_TABLE(table),0);
	gtk_table_set_col_spacings(GTK_TABLE(table),0);
	
	gtk_container_set_border_width(GTK_CONTAINER(table),0);
	
	GtkWidget *container_vbox=gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	
	gtk_box_pack_start(GTK_BOX(container_vbox),table,TRUE,TRUE,0);
	
	// --

	gtk_widget_show_all(dialog);
	
	result=gtk_dialog_run(GTK_DIALOG(dialog));
	
	gtk_widget_destroy(dialog);
	
	// Free all used strings and data
	g_free(about_text);
	g_free(about_text2);
	g_free(gtk_string);
	g_free(buf_homepage);
	
	g_object_unref(tag);
	
	
#ifdef _DEBUG
	g_object_unref(debug_tag);
#endif
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
