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

	while ((short_filename = g_dir_read_name(dir))) {

		current_file = g_build_filename(folder_path, short_filename,NULL);

		if (short_filename[0]!='.') {
			printf("%s  --  %s  ", short_filename, current_file);

			printf("\n");

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
				//iter=&new_iter;

				last_type=1;

			} else {
				// add as a file

				if (parse_file->depth<=0) {

					if (prevFileIterValid[currentFilePrevFileIter]) {
						add_tree_file(&(prevFileIterArray[currentFilePrevFileIter]), ADD_AFTER, short_filename, &(prevFileIterArray[currentFilePrevFileIter]), FALSE, error);
					} else {
						add_tree_file(NULL, ADD_CHILD, short_filename, &(prevFileIterArray[currentFilePrevFileIter]), FALSE, error);
					}

				} else {

					if (prevFileIterValid[currentFilePrevFileIter]) {
						add_tree_file(&(prevFileIterArray[currentFilePrevFileIter]), ADD_AFTER, short_filename, &(prevFileIterArray[currentFilePrevFileIter]), FALSE, error);
					} else {
						add_tree_file(&(parse_file->current_iter), ADD_CHILD, short_filename, &(prevFileIterArray[currentFilePrevFileIter]), FALSE, error);
					}
				}
			}
		}	

		if (folder_number>0) folder_number--;

		g_free(current_file);
	}

	GtkTreeIter parent_iter;
	GtkTreeIter temp_iter=prevFileIterArray[currentFilePrevFileIter];

	if (gtk_tree_model_iter_parent(GTK_TREE_MODEL(store), &parent_iter, &(temp_iter))) {

		parse_file->current_iter=parent_iter;

		if (parse_file->depth>0) parse_file->depth-=1;
	} else {
		parse_file->current_iter=temp_iter;

		if (parse_file->depth>0) parse_file->depth-=1;
	}

	g_dir_close(dir);
}


/**
 * @param project_path - the folder to read the contents from
 * @param err - errors are returned here
 *
 */
gboolean load_folder(gchar *project_path, GError **err)
{
	GtkTreeModel *model;

	model=gtk_tree_view_get_model(GTK_TREE_VIEW(projectTreeView));

	ParseFileStruct parse_struct;

	parse_struct.depth=0;

	if (project_path) {
		
		printf("Project path: %s\n",project_path);
		
		if (!set_project_filepath(project_path, err)) {
			goto EXITPOINT;
		}
	}
	
	read_folder(GTK_TREE_STORE(model), project_path, &parse_struct, NULL);
	
	
EXITPOINT:
	
	return TRUE;
}
