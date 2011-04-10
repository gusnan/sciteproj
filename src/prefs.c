/**
 * prefs.c - prefs for SciteProj
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
 
#include <gtk/gtk.h>
#include <string.h>

#include "prefs.h"

#include "string_utils.h"

sciteproj_prefs gPrefs;

gchar *default_config_string=(gchar*)"" \
				"# ---------------------------\n"
				"# Configuration for SciteProj\n"
				"# ---------------------------\n"
				"\n"
				"# Window geometry:\n"
				"xpos=40\n"
				"ypos=40\n"
				"width=200\n"
				"height=400\n"
				"\n"
				"# Search window geometry (-1 on xpos and ypos means default center screen):\n"
				"search_xpos=-1\n"
				"search_ypos=-1\n"
				"search_width=500\n"
				"search_height=400\n"
				"\n"
				"#other:\n"
				"give_scite_focus=FALSE\n"
				"search_give_scite_focus=TRUE\n"
				"dirty_on_folder_change=FALSE\n"
				"\n"
				"allow_duplicates=TRUE\n"
				"\n"
				"identify_sciteproj_xml=TRUE\n"
				"\n"
				"show_recent=FALSE\n"
				"recent_add_to_bottom=FALSE\n"
				"\n"
				"\n";


/**
 *		Check config string - is it valid?
 */
gboolean check_config_string(gchar *in_config)
{
	gboolean result=FALSE;
	int co=0;
	gdouble tempdouble;
	
	gchar *tempstring=NULL;

	int pos=-1;
	
	gchar *value=in_config;
	
	// clear scite Path
	
	gPrefs.scite_path=NULL;
	
	for (co=0;co<(int)strlen(in_config);co++) {
		if (in_config[co]=='=') pos=co;
		
		if (pos==-1) {
			value++;
		}
	}
	
	if (pos!=-1) {
		tempstring=g_strndup(in_config,pos);
		value++;
	}
	
	//gchar *new_tempstring,*new_value;
	
	if ((tempstring!=NULL) && (value!=NULL)) {
	
		tempstring=g_strchug(tempstring);
		tempstring=g_strchomp(tempstring);
		//new_tempstring=g_ascii_strup(tempstring,-1);

		value=g_strchug(value);
		value=g_strchomp(value);
		
		if (g_ascii_strcasecmp(tempstring,"xpos")==0) {
			tempdouble=g_ascii_strtod(value,NULL);
			gPrefs.xpos=(int) tempdouble;
		}
		
		if (g_ascii_strcasecmp(tempstring,"ypos")==0) {
			tempdouble=g_ascii_strtod(value,NULL);
			gPrefs.ypos=(int)tempdouble;
		}
		
		if (g_ascii_strcasecmp(tempstring,"width")==0) {
			tempdouble=g_ascii_strtod(value,NULL);
			gPrefs.width=(int)tempdouble;
		}
		
		if (g_ascii_strcasecmp(tempstring,"height")==0) {
			tempdouble=g_ascii_strtod(value,NULL);
			gPrefs.height=(int)tempdouble;
		}
		
		
		if (g_ascii_strcasecmp(tempstring,"search_xpos")==0) {
			tempdouble=g_ascii_strtod(value,NULL);
			gPrefs.search_xpos=(int) tempdouble;
		}
		
		if (g_ascii_strcasecmp(tempstring,"search_ypos")==0) {
			tempdouble=g_ascii_strtod(value,NULL);
			gPrefs.search_ypos=(int)tempdouble;
		}
		
		if (g_ascii_strcasecmp(tempstring,"search_width")==0) {
			tempdouble=g_ascii_strtod(value,NULL);
			gPrefs.search_width=(int)tempdouble;
		}
		
		if (g_ascii_strcasecmp(tempstring,"search_height")==0) {
			tempdouble=g_ascii_strtod(value,NULL);
			gPrefs.search_height=(int)tempdouble;
		}
		
		
		gPrefs.give_scite_focus=FALSE;
		if (g_ascii_strcasecmp(tempstring,"give_scite_focus")==0) {
			
			if (g_ascii_strcasecmp(value,"TRUE")==0) {
				//printf("give_scite_focus=TRUE");
				gPrefs.give_scite_focus=TRUE;
				
			}
		}
		
		gPrefs.search_give_scite_focus=FALSE;
		if (g_ascii_strcasecmp(tempstring,"search_give_scite_focus")==0) {
			
			if (g_ascii_strcasecmp(value,"TRUE")==0) {
				gPrefs.search_give_scite_focus=TRUE;
			}
		}
		
		gPrefs.dirty_on_folder_change=FALSE;
		if (g_ascii_strcasecmp(tempstring,"dirty_on_folder_change")==0) {
			
			if (g_ascii_strcasecmp(value,"TRUE")==0) {
				//printf("give_scite_focus=TRUE");
				gPrefs.dirty_on_folder_change=TRUE;
				
			}
		}
		
		gPrefs.allow_duplicates=TRUE;
		if (g_ascii_strcasecmp(tempstring,"allow_duplicates")==0) {
			if (g_ascii_strcasecmp(value,"FALSE")==0) {
				gPrefs.allow_duplicates=FALSE;
			}
		}
		
		gPrefs.scite_path=NULL;
		if (g_ascii_strcasecmp(tempstring,"scite_path")==0) {
			gPrefs.scite_path=g_strdup_printf("%s",value);
		}
		
		gPrefs.file_to_load=NULL;
		if (g_ascii_strcasecmp(tempstring,"file_to_load")==0) {
			gPrefs.file_to_load=g_strdup_printf("%s",value);
		}
		
		gPrefs.identify_sciteproj_xml=FALSE;
		if (g_ascii_strcasecmp(tempstring,"identify_sciteproj_xml")==0) {
			if (g_ascii_strcasecmp(value,"TRUE")==0) {
				gPrefs.identify_sciteproj_xml=TRUE;
			}
		}
		
		gPrefs.show_recent=FALSE;
		if (g_ascii_strcasecmp(tempstring,"show_recent")==0) {
			if (g_ascii_strcasecmp(value,"TRUE")==0) {
				gPrefs.show_recent=TRUE;
			}
		}
		
		gPrefs.recent_add_to_bottom=FALSE;
		if (g_ascii_strcasecmp(tempstring,"recent_add_to_bottom")==0) {
			if (g_ascii_strcasecmp(value,"TRUE")==0) {
				gPrefs.recent_add_to_bottom=TRUE;
			}
		}
	}
	
	if (tempstring!=NULL) g_free(tempstring);
	
	return result;
}



