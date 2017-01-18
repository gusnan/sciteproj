/**
 * string_utils.c - misc string utils for SciteProj
 *
 *  Copyright 2006 Roy Wood, 2009-2017 Andreas Rönnquist
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
#include <glib/gi18n.h>

#include <locale.h>

#include "string_utils.h"
#include "file_utils.h"

#include <string.h>
#include <unistd.h>
#include <stdlib.h>


#define APP_SCITEPROJ_ERROR g_quark_from_static_string("APP_STRINGUTILS_ERROR")


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

	if (s[len-1] == '\n') {
		s[len-1] = '\0';
	}

	return s;

}

