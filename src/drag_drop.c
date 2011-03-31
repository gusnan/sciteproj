/**
 * drag_drop.c - Drag-and-drop support for SciteProj
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
#include <string.h>

#include <gtk/gtk.h>
#include <glib.h>



#include "drag_drop.h"
#include "string_utils.h"
#include "tree_manipulation.h"


// #define DEBUG_DRAG_DROP
#define APP_SCITEPROJ_ERROR g_quark_from_static_string("APP_DRAGDROP_ERROR")


// Forward-declare static functions

static void global_coords_to_local(GtkTreeView* treeView, gint *x, gint *y);
static gboolean handle_local_drag_drop(TreeViewDragStruct *dragStruct, GtkTreePath *dropPath, enum NodePosition position);


/**
 * Process Gtk drag-data-received events on the GtkTreeView.  This gets uglier than I'd hoped....
 *
 * 
 * @param widget is the pointer to the drag target (should always be the GtkTreeView widget)
 * @param drag_context is a pointer to the GdkDragContext object
 * @param x is the x coordinate of the mouse in window coordinates (I think)
 * @param y is the y coordinate of the mouse in window coordinates (I think)
 * @param selection_data is the dropped data
 * @param info is the data flavour as registered in the call to gtk_tree_view_enable_model_drag_dest()
 * @param time is the timestamp for the event
 * @param user_data is a pointer to the TreeViewDragStruct for the drag operation
 */
