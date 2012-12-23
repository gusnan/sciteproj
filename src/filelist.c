/**
 * filelist.c - list containing all files in the project
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
 
#include <gtk/gtk.h>
#include <glib.h>

#include <stdio.h>
#include <stdlib.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <unistd.h>

#include <string.h>

#include "filelist.h"

#include "file_utils.h"

#include "tree_manipulation.h"

#include "script.h"

#include "load_folder.h"


/**
 *
 */
GList *list=NULL;
gchar *project_dir=NULL;

GSList *filter_list=NULL;


/**
 *
 */
gboolean filelist_traverse_folder(gchar *folder, gint *current_depth, gint max_depth)
{
	if (chdir(folder)!=0) {
		return FALSE;
	}
	
	GDir *current_dir = g_dir_open(".", 0, NULL);
	
	gchar *current_file="";
	
	(*current_depth)++;
	
	GList *folder_list=NULL;
	GList *file_list=NULL;
	
	while ((current_file = (gchar*)(g_dir_read_name(current_dir))))
	{
		if (current_file) {
			
			gchar *fixed_name = g_strdup_printf("%s%c", current_file, G_DIR_SEPARATOR);
				// Ignore hidden folders
			if (current_file[0]!='.') {
				if (g_file_test(current_file, G_FILE_TEST_IS_DIR)) {
	
					gchar *temp_folder_name = g_strdup_printf("%s%s", folder, fixed_name);
					folder_list = g_list_prepend(folder_list, (gpointer)temp_folder_name);

				} else {
					gchar *full_filename = g_strdup_printf("%s%s", folder, current_file);
					file_list = g_list_prepend(file_list, (gpointer)full_filename);
				}
			}
			g_free(fixed_name);
		}
	}
	
	GList *current_item=folder_list;
	do {
		if (current_item) {
			gchar *current = (gchar*)(current_item->data);
			
			if ((*current_depth)<max_depth) {
				gboolean match = FALSE;
				
				if (filter_list) {
					if (ignore_pattern_matches(folder, current, filter_list)) match=TRUE;
				}
				
				if (!match)
					filelist_traverse_folder(current, current_depth, max_depth);
			}
				
			current_item = current_item->next;
		}
	} while(current_item);
	
	current_item=file_list;
	do {
		if (current_item) {
		
			// filename (without path) - a pointer in the string is returned
			// - no need to free the result
			gchar *filename=get_filename_from_full_path((gchar*)current_item->data);
			
			gboolean match=FALSE;
			
			if (filter_list) {
				if (ignore_pattern_matches(folder, filename, filter_list)) match=TRUE;
			}
			
			if (!match)
				add_item((gchar*)(current_item->data), filename);
			
			current_item = current_item->next;
		}
	} while(current_item);
	
	(*current_depth)--;
	
	g_dir_close(current_dir);
		
	return TRUE;
}


/**
 * init the list of files in the project
 *
 */
gboolean init_filelist(gchar *folder)
{
	// go through the folder and sub-folders to a depth of ... 3?

	int current_depth=0;

	gchar *current_directory = g_get_current_dir();
	
	filter_list=load_filter_from_lua();

	if (folder[strlen(folder)-1]!='/') {
		project_dir = g_strdup_printf("%s/", folder);
	} else {
		project_dir = g_strdup(folder);
	}

	filelist_traverse_folder(project_dir, &current_depth, 3);

	if (chdir(current_directory)!=0) {
		return FALSE;
	}

	g_free(current_directory);
	g_free(project_dir);
	
	// TODO! fix going through list, and release every filter
	g_slist_free(filter_list);

	return TRUE;
}


/**
 * free the data of every item
 */
static void free_list_data(gpointer data,gpointer priv)
{
	File *file=(File*)data;

	g_free(file);
}


/**
 *
 */
void done_filelist()
{
	// go through the list and free all
	g_list_foreach(list,free_list_data,NULL);

	// free the actual list structure
	if (list!=NULL) g_list_free(list);

}


/**
 *
 */
static void printitem(gpointer data,gpointer priv)
{
	File *file=(File*)data;

	gchar *filename=file->filename;
	gchar *full_path=file->full_path;

	printf("File: %s, %s\n",filename,full_path);

}


/**
 *
 */
void print_filelist()
{
	printf("Filelist:-----------------\n");
	g_list_foreach(list,printitem,NULL);
}


/**
 *
 */
File *exists_in_list(gchar *full_path)
{
	File *result=NULL;

	GList *iter=list;

	while(iter) {

		File *data=(File*)iter->data;

		if (result==NULL) {

			//printf("N:%s compared to %s\n",data->full_path,full_path);

			if (g_strcmp0(data->full_path,full_path)==0) {
				result=data;
			}
		}

		iter=iter->next;
	}

	return result;

}


/**
 *
 */
void add_item(gchar *filename, gchar *full_path)
{
	File *newFile;

//	printf("Full:%s\n",full_path);
	//newFile=exists_in_list(full_path);

	//if (newFile==NULL) {
		// doesn't exist in list, create it

		newFile=(File*)(g_malloc0(sizeof(File)));

		newFile->filename=g_strdup(filename);
		newFile->full_path=g_strdup(full_path);
		//newFile->count=1;

		list=g_list_append(list,newFile);

	/*
	} else {
		newFile->count++;
	}
	*/
}


/**
 *
 */
gboolean remove_item(gchar *filename, gchar *full_path)
{
	File *file=exists_in_list(full_path);

	if (file) {

			// if the usage count is zero, remove the object from the list
		list=g_list_remove(list,file);
		return TRUE;
	}

	return FALSE;
}
