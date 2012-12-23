/**
 * load_folder.h - folder loading support for sciteproj
 *
 *  Copyright 2012 Andreas Rönnquist
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
#ifndef __HEADER_LOAD_FOLDER_
#define __HEADER_LOAD_FOLDER_

gboolean ignore_pattern_matches(gchar *folder_name, const gchar *filename, GSList *filter_list);

gboolean load_folder(gchar *project_path, GError **err);

GSList *load_folder_to_list(gchar *folder_path, gboolean read_directories, GCompareFunc compare_func, GSList *filter_list);


#endif /*__HEADER_LOAD_FOLDER_*/
