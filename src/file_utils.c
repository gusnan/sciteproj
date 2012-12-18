/**
 * file_utils.c - file utilities for SciteProj
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

#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <glib/gi18n.h>

#include <unistd.h>
#include <stdlib.h>

#include <locale.h>

#include <errno.h>

#include "string_utils.h"
#include "file_utils.h"
#include "load_folder.h"
#include "sort.h"


/**
 *
 */
#define APP_SCITEPROJ_ERROR g_quark_from_static_string("APP_STRINGUTILS_ERROR")

/**
 *
 */
gchar *current_directory=0;


/**
 *
 */
gboolean is_separator(gchar ch)
{
	gboolean result=FALSE;

	if ((ch=='/') || (ch==G_DIR_SEPARATOR) || (ch=='\\')) {
		result=TRUE;
	}
	return result;
}


/**
 *
 */
gchar *path_up_string(gchar *instring)
{
	int len=(int)strlen(instring);
	int co;

	int start=len--;

	if (is_separator(instring[start])) {
		start--;
	}

	int res=-1;
	for (co=start;co>0;co--) {
		if (is_separator(instring[co])) {
			if (res==-1) res=co;
		}
	}

	gchar *resstring=g_strdup_printf("%s",instring);

	if (res!=-1) {
		resstring[res]='\0';
	}

	return resstring;
}


/**
 *		fix_separators
 *
 *		Replaces \\ and \ and // with /, and replaces stuff like /./ with just one separator
 */
gchar *fix_separators(gchar *source)
{
	gchar *result=g_strdup(source);
	gchar *pointer=result;

	if (pointer) {
		int co=0;
		for (co=0;co<strlen(source);co++) {
			if (is_separator(source[co])) {
			// if ((*pointer==G_DIR_SEPARATOR) || (*pointer=='\\') || (*pointer=='/')) {

				*pointer='/';

				// skip /./ and similar
				if ((source[co+1]=='.') && (is_separator(source[co+2]))) {
					co+=2;
				}


			} else {
				*pointer=source[co];
			}
			pointer++;

		}

		*pointer='\0';
	}

	return result;
}


/**
 *
 */
gchar *fix_path(char *base_dir,char *temp)
{
	// first, make slashes real ones for the platform.
	gchar *curr=g_strdup(base_dir);

	gchar *infile=fix_separators(temp);

	//gchar *pointer=infile;


	gchar *out_path=g_strdup_printf("%s",curr);
	gchar *current_path=0;


	if (!g_path_is_absolute(infile)) {
		int co;
		for (co=0;co<(int)strlen(infile);co++) {
			char ch=infile[co];

			if (!is_separator(ch)) {

				if (current_path!=NULL) {
					gchar *buf=g_strdup_printf("%s%c",current_path,ch);

					g_free(current_path);

					current_path=buf;

				} else {
					current_path=g_strdup_printf("%c",ch);
				}

			} else {
				//g_print("%s\n",current_path);

				if (strcmp(current_path,"..")==0) {
					//g_print("Up! curr:%s\n",out_path);

					//file_pointer=&infile[co];

					gchar *buf=path_up_string(out_path);

					g_free(out_path);
					out_path=buf;

					g_free(current_path);
					current_path=0;

				}
			}
		}
	} else {
		infile++;
	}
	
	gchar *tempfile=infile;
	
	do {
	
		if ((tempfile[0]=='.') && (tempfile[1]==G_DIR_SEPARATOR)) {
			tempfile+=2;
		}
	
	} while((tempfile[0]=='.') && (tempfile[0]==G_DIR_SEPARATOR));

	if (current_path!=0) g_free(current_path);

	//gchar *new_res=g_strdup_printf("%s%c%s",out_path,G_DIR_SEPARATOR,get_filename_from_full_path(file_pointer));
	gchar *new_res=g_strdup_printf("%s%c%s",out_path,G_DIR_SEPARATOR,tempfile);

	g_free(curr);
	//g_free(infile);

	return new_res;
}


/**
 *
 */
void init_file_utils()
{
	gchar *temp=g_get_current_dir();
	current_directory=fix_separators(temp);

	g_free(temp);

	if (current_directory[strlen(current_directory)-1]!=G_DIR_SEPARATOR) {

	}
}


