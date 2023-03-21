/**
 * selection.c - Code for working with the tree view selection
 *
 *  Copyright 2023 Andreas Rönnquist
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
#include <sys/stat.h>
#include <glib.h>
#include <gtk/gtk.h>

#include <gdk/gdkkeysyms.h>

#include <stdlib.h>
#include <glib/gi18n.h>

#include <locale.h>

#include "clicked_node.h"

#include "gui.h"

#include "tree_manipulation.h"


/**
 *
 */
GList *selected_items = NULL;

int old_depth = -1;

static gboolean in_function = FALSE;

/**
 *
 */
void init_selection ()
{
}


/**
 *
 */
void delete_list (GList *list)
{
   g_list_free_full (list, (GDestroyNotify) gtk_tree_path_free);
}


/**
 *
 */
void done_selection ()
{
   if (selected_items) {
      delete_list (selected_items);
   }
}


/**
 *
 */
GtkTreePath *get_newest_path_added (GList *list1, GList *list2) 
{
   if ((!list1)  && (!list2)) return NULL;

   GtkTreePath *current1 = NULL;
   GtkTreePath *current2 = NULL;

   GtkTreePath *result = NULL;

   for (GList *l1 = list1; l1 != NULL; l1 = l1->next) {

      current1 = l1->data;

      result = current1;

      for (GList *l2 = list2; l2 != NULL; l2 = l2->next) {

         current2 = l2->data;

         if ((current1 != NULL) && (current2 != NULL)) {

            if (gtk_tree_path_compare (current1, current2) == 0) {
               result = NULL;
            }
         }
      }

      if (result != NULL) return result;
   }

   return NULL;
}


/**
 * selection_changed_cb
 *
 * this function is used to check if a new item in the selection (added by
 * pressing Ctrl + mouseclick) is on the same folder level as previous
 * selections in the treeview - if on a new level, remove the old selection
 * and start working on a new one.
 */
void selection_changed_cb (GtkTreeSelection *tree_selection, gpointer user_data)
{
   // the in_function variable is used in a construction to make sure that this
   // changed callback isn't called when changing the selection below in
   // gtk_tree_selection_unselect_all and gtk_tree_selection_select_path
   if (in_function) return;

   in_function = TRUE;

   GList *list;

   list = gtk_tree_selection_get_selected_rows (tree_selection, NULL);

   GtkTreePath *added_path = get_newest_path_added (list, selected_items);

   int depth = -1;

   if (added_path != NULL) {
      depth = gtk_tree_path_get_depth(added_path);
   }

   if (depth != old_depth) {

      gtk_tree_selection_unselect_all ( tree_selection);

      if (added_path != NULL)
         gtk_tree_selection_select_path (tree_selection, added_path);
   }

   // copy list to selected_items for next comparison
   selected_items = g_list_copy_deep (list, (GCopyFunc)gtk_tree_path_copy, NULL);

   old_depth = depth;

   delete_list (list);

   in_function = FALSE;
}


/**
 *
 */
GList *get_list_of_marked_files()
{
   GList *list;
   GtkTreePath *current;
   GList *result_list = NULL;

   GtkTreeModel *tree_model = gtk_tree_view_get_model(GTK_TREE_VIEW(projectTreeView));

   GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (projectTreeView));

   list = gtk_tree_selection_get_selected_rows (selection, NULL);

   if (list != NULL) {

      for (GList *l = list; l != NULL; l = l -> next) {
         current = l->data;

         if (current != NULL) {

            GtkTreeIter iter;

            gtk_tree_model_get_iter (GTK_TREE_MODEL (tree_model), &iter, current);

            gchar *filepath;
            gtk_tree_model_get (GTK_TREE_MODEL (tree_model), &iter, COLUMN_FILEPATH, &filepath, -1);

            // printf("File: %s\n", filepath);

            GtkTreeIter *newiter = gtk_tree_iter_copy (&iter);

            gint type;
            gtk_tree_model_get (GTK_TREE_MODEL (tree_model), &iter, COLUMN_ITEMTYPE, &type, -1);

            ClickedNode *new_node = create_clicked_node (TRUE, *newiter, filepath, type);

            result_list = g_list_prepend (result_list, new_node);

         }
      }
   }

   delete_list (list);

   return result_list;
}
