/**
 * delete.c - code for deleting items
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
#include <glib/gstdio.h>

#include <gdk/gdkkeysyms.h>

#include <stdlib.h>
#include <glib/gi18n.h>

#include <locale.h>

#include "clicked_node.h"

#include "gui.h"

#include "tree_manipulation.h"
#include "create_folder.h"

#include "gui_callbacks.h"

#include "string_utils.h"

#include "selection.h"

#include "file_utils.h"

#include "dialogs.h"


/**
 *
 */
void recurse_folder (gchar *folder_name, GList **filenames)
{
   GDir *dir = g_dir_open (folder_name, 0, NULL);
   const gchar *filename;

   if (!g_file_test (folder_name, G_FILE_TEST_IS_DIR))
      return;

   while ((filename = g_dir_read_name (dir))) {

      gchar *temp_filename = g_build_filename (folder_name, filename, NULL);

      gchar *full_filename = fix_separators (temp_filename);

      if (g_file_test (full_filename, G_FILE_TEST_IS_DIR)) {

         *filenames = g_list_prepend (*filenames, full_filename);
         recurse_folder ((gchar*)full_filename, filenames);
      } else {

         *filenames = g_list_prepend (*filenames, full_filename);
      }
   }

   g_dir_close (dir);

   return;
}


/**
 *
 */
void delete_item_cb ()
{
   debug_printf ("Delete item cb...\n");

   gchar *filename = NULL;

   GtkTreeIter *iter = gtk_tree_iter_copy (&(clicked_node.iter));

   GList *curr;

   GList *list_filenames = NULL;

   if (tree_iter_is_valid (iter)) {

      GList *list_of_files = get_list_of_marked_files ();

      for (curr = list_of_files; curr != NULL; curr = curr -> next) {
         ClickedNode *node_iter = (ClickedNode *) curr->data;

         filename = NULL;

         if (node_iter->type == ITEMTYPE_GROUP) {
            filename = node_iter->name;

            recurse_folder (filename, &list_filenames);

         } else {
            gchar *full_filename = g_build_filename (get_project_directory (), node_iter->name, NULL);
            filename = fix_separators (full_filename);

         }

         // debug_printf ("File: %s\n", filename);

         list_filenames = g_list_append (list_filenames, filename);
      }

      if (do_dialog_with_file_list (_("Delete files"), list_filenames) != 0) {

         debug_printf("Remove the files!\n");

         // We should reverse this list, so that we get to the files before the
         // folders they are in.

         for (curr = list_filenames; curr != NULL; curr = curr -> next) {
            gchar *name = curr->data;

            debug_printf ("Data: %s\n", name);

            GFile *file_to_delete;

            file_to_delete = g_file_new_for_path(name);

            g_autoptr (GError) err = NULL;

            if (!g_file_trash (file_to_delete, NULL, &err)) {
               g_warning ("Failed to move to trash %s: %s\n", g_file_peek_path (file_to_delete), err->message);
            }

            g_object_unref (file_to_delete);

         }
      }

      // TODO: Free the strings in list_filenames

   }

}
