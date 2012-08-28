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

#include <string.h>
#include <glib.h>
#include <glib/gi18n.h>

#include <locale.h>


#include "file_utils.h"


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

	gchar *pointer=infile;


	gchar *out_path=g_strdup_printf("%s",curr);
	gchar *current_path=0;


	gchar *file_pointer=pointer;

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

					file_pointer=&infile[co];

					gchar *buf=path_up_string(out_path);

					g_free(out_path);
					out_path=buf;

					g_free(current_path);
					current_path=0;

				}
			}
		}
	} else {
		file_pointer=infile++;
	}

	if (current_path!=0) g_free(current_path);

	gchar *new_res=g_strdup_printf("%s%c%s",out_path,G_DIR_SEPARATOR,file_pointer);

	g_free(curr);
	//g_free(infile);

	return new_res;
}


/**
 *
 */
void init_file_utils()
{
	gchar *temp=NULL;
	temp=g_get_current_dir();
	current_directory=fix_separators(temp);

	g_free(temp);

	if (current_directory[strlen(current_directory)-1]!=G_DIR_SEPARATOR) {

	}
}
