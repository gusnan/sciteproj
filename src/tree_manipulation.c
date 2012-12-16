/**
 * tree_manipulation.c - GtkTreeView manipulation code for SciteProj
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
#include <glib.h>
#include <gtk/gtk.h>

#include <string.h>
#include <sys/stat.h>
#include <glib/gi18n.h>

#include <locale.h>

#include "tree_manipulation.h"
#include "xml_processing.h"
#include "string_utils.h"
#include "graphics.h"
#include "prefs.h"

#include "clicked_node.h"

#include "gui.h"

#include "filelist.h"

#include "icon.h"

#include "file_utils.h"


#define APP_SCITEPROJ_ERROR g_quark_from_static_string("APP_TREEMANIPULATION_ERROR")


GtkTreeStore *sTreeStore = NULL;
static gchar *sProjectFilepath = NULL;
static gchar *sProjectDir = NULL;


gchar *saved_file_folder=NULL;

// Predeclare static functions

gboolean add_tree_filelist(GtkTreeIter *parentIter, GSList *fileList, GError **err);
gboolean set_project_filepath(const gchar *filepath, GError **err);




/**
 * Set the project filepath
 *
 * @param filepath is the new project filepath
 * @param err returns any error information
 */
gboolean set_project_filepath(const gchar *filepath, GError **err)
{
	gboolean finalResult = FALSE;
	gchar *windowTitle = NULL;

	
	
	// Clear old data
	if (sProjectFilepath) g_free(sProjectFilepath);
	sProjectFilepath = NULL;
	
	if (sProjectDir) g_free(sProjectDir);
	sProjectDir = NULL;

	// Copy the filepath
	sProjectFilepath = g_strdup(filepath);

	/*
	// Is filepath a directory and not a file - then leave it be
	if (g_file_test(filepath, G_FILE_TEST_IS_DIR)) {
		sProjectDir = g_strdup(sProjectFilepath);
	} else {
	*/

		// Extract the project's base directory
		if (sProjectFilepath) {

			// Check for absolut path
			if (g_path_is_absolute(sProjectFilepath)==TRUE) {
				sProjectDir = g_strdup(sProjectFilepath);
			}
			else {
				if (!relative_path_to_abs_path(sProjectFilepath, &sProjectDir, NULL, err)) {
					goto EXITPOINT;
				}
			}

			if (sProjectDir[strlen(sProjectDir)-1]==G_DIR_SEPARATOR) {
				gchar *finalSlash = strrchr(sProjectDir, G_DIR_SEPARATOR);

				if (finalSlash != NULL) {
					*finalSlash = '\0';
				};
			}
			
		}
	//}

	windowTitle=g_strdup_printf("%s",get_filename_from_full_path(sProjectFilepath));

	set_window_title(windowTitle);

	finalResult = TRUE;

	goto EXITPOINT;


EXITPOINT:

	if (windowTitle) g_free(windowTitle);

	return finalResult;
}


/**
 *
 */
gchar *get_project_filepath()
{
	return sProjectFilepath;
}



/**
 * Get the project file directory.
 *
 * @return the project file's directory (pointer to static global-- do not modify!)
 */
const gchar* get_project_directory()
{
	return sProjectDir;
}



/**
 * Get the GTKTreeStore (evil, but necessary for setup_gui).
 *
 * @return the GtkTreeStore* for the project or NULL if a GtkTreeStore could not be allocate
 *
 * @param err returns any error information
 */
GtkTreeStore* create_treestore(GError **err)
{
	if (sTreeStore == NULL) {
		sTreeStore = gtk_tree_store_new(COLUMN_EOL,
													TYPE_ITEMTYPE,
													TYPE_FILEPATH,
													TYPE_FILENAME,
													TYPE_FILESIZE,
													TYPE_FONTWEIGHT,
													TYPE_FONTWEIGHTSET,
													TYPE_ICON,
													TYPE_EXPANDED,
													TYPE_FOLDER_CONTENT_LOADED);

		if (sTreeStore == NULL) {
			g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: Could not create GtkTreeStore, gtk_tree_store_new() = NULL", __func__);
		}
	}

	return sTreeStore;
}



/**
 * Clear the GtkTreeStore.
 *
 * @param treeStore is the GtkTreeStore to add to
 */
