/**
 * filelist.c - list containing all files in the project
 *
 *  Copyright 2009-2011 Andreas Rönnquist
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
#include <stdio.h>
#include <stdlib.h>

#include "filelist.h"


/**
 *
 */
GList *list=NULL;


/**
 * init the list of files in the project
 * 
 */
gboolean init_filelist()
{
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
	
	printf("File: %d - %s, %s\n",file->count,filename,full_path);
	
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
	newFile=exists_in_list(full_path);
	
	if (newFile==NULL) {
		// doesn't exist in list, create it
		
		newFile=(File*)(g_malloc0(sizeof(File)));
		
		newFile->filename=g_strdup(filename);
		newFile->full_path=g_strdup(full_path);
		newFile->count=1;
		
		list=g_list_append(list,newFile);
		
	} else {
		newFile->count++;
	}		
}


/**
 *
 */
gboolean remove_item(gchar *filename, gchar *full_path)
{
	File *file=exists_in_list(full_path);
	
	if (file) {
		
		if (file->count>1) {
			// decrease the usage count
			file->count--;
		
		} else {
			// if the usage count is zero, remove the object from the list
			list=g_list_remove(list,file);
			
		}
		return TRUE;
	}
	
	return FALSE;
}