/**
 *	get_filename_from_full_path
 */
gchar *get_filename_from_full_path(gchar *src)
{
	gchar *pointer;
	gchar *result=NULL;
	gboolean slashFound=FALSE;

	g_assert(src!=NULL);

	pointer=src;

	do {
		if ((*pointer==G_DIR_SEPARATOR) || (*pointer=='\\') || (*pointer=='/')) {

			// point to the character after the slash
			result=++pointer;
			slashFound=TRUE;
		}

		pointer++;

	} while((*pointer)!='\0');

	if (!slashFound) result=src;

	return result;
}


/**
 * Convert an absolute file path to a relative file path.
 *
 * @return TRUE on success, FALSE on failure (further details returned in err)
 *
 * @param absPath is the absolute file path
 * @param relativePath returns the relative file path (must be later freed with g_free() )
 * @param basePath is the base file path against which the relative path is determined; pass NULL if the current working directory should be used as the base path
 * @param err returns any errors
 */
gboolean abs_path_to_relative_path(const gchar *absPath, gchar **relativePath, const gchar *basePath, GError **err)
{
	g_assert(absPath != NULL);
	g_assert(relativePath != NULL);

	gboolean finalResult = FALSE;
	gchar *localBasePath = NULL;
	char *workingDir = NULL;
	int localBasePathLength = 0;
	int absPathLength = strlen(absPath);
	int i = 0;
	int dirSlashOffset = 0;


	if (!g_path_is_absolute(absPath)) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: Specified path, '%s', is not absolute", __func__ , absPath);

		goto EXITPOINT;
	}

	// If no basepath specified, use current working dir

	if (!basePath) {
		workingDir = getcwd(NULL, 0);

		if (!workingDir) {
			g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: Could not determine current working directory, getcwd() = NULL, errno = %d", __func__ , errno);

			goto EXITPOINT;
		}

		basePath = workingDir;

	}

	// Set up a local copy of the base path, and ensure it ends with a '/'

	if (!str_append(&localBasePath, basePath, err)) {
		goto EXITPOINT;
	}

	localBasePathLength = strlen(localBasePath);

	if (localBasePathLength <= 0 || localBasePath[localBasePathLength - 1] != G_DIR_SEPARATOR) {
		if (!str_append(&localBasePath, G_DIR_SEPARATOR_S, err)) {
			goto EXITPOINT;
		}

		++localBasePathLength;
	}


	// Start relative path with local dir

	if (!str_append(relativePath, "./", err)) {
		goto EXITPOINT;
	}

	// Skip over matching parent dirs

	while (i < localBasePathLength && i < absPathLength && localBasePath[i] == absPath[i]) {
		if (absPath[i] == G_DIR_SEPARATOR) {
			dirSlashOffset = i + 1;
		}

		++i;
	}

	// Step up one dir for every dir in the leftover part of the base path

	for (i = dirSlashOffset; i < localBasePathLength; ++i) {
		if (localBasePath[i] == G_DIR_SEPARATOR) {
			if (!str_append(relativePath, "../", err)) {
				goto EXITPOINT;
			}
		}
	}

	// Finally, add the leftover part of the absolute path

	if (!str_append(relativePath, absPath + dirSlashOffset, err)) {
		goto EXITPOINT;
	}

	finalResult = TRUE;


EXITPOINT:

	free(workingDir);
	if (localBasePath) g_free(localBasePath);

	return finalResult;
}


/**
 * Convert a relative file path to an absolute file path.
 *
 * @return TRUE on success, FALSE on failure (further details returned in err)
 *
 * @param relativePath is the relative file path
 * @param absPath returns the absolute file path (must be later freed with g_free() )
 * @param basePath is the base file path used to formulate the absolute path; pass NULL if the current working directory should be used as the base path
 * @param err returns any errors
 */
