/**
 * load_folder.c - folder loading support for sciteproj
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
#include <gtk/gtk.h>

#include "tree_manipulation.h"

#include "load_folder.h"

/**
 *
 */
gboolean load_folder(gchar *project_path, GError **err)
{
	if (project_path) {
		
		printf("Project path: %s\n",project_path);
		
		if (!set_project_filepath(project_path, err)) {
			goto EXITPOINT;
		}
	} 
	
EXITPOINT:
	
	return TRUE;
}