/**
 *		init_prefs
 */
gboolean init_prefs(GError **err)
{
	//FILE *fp;
	//gchar buf[PREFS_BUFSIZE];
	
	gchar *config_string=NULL;
	
	gboolean result=TRUE;
	
	// Set default values
	gPrefs.lhs=1;
	gPrefs.width=200;
	gPrefs.height=600;
	gPrefs.verbosity=0; // No informational messages
	gPrefs.last_file_filter=-1; // All files (my choice)
	
	gPrefs.search_xpos=-1;
	gPrefs.search_ypos=-1;
	
	gPrefs.search_width=500;
	gPrefs.search_height=400;
	
	gPrefs.give_scite_focus=FALSE;
	gPrefs.search_give_scite_focus=TRUE;
	gPrefs.dirty_on_folder_change=FALSE;
	
	gPrefs.allow_duplicates=TRUE;
	
	gPrefs.show_recent=FALSE;
	gPrefs.recent_add_to_bottom=FALSE;
	
	gchar *config_filename=NULL;
	
	config_filename=g_build_filename(g_get_home_dir(),".sciteproj",NULL);
	
	// Check if a config-file exists
	if (!g_file_test(config_filename,G_FILE_TEST_IS_REGULAR)) {
		
		// No config-file exists, create a new one and write default values
		g_file_set_contents(config_filename,default_config_string,-1,err);
	}

	// Load preferences from config dot-file
	
	if (!g_file_get_contents(config_filename,&config_string,NULL,err)) {
		
		result=FALSE;
		goto ERROR;
	}
	
	// the list of strings
	gchar **list=NULL;
	
	// split out the lines, and add each to the list of strings
	list=g_strsplit(config_string,"\n",-1);
	
	gchar **savedlist=list;
	
	gchar *temp=NULL;
	
	do {
		temp=*list;
		
		if (temp!=NULL) {
		
			if ((temp[0]!='#') && (strcmp(temp,"")!=0)) {
				// We got a valid string:
				// no starting #, and not an empty string.
				
				check_config_string(temp);
			}
			list++;
		}
		
	} while (temp!=NULL);
	
	g_strfreev(savedlist);

ERROR:
	
	g_free(config_filename);
	g_free(config_string);
	
	return result;
}
