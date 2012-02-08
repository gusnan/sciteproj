/**
 * main.c - main for SciteProj
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

#include <stdlib.h>

#include <gtk/gtk.h>
#include <glib.h>
#include <string.h>
#include <glib/gi18n.h>

#include <locale.h>

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


/*
 *		Program main entry
 */
int main(int argc, char *argv[])
{
	int returnCode = EXIT_FAILURE;
	GError *err = NULL;
	gchar *file_to_load=NULL;
	GOptionContext *context=NULL;
	
	static gboolean version=FALSE;
	static gchar *scite_instance=NULL;
	//static gboolean gene
	static gchar *generate_xml_file=NULL;
	static int max_depth_generated=-1;
	
	static const GOptionEntry options[]={
		{ "version",		'v',	0, G_OPTION_ARG_NONE,		&version,
			N_("Show program version and quit")},
		{ "scite",			's',	0, G_OPTION_ARG_STRING,		&scite_instance,
			N_("Set a filename for the instance of SciTE to open"),	N_("SCITE_FILENAME")},
		{ "generate",		'g',	0, G_OPTION_ARG_STRING,		&generate_xml_file,
			N_("Generate a sciteproj project file with name XML_FILENAME, recursively from current folder"),
			N_("XML_FILENAME")},
		{ "max_depth",		'm',	0, G_OPTION_ARG_INT,	&max_depth_generated,
			N_("Set maximum depth of folders to read through to MAX_DEPTH when generating project file"),
			N_("MAX_DEPTH")},
		{ NULL }
	};
	
	// Init gettext stuff
	setlocale(LC_ALL,"");
	
	bindtextdomain(PACKAGE,LOCALEDIR);
	bind_textdomain_codeset(PACKAGE,"");
	textdomain(PACKAGE);
	
	gchar *sciteproj_description=NULL;
	sciteproj_description=g_strdup_printf(_("SciTE Project Manager"));
	
	gchar *full_desc_string=g_strdup_printf("- %s",sciteproj_description);
	
	context=g_option_context_new(full_desc_string);
	g_option_context_add_main_entries(context,options,NULL);
	if (!g_option_context_parse(context, &argc, &argv, &err)) {
		g_print(_("option parsing failed: %s"),err->message);
		printf("\n");
		exit(EXIT_FAILURE);
	}
	
	g_free(sciteproj_description);
	g_free(full_desc_string);
	
	/*
		Interpret the options
	 */
	/*
		set instance of SciTE to run
	*/
	if (scite_instance) {
		cmd.scite_filename=scite_instance;
	}
	
	/*
		Generate a project file going down at max max_depth levels in the folder
		hierarchy
	*/
	if (generate_xml_file) {
		int max_depth=4;
		if (max_depth_generated!=-1) max_depth=max_depth_generated;
		
		//gboolean result=folder_to_xml(folder,filename,max_depth);
		gboolean result=folder_to_xml(".",generate_xml_file,max_depth);
		
		if (result) {
			printf(_("Generated '%s' successfully!"),generate_xml_file);
			printf("\n\n");
		} else {
			exit(EXIT_FAILURE);
		}
		
		exit(EXIT_SUCCESS);
		
	}
	
	/*
		Show SciteProj version 
	*/
	if (version) {
		show_version();
		printf("\n");
		exit(EXIT_SUCCESS);
	}

	// If there is any indata left - load it as a sciteproj project
	if (argc>1)
		cmd.file_to_load=g_strdup_printf("%s",argv[1]);
	
	// Init gtk
	gtk_init(&argc, &argv);
	
	g_thread_init(NULL);
	
	init_file_utils();
	
	// Init preferences
	if (!init_prefs(&err)) {
		g_print(_("Error initing preferences: %s"), err->message);
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
			g_warning(_("Environment variable exists, but doesn't point to a folder containing scite."));
		}

		if (env_filename!=NULL) g_free(env_filename);
		env_filename=g_build_filename(scite_path_env,"SciTE",NULL);
		if (g_file_test(env_filename,G_FILE_TEST_EXISTS)) {
			if (cmd.scite_filename==NULL) {
				cmd.scite_filename=g_strdup(env_filename);
			}
		} else {
			g_warning(_("Environment variable exists, but doesn't point to a folder containing scite."));
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
			g_print(_("Couldn't find a SciTE executable named '%s'!\n"),cmd.scite_filename);
			g_print(_("Checking for SciTE in the standard locations instead.\n"));
		}
	} 
	
	//g_option_context_free(context);
	
	/*
	 * Any "used" options has been removed from the argv/argc array here.
	 */
	
	// Check for SciTE
	if (!check_if_scite_exists()) {
		GtkWidget *warningDialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, 
				_("Warning! Couldn't locate SciTE!\n"
				"Program will start, but you won't be able to open SciTE to edit files."));
		gtk_dialog_run(GTK_DIALOG(warningDialog));
		gtk_widget_destroy(warningDialog);
	}
	
	// Set up the GUI
	if (!setup_gui(&err)) {
		g_print("Could not setup the gui: %s", err->message);
		g_print("\n");
		goto EXITPOINT;
	}
	
	
	file_to_load=NULL;
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
	
	done_prefs();
	
	if (err) g_error_free(err);
	
	return returnCode;
}