void drag_data_received_cb(GtkWidget *widget, GdkDragContext *drag_context, gint x, gint y, GtkSelectionData *selection_data, guint info, guint t, gpointer user_data)
{
#ifdef __DEBUG_FOLDERS_DRAG_AND_DROP_
	debug_printf("drag_data_received_cb\n");
#endif
	TreeViewDragStruct *dragStruct = (TreeViewDragStruct *) user_data;
	
	g_assert(drag_context != NULL);
	g_assert(selection_data != NULL);
	g_assert(dragStruct != NULL);
	g_assert(dragStruct->treeView != NULL);
	g_assert(dragStruct->treeStore != NULL);
	g_assert(GTK_TREE_VIEW(widget) == dragStruct->treeView);
	
	
	gchar **uriList = (gchar**)NULL;
	GtkTreePath *dropPath = NULL;
	GtkTreeIter dropIter;
	gint nodeItemType;
	gchar *nodeName = NULL;
	GtkTreePath *dragPath = NULL;
	GError *err = NULL;
	enum NodePosition position = ADD_CHILD;
	
	int i=0;
	
	//static gchar *fileURI; // = (gchar*)"file://";
	static int fileURILength; // = strlen(fileURI);
	
#ifdef DEBUG_DRAG_DROP	
	gchar *typeName = gdk_atom_name(selection_data->type);
	g_print("%s: info = %d, type = '%s', data = '%s'\n", __func__, info, typeName, (gchar *) selection_data->data);
	if (typeName) g_free(typeName);
#endif
	
	// Filter the types of drop data we accept
	
	if (!(info == DND_URI_TYPE || info == 0)) {
		goto EXITPOINT;
	}
	
	// Get a GtkTreeIter that refers to the drop point in the tree, and figure out what type of node it is
	
	global_coords_to_local(dragStruct->treeView, &x, &y);
	
	if (gtk_tree_view_get_path_at_pos(dragStruct->treeView, x, y, &dropPath, NULL, NULL, NULL)) {
		if (!gtk_tree_model_get_iter(GTK_TREE_MODEL(dragStruct->treeStore), &dropIter, dropPath)) {
			goto EXITPOINT;
		}
		
		gtk_tree_model_get(GTK_TREE_MODEL(dragStruct->treeStore), &dropIter, COLUMN_ITEMTYPE, &nodeItemType, COLUMN_FILEPATH, &nodeName, -1);
		
#ifdef DEBUG_DRAG_DROP	
		g_print("%s: drop node = '%s'\n", __func__, nodeName);
#endif
	}
	
	
	// Figure out where to drop the data relative to the target node
	
	if (dropPath == NULL) {
		position = ADD_AFTER;
	}
	else if (dragStruct->dropPositionHint == GTK_TREE_VIEW_DROP_BEFORE) {
		position = ADD_BEFORE;
	}
	else if (dragStruct->dropPositionHint == GTK_TREE_VIEW_DROP_AFTER) {
		position = ADD_AFTER;
	}
	else if (nodeItemType == ITEMTYPE_FILE && dragStruct->dropPositionHint == GTK_TREE_VIEW_DROP_INTO_OR_BEFORE) {
		position = ADD_BEFORE;
	}
	else if (nodeItemType == ITEMTYPE_FILE && dragStruct->dropPositionHint == GTK_TREE_VIEW_DROP_INTO_OR_AFTER) {
		position = ADD_AFTER;
	}
	else {
		position = ADD_CHILD;
	}
	
	
	// If the drag data originated locally, deal with it
	
	if (dragStruct->isLocalDrag && dragStruct->dragNodes) {
		handle_local_drag_drop(dragStruct, dropPath, position);
		
		goto EXITPOINT;
	}
	
	
	// Drag originated externally, so parse all the received URIs and add them to the tree
	
	if (!(uriList = gtk_selection_data_get_uris(selection_data))) {
		goto EXITPOINT;
	}
	
	
	for (i = 0; uriList[i] != NULL; ++i) {
		gchar *fileURI = (gchar*)"file://";
		fileURILength = strlen(fileURI);
		
#ifdef DEBUG_DRAG_DROP	
		g_print("%s: checking drag/drop data '%s'\n", __func__, uriList[i]);
#endif
		
		// If it doesn't start with "file://", then ignore it
		
		if (!g_str_has_prefix(uriList[i], fileURI)) {
			continue;
		}
		
		// Skip the "file://" prefix
		
		gchar *filePath = uriList[i] + fileURILength;
		
		
		// Add the data to the tree
		
		GError *err = NULL;
		
		// Append to a node, or to the root of the tree?
		
#ifdef DEBUG_DRAG_DROP	
		if (position == ADD_AFTER) g_print("%s: add_tree_file(ADD_AFTER)\n", __func__);
		if (position == ADD_BEFORE) g_print("%s: add_tree_file(ADD_BEFORE)\n", __func__);
		if (position == ADD_CHILD) g_print("%s: add_tree_file(ADD_CHILD)\n", __func__);
#endif
		
		if (dropPath == NULL && i == 0) {
			if (!add_tree_file(NULL, ADD_CHILD, filePath, &dropIter, TRUE, &err)) {
				g_warning("%s: Could not add drag/drop data: %s\n", __func__, err->message);
			}
		}
		else {
			if (!add_tree_file(&dropIter, position, filePath, &dropIter, TRUE, &err)) {
				g_warning("%s: Could not add drag/drop data: %s\n", __func__, err->message);
			}
		}
		
		
		// Everything after the first dropped item is added as a sibling to preserve the drop order!
		
		position = ADD_AFTER;
		
		
		// Clean up the error message for the next pass around
		
		if (err) g_error_free(err);
	}
	
	
EXITPOINT:
	
	if (dragStruct) {
		dragStruct->isLocalDrag = FALSE;
		
		if (dragStruct->dragNodes) {
			for (i = 0; i < dragStruct->numDragNodes; ++i) {
				if (dragStruct->dragNodes[i].nodePath) {
					gtk_tree_path_free(dragStruct->dragNodes[i].nodePath);
				}
			}
			g_free(dragStruct->dragNodes);
			dragStruct->dragNodes = NULL;
		}
	}
	
	if (dropPath) gtk_tree_path_free(dropPath);
	if (dragPath) gtk_tree_path_free(dragPath);
	if (uriList) g_strfreev(uriList);
	if (nodeName) g_free(nodeName);
	if (err) g_error_free(err);
}



