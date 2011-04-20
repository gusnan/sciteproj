/**
 * string_utils.c - misc string utils for SciteProj
 *
 *  Copyright 2006 Roy Wood, 2009-2011 Andreas Ronnquist
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
#include <ctype.h>

#include "string_utils.h"
#include "file_utils.h"

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>


#define APP_SCITEPROJ_ERROR g_quark_from_static_string("APP_STRINGUTILS_ERROR")

void debug_printf(const char *st, ...);


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
	
	while (localBasePath[i] == absPath[i] && i < localBasePathLength && i < absPathLength) {
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
 * Append a string to an existing string, expanding the allocation of the string.
 *
 * @return TRUE on success, FALSE on failure (further details returned in err)
 *
 * @param dst is the string to append onto
 * @param src is the string to append to dst
 * @param err returns any errors
 */
gboolean str_append(gchar **dst, const gchar *src, GError **err)
{
	gboolean finalResult = FALSE;
	
	g_assert(dst != NULL);
	g_assert(src != NULL);
	
	gulong dstLength = (*dst == NULL) ? 0 : strlen(*dst);
	gulong srcLength = strlen(src);
	gulong totalLength = dstLength + srcLength + 1;
	
	gchar *newDst = (gchar *) g_try_realloc(*dst, totalLength);
	
	if (newDst == NULL) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: Could not allocate memory, g_try_realloc(%ld) = NULL", __func__, totalLength);
		
		goto EXITPOINT;
	}
	
	g_memmove(newDst + dstLength, src, srcLength + 1);
	
	*dst = newDst;
	
	finalResult = TRUE;

EXITPOINT:
	
	return finalResult;
}


/**
 *	get_filename_from_full_path
 */
gchar *get_filename_from_full_path(gchar *src)
{
	gchar *pointer=NULL;
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
 *
 */
void debug_printf(const char *st, ...)
{
	va_list ap;
	va_start(ap,st);
	
#ifdef _DEBUG
	vprintf(st,ap);
#endif
	
	va_end(ap);
}


/**
 *
 */
char *remove_newline( char *s )
{
	int len=strlen(s);
	
	if (s[len-1]=='\n') {
		s[len-1]='\0';
	}
	
	return s;
	 
}

/**
 * returns true if a gchar is an integer
 *	(Very limited functionality at the moment)
 */
gboolean is_integer(gchar *string)
{
	int co;
	
	for (co=0;co<(int)strlen(string);co++) {
		// returns zero if it isn't a digit
		if (isdigit(string[co])==0) {
			return FALSE;
		}
	}
	
	return TRUE;
}

/**
 *
 */
gboolean is_word_character(char ch)
{
	gboolean result=FALSE;
	
	if ((ch=='_') || (g_ascii_isalnum(ch))) result=TRUE;
	
	return result;
}
