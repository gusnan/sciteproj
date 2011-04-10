/**
 * prefs.h - prefs for SciteProj
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
#ifndef __HEADER_PREFS_
#define __HEADER_PREFS_

#define PREFS_BUFSIZE 256

/**
 *
 */
typedef struct {
	int lhs;
	int width, height;
	int verbosity;
	int last_file_filter;
	int xpos,ypos;
	
	int search_xpos,search_ypos;
	int search_width,search_height;
	
	gboolean dirty_on_folder_change;
	gboolean give_scite_focus;
	gboolean search_give_scite_focus;
	
	gboolean allow_duplicates;
	
	gchar *scite_path;
	
	gchar *file_to_load;
	
	gboolean identify_sciteproj_xml;
	
	gboolean show_recent;
	gboolean recent_add_to_bottom;
	
} sciteproj_prefs;

extern sciteproj_prefs gPrefs;

gboolean init_prefs();

#endif /*__HEADER_PREFS_*/