/**
 * Process Gtk drag-motion events on the GtkTreeView.
 * 
 * @param widget is the pointer to the drag target (should always be the GtkTreeView widget)
 * @param drag_context is a pointer to the GdkDragContext object
 * @param x is the x coordinate of the mouse in window coordinates (I think)
 * @param y is the y coordinate of the mouse in window coordinates (I think)
 * @param time is the timestamp for the event
 * @param user_data is a pointer to the TreeViewDragStruct for the drag operation
 */
gboolean drag_motion_cb(GtkWidget *widget, GdkDragContext *drag_context, gint x, gint y, guint time, gpointer user_data)
{
#ifdef __DEBUG_FOLDERS_DRAG_AND_DROP_
	debug_printf("drag_motion_cb\n");
#endif
	TreeViewDragStruct *dragStruct = (TreeViewDragStruct *) user_data;
	
	g_assert(dragStruct != NULL);
	g_assert(dragStruct->treeView != NULL);
	g_assert(GTK_TREE_VIEW(widget) == dragStruct->treeView);
	
	
	// Note the current drop position for use later when we receive the actual drag-data-received message
	
	gtk_tree_view_get_drag_dest_row(dragStruct->treeView, NULL, &(dragStruct->dropPositionHint));
	
	
	// Return FALSE to indicate that we do NOT take responsibility for this event, and the GtkTreeView should do its usual visual feedback work
	
	return FALSE;
}


/**
 * Process Gtk drag-data-get events on the GtkTreeView.
 * 
 * @param widget is the pointer to the drag target (should always be the GtkTreeView widget)
 * @param drag_context is a pointer to the GdkDragContext object
 * @param selection_data is the dropped data
 * @param info is the data flavour as registered in the call to gtk_tree_view_enable_model_drag_dest()
 * @param time is the timestamp for the event
 * @param user_data is a pointer to the TreeViewDragStruct for the drag operation
 */
