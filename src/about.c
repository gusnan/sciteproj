/**
 * about.c - about dialog for SciteProj
 *
 *	 Copyright 2008-2019 Andreas Rönnquist
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
#include <glib/gi18n.h>

#include <locale.h>

#include "about.h"

#include "graphics.h"

#include "prefs.h"

gchar *homepage_string=(gchar*)"https://www.nongnu.org/sciteproj";

gchar *sVersion = (gchar*)SCITEPROJ_VERSION;

static GtkWidget *window;

void create_about_dialog();

gboolean handle_about_close_event(GtkWidget *widget,GdkEvent *event,gpointer user_data);
void link_button_cb(GtkButton *button,gpointer user_data);

gchar *version_string = NULL;

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
 *  Handle keyboard pressing escape in about dialog
 */
gboolean handle_keyboard_event_cb(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    if (event->keyval == GDK_KEY_Escape) {
        // printf("Escape pressed.\n");
        gtk_widget_hide(window);
        return TRUE;
    }

    return FALSE;
}


/**
 *	creates a new dialog box, and fills it will all necessary information
 */
void create_about_dialog()
{
	GtkWidget *grid;
	GtkWidget *textview_info;
	GtkWidget *logo_image;
	GtkWidget *linkbutton;
	GtkWidget *ok_button;
	GtkWidget *notebook;
	GtkWidget *notebook_label1;
	GtkWidget *notebook_label2;

	GtkWidget *sciteproj_label;
	GtkWidget *version_string_label;
	GtkWidget *copyright_label;
	GtkWidget *gtk_version_label;

	GtkTextBuffer *textbuffer_info;
	GtkTextBuffer *textbuffer_license;
	GtkWidget *textview_license;
	GtkTextIter iter;

	gchar *copyrightstring;


	// Make the dialog
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(window), 8);

	gtk_widget_set_size_request(window, 500, 400);

	// Make a container
	grid = gtk_grid_new();

	gtk_grid_set_row_spacing (GTK_GRID (grid), 6);

	gtk_container_add(GTK_CONTAINER(window), grid);

	logo_image = gtk_image_new_from_pixbuf(program_icon_pixbuf);

	gtk_grid_attach(GTK_GRID(grid), logo_image, 0, 0, 5, 1);

	sciteproj_label = gtk_label_new(NULL);
	gtk_label_set_selectable(GTK_LABEL(sciteproj_label), FALSE);
	gtk_label_set_markup(GTK_LABEL(sciteproj_label), "<big><b>SciteProj</b></big>");


	gtk_grid_attach_next_to(GTK_GRID(grid), sciteproj_label, logo_image, GTK_POS_BOTTOM, 5, 1);

	// Show version of SciteProj

	gchar *about_dialog_version_string;

 #ifdef _DEBUG
	about_dialog_version_string = g_strdup_printf("%s DEBUG",version_string);
#else
	about_dialog_version_string = g_strdup_printf("%s", version_string);
#endif

	version_string_label = gtk_label_new(about_dialog_version_string);
	gtk_label_set_selectable(GTK_LABEL(version_string_label), FALSE);

	gtk_grid_attach_next_to(GTK_GRID(grid), version_string_label, sciteproj_label, GTK_POS_BOTTOM, 5, 1);

	// Show SciteProj copyrights
	copyrightstring = g_strdup_printf("Copyright (C) 2008-2017 Andreas Rönnquist <andreas@ronnquist.net>");

	copyright_label = gtk_label_new(copyrightstring);
	gtk_label_set_selectable(GTK_LABEL(copyright_label), FALSE);

	gtk_grid_attach_next_to(GTK_GRID(grid), copyright_label, version_string_label, GTK_POS_BOTTOM, 5, 1);

	// show GTK versions
	gchar *gtk_string = g_strdup_printf("GTK+ %d.%d.%d / GLib %d.%d.%d",
		   //"Operating System: unknown",
		   gtk_major_version, gtk_minor_version, gtk_micro_version,
		   glib_major_version, glib_minor_version, glib_micro_version);

	gtk_version_label = gtk_label_new(gtk_string);

	gtk_label_set_selectable(GTK_LABEL(gtk_version_label), FALSE);

	gtk_grid_attach_next_to(GTK_GRID(grid), gtk_version_label, copyright_label, GTK_POS_BOTTOM, 5, 1);


	// Show a link to the SciteProj homepage
	linkbutton = gtk_link_button_new_with_label(homepage_string, homepage_string);

	gtk_grid_attach_next_to(GTK_GRID(grid), linkbutton, gtk_version_label, GTK_POS_BOTTOM, 5, 1);

	// New notebook - we want tabs for different sets of text

	notebook = gtk_notebook_new();

	notebook_label1 = gtk_label_new(_("Information"));
	notebook_label2 = gtk_label_new(_("License"));

	// create a scrolled_window and a textview for the license
	textbuffer_license = gtk_text_buffer_new(NULL);
	gtk_text_buffer_get_start_iter(textbuffer_license, &iter);



	gchar *sLicense =	_("SciteProj is free software: you can redistribute it and/or modify "
	                      "it under the terms of the GNU General Public License as published by "
	                      "the Free Software Foundation, either version 3 of the License, or "
	                      "(at your option) any later version.\n"
	                      "\n"
	                      "SciteProj is distributed in the hope that it will be useful, "
	                      "but WITHOUT ANY WARRANTY; without even the implied warranty of "
	                      "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
	                      "GNU General Public License for more details.\n"
	                      "\n"
	                      "You should have received a copy of the GNU General Public License "
	                      "along with SciteProj.  If not, see <http://www.gnu.org/licenses/>.\n");



	gtk_text_buffer_insert(textbuffer_license, &iter, sLicense, -1);

	textview_license = gtk_text_view_new_with_buffer(textbuffer_license);

	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview_license), GTK_WRAP_WORD);

	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(textview_license), TRUE);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(textview_license), FALSE);

	GtkWidget *scrolled_window_license;

	scrolled_window_license = gtk_scrolled_window_new(NULL, NULL);

	// Never show horisontal scrollbar, always show vertical
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window_license), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

	gtk_container_add(GTK_CONTAINER(scrolled_window_license), textview_license);



	// New textbuffer - and we get the beginning of the textbuffer
	textbuffer_info = gtk_text_buffer_new(NULL);
	gtk_text_buffer_get_start_iter(textbuffer_info, &iter);

	gchar *about_text2 = g_strdup_printf(""
		"%s\n"
		"Roy Wood <roy.wood@gmail.com> and\n"
		"Martin Andrews <ScitePM@PLATFORMedia.com>\n\n"
		"%s\n"
		"Mattias Wecksten <wecksten@gmail.com>\n"
		"Frank Wunderlich\n\n"
		"%s",
		_("SciteProj is based on ScitePM by"),
		_("Many thanks to"),
		_("For more information about SciteProj, see the README file that\n"
											"is provided with this package."));

	gchar *text_to_add;

