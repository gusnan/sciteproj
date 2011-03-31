/**
 * drag_drop.h - Drag-and-drop support for SciteProj
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
#ifndef __HEADER_DRAG_DROP_
#define __HEADER_DRAG_DROP_



struct TreeNodeStruct {
	gint nodeItemType;
	GtkTreePath *nodePath;
	GtkTreeIter nodeIter;
};

typedef struct TreeNodeStruct TreeNodeStruct;


struct TreeViewDragStruct {
	GtkTreeView *treeView;
	
	GtkTreeStore *treeStore;
	
	gboolean isLocalDrag;
	
	TreeNodeStruct *dragNodes;
	gint numDragNodes;
	
	GtkTreeViewDropPosition dropPositionHint;
};

typedef struct TreeViewDragStruct TreeViewDragStruct;


#define DND_STRING_TYPE		12345
#define DND_URI_TYPE		12346


// Callback for "drag-drop" event
extern gboolean drag_drop_cb(GtkWidget *widget, GdkDragContext *drag_context, gint x, gint y, guint time, gpointer user_data);

// Callback for "drag-motion" event
extern gboolean drag_motion_cb(GtkWidget *widget, GdkDragContext *drag_context, gint x, gint y, guint time, gpointer user_data);

// Callback for "drag-data-received" event
extern void drag_data_received_cb(GtkWidget *widget, GdkDragContext *dc, gint x, gint y, GtkSelectionData *selection_data, guint info, guint t, gpointer data);

// Callback for "drag-data-get" event
extern void drag_data_get_cb(GtkWidget *widget, GdkDragContext *dc, GtkSelectionData *selection_data, guint info, guint t, gpointer data);


#endif /*__HEADER_DRAG_DROP_*/
