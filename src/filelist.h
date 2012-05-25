/**
 * filelist.h - list containing all files in the project
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
#ifndef __HEADER_FILE_LIST_
#define __HEADER_FILE_LIST_


/**
 * variables and structs
 */

struct File {
	gchar *filename;
	gchar *full_path;
	int count;
};

typedef struct File File;

extern GList *list;

/**
 * functions
 */
gboolean init_filelist();
void done_filelist();

void print_filelist();

void add_item(gchar *filename, gchar *full_path);

gboolean remove_item(gchar *filename, gchar *full_path);
File *exists_in_list(gchar *full_path);


#endif /*__HEADER_FILE_LIST_*/