gboolean relative_path_to_abs_path(gchar *relativePath, gchar **absPath, const gchar *basePath, GError **err)
{
	g_assert(absPath != NULL);
	g_assert(relativePath != NULL);

	int i=0;
	gboolean finalResult = FALSE;
	char *workingDir = NULL;
	gchar *localAbsPath = NULL;
	int absPathLength;
	int co;

	for (co=0;co<strlen(relativePath);co++) {
		if ((relativePath[co]=='/') || (relativePath[co]=='\\')) {
			relativePath[co]=G_DIR_SEPARATOR;
		}
	}

	// On linux check if first character is '/', and on windows check for something
	// like "C:" in the beginning of the string
	if (relativePath[0] == G_DIR_SEPARATOR) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: Specified path, '%s', is absolute", __func__ , relativePath);
		g_print("Separator at birth.\n");
		goto EXITPOINT;
	}


	// If no basepath specified, use current working dir

	if (!basePath) {
		debug_printf("No basepath.\n");

		basePath = g_get_current_dir ();
	}

	// Absolute path is base path + '/' + relative path

	if (!str_append(&localAbsPath, basePath, err)) {
		goto EXITPOINT;
	}

	if (!str_append(&localAbsPath, G_DIR_SEPARATOR_S, err)) {
		goto EXITPOINT;
	}

	//// Only do this if the relativepath REALLY is a realative path.
	if (!str_append(&localAbsPath, relativePath, err)) {
		goto EXITPOINT;
	}

	// Now go through and collapse elements like  "/./" and "/foo/../" and "//"

	absPathLength = strlen(localAbsPath);

	for (i = 0; i < absPathLength; ) {
		gchar *tail = NULL;
		int tailLength;

		if (g_str_has_prefix(localAbsPath + i, "/./")) {
			tail = localAbsPath + i + 2;
			tailLength = absPathLength - i - 1;
		}
		else if (g_str_has_prefix(localAbsPath + i, "\\./")) {
			tail = localAbsPath + i + 2;
			tailLength = absPathLength - i - 1;
		}
		else if (g_str_has_prefix(localAbsPath + i, "\\.\\")) {
			tail = localAbsPath + i + 2;
			tailLength = absPathLength - i - 1;
		}
		else if (g_str_has_prefix(localAbsPath + i, "//")) {
			tail = localAbsPath + i + 1;
			tailLength = absPathLength - i;
		}
		else if (
				(g_str_has_prefix(localAbsPath + i, "/../")) ||
				(g_str_has_prefix(localAbsPath + i, "\\..\\"))
				) {
			tail = localAbsPath + i + 3;
			tailLength = absPathLength - i - 2;

			do {
				i = (i > 0) ? i - 1 : 0;
			} while (i > 0 && !is_separator(localAbsPath[i])/* != G_DIR_SEPARATOR*/);
		}

		if (tail != NULL) {
			g_memmove(localAbsPath + i, tail, tailLength);

			absPathLength -= (tail - localAbsPath - i);
		}
		else {
			++i;
		}
	}

	*absPath = localAbsPath;

	localAbsPath = NULL;

	finalResult = TRUE;

EXITPOINT:

	if (workingDir) free(workingDir);
	if (localAbsPath) g_free(localAbsPath);

	return finalResult;
}


/**
 *
 */
gchar *get_file_extension(gchar *filename)
{
	gchar *result=g_strrstr(filename,".");

	if (!result) result="";

	return result;
}


/**
 *
 */
int get_number_of_files_in_folder(gchar *folder_name)
{
	int result=0;
	
	GSList *file_list;
	GSList *folder_list;
			
	file_list=load_folder_to_list(folder_name, FALSE, file_sort_by_extension_bigger_func, NULL);
	folder_list=load_folder_to_list(folder_name, TRUE, compare_strings_bigger, NULL);
	
	if (file_list) {
		while (file_list)	{
			result++;
			file_list=file_list->next;
		};
	}
	
	if (folder_list) {
		while (folder_list)	{
			result++;
			folder_list=folder_list->next;
		};
	}

	return result;
}


/**
 * is_string_folder
 * @return gboolean TRUE if the entered string represent a folder in the 
 *         systems folder structure.
 */
gboolean is_string_folder(gchar *instring)
{
	gboolean result=FALSE;
	
	GDir *dir=g_dir_open(instring, 0, NULL);
	
	if (dir) {
		result=TRUE;
	}
	
	if (dir) g_dir_close(dir);
	
	return result;
}