void drag_data_get_cb(GtkWidget *widget, GdkDragContext *drag_context, GtkSelectionData *selection_data, guint info, guint time, gpointer user_data)
{
#ifdef __DEBUG_FOLDERS_DRAG_AND_DROP_
	debug_printf("drag_data_get_cb\n");
#endif
	TreeViewDragStruct *dragStruct = (TreeViewDragStruct *) user_data;
	
	g_assert(dragStruct != NULL);
	g_assert(dragStruct->treeView != NULL);
	g_assert(dragStruct->treeStore != NULL);
	g_assert(GTK_TREE_VIEW(widget) == dragStruct->treeView);
	
	
	gchar *fileURI = NULL;
	GError *err = NULL;
	gchar** uriList = NULL;
	GList* selectedNodeList = NULL;
	GtkTreeSelection *treeSelection = NULL;
	gint numSelectedRows;
	gchar *nodeContentString = NULL;
	TreeNodeStruct *nodeInfoPtr = NULL;
	
	GList *selectionIter=NULL;
#ifdef DEBUG_DRAG_DROP
	g_print("------------------------------\n");
#endif
	
	// Get the currently-selected nodes, if possible
	
	treeSelection = gtk_tree_view_get_selection(dragStruct->treeView);
	
	selectedNodeList = gtk_tree_selection_get_selected_rows(treeSelection, NULL);
	
	if (!selectedNodeList) {
		goto EXITPOINT;
	}
	
	
	// We need to store the iters that refer to the nodes in the TreeViewDragStruct so drag_data_received_cb() can use it later
	
	numSelectedRows = g_list_length(selectedNodeList);
	
#ifdef DEBUG_DRAG_DROP	
	g_print("%s: numSelectedRows = %d\n", __func__, numSelectedRows);
#endif
	
	if (dragStruct->dragNodes) {
		g_free(dragStruct->dragNodes);
		dragStruct->dragNodes = NULL;
	}
	
	dragStruct->dragNodes = (TreeNodeStruct*) g_malloc(numSelectedRows * sizeof(TreeNodeStruct));
	dragStruct->numDragNodes = 0;
	
	
	// Iterate over all selected nodes, accumulating the list of URIs and populating dragStruct->dragNodes
	
	nodeInfoPtr = dragStruct->dragNodes;
	
	for (selectionIter = g_list_first(selectedNodeList); selectionIter != NULL; selectionIter = g_list_next(selectionIter)) {
		GtkTreePath *path = NULL;
		gint nodeItemType;
		GtkTreeIter nodeIter;
		
		
		// Get the iter to the next selected node
		
		path = (GtkTreePath *) selectionIter->data;
		
		if (!gtk_tree_model_get_iter(GTK_TREE_MODEL(dragStruct->treeStore), &nodeIter, path)) {
			continue;
		}
		
		
		// Free up the storage from the last pass
		
		if (nodeContentString) {
			g_free(nodeContentString);
			nodeContentString = NULL;
		}
		
		
		// Get the data from the selected node
		
		gtk_tree_model_get(GTK_TREE_MODEL(dragStruct->treeStore), &nodeIter, COLUMN_ITEMTYPE, &nodeItemType, COLUMN_FILEPATH, &nodeContentString, -1);
		
		
		// Keep the node type, path, and iterator so drag_data_received_cb() can use it later
		
		nodeInfoPtr->nodeItemType = nodeItemType;
		nodeInfoPtr->nodeIter = nodeIter;
		nodeInfoPtr->nodePath = path;
		
		dragStruct->numDragNodes++;
		
		nodeInfoPtr++;
		
#ifdef DEBUG_DRAG_DROP	
		g_print("%s: selected node = '%s'\n", __func__, nodeContentString);
#endif
		
		// Format the node content (i.e. filepath) as a URI and append it to the list
		
		if (fileURI != NULL && !str_append(&fileURI, "\n", &err)) {
			g_warning("%s: Could not format drag/drop data: %s\n", __func__, err->message);
			goto EXITPOINT;
		}
		
		
		if (!str_append(&fileURI, "file://", &err) || !str_append(&fileURI, nodeContentString, &err)) {
			g_warning("%s: Could not format drag/drop data: %s\n", __func__, err->message);
			goto EXITPOINT;
		}
	}
	
	
	// Note that this drag originates locally
	
	dragStruct->isLocalDrag = TRUE;
	
	
#ifdef DEBUG_DRAG_DROP	
	g_print("%s: data = '%s'\n\n", __func__, fileURI);
#endif
	
	// Format and set the selection data as the list of URIs
	
	uriList = g_strsplit(fileURI, "\n", -1);
	
	if (!uriList || !gtk_selection_data_set_uris(selection_data, uriList)) {
		g_warning("%s: Could not set drag/drop data\n", __func__);
	}
	
	
EXITPOINT:
	
	if (selectedNodeList) {
		// We're actually using the individual paths, so don't free them!
		// g_list_foreach(selectedNodeList, (GFunc) gtk_tree_path_free, NULL);
		
		g_list_free(selectedNodeList);
	}
	
	if (fileURI) g_free(fileURI);
	if (err) g_error_free(err);
	if (uriList) g_strfreev(uriList);
	if (nodeContentString) g_free(nodeContentString);
}



/**
 * Convert global coordinates to local coordinates for a GtkTreeView.
 *
 * @param treeView is the GtkTreeView
 * @param x is the x coordinate to be shifted
 * @param y is the y coordinate to be shifted
 */
static void global_coords_to_local(GtkTreeView* treeView, gint *x, gint *y)
{
	g_assert(treeView != NULL);
	g_assert(x != NULL);
	g_assert(y != NULL);
	
	gint wx, wy;
	GdkWindow* window = gtk_tree_view_get_bin_window(treeView);
	
	if (window) {
		gdk_window_get_position(window, &wx, &wy);
		
		*x -= wx;
		*y -= wy;
	}
}



/**
 * Handle a drag-and-drop operation that was initiated locally.  This involves checking that dragged
 * groups are not being dropped inside themselves, and also correctly handling files that are part
 * of a dragged group.
 *
 * @return FALSE if errors occur; TRUE otherwise
 * 
 * @param dragStruct is a pointer to the TreeViewDragStruct for the drag operation
 * @param dropPath is the GtkTreePath referring to the drop point; if NULL, then the drag list is added to the root of the tree
 * @param position is the location to drop the data, relative to the dropPath
 */
