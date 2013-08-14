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

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

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
#include "load_folder.h"
#include "script.h"

static struct CommandLineIndata {
	const gchar *scite_filename;
} cmd;


/*
 *		Program main entry
 */
int main(int argc, char *argv[])
{
	int returnCode = EXIT_FAILURE;
	GError *err = NULL;
	GOptionContext *context=NULL;

	static gboolean version=FALSE;
	static gchar *scite_instance=NULL;
	static gboolean load_a_folder=FALSE;
	static gboolean start_scite = FALSE;

	static const GOptionEntry options[]={
		{ "version",		'v',	0, G_OPTION_ARG_NONE,		&version,
			N_("Show program version and quit")},
		{ "scite",			's',	0, G_OPTION_ARG_STRING,		&scite_instance,
			N_("Set a filename for the instance of SciTE to open"),	N_("SCITE_FILENAME")},
		{ "load_folder",	'l',	0,	G_OPTION_ARG_NONE,		&load_a_folder,
			N_("Load a folder")}, 
		{ "start_scite",	't',	0,	G_OPTION_ARG_NONE,		&start_scite,
			N_("Start SciTE automatically with SciteProj")},
		{ NULL }
	};

	// Init gettext stuff
	setlocale(LC_ALL,"");

	bindtextdomain(PACKAGE,LOCALEDIR);
	bind_textdomain_codeset(PACKAGE,"");
	textdomain(PACKAGE);

	gchar *sciteproj_description=g_strdup_printf(_("SciTE Project Manager"));

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

	init_version_string();

	/*
		Show SciteProj version
	*/
	if (version) {
		show_version();
		printf("\n");
		done_version_string();
		exit(EXIT_SUCCESS);
	}

	// Init gtk
	gtk_init(&argc, &argv);

	g_type_init();

	init_file_utils();

	gchar *current_dir=g_get_current_dir();

	if (argc>2) {
		printf("A folder is expected as parameter to sciteproj...\n");
		return EXIT_FAILURE;
	}

	gchar *dir_to_load;
	if (argc==1) { // only "sciteproj" on the command-line
		dir_to_load=current_dir;

	} else { // "sciteproj <folder_name>" on the command-line
		dir_to_load=argv[1];

		gchar *newpath;

		if (relative_path_to_abs_path(dir_to_load, &newpath, current_dir, NULL)) {
			dir_to_load=newpath;
		}
	}

	// Init preferences
	if (!init_prefs(dir_to_load, &err)) {
		/*
		g_print(_("Error initing preferences: %s"), err->message);
		done_version_string();
		return EXIT_FAILURE;
		*/
	}

	// check environment variable
	gchar *scite_path_env=getenv("SciTE_HOME");

	// test for scite
	if (scite_path_env!=NULL) {
		gchar *env_filename=g_build_filename(scite_path_env,"scite",NULL);
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
			if (prefs.scite_path!=NULL) g_free(prefs.scite_path);

			// Set the new one
			prefs.scite_path=g_strdup(cmd.scite_filename);

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

	if (!is_string_folder(dir_to_load)) {
		printf("Not a valid folder!\n");
		return EXIT_FAILURE;
	}

	// Should we load a folder?
	set_project_filepath(dir_to_load,NULL);

	load_folder(dir_to_load,NULL);
	
	init_scite_connection();

	// open scite, if prefs says we should
	if (prefs.start_scite == TRUE || start_scite) {
		launch_scite("", NULL);
	}

	// Run the app

	gtk_main();

	returnCode = EXIT_SUCCESS;

EXITPOINT:

	gui_close();

	done_prefs();

	done_version_string();

	g_free(current_dir);

	if (err) g_error_free(err);

	return returnCode;
}
