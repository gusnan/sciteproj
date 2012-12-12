/**
 * load_folder.c - folder loading support for sciteproj
 *
 *  Copyright 2012 Andreas Rönnquist
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

#include "tree_manipulation.h"

#include "load_folder.h"
#include "clicked_node.h"
#include "gui.h"
#include "file_utils.h"
#include "sort.h"


static int folder_number=0;

struct ParseFileStruct {
	GtkTreeIter current_iter;

	int depth;
};

typedef struct ParseFileStruct ParseFileStruct;

GtkTreeIter prevFileIterArray[100];
int currentFilePrevFileIter=0;

gboolean prevFileIterValid[100];


/**
 * @param store - the treestore where to store the data
 * @param folder_path - the folder to read the contents from
 *
 */
void read_folder(GtkTreeStore *store, gchar *folder_path,ParseFileStruct *parse_file, GError **error)
{
	GDir *dir=g_dir_open(folder_path, 0, NULL);
	
	gchar *current_file=NULL;
	const gchar *short_filename;

	int last_type=0;
	
	GSList *file_list=NULL;
	GSList *folder_list=NULL;
	
	while ((short_filename = g_dir_read_name(dir))) {
		
		gchar *temp_file=g_build_filename(folder_path, short_filename, NULL);
		
		if (g_file_test(temp_file, G_FILE_TEST_IS_DIR)) {
			folder_list=g_slist_prepend(folder_list,(gpointer)short_filename);
		} else {
			file_list=g_slist_prepend(file_list,(gpointer)short_filename);
		}
	};
	
	file_list=g_slist_sort(file_list,file_sort_by_extension_bigger_func);
	folder_list=g_slist_sort(folder_list,compare_strings_bigger);
	
	// Treat folders and files by themselves
	if (folder_list!=NULL) {
		while (folder_list!=NULL) {
			
			short_filename=(gchar*)(folder_list->data);
			
			current_file = g_build_filename(folder_path, short_filename,NULL);

			if (short_filename[0]!='.') {
				if (g_file_test(current_file, G_FILE_TEST_IS_DIR)) {

					folder_number++;

					if (parse_file->depth<=0) {

						add_tree_group(NULL, ADD_CHILD, short_filename, TRUE, &(parse_file->current_iter), NULL);

						prevFileIterValid[currentFilePrevFileIter]=FALSE;
						prevFileIterArray[currentFilePrevFileIter]=parse_file->current_iter;

						currentFilePrevFileIter++;
						prevFileIterArray[currentFilePrevFileIter]=parse_file->current_iter;

					} else {

						add_tree_group(&(parse_file->current_iter), ADD_CHILD, short_filename, TRUE, &(parse_file->current_iter), NULL);

						prevFileIterValid[currentFilePrevFileIter]=FALSE;
						prevFileIterArray[currentFilePrevFileIter]=parse_file->current_iter;

						currentFilePrevFileIter++;
						prevFileIterValid[currentFilePrevFileIter]=prevFileIterValid[currentFilePrevFileIter];
					}

					parse_file->depth++;

					prevFileIterValid[currentFilePrevFileIter]=FALSE;

					// We ignore hidden folders
					if (short_filename[0]!='.') {
						read_folder(store, current_file, parse_file, error);

					}

					last_type=1;

				}
			}	

			if (folder_number>0) folder_number--;

			g_free(current_file);
			
			folder_list=folder_list->next;
		}
	}
	
	if (file_list!=NULL) {
		while (file_list!=NULL) {
			
			short_filename=(gchar*)(file_list->data);
			
			current_file = g_build_filename(folder_path, short_filename,NULL);

			if (short_filename[0]!='.') {
				if (!g_file_test(current_file, G_FILE_TEST_IS_DIR)) {
					// add as a file

					if (parse_file->depth<=0) {

						if (prevFileIterValid[currentFilePrevFileIter]) {
							add_tree_file(&(prevFileIterArray[currentFilePrevFileIter]), ADD_AFTER, current_file, &(prevFileIterArray[currentFilePrevFileIter]), TRUE, error);
						} else {
							add_tree_file(NULL, ADD_CHILD, current_file, &(prevFileIterArray[currentFilePrevFileIter]), TRUE, error);
						}

					} else {

						if (prevFileIterValid[currentFilePrevFileIter]) {
							add_tree_file(&(prevFileIterArray[currentFilePrevFileIter]), ADD_AFTER, current_file, &(prevFileIterArray[currentFilePrevFileIter]), TRUE, error);
						} else {
							add_tree_file(&(parse_file->current_iter), ADD_CHILD, current_file, &(prevFileIterArray[currentFilePrevFileIter]), TRUE, error);
						}
					}
					
					prevFileIterValid[currentFilePrevFileIter]=TRUE;
				}
			}	

			if (folder_number>0) folder_number--;

			g_free(current_file);
			
			file_list=file_list->next;
		}
	}
	
	if (currentFilePrevFileIter>0) currentFilePrevFileIter--;
	
	if (parse_file->depth>0) {

		GtkTreeIter parent_iter;
		GtkTreeIter temp_iter=prevFileIterArray[currentFilePrevFileIter];

		if (gtk_tree_model_iter_parent(GTK_TREE_MODEL(store), &parent_iter, &(temp_iter))) {

			parse_file->current_iter=parent_iter;

			if (parse_file->depth>0) parse_file->depth-=1;
		} else {
			parse_file->current_iter=temp_iter;

			if (parse_file->depth>0) parse_file->depth-=1;
		}
	}

	g_dir_close(dir);
}


/**
 * @param project_path - the folder to read the contents from
 * @param err - errors are returned here
 *
 */
gboolean load_folder(gchar *path, GError **err)
{
	GtkTreeModel *model;

	model=gtk_tree_view_get_model(GTK_TREE_VIEW(projectTreeView));

	ParseFileStruct parse_struct;

	parse_struct.depth=0;

	GtkTreeIter dot_folder_iterator;

	add_tree_group(NULL, ADD_CHILD, "." /*get_filename_from_full_path(project_path)*/ , TRUE, &(parse_struct.current_iter), NULL);
	//add_tree_file(NULL, ADD_CHILD, project_path , &(parse_struct.current_iter), TRUE, NULL);

	dot_folder_iterator=parse_struct.current_iter;

	prevFileIterValid[currentFilePrevFileIter]=FALSE;
	prevFileIterArray[currentFilePrevFileIter]=parse_struct.current_iter;

	currentFilePrevFileIter++;
	prevFileIterArray[currentFilePrevFileIter]=parse_struct.current_iter;

	parse_struct.depth++;
	
	read_folder(GTK_TREE_STORE(model), path, &parse_struct, NULL);
	
	// Expand the dot-folder
	// void expand_tree_row(GtkTreePath *path, gboolean expandChildren);
	
	GtkTreePath *iter_path=gtk_tree_model_get_path(GTK_TREE_MODEL(model), &dot_folder_iterator);

	if (iter_path) 
		expand_tree_row(iter_path,FALSE);
	
	gtk_tree_path_free(iter_path);
	
EXITPOINT:
	
	return TRUE;
}