static gboolean handle_local_drag_drop(TreeViewDragStruct *dragStruct, GtkTreePath *dropPath, enum NodePosition position)
{
#ifdef __DEBUG_FOLDERS_DRAG_AND_DROP_
	debug_printf("handle_local_drag_drop\n");
#endif
	g_assert(dragStruct != NULL);
	g_assert(dragStruct->treeView != NULL);
	g_assert(dragStruct->treeStore != NULL);
	
	gboolean finalResult = FALSE;
	GtkTreeIter dropIter;
	GError *err = NULL;
	int i=0,j=0;
	
	
	// If the drag data did not originate locally, don't do anything
	
	if (!dragStruct->isLocalDrag || !dragStruct->dragNodes || dragStruct->numDragNodes <= 0) {
		finalResult = TRUE;
		goto EXITPOINT;
	}
	
	
	// Check to see if the drop point is a child of one of the drag nodes, and bail if so
	
	if (dropPath != NULL) {
		for (i = 0 ; i < dragStruct->numDragNodes; ++i) {
			if (gtk_tree_path_is_descendant(dropPath,dragStruct->dragNodes[i].nodePath)) {
					 
				g_set_error(&err, APP_SCITEPROJ_ERROR, -1, "%s: Cannot drop a group node into itself", __func__);
				
				goto EXITPOINT;
			}
		}
		
		for (i = 0; i < dragStruct->numDragNodes; ++i) {
			if (gtk_tree_path_compare(dropPath,dragStruct->dragNodes[i].nodePath)==0) {
					 
				//g_set_error(&err, APP_SCITEPROJ_ERROR, -1, "%s: Cannot drop a group node into itself", __func__);
				
				goto EXITPOINT;
			}
		}
	}
	
#ifdef DEBUG_DRAG_DROP
	g_print("\n%s: Initial drag list:\n", __func__);
	
	for (i = 0 ; i < dragStruct->numDragNodes; ++i) {
		gchar *nodeName = NULL;
		
		gtk_tree_model_get(GTK_TREE_MODEL(dragStruct->treeStore), &(dragStruct->dragNodes[i].nodeIter), COLUMN_FILEPATH, &nodeName, -1);
		
		g_print("%02d: %s\n", i, nodeName);
		
		g_free(nodeName);
	}
	
	g_print("\n");
#endif
	
	
	// Now remove all children whose parents are also part of the drag list
	
	for (i = 0 ; i < dragStruct->numDragNodes - 1; ++i) {
		GtkTreePath *parentPath = dragStruct->dragNodes[i].nodePath;
		
		for (j = i+1; j < dragStruct->numDragNodes; ++j) {
			GtkTreePath *childPath = dragStruct->dragNodes[j].nodePath;
			
			if (!gtk_tree_path_is_ancestor(parentPath, childPath)) {
				continue;
			}
			
#ifdef DEBUG_DRAG_DROP
			g_print("%s: Drag item %d is a child of item %d; removing redundant node reference\n", __func__, j, i);
#endif
			
			// It's a child of the parent, so remove it from the list
			
			gtk_tree_path_free(childPath);
			
			size_t numBytes = (dragStruct->numDragNodes - j) * sizeof(TreeNodeStruct);
			
			if (numBytes > 0) {
				g_memmove(&(dragStruct->dragNodes[j]), &(dragStruct->dragNodes[j+1]), numBytes);
			}
			
			dragStruct->numDragNodes -= 1;
			
			--j;
		}
	}
	
	
#ifdef DEBUG_DRAG_DROP
	g_print("\n%s: Final drag list:\n", __func__);
	
	for (i = 0 ; i < dragStruct->numDragNodes; ++i) {
		gchar *nodeName = NULL;
		
		if (nodeName) g_free(nodeName);
		
		gtk_tree_model_get(GTK_TREE_MODEL(dragStruct->treeStore), &(dragStruct->dragNodes[i].nodeIter), COLUMN_FILEPATH, &nodeName, -1);
		
		g_print("%02d: %s\n", i, nodeName);
		
		g_free(nodeName);
	}
		
	g_print("\n");
#endif
	
	
	// We'll need the iter for the drop point
	
	if (dropPath && !gtk_tree_model_get_iter(GTK_TREE_MODEL(dragStruct->treeStore), &dropIter, dropPath)) {
		goto EXITPOINT;
	}
	
	
	// Finally, move the selected nodes
	
	for (i = 0 ; i < dragStruct->numDragNodes; ++i) {
#ifdef DEBUG_DRAG_DROP
		g_print("%s: Moving node #%d\n", __func__, i);
#endif
		// Drop to the root of the tree?
		
		if (dropPath == NULL && i == 0) {
			if (!copy_tree_node(&(dragStruct->dragNodes[i].nodeIter), NULL, position, &dropIter, &err)) {
				goto EXITPOINT;
			}
		}
		else {
			if (!copy_tree_node(&(dragStruct->dragNodes[i].nodeIter), &dropIter, position, &dropIter, &err)) {
				goto EXITPOINT;
			}
		}
		
		if (!remove_tree_node(&(dragStruct->dragNodes[i].nodeIter), &err)) {
			goto EXITPOINT;
		}
		
		// Everything after the first dropped item is added as a sibling to preserve the drop order!
		
		position = ADD_AFTER;
	}
	
	
	finalResult = TRUE;
	

EXITPOINT:
	
	if (err != NULL) {
		GtkWidget *errDialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "An error occurred while moving the node(s): \n\n%s", err->message);
		
		gtk_dialog_run(GTK_DIALOG (errDialog));
		
		gtk_widget_destroy(errDialog);
	}
	
	if (err) g_error_free(err);
	
	return finalResult;
}



