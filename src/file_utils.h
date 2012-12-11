/**
 * file_utils.h - file utilities for SciteProj
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
#ifndef __HEADER_FILE_UTILS_
#define __HEADER_FILE_UTILS_

/*
 *
 */
extern gchar *current_directory;

/*
 *
 */
void init_file_utils();
gchar *fix_path(gchar *base_dir,gchar *original);
gboolean is_separator(gchar ch);

gchar *fix_separators(gchar *source);

// get the filename from a full path+filename
gchar *get_filename_from_full_path(gchar *src);

// Convert an absolute file path to a relative file path
gboolean abs_path_to_relative_path(const gchar *absPath, gchar **relativePath, const gchar *basePath, GError **err);

// Convert a relative file path to an absolute file path
gboolean relative_path_to_abs_path(gchar *relativePath, gchar **absPath, const gchar *basePath, GError **err);

#endif /*__HEADER_FILE_UTILS_*/
