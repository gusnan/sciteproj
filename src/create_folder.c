/**
 * create_folder.c - code for creating folder
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


/**
 *
 */
void create_new_folder_cb ()
{
   gchar *filename = NULL;
   GtkTreeModel *tree_model = gtk_tree_view_get_model (GTK_TREE_VIEW(projectTreeView));

   GtkTreeIter *iter = gtk_tree_iter_copy (&(clicked_node.iter));

   // We can only use this on a folder
   if (!clicked_node.valid || clicked_node.type != ITEMTYPE_GROUP) {
      goto EXITPOINT;
   }

   GtkTreeIter *itercopy = gtk_tree_iter_copy (&(clicked_node.iter));

   if (tree_iter_is_valid (iter)) {

      debug_printf ("Clicked: %s\n", clicked_node.name);

      if (get_requested_file_name (_("Create folder"), _("Folder name:"), &filename) == 0) {
         debug_printf ("Cancelled!\n");
         return;
      }

      gchar *full_filename;

      full_filename = g_build_filename (clicked_node.name, filename, NULL);

      debug_printf ("full filename: '%s'\n", full_filename);

      if (g_mkdir (full_filename, S_IRWXU | S_IRWXG | S_IRWXO) != 0) {
         warning_dialog(_("The folder '%s' does already exist - cannot create it!"), filename);
      }
      GtkTreeIter iterHolder;
      GtkTreeIter *newIter = &iterHolder;

      if (!gtk_tree_model_iter_parent (tree_model, newIter, itercopy)) {
         newIter = gtk_tree_iter_copy (itercopy);
      }

      refresh_folder_with_iter (newIter);
   }

EXITPOINT:
   return;
}
