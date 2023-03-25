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

GtkWidget *delete_text_entry;

GtkWidget *file_list_dialog;
gchar *resulting_text;


/**
 *
 */
int do_dialog_with_file_list(GList *file_list)
{
   GtkWidget *content_area;

   GtkWidget *label;

   GtkDialogFlags flags;
   GtkWidget *scrolled_window;
   GtkWidget *grid;

   // gchar *string_result = NULL;
   int int_result = 0;

   GtkWindow *main_window = get_main_window();

   // Create the widgets
   flags = GTK_DIALOG_DESTROY_WITH_PARENT;
   file_list_dialog = gtk_dialog_new_with_buttons ("Delete files",
                                       main_window,
                                       flags,
                                       _("_OK"),
                                       GTK_RESPONSE_ACCEPT,
                                       _("_Cancel"),
                                       GTK_RESPONSE_REJECT,
                                       NULL);
   content_area = gtk_dialog_get_content_area (GTK_DIALOG (file_list_dialog));
   label = gtk_label_new (_("The following files will be deleted:"));

   // Ensure that the dialog box is destroyed when the user responds

   // Add the label, and show everything we’ve added
   grid = gtk_grid_new();

   gtk_grid_attach (GTK_GRID (grid), label, 0, 0, 1, 1);

   gtk_widget_set_halign (GTK_WIDGET (label), GTK_ALIGN_START);

   delete_text_entry = gtk_text_view_new ();

   gtk_text_view_set_editable (GTK_TEXT_VIEW (delete_text_entry), FALSE);
   gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (delete_text_entry), FALSE);

   // gtk_entry_set_max_length (GTK_ENTRY (delete_text_entry), 0);

   // g_signal_connect (GTK_EDITABLE (delete_text_entry), "changed", G_CALLBACK (text_entry_text_changed), NULL);
   // g_signal_connect (GTK_ENTRY (delete_text_entry), "activate", G_CALLBACK (text_entry_activate), NULL);

   GtkTextBuffer *text_buffer;

   text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (delete_text_entry));

   for (GList *list_iter = file_list; list_iter != NULL; list_iter = list_iter -> next) {
      gchar *curr_text = list_iter->data;

      gchar *temp = g_strdup_printf("%s\n", curr_text);

      gtk_text_buffer_insert_at_cursor (GTK_TEXT_BUFFER (text_buffer), temp, strlen (temp));
   }

   scrolled_window = gtk_scrolled_window_new(NULL, NULL);
   gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);

   gtk_widget_set_vexpand(scrolled_window, TRUE);
   gtk_widget_set_hexpand(scrolled_window, TRUE);

   gtk_container_add (GTK_CONTAINER (scrolled_window), delete_text_entry);
   gtk_widget_show_all (scrolled_window);

   gtk_widget_show (GTK_WIDGET (delete_text_entry));

   gtk_grid_attach (GTK_GRID (grid), scrolled_window, 0, 1, 1, 20);

   gtk_widget_show (GTK_WIDGET (label));
   gtk_widget_show (GTK_WIDGET (scrolled_window));
   gtk_widget_show (GTK_WIDGET (grid));

   gtk_container_add (GTK_CONTAINER (content_area), grid);

   gtk_widget_show (GTK_WIDGET (content_area));

   gtk_dialog_set_default_response (GTK_DIALOG (file_list_dialog), GTK_RESPONSE_ACCEPT);

   gtk_window_resize (GTK_WINDOW (file_list_dialog), 500, 200);

   gint result = gtk_dialog_run (GTK_DIALOG (file_list_dialog));

   switch (result) {
   case GTK_RESPONSE_ACCEPT:
      int_result = 1;
      break;
   case GTK_RESPONSE_REJECT:
      int_result = 0;
      break;
   default:
      int_result = 0;
   };

   gtk_widget_destroy (GTK_WIDGET (file_list_dialog));

   return int_result;
}


/**
 *
 */
void recurse_folder(gchar *folder_name, GList **filenames)
{
   GDir *dir = g_dir_open (folder_name, 0, NULL);
   const gchar *filename;

   if (!g_file_test (folder_name, G_FILE_TEST_IS_DIR))
      return;

   while ((filename = g_dir_read_name (dir))) {

      gchar *full_filename = g_build_filename(folder_name, filename, NULL);

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

      if (do_dialog_with_file_list (list_filenames) != 0) {

         debug_printf("Remove the files!\n");

         // We should reverse this list, so that we get to the files before the
         // folders they are in.

         for (curr = list_filenames; curr != NULL; curr = curr -> next) {
            gchar *name = curr->data;

            debug_printf ("Data: %s\n", name);

            GFile *file_to_delete;

            file_to_delete = g_file_new_for_path(name);

            g_autoptr(GError) err = NULL;

            if (!g_file_delete (file_to_delete, NULL, &err)) {
               g_warning ("Failed to delete %s: %s\n", g_file_peek_path (file_to_delete), err->message);
            }

            g_object_unref (file_to_delete);

            /*
            int delete_result = g_remove(name);

            if (delete_result != 0) {

               printf ("Filename: %s\nError: %d\n", name, delete_result);
            }
            */
         }
      }

      // TODO: Free the strings in list_filenames

      /*
      printf ("Clicked: %s\n", clicked_node.name);

      if (get_requested_file_name (&filename) == 0) {
         printf ("Cancelled!\n");
         return;
      }

      gchar *full_filename;

      full_filename = g_build_filename (clicked_node.name, filename, NULL);

      debug_printf ("full filename: '%s'\n", full_filename);

      if (g_mkdir (full_filename, S_IRWXU | S_IRWXG | S_IRWXO) != 0) {
         warning_dialog("The folder '%s' does already exist - cannot create it!", filename);
      }
      GtkTreeIter iterHolder;
      GtkTreeIter *newIter = &iterHolder;

      if (!gtk_tree_model_iter_parent (tree_model, newIter, itercopy)) {
         newIter = gtk_tree_iter_copy (itercopy);
      }

      refresh_folder_with_iter (newIter);
      */
   }

}