void empty_tree(GtkTreeStore *treeStore)
{
	g_assert(treeStore != NULL);

	gtk_tree_store_clear(treeStore);
}


/*
 *
 */
void build_file_filter(int n, GtkFileFilter* fileFilter ) {
 	switch(n) {
	case 0 :
		gtk_file_filter_set_name(fileFilter, _("C/C++ Files"));
		gtk_file_filter_add_pattern(fileFilter, "*.c");
		gtk_file_filter_add_pattern(fileFilter, "*.cc");
		gtk_file_filter_add_pattern(fileFilter, "*.cpp");
		gtk_file_filter_add_pattern(fileFilter, "*.cxx");
		gtk_file_filter_add_pattern(fileFilter, "*.C");
		gtk_file_filter_add_pattern(fileFilter, "*.CC");
		gtk_file_filter_add_pattern(fileFilter, "*.CPP");
		gtk_file_filter_add_pattern(fileFilter, "*.CXX");
		break;
	case 1 :
		gtk_file_filter_set_name(fileFilter, _("LAMP Script Files"));
		gtk_file_filter_add_pattern(fileFilter, "*.pm");
		gtk_file_filter_add_pattern(fileFilter, "*.pl");
		gtk_file_filter_add_pattern(fileFilter, "*.py");
		gtk_file_filter_add_pattern(fileFilter, "*.php*");
		break;
	case 2 :
		gtk_file_filter_set_name(fileFilter, _("Java Files"));
		gtk_file_filter_add_pattern(fileFilter, "*.java");
		break;
	case 3 :
		gtk_file_filter_set_name(fileFilter, _("JavaScript Files"));
		gtk_file_filter_add_pattern(fileFilter, "*.js");
		break;
	case 4 :
		gtk_file_filter_set_name(fileFilter, _("HTML Files"));
		gtk_file_filter_add_pattern(fileFilter, "*.html");
		gtk_file_filter_add_pattern(fileFilter, "*.htm");
		gtk_file_filter_add_pattern(fileFilter, "*.txt");
		break;
	default :
		gtk_file_filter_set_name(fileFilter, _("All Files"));
		gtk_file_filter_add_pattern(fileFilter, "*");
		break;
	}
	return;
}


/**
 * Display a file selection dialog to allow a user to add files to the current project.
 *
 * @return FALSE if errors occurred during loading of the project; TRUE otherwise
 *
 * @param parentIter is a pointer to the parent GtkTreeIter to add to, or NULL to add to the root of the tree
 * @param wrongFiles returns number of files that errored due to name duplicates
 * @param err returns any errors
 */
gboolean add_files_to_project(GtkTreeIter *parentIter, GError **err)
{

	gboolean finalResult = FALSE;
	GtkWidget *dialog = NULL;
	gchar *filepath = NULL;
	GtkFileFilter* fileFilter = NULL;
	GSList *fileList = NULL;
	int i=0;

	gchar *result_string=NULL;

	GSList *non_added_files=NULL;
	GSList *files_to_add=NULL;

	// Check if we have saved before
	if (get_project_directory()==NULL) {

		result_string=g_strdup_printf(_("You need to save the project before adding files."));

		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s", result_string);

		finalResult=FALSE;

		goto EXITPOINT;

	}


	// Create the file selection dialog and add filters

	dialog = gtk_file_chooser_dialog_new (_("Add Files"), 
														NULL,
														GTK_FILE_CHOOSER_ACTION_OPEN,
														GTK_STOCK_CANCEL,
														GTK_RESPONSE_CANCEL,
														GTK_STOCK_OPEN,
														GTK_RESPONSE_ACCEPT,
														NULL);

	if (saved_file_folder!=NULL) {
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), saved_file_folder);
	}

	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);

	// Select the choice from last time
	if(1) {
		fileFilter = gtk_file_filter_new();
		build_file_filter(gPrefs.last_file_filter, fileFilter);
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), fileFilter);
	}
	for(i=0; i<6;i++) {
		fileFilter = gtk_file_filter_new();
		build_file_filter(i, fileFilter);
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), fileFilter);
	}

	// Select the files

	if (gtk_dialog_run(GTK_DIALOG (dialog)) != GTK_RESPONSE_ACCEPT) {
		finalResult = TRUE;
		goto EXITPOINT;
	}

	// Remember the choice, so that we can select it again
	saved_file_folder=gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dialog));

	// Process the list of chosen files
	fileList = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));

	// Check for duplicates
	GSList *listIter;
	gchar *absFilename;

	gchar *newfilename;
	gchar *shortFileName;


	// go through the list of files, check if they are already in the tree
	for (listIter = fileList; listIter != NULL; listIter = g_slist_next(listIter)) {
		absFilename = (gchar *) (listIter->data);

		newfilename=g_strdup_printf("%s",absFilename);
		shortFileName=get_filename_from_full_path(absFilename);

		if ((tree_contains(shortFileName)) && (!gPrefs.allow_duplicates)) {
			non_added_files=g_slist_prepend(non_added_files,newfilename);
		} else {
			files_to_add=g_slist_prepend(files_to_add,newfilename);
		}
	}

	if (fileList) {
		g_slist_foreach(fileList, (GFunc) g_free, NULL);
		g_slist_free(fileList);
	}

	// actually add the filelist
	if (files_to_add) {
		if (!add_tree_filelist(parentIter, files_to_add, err)) {

			goto EXITPOINT;
		}
	}

	finalResult = TRUE;


