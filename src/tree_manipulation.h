/**
 * tree_manipulation.h - GtkTreeView manipulation code for SciteProj
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
 * along with SciteProj.
 * If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef __HEADER_TREE_MANIPULATION_
#define __HEADER_TREE_MANIPULATION_


// Node relative-position indicators

enum NodePosition {
	ADD_BEFORE,
	ADD_AFTER,
	ADD_CHILD
};


// Mnemonic identifiers for the columns in the GtkTreeStore

enum {
	COLUMN_ITEMTYPE = 0,
	COLUMN_FILEPATH,
	COLUMN_FILENAME,
	COLUMN_FILESIZE,
	COLUMN_FONTWEIGHT,
	COLUMN_FONTWEIGHTSET,
	COLUMN_ICON,
	COLUMN_EXPANDED,
	COLUMN_FOLDER_CONTENT_LOADED,
	COLUMN_EOL
};


// The type of each row in the tree datamodel

enum {
	ITEMTYPE_GROUP,
	ITEMTYPE_FILE
};




// The types for each column in the tree datamodel

#define TYPE_ITEMTYPE					G_TYPE_UINT
#define TYPE_FILEPATH					G_TYPE_STRING
#define TYPE_FILENAME					G_TYPE_STRING
#define TYPE_FILESIZE					G_TYPE_STRING
#define TYPE_FONTWEIGHT					G_TYPE_INT
#define TYPE_FONTWEIGHTSET				G_TYPE_BOOLEAN
#define TYPE_ICON							GDK_TYPE_PIXBUF
#define TYPE_EXPANDED					G_TYPE_BOOLEAN
#define TYPE_FOLDER_CONTENT_LOADED	G_TYPE_BOOLEAN



typedef gint(*StringCompareFunction)(gconstpointer,gconstpointer);


// Set the Project filepath
gboolean set_project_filepath(const gchar *filepath, GError **err);

// Get the GTKTreeStore
GtkTreeStore* create_treestore(GError **err);

// Get the project file directory
const gchar* get_project_directory();
gchar* get_project_filepath();

// Allow user to select and add files to the project
gboolean add_files_to_project(GtkTreeIter *parentIter, GError **err);

// Add a file node to a GtkTreeModel
gboolean add_tree_file(GtkTreeIter *currentIter,
								enum NodePosition position,
								const gchar* filepath,
								GtkTreeIter *newIter,
								gboolean makeRelative,
								GError **err);

//gboolean add_tree_filelist(GtkTreeIter *parentIter, GSList *fileList, GError **err);
gboolean add_tree_filelist(GtkTreeIter *parentIter, GSList *fileList, GError **err);

// Add a group node to a GtkTreeModel
gboolean add_tree_group(GtkTreeIter *parentIter,
								enum NodePosition position,
								const gchar* groupname,
								const gchar* full_name,
								gboolean expanded,
								GtkTreeIter *newIter,
								GError **err);

// Remove a node from a GtkTreeModel
gboolean remove_tree_node(GtkTreeIter *iter, GError **err);

// Remove a node from a GtkTreeModel
gboolean set_tree_node_name(GtkTreeIter *iter, const gchar *newContents, GError **err);

gboolean set_tree_node_expanded(GtkTreeIter *iter, gboolean expanded, GError **err);

gboolean set_tree_node_icon(GtkTreeIter *iter, GdkPixbuf *pixbuf, GError **err);

gboolean set_tree_node_loaded(GtkTreeIter *iter, gboolean loaded, GError **err);

// Copy a node in the tree (including children)
gboolean copy_tree_node(GtkTreeIter *srcIter,
								GtkTreeIter *dstIter,
								enum NodePosition position,
								GtkTreeIter *newIter,
								GError **err);

gchar *get_path_string(GtkTreeIter *iter);

void sort_children(GtkTreeIter *node,GError **err,StringCompareFunction compare_func);

gboolean add_tree_folderlist(GtkTreeIter *iter, GSList *folder_list, gchar *folder_path);

#endif /*__HEADER_TREE_MANIPULATION_*/
