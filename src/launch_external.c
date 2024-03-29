/**
 * clipboard.c - clipboard code for SciteProj
 *
 *  Copyright 2021 Andreas Rönnquist
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


#include "launch_external.h"


/**
 *
 */
void launch_default_for_uri (GtkTreeModel *model, GtkTreeIter *iter)
{
   GError *err = NULL;

   gchar *nodename = NULL;
   gchar *filePath = NULL;
   int nodeType = -1;

   gtk_tree_model_get (model, iter,
                      COLUMN_FILENAME, &nodename,
                      COLUMN_ITEMTYPE, &nodeType,
                      COLUMN_FILEPATH, &filePath,
                      -1);

   gchar *absFilePath = NULL; //g_strdup_printf("%s",filePath);

   if (!relative_path_to_abs_path (filePath, &absFilePath, get_project_directory (), &err)) {
      goto EXITPOINT;
   }

   gchar *newString = g_strdup_printf ("%s%s", "file://", absFilePath);

   g_app_info_launch_default_for_uri ((const gchar*)newString, NULL, &err);

EXITPOINT:
   if (err) g_error_free (err);
}


/**
 *
 */
void launch_file_browser (gchar *folder_name)
{
   GError *err = NULL;

   gchar *newString = g_strdup_printf ("%s%s", "file://", folder_name);

   g_app_info_launch_default_for_uri ((const gchar*)newString, NULL, &err);
}


/**
 *
 */
void launch_default_for_uri_cb ()
{
   if (clicked_node.valid) {
      if (clicked_node.type == ITEMTYPE_FILE) {
         launch_default_for_uri (gtk_tree_view_get_model (GTK_TREE_VIEW (projectTreeView)), &(clicked_node.iter));
      } else {
         gchar *name = clicked_node.name;
         gchar *newname = g_strdup_printf ("%s", name);

         launch_file_browser (newname);

         g_free (newname);
      }
   }
}