EXITPOINT:

	if (dialog) gtk_widget_destroy(dialog);
	if (filepath) g_free(filepath);

	// non_added_files list not empty?
	if (non_added_files!=NULL) {

		gchar *temp=NULL;

		result_string=g_strdup_printf(_("\nThe following files:\n\n"));

		int q=0;

		for (listIter = non_added_files; listIter != NULL; listIter = g_slist_next(listIter)) {
			gchar *tempstring = (gchar *) (listIter->data);

			if (q<=10) {
				temp=g_strdup_printf("%s%s\n",result_string,get_filename_from_full_path(tempstring));

				g_free(result_string);

				result_string=temp;
			}

			q++;
		}

		if (q>=10) {
			temp=g_strdup_printf("%s...\n",result_string);
			g_free(result_string);
			result_string=temp;

		}

		temp=g_strdup_printf(_("%s\ncouldn't be added, because they were already present in the project.\n"),result_string);
		g_free(result_string);
		result_string=temp;

		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s", result_string);

		finalResult=FALSE;

	}

	if (files_to_add) {
		g_slist_foreach(files_to_add, (GFunc) g_free, NULL);
		g_slist_free(files_to_add);
	}

	// fix an error message

	if (non_added_files) {
		g_slist_foreach(non_added_files, (GFunc) g_free, NULL);
		g_slist_free(non_added_files);
	}

	return finalResult;
}

/**
 * Add a list of files to a GtkTreeStore.
 *
 * @return TRUE on success, FALSE on failure (further details returned in err)
 *
 * @param parentIter is a pointer to the parent GtkTreeIter to add to, or NULL to add to the root of the tree
 * @param fileList is the list of files to add to the tree
 * @param number_of_errors returns the number of files that couldn't be added due to duplicates
 * @param err returns any errors
 */
gboolean add_tree_filelist(GtkTreeIter *parentIter, GSList *fileList, GError **err)
{
	debug_printf("%s\n",__func__);

	g_assert(sTreeStore != NULL);
	g_assert(fileList != NULL);

	gboolean finalResult = FALSE;
	GtkTreeIter newIter;
	GSList *listIter;
	gchar *absFilename;

	// Reverse the list
	fileList=g_slist_reverse(fileList);

	listIter = fileList;

	for (listIter = fileList; listIter != NULL; listIter = g_slist_next(listIter)) {

		absFilename = (gchar *) (listIter->data);

		if (!absFilename) {
			continue;
		}

		if (listIter == fileList) {
			add_tree_file(parentIter, ADD_CHILD, absFilename, &newIter, TRUE, err);
		}
		else {
			add_tree_file(&newIter, ADD_AFTER, absFilename, &newIter, TRUE, err);
		}
	}

	finalResult = TRUE;

	return finalResult;
}


/**
 * Add a group node to an existing parent node, or to the root of the GtkTreeStore.
 *
 * @return TRUE on success, FALSE on failure (further details returned in err)
 *
 * @param parentIter is a pointer to the parent GtkTreeIter to add to, 
 *        or NULL to add to the root of the tree
 * @param position indicates the relative position to add the file node
 * @param newIter returns the new GtkTreeIter (pass NULL if this result is not needed)
 * @param groupname is the name of the group to add to the tree
 * @param err returns any errors
 */
