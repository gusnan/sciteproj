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
#include <glib.h>

#include <string.h>

#include "tree_manipulation.h"

#include "load_folder.h"
#include "clicked_node.h"
#include "gui.h"
#include "file_utils.h"
#include "sort.h"


//static int folder_number=0;

struct ParseFileStruct {
	GtkTreeIter current_iter;

	int depth;
};

typedef struct ParseFileStruct ParseFileStruct;

GtkTreeIter prevFileIterArray[100];
int currentFilePrevFileIter=0;

gboolean prevFileIterValid[100];

int number_of_files=0;


/**
 *
 */
gboolean ignore_pattern_matches(const gchar *filename, GSList *filter_list)
{
	gboolean result=FALSE;
	
	int len=strlen(filename);
	
	if (filter_list) {
		while (filter_list)	{
			
			GPatternSpec *pattern_spec=g_pattern_spec_new((gchar*)(filter_list->data));
			
			if (g_pattern_match(pattern_spec, len, filename, NULL)) {
				result=TRUE;
			}
			
			g_pattern_spec_free(pattern_spec);
			
			filter_list=filter_list->next;
		};
	}

	
	return result;
}


/**
 *
 */
GSList *load_folder_to_list(gchar *folder_path, gboolean read_directories, GCompareFunc compare_func, GSList *filter_list)
{
	GSList *result_list=NULL;
	
	GDir *dir=g_dir_open(folder_path, 0, NULL);
	
	const gchar *short_filename;
	
	while((short_filename = g_dir_read_name(dir))) {
		
		gchar *temp_file=g_build_filename(folder_path, short_filename, NULL);
		
		if (read_directories) {
			
			if (g_file_test(temp_file, G_FILE_TEST_IS_DIR)) {
				
				if (filter_list!=NULL) {
					if (!ignore_pattern_matches(short_filename, filter_list)) {
						result_list=g_slist_prepend(result_list, (gpointer)short_filename);
					}
				} else {
					result_list=g_slist_prepend(result_list, (gpointer)short_filename);
				}
			}
			
		} else {
						
			if (!g_file_test(temp_file, G_FILE_TEST_IS_DIR)) {
				
				if (filter_list!=NULL) {
					if (!ignore_pattern_matches(short_filename, filter_list)) {
						result_list=g_slist_prepend(result_list, (gpointer)temp_file);
					}
				} else {
					result_list=g_slist_prepend(result_list, (gpointer)temp_file);
				}
			}
		}
	}
	
	result_list=g_slist_sort(result_list, compare_func);
	
	return result_list;
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
	number_of_files=0;
	
	GtkTreeIter dot_folder_iterator;

	add_tree_group(NULL, ADD_CHILD, ".", path/*get_filename_from_full_path(project_path)*/ , TRUE, &(parse_struct.current_iter), NULL);
	//add_tree_file(NULL, ADD_CHILD, project_path , &(parse_struct.current_iter), TRUE, NULL);

	dot_folder_iterator=parse_struct.current_iter;

	prevFileIterValid[currentFilePrevFileIter]=FALSE;
	prevFileIterArray[currentFilePrevFileIter]=parse_struct.current_iter;

	currentFilePrevFileIter++;
	prevFileIterArray[currentFilePrevFileIter]=parse_struct.current_iter;

	parse_struct.depth++;
	
						
	add_tree_file(&parse_struct.current_iter, ADD_CHILD, "<loading...>", &parse_struct.current_iter, FALSE, NULL);

	
	//current_max_depth=0;
	
	//read_folder(GTK_TREE_STORE(model), path, &parse_struct, NULL);
	
	// Expand the dot-folder
	// void expand_tree_row(GtkTreePath *path, gboolean expandChildren);
	
	GtkTreePath *iter_path=gtk_tree_model_get_path(GTK_TREE_MODEL(model), &dot_folder_iterator);

	if (iter_path) 
		expand_tree_row(iter_path,FALSE);
	
	gtk_tree_path_free(iter_path);
	
	return TRUE;
}
