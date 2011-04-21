/**
 * main.c - main for SciteProj
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

#include <stdlib.h>

#include <gtk/gtk.h>
#include <glib.h>
#include <string.h>

#include "clicked_node.h"
#include "gui.h"
#include "tree_manipulation.h"
#include "prefs.h"
#include "folder_to_xml.h"
#include "graphics.h"
#include "scite_utils.h"
#include "about.h"
#include "file_utils.h"
#include "string_utils.h"

static struct CommandLineIndata {
	const gchar *scite_filename;
	gchar *file_to_load;
} cmd;


static void parse_cmd_options(int argc,char *argv[]);


/*
 *		Program main entry
 */
int main(int argc, char *argv[])
{
	int returnCode = EXIT_FAILURE;
	GError *err = NULL;
	
	parse_cmd_options(argc,argv);
	
	// Init gtk
	gtk_init(&argc, &argv);
	
	g_thread_init(NULL);
	
	init_file_utils();
	
	// Init sciteproj prefs
	if (!init_prefs(&err)) {
		g_print("Error initing preferences: %s", err->message);
		return EXIT_FAILURE;
	}
	
	// check environment variable
	gchar *scite_path_env=getenv("SciTE_HOME");
	
	gchar *env_filename=NULL;
	
	// test for scite
	if (scite_path_env!=NULL) {
		env_filename=g_build_filename(scite_path_env,"scite",NULL);
		if (g_file_test(env_filename,G_FILE_TEST_EXISTS)) {
			if (cmd.scite_filename==NULL) {
				cmd.scite_filename=g_strdup(env_filename);
			}
		} else {
			g_warning("Environment variable exists, but doesn't point to a folder containing scite.");
		}

		if (env_filename!=NULL) g_free(env_filename);
		env_filename=g_build_filename(scite_path_env,"SciTE",NULL);
		if (g_file_test(env_filename,G_FILE_TEST_EXISTS)) {
			if (cmd.scite_filename==NULL) {
				cmd.scite_filename=g_strdup(env_filename);
			}
		} else {
			g_warning("Environment variable exists, but doesn't point to a folder containing scite.");
		}
	}
	
	// do we have a custom scite executable string as command line option?
	if (cmd.scite_filename!=NULL) {
		
		// Does SciTE exist at that location?
		if (g_file_test(cmd.scite_filename,G_FILE_TEST_IS_REGULAR)) {
			
			// If we have already allocated memory for scite path, free it
			if (gPrefs.scite_path!=NULL) g_free(gPrefs.scite_path);
		
			// Set the new one
			gPrefs.scite_path=g_strdup(cmd.scite_filename);
			
		} else {
			g_print("Couldn't find a SciTE executable named '%s'!\n",cmd.scite_filename);
			g_print("Checking for SciTE in the standard locations instead.\n");
		}
	} 
	
	//g_option_context_free(context);
	
	/*
	 * Any "used" options has been removed from the argv/argc array here.
	 */
	
	// Check for SciTE
	if (!check_if_scite_exists()) {
		GtkWidget *warningDialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, 
				"Warning! Couldn't locate SciTE!\n"
				"Program will start, but you won't be able to open SciTE to edit files.");
		gtk_dialog_run(GTK_DIALOG(warningDialog));
		gtk_widget_destroy(warningDialog);
	}
	
	// Set up the GUI
	if (!setup_gui(&err)) {
		g_print("Could not initialize application globals: %s\n", err->message);
		goto EXITPOINT;
	}
	
	
	gchar *file_to_load=NULL;
	if (cmd.file_to_load!=NULL) {
		file_to_load=cmd.file_to_load;
	} else {
		if (gPrefs.file_to_load!=NULL) file_to_load=gPrefs.file_to_load;
	}
	
	
	// Was a project file specified on the command line?
	if (file_to_load!=NULL) {
		if (!load_project(file_to_load, &err)) {
			GtkWidget *errDialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "An error occurred while trying to load the specified project file: %s", err->message);
			gtk_dialog_run(GTK_DIALOG(errDialog));
			gtk_widget_destroy(errDialog);
			
			g_error_free(err);
			err = NULL;
		}
	}
	
	if (cmd.file_to_load!=NULL) {
		g_free(cmd.file_to_load);
		cmd.file_to_load=NULL;
	}
	
	init_scite_connection();
	
	// Run the app
	
	gtk_main();
	
	returnCode = EXIT_SUCCESS;
	
EXITPOINT:
	
	gui_close();
	
	if (err) g_error_free(err);
	
	return returnCode;
}


/**
 *		parse_cmd_options
 *		Parses the argc/argv data
 */
static void parse_cmd_options(int argc,char *argv[])
{
	gint i;
	
	gchar *indata_left=NULL;

	
	for (i=1;i<argc;i++) {
		if (!strncmp(argv[i],"--help",6)) {
			g_print("\n");
			g_print("Usage: %s [OPTION] [FILE]...",
				g_basename(argv[0]));
			
			g_print("\n\n");
			g_print("%s\n\n", "Options can be one of the following:");
			g_print("%s\n", "  --help                            display this help and exit");
			g_print("%s\n", "  --version                         show version of sciteproj and exit");
			g_print("%s\n", "  --scite FILENAME                  set a filename for the instance of SciTE to open");			
			g_print("%s\n", "  --generate FILENAME [MAX_DEPTH]   generate a sciteproj project file with name FILENAME,");
			g_print("%s\n", "                                      recursively from current folder contents, at most");
			g_print("%s\n", "                                      MAX_DEPTH folders down in the hierarchy");
			g_print("\n");
			exit(EXIT_SUCCESS);
			
		} else if (!strncmp(argv[i],"--scite",7)) {
			const gchar *p = argv[i + 1];

			//cmd.set_scite_folder = TRUE;
			cmd.scite_filename= NULL;
			if (p && *p != '\0' && *p != '-') {
				/*
				if (!strncmp(p, "mailto:", 7))
					cmd.scite_folder = p + 7;
				else
				*/
				cmd.scite_filename= p;
				i++;
			}	
		} else if (!strncmp(argv[i],"--version",9)) {
			
			show_version();
			exit(EXIT_SUCCESS);
		} else if (!strncmp(argv[i],"--generate",10)) {
			
			if ((argc!=3) && (argc!=4)) {
				
				printf("\nThe syntax for that command is:\n\n");
				printf("sciteproj --generate FILENAME [MAX_DEPTH]\n\n");
				
				exit(EXIT_FAILURE);
			}
			
			gchar *filename=argv[2];
			int max_depth=4;
			
			// 4 inputs (means we should expect an max_depth value too)
			// sciteproj --generate file.xml [depth]
			if (argc==4) {
				gchar *max_depth_s=argv[3];
				
				if (!is_integer(max_depth_s)) {
					printf("Expected an integer as max_depth...\n\n");
					exit(EXIT_FAILURE);
				} else {
					max_depth=atoi(max_depth_s);
				}
			}
			
			//gboolean result=folder_to_xml(folder,filename,max_depth);
			gboolean result=folder_to_xml(".",filename,max_depth);
			
			if (result) {
				printf("Generated '%s' successfully!\n\n",filename);
			} else {
				exit(EXIT_FAILURE);
			}
			
			exit(EXIT_SUCCESS);
		} else {
			
			indata_left=argv[i];
		}
	}
	
#ifdef _DEBUG
	if (cmd.scite_filename!=NULL) {
		g_print("scite_filename:%s\n",cmd.scite_filename);
	}
#endif
	
	if (indata_left!=NULL) {
		cmd.file_to_load=g_strdup(indata_left);
	}
	
}
