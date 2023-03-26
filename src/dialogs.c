/**
 * dialogs.h - the programs dialogs in one place
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
int do_dialog_with_file_list (gchar *title, GList *file_list)
{
   GtkWidget *content_area;

   GtkWidget *label;

   GtkDialogFlags flags;
   GtkWidget *scrolled_window;
   GtkWidget *grid;

   // gchar *string_result = NULL;
   int int_result = 0;

   GtkWindow *main_window = get_main_window();
   
   GtkWidget *file_list_dialog;
   
   GtkWidget *file_list_entry;

   // Create the widgets
   flags = GTK_DIALOG_DESTROY_WITH_PARENT;
   file_list_dialog = gtk_dialog_new_with_buttons (title,
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


   GtkWidget *image;
   image = gtk_image_new_from_icon_name ("dialog-warning", GTK_ICON_SIZE_DIALOG);

   gtk_widget_show (GTK_WIDGET (image));

   gtk_grid_attach (GTK_GRID (grid), image, 0, 0, 1, 5);

   gtk_grid_attach (GTK_GRID (grid), label, 1, 0, 2, 1);

   gtk_widget_set_halign (GTK_WIDGET (label), GTK_ALIGN_START);

   file_list_entry = gtk_text_view_new ();

   gtk_text_view_set_editable (GTK_TEXT_VIEW (file_list_entry), FALSE);
   gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (file_list_entry), FALSE);

   // gtk_entry_set_max_length (GTK_ENTRY (delete_text_entry), 0);

   // g_signal_connect (GTK_EDITABLE (delete_text_entry), "changed", G_CALLBACK (text_entry_text_changed), NULL);
   // g_signal_connect (GTK_ENTRY (delete_text_entry), "activate", G_CALLBACK (text_entry_activate), NULL);

   GtkTextBuffer *text_buffer;

   text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (file_list_entry));

   for (GList *list_iter = file_list; list_iter != NULL; list_iter = list_iter -> next) {
      gchar *curr_text = list_iter->data;

      gchar *temp = g_strdup_printf("%s\n", curr_text);

      gtk_text_buffer_insert_at_cursor (GTK_TEXT_BUFFER (text_buffer), temp, strlen (temp));
   }

   scrolled_window = gtk_scrolled_window_new (NULL, NULL);
   gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);

   gtk_widget_set_vexpand (scrolled_window, TRUE);
   gtk_widget_set_hexpand (scrolled_window, TRUE);

   gtk_container_add (GTK_CONTAINER (scrolled_window), file_list_entry);
   gtk_widget_show_all (scrolled_window);

   gtk_widget_show (GTK_WIDGET (file_list_entry));

   gtk_grid_attach (GTK_GRID (grid), scrolled_window, 1, 1, 1, 20);

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
int warning_dialog(const char *window_title, const char *fmt, ...)
{
   GtkWidget *content_area;
   GtkWidget *warning_dialog;

   GtkWidget *label;

   GtkDialogFlags flags;

   GtkWidget *grid;

   GtkWidget *image;

   // gchar *string_result = NULL;
   int int_result = 0;

   GtkWindow *main_window = get_main_window();

   gchar label_text[256];

   va_list args;
   va_start(args, fmt);
   vsprintf(label_text, fmt, args);

   // dialog = do_question_dialog(buffer);

   va_end(args);


   // Create the widgets
   flags = GTK_DIALOG_DESTROY_WITH_PARENT;
   warning_dialog = gtk_dialog_new_with_buttons (window_title,
                                       main_window,
                                       flags,
                                       _("_OK"),
                                       GTK_RESPONSE_ACCEPT,
                                       // _("_Cancel"),
                                       // GTK_RESPONSE_REJECT,
                                       NULL);
   content_area = gtk_dialog_get_content_area (GTK_DIALOG (warning_dialog));
   label = gtk_label_new (label_text);

   image = gtk_image_new_from_icon_name ("dialog-warning", GTK_ICON_SIZE_DIALOG);

   // Ensure that the dialog box is destroyed when the user responds

   // Add the label, and show everything we’ve added

   grid = gtk_grid_new();

   gtk_grid_set_column_spacing (GTK_GRID (grid), 10);
   gtk_grid_set_row_spacing (GTK_GRID (grid), 10);

   gtk_grid_attach (GTK_GRID (grid), image, 0, 0, 1, 2);

   gtk_grid_attach (GTK_GRID (grid), label, 1, 0, 4, 2);

   // gtk_grid_attach (GTK_GRID (grid), entry, 1, 0, 1, 1);

   gtk_widget_show (GTK_WIDGET (image));
   gtk_widget_show (GTK_WIDGET (grid));
   gtk_widget_show (GTK_WIDGET (label));

   gtk_container_add (GTK_CONTAINER (content_area), grid);

   gtk_dialog_set_default_response (GTK_DIALOG (warning_dialog), GTK_RESPONSE_ACCEPT);

   gint result = gtk_dialog_run (GTK_DIALOG (warning_dialog));

   switch (result) {
   case GTK_RESPONSE_ACCEPT:
      int_result = 1;
      // if (string_result != NULL) {
         // (*string_result) = (gchar *)g_strdup_printf("%s", entry_text);
      // }
      break;
   case GTK_RESPONSE_REJECT:
      int_result = 0;
      break;
   default:
      int_result = 0;
   };

   gtk_widget_destroy (GTK_WIDGET (warning_dialog));

   return int_result;
}