gboolean add_tree_group(GtkTreeIter *parentIter, 
								enum NodePosition position,
								const gchar* groupname,
								const gchar* full_name,
								gboolean expanded,
								GtkTreeIter *newIter,
								GError **err)
{
	g_assert(sTreeStore != NULL);
	g_assert(groupname != NULL);

	gboolean finalResult = FALSE;
	GtkTreeIter iter;


	// Append to root, or before/after/within an existing node?

	// swap before and after

	if (parentIter == NULL) {
		gtk_tree_store_insert_before(sTreeStore, &iter, NULL, NULL);
	}
	else if (position == ADD_BEFORE) {
		gtk_tree_store_insert_before(sTreeStore, &iter, NULL, parentIter);
	}
	else if (position == ADD_AFTER) {
		gtk_tree_store_insert_after(sTreeStore, &iter, NULL, parentIter);
	}
	else if (position == ADD_CHILD) {
		gtk_tree_store_insert(sTreeStore,&iter,parentIter,1000);
	}

	if (newIter) {
		*newIter = iter;
	}

	gtk_tree_store_set(sTreeStore, &iter, COLUMN_ITEMTYPE, ITEMTYPE_GROUP, -1);
	gtk_tree_store_set(sTreeStore, &iter, COLUMN_FILENAME, groupname, -1);
	gtk_tree_store_set(sTreeStore, &iter, COLUMN_FILEPATH, full_name, -1);
	gtk_tree_store_set(sTreeStore, &iter, COLUMN_FONTWEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_tree_store_set(sTreeStore, &iter, COLUMN_FONTWEIGHTSET, TRUE, -1);
	gtk_tree_store_set(sTreeStore, &iter, COLUMN_FOLDER_CONTENT_LOADED, FALSE, -1);

	gtk_tree_store_set(sTreeStore, &iter, COLUMN_ICON, directory_closed_pixbuf, -1);

	gtk_tree_store_set(sTreeStore, &iter, COLUMN_EXPANDED, expanded, -1);

	finalResult = TRUE;

	return finalResult;
}



/**
 * Add a file node to an existing parent node, or to the root of the GtkTreeStore.
 *
 * @return TRUE on success, FALSE on failure (further details returned in err)
 *
 * @param currentIter is a pointer to the parent GtkTreeIter to add to, or NULL to add to the root of the tree
 * @param position indicates the relative position to add the file node
 * @param filepath is the filepath to add to the tree
 * @param newIter returns the GtkTreeIter that refers to the new node
 * @param makeRelative indicates whether the filepath should be converted to a relative path before being added to the tree
 * @param err returns any errors
 */
gboolean add_tree_file(GtkTreeIter *currentIter, 
								enum NodePosition position, 
								const gchar* filepath,
								GtkTreeIter *newIter,
								gboolean makeRelative,
								GError **err)
{
	g_assert(sTreeStore != NULL);
	g_assert(filepath != NULL);
	g_assert(position == ADD_BEFORE || position == ADD_AFTER || position == ADD_CHILD);

	gboolean finalResult = FALSE;
	GtkTreeIter iter;
	const gchar* fileName = NULL;
	gchar *relFilename = NULL;

	gchar *fileExt=NULL;

	if (!makeRelative) {
		relFilename = g_strdup(filepath);
	}
	else if (!abs_path_to_relative_path(filepath, &relFilename, sProjectDir, err)) {
		printf("abs_path_to_relative_path FAILED!\n");
		goto EXITPOINT;
	}
	
	// Extract filename from filepath
	fileName = get_filename_from_full_path((gchar*)filepath);

	add_item((gchar*)fileName,(gchar*)relFilename);

	// Append to root, or before/after/within an existing node?

	if (currentIter == NULL) {
		gtk_tree_store_insert_before(sTreeStore, &iter, NULL, NULL);
	}
	else if (position == ADD_BEFORE) {
		gtk_tree_store_insert_before(sTreeStore, &iter, NULL, currentIter);
	}
	else if (position == ADD_AFTER) {
		gtk_tree_store_insert_after(sTreeStore, &iter, NULL, currentIter);
	}
	else if (position == ADD_CHILD) {
		gtk_tree_store_insert(sTreeStore,&iter,currentIter,1000);
	}

	fileExt=strrchr(fileName,'.');

	if (fileExt!=NULL) {
		++fileExt;
	}

	if (fileExt == NULL || strlen(fileExt) <= 0) {
		fileExt=(gchar*)fileName;
	}


	gtk_tree_store_set(sTreeStore, &iter, COLUMN_ITEMTYPE, ITEMTYPE_FILE, -1);
	gtk_tree_store_set(sTreeStore, &iter, COLUMN_FILEPATH, relFilename, -1);
	gtk_tree_store_set(sTreeStore, &iter, COLUMN_FILENAME, fileName, -1);

	gtk_tree_store_set(sTreeStore, &iter, COLUMN_EXPANDED, FALSE, -1);
	
	gtk_tree_store_set(sTreeStore, &iter, COLUMN_FOLDER_CONTENT_LOADED, FALSE, -1);

	/*
	if (
		(strcmp(fileExt,"cc")==0) ||
		(strcmp(fileExt,"c++")==0) ||
		(strcmp(fileExt,"c")==0) ||
		(strcmp(fileExt,"cpp")==0)
		) {
		gtk_tree_store_set(sTreeStore, &iter, COLUMN_ICON, cpp_file_pixbuf, -1);

	} else if (
		(strcmp(fileExt,"hh")==0) ||
		(strcmp(fileExt,"h++")==0) ||
		(strcmp(fileExt,"h")==0) ||
		(strcmp(fileExt,"hpp")==0)
	) {
		gtk_tree_store_set(sTreeStore, &iter, COLUMN_ICON, header_file_pixbuf, -1);

	} else if (
		(strcmp(fileExt,"lua")==0)
	) {
		gtk_tree_store_set(sTreeStore, &iter, COLUMN_ICON, lua_file_pixbuf, -1);

	} else if (
		(strcmp(fileExt,"java")==0)
	) {
		gtk_tree_store_set(sTreeStore, &iter, COLUMN_ICON, java_file_pixbuf, -1);
	} else {
		gtk_tree_store_set(sTreeStore, &iter, COLUMN_ICON, txt_file_pixbuf, -1);
	}
	*/

	GdkPixbuf *icon_pixbuf=get_pixbuf_from_filename((gchar*)(filepath),GTK_ICON_SIZE_MENU);

	gtk_tree_store_set(sTreeStore, &iter, COLUMN_ICON, icon_pixbuf, -1);

	if (newIter) {
		*newIter = iter;
	}

	finalResult = TRUE;

EXITPOINT:

	if (relFilename) g_free(relFilename);
	
	return finalResult;
}


/**
 *
 */
void helper_remove(GtkTreeIter *iter)
{

	GtkTreeIter *tempIter=gtk_tree_iter_copy(iter);
	GtkTreeIter newIter;

	if (gtk_tree_model_iter_children(GTK_TREE_MODEL(sTreeStore),&newIter,tempIter)) {

		gboolean next_valid=TRUE;

		do {

			if (next_valid) {

				gchar *nodeContents;
				int itemType;

				gtk_tree_model_get(GTK_TREE_MODEL(sTreeStore), &newIter, 
											COLUMN_ITEMTYPE, &itemType,
											COLUMN_FILEPATH, &nodeContents, -1);

				gchar *fileName = get_filename_from_full_path((gchar*)nodeContents);

				if (itemType==ITEMTYPE_GROUP) {
					helper_remove(&newIter);
				} else {
					//printf("Removed: %s, %s\n",fileName,nodeContents);

					remove_item(fileName,nodeContents);
				}
			}

			next_valid=gtk_tree_model_iter_next(GTK_TREE_MODEL(sTreeStore),&newIter);

		} while(next_valid);

	}

}


/**
 * Remove a node from the GtkTreeStore.
 *
 * @return TRUE on success, FALSE on failure (further details returned in err)
 *
 * @param iter references the node to remove
 * @param err returns any errors
 */
extern gboolean remove_tree_node(GtkTreeIter *iter, GError **err)
{
	gchar *file_path;

	int itemType;

	// Get the node type and content

	gtk_tree_model_get(GTK_TREE_MODEL(sTreeStore), iter, COLUMN_ITEMTYPE, &itemType, COLUMN_FILEPATH, &file_path, -1);

	if (itemType==ITEMTYPE_GROUP) {

		helper_remove(iter);

	} else {

		gchar *fileName = get_filename_from_full_path((gchar*)file_path);

		remove_item(fileName,file_path);
	}

	gtk_tree_store_remove(sTreeStore, iter);

	return TRUE;
}



/**
 * Set the contents of a node. (Should only be used for directories, and not for
 * adding files)
 *
 * @return TRUE on success, FALSE on failure (further details returned in err)
 *
 * @param iter is a reference to the node to change
 * @param newContents is the new content for the node
 * @param err returns any errors
 */
gboolean set_tree_node_name(GtkTreeIter *iter, const gchar *newContents, GError **err)
{
	g_assert(iter != NULL);
	g_assert(newContents != NULL);


	// What is saved on disk
	gtk_tree_store_set(sTreeStore, iter, COLUMN_FILEPATH, newContents, -1);

	// What is visible in gui
	gtk_tree_store_set(sTreeStore, iter, COLUMN_FILENAME, newContents, -1);

	return TRUE;
}


/**
 *	Set the icon of a node
 *
 * @return TRUE on success, FALSE on error
 *
 * @param iter is a reference to the node who's icon we want to change
 * @param pixbuf is the pixbuf we want as icon
 * @param err to return any error details
 *
 */
gboolean set_tree_node_icon(GtkTreeIter *iter, GdkPixbuf *pixbuf, GError **err)
{
	g_assert(iter != NULL);
	g_assert(pixbuf != NULL);

	/* What is saved on disk */
	gtk_tree_store_set(sTreeStore, iter, COLUMN_ICON, pixbuf, -1);

	g_object_ref(pixbuf);

	return TRUE;
}

/**
 * set_tree_node_expanded
 */
gboolean set_tree_node_expanded(GtkTreeIter *iter, gboolean expanded, GError **err)
{
	g_assert(iter != NULL);

	gtk_tree_store_set(sTreeStore, iter, COLUMN_EXPANDED, expanded, -1);

	return TRUE;
}


/**
 *
 */
gboolean set_tree_node_loaded(GtkTreeIter *iter, gboolean loaded, GError **err)
{
	g_assert(iter!=NULL);
	
	gtk_tree_store_set(sTreeStore, iter, COLUMN_FOLDER_CONTENT_LOADED, loaded, -1);
	
	return TRUE;
}


/**
 * Copy a node (and children) within the tree.
 *
 * @return TRUE on success, FALSE on failure (further details returned in err)
 *
 * @param srcIter is a reference to the node to move
 * @param dstIter is a reference to the destination node; NULL if the root of the tree
 * @param newIter returns the iter of the copy
 * @param position indicates where copy the node, relative to srcIter
 * @param err returns any errors
 */
gboolean copy_tree_node(GtkTreeIter *srcIter, 
								GtkTreeIter *dstIter, 
								enum NodePosition position, 
								GtkTreeIter *newIter, 
								GError **err)
{
	g_assert(srcIter != NULL);
	g_assert(position == ADD_BEFORE || position == ADD_AFTER || position == ADD_CHILD);

	gchar *nodeContents = NULL;
	gboolean finalResult = FALSE;
	gint itemType;
	GtkTreeIter srcChildIter;
	GtkTreeIter dstChildIter;
	GtkTreePath *srcPath = NULL;
	GtkTreePath *newPath = NULL;
	gboolean groupIsExpanded = FALSE;
	GtkTreeIter newGroupIter;


	// Get the node type and content

	gtk_tree_model_get(GTK_TREE_MODEL(sTreeStore), srcIter, 
								COLUMN_ITEMTYPE, &itemType,
								COLUMN_FILEPATH, &nodeContents, -1);


	// Add a file or group?

	if (itemType == ITEMTYPE_FILE) {
		if (!add_tree_file(dstIter, position, nodeContents, newIter, FALSE, err)) {
			goto EXITPOINT;
		}
	}
	else {
		// Is the source group currently expanded?

		srcPath = gtk_tree_model_get_path(GTK_TREE_MODEL(sTreeStore), srcIter);
		groupIsExpanded = tree_row_is_expanded(srcPath);


		// Add the copy of the group

		if (!add_tree_group(dstIter, position, nodeContents, nodeContents, groupIsExpanded, &newGroupIter, err)) {
			goto EXITPOINT;
		}

		if (newIter) {
			*newIter = newGroupIter;
		}


		// Recursively copy the group's contents, too

		if (gtk_tree_model_iter_children(GTK_TREE_MODEL(sTreeStore), &srcChildIter, srcIter)) {
			// Copy the first child as ADD_CHILD

			if (!copy_tree_node(&srcChildIter, &newGroupIter, ADD_CHILD, &dstChildIter, err)) {
				goto EXITPOINT;
			}

			// Copy each subsequent child ADD_AFTER its prior sibling

			while (gtk_tree_model_iter_next(GTK_TREE_MODEL(sTreeStore), &srcChildIter)) {
				if (!copy_tree_node(&srcChildIter, &dstChildIter, ADD_AFTER, &dstChildIter, err)) {
					goto EXITPOINT;
				}
			}
		}

		// Expand the new group?

		if (groupIsExpanded) {
			newPath = gtk_tree_model_get_path(GTK_TREE_MODEL(sTreeStore), &newGroupIter);
			expand_tree_row(newPath, FALSE);
		}
	}

	finalResult = TRUE;


EXITPOINT:

	if (nodeContents) g_free(nodeContents);
	if (srcPath) gtk_tree_path_free(srcPath);
	if (newPath) gtk_tree_path_free(newPath);

	return finalResult;
}

/**
 *
 */
gchar *get_path_string(GtkTreeIter *iter)
{
	GtkTreePath *treePath=gtk_tree_model_get_path(GTK_TREE_MODEL(sTreeStore),iter);
	gchar *buf;
	buf=g_strdup_printf("%s",gtk_tree_path_to_string(treePath));

	return buf;
}


/**
 *
 */
void sort_children(GtkTreeIter *node,GError **err,StringCompareFunction compare_func)
{

	GtkTreeIter *saved_iter=node;

	GtkTreeIter childIter;

	GtkTreeModel *tree_model=GTK_TREE_MODEL(sTreeStore);

	gint nodeType=-1;

	GSList *itemList=NULL;


	if (gtk_tree_model_iter_children(tree_model,&childIter,node)) {

		int q=gtk_tree_model_iter_n_children(tree_model,node);

		while (q>0) {

			gchar *nodeContents;

			gtk_tree_model_get(tree_model, &childIter, 
										COLUMN_ITEMTYPE, &nodeType, 
										COLUMN_FILEPATH, &nodeContents, -1);

			if (nodeType==ITEMTYPE_FILE) {

				gchar *newAbsPath=NULL;

				relative_path_to_abs_path(nodeContents,&newAbsPath,get_project_directory(),err);

				if (itemList==NULL) {
					itemList=g_slist_append(itemList,newAbsPath);
				} else {

					itemList=g_slist_insert_sorted(itemList,newAbsPath,compare_func);
				}

				gtk_tree_store_remove(sTreeStore,&childIter);
			} else {

				gtk_tree_model_iter_next(tree_model,&childIter);
			}

			q--;
		}

	}

	if (itemList!=NULL)
		add_tree_filelist(saved_iter,itemList, err);

	GtkTreePath *path=gtk_tree_model_get_path(tree_model,saved_iter);
	expand_tree_row(path,TRUE);
}


/*
 *
 */
struct TestStruct
{
	gchar *string_to_check_for;
	gboolean found;
};

typedef struct TestStruct TestStruct;

/**
 *
 */
gboolean foreach_finder(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
	gint nodeType=-1;
	gchar *node_contents;

	gtk_tree_model_get(model, iter, COLUMN_ITEMTYPE, &nodeType, COLUMN_FILENAME, &node_contents, -1);

	TestStruct *test=(TestStruct*)(data);

	gboolean res=FALSE;

	if (g_ascii_strcasecmp(node_contents,test->string_to_check_for)==0) {
		test->found=TRUE;
		res=TRUE;
	}

	g_free(node_contents);

	return res;
}


/**
 *
 */
gboolean tree_contains(gchar *test_string)
{
	TestStruct test;

	test.string_to_check_for=test_string;
	test.found=FALSE;

	gtk_tree_model_foreach(GTK_TREE_MODEL(sTreeStore),foreach_finder,(gpointer)(&test));

	return test.found;
}