#ifdef _DEBUG

	gchar *prefs_dir_string = g_strdup_printf("Preferences loaded from %s",prefs_filename);

	text_to_add = g_strdup_printf("%s\n\n%s", about_text2,
	                            prefs_dir_string);

#else
	text_to_add = about_text2;

#endif

	gtk_text_buffer_insert(textbuffer_info, &iter, text_to_add, -1);

	gtk_text_buffer_get_end_iter(textbuffer_info, &iter);

	// Setup the textview and windows
	textview_info = gtk_text_view_new_with_buffer(textbuffer_info);

	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(textview_info), TRUE);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(textview_info), FALSE);

	GtkWidget *scrolled_window_info;

	scrolled_window_info = gtk_scrolled_window_new(NULL, NULL);

	// Never show horisontal scrollbar, always show vertical
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window_info), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

	gtk_container_add(GTK_CONTAINER(scrolled_window_info), textview_info);

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scrolled_window_info, notebook_label1);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scrolled_window_license, notebook_label2);

	gtk_widget_set_vexpand(notebook, TRUE);

	gtk_widget_set_hexpand(notebook, TRUE);

	gtk_grid_attach_next_to(GTK_GRID(grid), notebook, linkbutton/*gtk_version_label*/, GTK_POS_BOTTOM, 5, 1);

	gtk_text_buffer_place_cursor(textbuffer_info, &iter);
	gtk_text_buffer_select_range(textbuffer_info, &iter, &iter);

	// Create an ok button
	ok_button = gtk_button_new();
	gtk_button_set_use_underline(GTK_BUTTON(ok_button), TRUE);
	gtk_button_set_label(GTK_BUTTON(ok_button), "_OK");

	gtk_widget_set_halign(ok_button, GTK_ALIGN_END);
	gtk_widget_set_hexpand(ok_button, FALSE);

	gtk_grid_attach_next_to(GTK_GRID(grid), ok_button, notebook, GTK_POS_BOTTOM, 5, 1);

	gtk_widget_grab_focus(ok_button);

	gtk_text_buffer_get_start_iter(textbuffer_info, &iter);
	gtk_text_buffer_place_cursor(textbuffer_info, &iter);

	g_signal_connect_closure
	(G_OBJECT(ok_button), "clicked",
	 g_cclosure_new_swap(G_CALLBACK(gtk_widget_hide_on_delete),
	                     window, NULL), FALSE);


	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(handle_about_close_event), window);
	g_signal_connect(G_OBJECT(linkbutton), "released", G_CALLBACK(link_button_cb), linkbutton);

    // Handle ESC in about window
    gtk_widget_add_events(window, GDK_KEY_PRESS_MASK);

    g_signal_connect(G_OBJECT(window), "key_press_event", G_CALLBACK(handle_keyboard_event_cb), NULL);

	gtk_widget_show_all(window);

}


/**
 * Show version
 */
void show_version()
{
	g_print("SciteProj %s\n", version_string);
}


/**
 *
 */
gboolean handle_about_close_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	gtk_widget_hide(window);

	return TRUE;
}


/**
 *	Callback for the link button, Without this, it would color the text purple when the
 *	link is followed, which I found really ugly.
 */
void link_button_cb(GtkButton *button, gpointer user_data)
{
	GtkWidget *linkbutton = (GtkWidget*)(user_data);

	gtk_link_button_set_visited(GTK_LINK_BUTTON(linkbutton), FALSE);
}


/**
 *
 */
void init_version_string()
{
	version_string = g_strdup_printf(_("version %s"), SCITEPROJ_VERSION);
}


/**
 *
 */
void done_version_string()
{
	g_free(version_string);
}
