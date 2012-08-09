/**
 * properties_dialog.c - Properties Dialogs code for SciteProj
 *
 *  Copyright 2009-2012 Andreas Rönnquist
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


/**
 *
 */
static int counter_step_through(GtkTreeModel *tree_model,int how_many_to_check,int *counter,GtkTreeIter newiter)
{
	gchar *relFilePath;

	gint nodeItemType;

	GtkTreeIter iter=newiter;

	int foldercounter=0;

	int result=0;

	do {

		gtk_tree_model_get(tree_model, &iter, COLUMN_ITEMTYPE, &nodeItemType, -1);

		if (nodeItemType==ITEMTYPE_GROUP) {

			(*counter)++;
			result++;

			if (gtk_tree_model_iter_has_child(tree_model,&iter)) {
				GtkTreeIter newIter=iter;

				int num=gtk_tree_model_iter_n_children(tree_model,&newiter);

				if (num!=0) {

					gtk_tree_model_iter_children(tree_model,&newIter,&iter);

					result+=counter_step_through(tree_model,num,counter,newIter);
				}
			}

			foldercounter++;
		} else {

			gtk_tree_model_get(tree_model, &iter, COLUMN_FILEPATH, &relFilePath, -1);

			(*counter)++;
			result++;


			foldercounter++;
		}
	} while((gtk_tree_model_iter_next(tree_model,&iter)) && (foldercounter<how_many_to_check) );

	return result;
}


/**
 * Group properties callback
 */
void group_properties_gui(GtkTreeModel *tree_model,GtkTreeIter *iter)
{
	GError *err = NULL;
	GtkWidget *dialog = NULL;
	gchar *nodename = NULL;

	GtkWidget *table;
	GtkWidget *label1,*label2;
	GtkWidget *filename,*number_of_files_label;

	gchar *filePath=NULL;
	int nodeType=-1;

	gtk_tree_model_get(tree_model, iter,
	                   COLUMN_FILENAME, &nodename,
	                   COLUMN_ITEMTYPE, &nodeType,
	                   COLUMN_FILEPATH, &filePath,
	                   -1);

	gchar *number_of_files_string;

	int num_of_files=0;

	int number_of_files=0;

	if (gtk_tree_model_iter_has_child(tree_model,iter)) {

		int num=gtk_tree_model_iter_n_children(tree_model,iter);

		GtkTreeIter newIter;
		gtk_tree_model_iter_children(tree_model,&newIter,iter);

		number_of_files=counter_step_through(tree_model,num,&num_of_files,newIter/*iter*/);

	}

	number_of_files_string=g_strdup_printf("%d",number_of_files);

	dialog=gtk_dialog_new_with_buttons(_("Group Properties"),NULL,GTK_DIALOG_MODAL,GTK_STOCK_OK,GTK_RESPONSE_OK,NULL);

	gtk_dialog_set_default_response(GTK_DIALOG(dialog),GTK_RESPONSE_OK);

	label1=gtk_label_new(_("Group name:"));
	label2=gtk_label_new(_("Contains number of Files:"));

	filename=gtk_label_new(nodename);


	number_of_files_label=gtk_label_new(number_of_files_string);

	gtk_misc_set_alignment(GTK_MISC(filename),0,0);
	gtk_misc_set_alignment(GTK_MISC(number_of_files_label),0,0);

	gtk_misc_set_alignment(GTK_MISC(label1),0,0);
	gtk_misc_set_alignment(GTK_MISC(label2),0,0);


#if GTK_MAJOR_VERSION>=3
	table=gtk_grid_new();

	gtk_grid_attach(GTK_GRID(table),label1,0,0,1,1);
	gtk_grid_attach(GTK_GRID(table),label2,0,1,1,1);

	gtk_grid_attach(GTK_GRID(table),filename,1,0,4,1);
	gtk_grid_attach(GTK_GRID(table),number_of_files_label,1,1,4,1);

	gtk_grid_set_row_spacing (GTK_GRID (table), 6);
	gtk_grid_set_column_spacing (GTK_GRID (table), 6);

#else
	table=gtk_table_new(3,2,FALSE);

	gtk_table_attach_defaults(GTK_TABLE(table),label1,0,1,0,1);
	gtk_table_attach_defaults(GTK_TABLE(table),label2,0,1,1,2);

	gtk_table_attach_defaults(GTK_TABLE(table),filename,1,2,0,1);
	gtk_table_attach_defaults(GTK_TABLE(table),number_of_files_label,1,2,1,2);

	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),5);

	gtk_container_set_border_width(GTK_CONTAINER(table),5);
