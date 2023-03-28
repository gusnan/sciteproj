/**
 * clipboard.c - clipboard code for SciteProj
 *
 *  Copyright 2010-2017 Andreas Rönnquist
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

#include "clicked_node.h"

#include "gui.h"
#include "tree_manipulation.h"
#include "file_utils.h"
#include "string_utils.h"


#include "clipboard.h"




/**
 * Copies the filename including full folder to the clipboard
 */
void copy_filename_to_clipboard (GtkTreeModel *model, GtkTreeIter *iter)
{
   GError *err = NULL;

   gchar *nodename = NULL;
   gchar *filePath = NULL;
   int nodeType = -1;
   GdkDisplay *display = NULL;
   GtkClipboard *clipboard = NULL;

   gtk_tree_model_get (model, iter,
                      COLUMN_FILENAME, &nodename,
                      COLUMN_ITEMTYPE, &nodeType,
                      COLUMN_FILEPATH, &filePath,
                      -1);

   gchar *absFilePath = NULL; //g_strdup_printf("%s",filePath);

   // filePath is NULL?
   //if (!relative_path_to_abs_path(sClickedNodeName, &absFilePath, get_project_directory(), &err)) {

   if (!relative_path_to_abs_path (filePath, &absFilePath, get_project_directory (), &err)) {
      goto EXITPOINT;
   }

   display = gdk_display_get_default ();

   clipboard = gtk_clipboard_get_for_display (display, GDK_SELECTION_CLIPBOARD);

   gtk_clipboard_set_text (clipboard, absFilePath, -1);

EXITPOINT:

   if (err) g_error_free (err);
}


/**
 * Callback for the menu item
 */
void copy_filename_to_clipboard_cb ()
{
   if (clicked_node.valid && clicked_node.type == ITEMTYPE_FILE) {
      copy_filename_to_clipboard (gtk_tree_view_get_model (GTK_TREE_VIEW (projectTreeView)), &(clicked_node.iter));
   }
}
