/**
 * properties_dialog.c - Properties Dialogs code for SciteProj
 *
 *  Copyright 2009-2020 Andreas Rönnquist
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
#include <string.h>
#include <sys/stat.h>
#include <glib/gi18n.h>

#include <locale.h>


#include "properties_dialog.h"
#include "tree_manipulation.h"
#include "clicked_node.h"
#include "gui.h"

#include "string_utils.h"
#include "file_utils.h"


/**
 * My set align
 */
void my_set_align(GtkWidget *widget)
{
	gtk_widget_set_halign(widget, GTK_ALIGN_START);
	gtk_widget_set_valign(widget, GTK_ALIGN_START);
}


/**
 * Group properties callback
 */
void group_properties_gui(GtkTreeModel *tree_model, GtkTreeIter *iter)
{
	GtkWidget *dialog;
	gchar *nodename = NULL;

	GtkWidget *table;
	GtkWidget *label1, *label2;
	GtkWidget *filename, *filepath_entry;
	GtkWidget *container_box;

	gchar *filePath = NULL;
	int nodeType = -1;

	gtk_tree_model_get(tree_model, iter,
	                   COLUMN_FILENAME, &nodename,
	                   COLUMN_ITEMTYPE, &nodeType,
	                   COLUMN_FILEPATH, &filePath,
	                   -1);

	dialog = gtk_dialog_new_with_buttons(_("Group Properties"), NULL, GTK_DIALOG_MODAL,
		_("OK"), GTK_RESPONSE_ACCEPT, NULL);

	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

	label1 = gtk_label_new(_("Group name:"));
	label2 = gtk_label_new(_("Complete folder:"));

	gtk_widget_set_valign(label2, GTK_ALIGN_CENTER);

	filename = gtk_label_new(nodename);

	//filepath_label = gtk_label_new(filePath);
	filepath_entry = gtk_entry_new();

	gtk_entry_set_text(GTK_ENTRY(filepath_entry), filePath);
	gtk_editable_set_editable(GTK_EDITABLE(filepath_entry), FALSE);

	my_set_align(filename);
	my_set_align(filepath_entry);

	my_set_align(label1);
	my_set_align(label2);

	table = gtk_grid_new();

	gtk_grid_attach(GTK_GRID(table), label1, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(table), label2, 0, 1, 1, 1);

	gtk_grid_attach(GTK_GRID(table), filename, 1, 0, 4, 1);
	gtk_grid_attach(GTK_GRID(table), filepath_entry, 1, 1, 4, 1);

	gtk_grid_set_row_spacing(GTK_GRID (table), 6);
	gtk_grid_set_column_spacing(GTK_GRID (table), 6);

	gtk_widget_set_halign(filename, GTK_ALIGN_START);
	gtk_widget_set_hexpand(filename, FALSE);
	gtk_widget_set_halign(filepath_entry, GTK_ALIGN_FILL);
	gtk_widget_set_hexpand(filepath_entry, TRUE);

	gtk_widget_set_valign(label2, GTK_ALIGN_CENTER);

	container_box = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	gtk_box_pack_start(GTK_BOX(container_box), table, TRUE, TRUE, 0);

	set_dialog_transient(dialog);
	
	gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 100);
	gtk_window_set_resizable (GTK_WINDOW(dialog), FALSE);

	gtk_widget_show_all(dialog);


	gtk_dialog_run(GTK_DIALOG(dialog));

	if (dialog) gtk_widget_destroy(dialog);

}


/**
 * File properties callback
 */
