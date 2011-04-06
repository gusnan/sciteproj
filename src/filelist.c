/**
 * filelist.c - list containing all files in the project
 *
 *  Copyright 2009-2011 Andreas Ronnquist
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
	
	printf("File: %s, %s\n",filename,full_path);
	
}

/**
 *
 */
void printlist()
{
	g_list_foreach(list,printitem,NULL);
}

/**
 *
 */
void add_item(gchar *filename, gchar *full_path)
{
	File newFile;
	
	newFile.filename=filename;
	newFile.full_path=full_path;
	
	printf("File: %s, %s\n",filename,full_path);
}


/**
 *
 */
gboolean remove_item(gchar *filename, gchar *full_path)
{
	return FALSE;
}