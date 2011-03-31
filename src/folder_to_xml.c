/**
 * folder_to_xml.c - file system to xml convertion for SciteProj
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

#include <stdio.h>
#include <gtk/gtk.h>
#include <string.h>

#include <glib.h>
#include <glib/gstdio.h>

#include <unistd.h>

#include "folder_to_xml.h"

/*
 *
 */
gchar *directory_string=0;

gchar *base_folder=0;

FILE *save_file=NULL;

gchar *stored_current_dir;

/**
 *
 */
gchar *get_short_folder_name(gchar *base,gchar *current)
{
	int co=0;
	
	gchar *result=current+strlen(base);
	
	// make sure the returned string don't start with a dir separator
	co=0;
	size_t len=strlen(base);
	do {
		if (result[0]==G_DIR_SEPARATOR) result++;
		co++;
	} while ((result[0]==G_DIR_SEPARATOR) && (co<len));
		
	return result;
}

/**
 *
 */
void print_indent(int in)
{
	int co;
	for (co=0;co<in;co++) { 
		//g_printf("  "); '
		if (save_file!=NULL) {
			fprintf(save_file,"  ");
		}
	}
}

/**
 * traverse_folder
 *		returns FALSE if folder isn't valid
 */
gboolean traverse_folder(gchar *folder,int *indent,gboolean ignore_hidden,int max_depth)
{
	gchar *folder_string=g_strdup_printf("%s%c",folder,G_DIR_SEPARATOR);
	
	int newin=*indent;
	
	gchar *newfolder;
	
	//int chdir_result=chdir(folder_string);
	// switch to wanted directory, return FALSE if not possible
	if (g_chdir(folder_string)!=0) {
		return FALSE;
	}
	
	gboolean ignore_item=FALSE;
	
	(*indent)++;
	
	GDir *dir=g_dir_open(".",0,NULL);
	
	g_free(folder_string);
	
	const gchar *file;
	
	gchar *current_dir=g_get_current_dir();
	
	print_indent(newin);
	
	gchar *trimmed=g_strdup(folder);
	
	gchar *suffix_string=g_strdup_printf("%c",G_DIR_SEPARATOR);
	
	if (g_str_has_suffix(trimmed,suffix_string)) {
		
		g_free(trimmed);
		trimmed=g_strndup(folder,strlen(folder)-1);
	}
	
	g_free(suffix_string);
	
	if (save_file!=NULL) {
		fprintf(save_file,"<group name='%s' expanded='TRUE'>\n",trimmed);
	}
	
	g_free(trimmed);


	while((file=(gchar*)g_dir_read_name(dir)))
	{
		if (file!=NULL) {
			
			ignore_item=FALSE;
			
			if (ignore_hidden) {
				if (file[0]=='.') {
					ignore_item=TRUE;
				}
			}
			
			// Ignore hidden folders
			if (!ignore_item) {
			
				newfolder=g_strdup_printf("%s%c",file,G_DIR_SEPARATOR);
				
				if (g_file_test(newfolder,G_FILE_TEST_IS_DIR)) {
					// success, we can step into that folder
					
					// check if we are below or equal to the max_depth
			
					if ((*indent)<=max_depth) {
						traverse_folder((char*)newfolder,indent,ignore_hidden,max_depth);
					}
				} else {
					print_indent(*indent);
					
					gchar *folder=get_short_folder_name(stored_current_dir,current_dir);
					
					if ((folder!=NULL) && ((int)(strlen(folder))!=0)) {
						
						if (save_file!=NULL) fprintf(save_file,"<file>./%s/%s</file>\n",folder,file);
					} else {
						
						if (save_file!=NULL) fprintf(save_file,"<file>./%s</file>\n",file);
					}
				}
				
				g_free(newfolder);
			}
		
		} else {
			break;
		}
		
	// end of while loop
	}
	
	g_free(current_dir);
	
	g_dir_close(dir);

	print_indent(newin);
	//g_printf("</group>\n");
	if (save_file!=NULL) {
		fprintf(save_file,"</group>\n");
	}
						
	if (g_chdir("..")!=0) {
		return FALSE;
	}
						
	(*indent)--;
	
	// success
	return TRUE;
}

/**
 *	folder_to_xml
 *
 *	@param folder folder to begin traverse
 * @param save_filename filename to save resulting XML in
 *	
 * @return gboolean TRUE if success, otherwise FALSE
 */
gboolean folder_to_xml(gchar *folder,gchar *save_filename_in,int max_depth)
{
	gboolean result=FALSE;
	
	gchar *save_filename;
	
	stored_current_dir=g_get_current_dir();

	directory_string=(gchar*)folder;

	save_filename=g_build_filename(stored_current_dir,save_filename_in,NULL);
	
	if (!g_file_test(directory_string,G_FILE_TEST_IS_DIR)) {
		g_error("Error: Invalid directory!");
		goto EXIT_POINT;
	}
	
	// Check if the directory is valid
	if (g_chdir(directory_string)!=0) {
		g_error("Error: Couldn't g_chdir to directory");
		goto EXIT_POINT;
	}
	
	if (save_filename!=NULL) {
		
		if ((save_file=fopen(save_filename,"w"))==NULL) {
			g_error("Couldn't create file with the name %s!",save_filename);
			goto EXIT_POINT;
		}
		
	} else {
		save_file=stdout;
	}
	
	if (save_file!=NULL) fprintf(save_file,"<root identifier='sciteproj project file'>\n");
	
	int q=0;
	
	base_folder=g_strdup(folder);
	
	traverse_folder(folder,&q,TRUE,max_depth);
	
	if (save_file!=NULL) {
		fprintf(save_file,"</root>\n");
	}

	if (save_filename!=NULL) {	
		fclose(save_file);
		save_file=NULL;
	}
	
	// change back to the stored directory
	if (g_chdir(stored_current_dir)!=0) {
		goto EXIT_POINT;
	}
	
	// it worked fine
	result=TRUE;

EXIT_POINT:	
	g_free(stored_current_dir);
	
	g_free(base_folder);
	
	g_free(save_filename);
	
	return result;
}