void file_properties_gui(GtkTreeModel *model, GtkTreeIter *iter)
{

	GError *err = NULL;
	gchar *nodename = NULL;

	GtkWidget *table;
	GtkWidget *label1, *label2, *label3;
	GtkWidget *path, *filename, *filesize_label;
	GtkWidget *container_box = NULL;
	GtkWidget *dialog;

	gchar *filePath = NULL;
	int nodeType = -1;

	gtk_tree_model_get(model, iter,
	                   COLUMN_FILENAME, &nodename,
	                   COLUMN_ITEMTYPE, &nodeType,
	                   COLUMN_FILEPATH, &filePath,
	                   -1);

	debug_printf((gchar*)"Node name: %s\n", nodename);
	debug_printf((gchar*)"File name: %s\n", filePath);

	dialog = gtk_dialog_new_with_buttons(_("File Properties"), NULL, GTK_DIALOG_MODAL,
		_("OK"), GTK_RESPONSE_ACCEPT,NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

	gchar *absFilePath = NULL; //g_strdup_printf("%s",filePath);

	// filePath is NULL?
	//if (!relative_path_to_abs_path(sClickedNodeName, &absFilePath, get_project_directory(), &err)) {

	if (!relative_path_to_abs_path(filePath, &absFilePath, get_project_directory(), &err)) {
		goto EXITPOINT;
	}

	gchar *size_string;

	// get the size of the file
	struct stat file_status;
	if(stat(absFilePath, &file_status) != 0) {
		perror("could not stat");
		//goto EXITPOINT;
	}

	size_string = g_strdup_printf("%d bytes", (int)(file_status.st_size));

	label1 = gtk_label_new(_("Filename:"));
	label2 = gtk_label_new(_("Path:"));
	label3 = gtk_label_new(_("File size:"));

	filename = gtk_label_new(nodename);
	// path = gtk_label_new(absFilePath);
	path = gtk_entry_new();

	// buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(path));

	gtk_entry_set_text(GTK_ENTRY(path), absFilePath);

	gtk_editable_set_editable(GTK_EDITABLE(path), FALSE);

	//gtk_widget_show(buffer);
	gtk_widget_show(path);

	filesize_label = gtk_label_new(size_string);

	my_set_align(filename);
	my_set_align(path);
	my_set_align(filesize_label);

	my_set_align(label1);
	my_set_align(label2);
	my_set_align(label3);

	table = gtk_grid_new();

	gtk_grid_attach(GTK_GRID(table), label1, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(table), label2, 0, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(table), label3, 0, 2, 1, 1);

	gtk_grid_attach(GTK_GRID(table), filename,			1, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(table), path,					1, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(table), filesize_label,	1, 2, 1, 1);

	gtk_grid_set_row_spacing (GTK_GRID (table), 6);
	gtk_grid_set_column_spacing (GTK_GRID (table), 6);

	gtk_widget_set_halign(filename, GTK_ALIGN_START);
	gtk_widget_set_hexpand(filename, FALSE);
	gtk_widget_set_halign(path, GTK_ALIGN_FILL);
	gtk_widget_set_hexpand(path, TRUE);
	gtk_widget_set_halign(filesize_label, GTK_ALIGN_START);
	gtk_widget_set_hexpand(filesize_label, FALSE);

	gtk_widget_set_valign(label2, GTK_ALIGN_CENTER);

	set_dialog_transient(dialog);

	container_box = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

	gtk_box_pack_end(GTK_BOX(container_box), table, TRUE, TRUE, 0);
	
	gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 100);
	gtk_window_set_resizable (GTK_WINDOW(dialog), TRUE);

	gtk_widget_show_all(dialog);

	gtk_dialog_run(GTK_DIALOG(dialog));


EXITPOINT:

	if (err) g_error_free(err);
	if (dialog) gtk_widget_destroy(dialog);
}


/**
 *
 */
void group_properties_cb()
{
	if (!clicked_node.valid || clicked_node.type != ITEMTYPE_GROUP) {
		//goto EXITPOINT;
	} else {
		group_properties_gui(gtk_tree_view_get_model(GTK_TREE_VIEW(projectTreeView)), &(clicked_node.iter));

	}
}


/**
 *
 */
void file_properties_cb()
{
	if (!clicked_node.valid || clicked_node.type != ITEMTYPE_FILE) {
		//goto EXITPOINT;
	} else {
		file_properties_gui(gtk_tree_view_get_model(GTK_TREE_VIEW(projectTreeView)), &(clicked_node.iter));

	}
}