/*
// Do I need to worry about this stuff?

if (GTK_IS_VIEWPORT(widget)) {
	gdk_window_get_position(GTK_VIEWPORT(widget)->bin_window, &wx, &wy);
	x -= wx;
	y -= wy;
}

if (GTK_IS_LAYOUT(widget)) {
	gdk_window_get_position(GTK_LAYOUT(widget)->bin_window, &wx, &wy);
	x -= wx;
	y -= wy;
}
*/


// Useful during debugging....

//~ switch (dragStruct->dropPositionHint) {
//~ 	case GTK_TREE_VIEW_DROP_BEFORE: {
//~ 		g_print("%s: %s, hint = DROP_BEFORE\n", __func__, dropNodeName);
//~ 		break;
//~ 	}
//~ 	case GTK_TREE_VIEW_DROP_AFTER: {
//~ 		g_print("%s: %s, hint = DROP_AFTER\n", __func__, dropNodeName);
//~ 		break;
//~ 	}
//~ 	case GTK_TREE_VIEW_DROP_INTO_OR_BEFORE: {
//~ 		g_print("%s: %s, hint = DROP_INTO_OR_BEFORE\n", __func__, dropNodeName);
//~ 		break;
//~ 	}
//~ 	case GTK_TREE_VIEW_DROP_INTO_OR_AFTER: {
//~ 		g_print("%s: %s, hint = DROP_INTO_OR_AFTER\n", __func__, dropNodeName);
//~ 		break;
//~ 	}
//~ 	default: {
//~ 		g_print("%s: %s, hint = <unknown>\n", __func__, dropNodeName);
//~ 		break;
//~ 	}
//~ }

//~ g_print(", drag_context->actions: ");
//~ if (drag_context->actions & GDK_ACTION_DEFAULT) g_print(" GDK_ACTION_DEFAULT");
//~ if (drag_context->actions & GDK_ACTION_COPY) g_print(" GDK_ACTION_COPY");
//~ if (drag_context->actions & GDK_ACTION_MOVE) g_print(" GDK_ACTION_MOVE\n");
//~ if (drag_context->actions & GDK_ACTION_LINK) g_print(" GDK_ACTION_LINK");
//~ if (drag_context->actions & GDK_ACTION_PRIVATE) g_print(" GDK_ACTION_PRIVATE");
//~ if (drag_context->actions & GDK_ACTION_ASK) g_print(" GDK_ACTION_ASK");