#endif
	GtkWidget *container_vbox=gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	gtk_box_pack_start(GTK_BOX(container_vbox),table,TRUE,TRUE,0);


	gtk_widget_show_all(dialog);


	gtk_dialog_run(GTK_DIALOG(dialog));


//EXITPOINT:

	if (err) g_error_free(err);
	if (dialog) gtk_widget_destroy(dialog);

}


/**
 * File properties callback
 */
void file_properties_gui(GtkTreeModel *model,GtkTreeIter *iter)
{

	GError *err = NULL;
	GtkWidget *dialog = NULL;
	gchar *nodename = NULL;

	GtkWidget *table;
	GtkWidget *label1,*label2,*label3;
	GtkWidget *path,*filename,*filesize_label;
	GtkWidget *container_vbox=NULL;

	gchar *filePath=NULL;
	int nodeType=-1;

	gtk_tree_model_get(model, iter,
	                   COLUMN_FILENAME, &nodename,
	                   COLUMN_ITEMTYPE, &nodeType,
	                   COLUMN_FILEPATH, &filePath,
	                   -1);

	debug_printf((gchar*)"Node name: %s\n",nodename);
	debug_printf((gchar*)"File name: %s\n",filePath);

	dialog=gtk_dialog_new_with_buttons(_("File Properties"),NULL,GTK_DIALOG_MODAL,GTK_STOCK_OK,GTK_RESPONSE_OK,NULL);

	gtk_dialog_set_default_response(GTK_DIALOG(dialog),GTK_RESPONSE_OK);

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

	size_string=g_strdup_printf("%d bytes",(int)(file_status.st_size));

	label1=gtk_label_new(_("Filename:"));
	label2=gtk_label_new(_("Path:"));
	label3=gtk_label_new(_("File size:"));

	filename=gtk_label_new(nodename);
	path=gtk_label_new(absFilePath/*sClickedNodeName*/);
	filesize_label=gtk_label_new(size_string);

	gtk_misc_set_alignment(GTK_MISC(filename),0,0);
	gtk_misc_set_alignment(GTK_MISC(path),0,0);
	gtk_misc_set_alignment(GTK_MISC(filesize_label),0,0);

	gtk_misc_set_alignment(GTK_MISC(label1),0,0);
	gtk_misc_set_alignment(GTK_MISC(label2),0,0);
	gtk_misc_set_alignment(GTK_MISC(label3),0,0);

#if GTK_MAJOR_VERSION>=3
	table=gtk_grid_new();

	gtk_grid_attach(GTK_GRID(table),label1,0,0,1,1);
	gtk_grid_attach(GTK_GRID(table),label2,0,1,1,1);
	gtk_grid_attach(GTK_GRID(table),label3,0,2,1,1);

	gtk_grid_attach(GTK_GRID(table),filename,1,0,4,1);
	gtk_grid_attach(GTK_GRID(table),path,1,1,4,1);
	gtk_grid_attach(GTK_GRID(table),filesize_label,1,2,4,1);

	gtk_grid_set_row_spacing (GTK_GRID (table), 6);
	gtk_grid_set_column_spacing (GTK_GRID (table), 6);

#else
	table=gtk_table_new(3,2,FALSE);

	gtk_table_attach_defaults(GTK_TABLE(table),label1,0,1,0,1);
	gtk_table_attach_defaults(GTK_TABLE(table),label2,0,1,1,2);
	gtk_table_attach_defaults(GTK_TABLE(table),label3,0,1,2,3);

	gtk_table_attach_defaults(GTK_TABLE(table),filename,1,2,0,1);
	gtk_table_attach_defaults(GTK_TABLE(table),path,1,2,1,2);
	gtk_table_attach_defaults(GTK_TABLE(table),filesize_label,1,2,2,3);

	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),5);

	gtk_container_set_border_width(GTK_CONTAINER(table),5);
#endif

	container_vbox=gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	gtk_box_pack_start(GTK_BOX(container_vbox),table,TRUE,TRUE,0);

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
		group_properties_gui(gtk_tree_view_get_model(GTK_TREE_VIEW(projectTreeView)),&(clicked_node.iter));

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
		file_properties_gui(gtk_tree_view_get_model(GTK_TREE_VIEW(projectTreeView)),&(clicked_node.iter));

	}
}
