/**
 * string_utils.h - misc string utils for SciteProj
 *
 *  Copyright 2006 Roy Wood, 2009-2012 Andreas Rönnquist
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
#ifndef __HEADER_STRING_UTILS_
#define __HEADER_STRING_UTILS_



// Append a string to a (possibly) existing string
gboolean str_append(gchar **dst, const gchar *src, GError **err);

void debug_printf(const char *st, ...);

char *remove_newline( char *s );

#endif /*__HEADER_STRING_UTILS_*/
