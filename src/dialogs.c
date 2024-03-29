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


const int BORDER_WIDTH = 5;

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

   gtk_container_set_border_width (GTK_CONTAINER (content_area), BORDER_WIDTH);

   gtk_widget_show (GTK_WIDGET (content_area));

   gtk_dialog_set_default_response (GTK_DIALOG (file_list_dialog), GTK_RESPONSE_ACCEPT);

   gtk_window_resize (GTK_WINDOW (file_list_dialog), 600, 300);

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

   gtk_container_set_border_width (GTK_CONTAINER (content_area), BORDER_WIDTH);

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





static void text_entry_text_changed (GtkEditable *self, gpointer user_data)
{
   // user_data is &changing_text, som we can use this to avoid using
   // globals to set the values.
   gchar **pointer = (gchar **)(user_data);
   if (self != NULL) {
      *pointer = g_strdup ((gchar*)gtk_entry_get_text (GTK_ENTRY (self)));
   }
}


static void text_entry_activate (GtkEntry *self, gpointer user_data)
{
   GtkWidget *dialog = GTK_WIDGET (user_data);

   gtk_dialog_response( GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);
}


/**
 *
 */
int
get_requested_file_name (gchar *window_title, gchar *label_text, gchar *info_label_text, gchar **string_result)
{
   GtkWidget *content_area;

   GtkWidget *label;

   GtkWidget *info_label;

   GtkDialogFlags flags;

   GtkWidget *grid;
   GtkWidget *entry;

   GtkWidget *request_filename_dialog;

   gchar *changing_text = NULL;

   // gchar *string_result = NULL;
   int int_result = 0;

   GtkWindow *main_window = get_main_window();

   // Create the widgets
   flags = GTK_DIALOG_DESTROY_WITH_PARENT;
   request_filename_dialog = gtk_dialog_new_with_buttons (window_title,
                                       main_window,
                                       flags,
                                       _("_OK"),
                                       GTK_RESPONSE_ACCEPT,
                                       _("_Cancel"),
                                       GTK_RESPONSE_REJECT,
                                       NULL);
   content_area = gtk_dialog_get_content_area (GTK_DIALOG (request_filename_dialog));
   // label = gtk_label_new (label_text);

   gchar *formatted_label_text = g_strdup_printf ("<b>%s</b>", label_text);

   label = gtk_label_new (NULL);
   gtk_label_set_markup (GTK_LABEL (label), formatted_label_text);

   info_label = gtk_label_new (info_label_text);


   // Ensure that the dialog box is destroyed when the user responds

   // Add the label, and show everything we’ve added

   grid = gtk_grid_new();

   gtk_grid_set_row_spacing (GTK_GRID (grid), 10);
   gtk_grid_set_column_spacing (GTK_GRID (grid), 10);

   GtkWidget *image;
   image = gtk_image_new_from_icon_name ("dialog-warning", GTK_ICON_SIZE_DIALOG);

   gtk_widget_show (GTK_WIDGET (image));


   gtk_grid_attach (GTK_GRID (grid), image, 0, 0, 1, 3);
   gtk_grid_attach (GTK_GRID (grid), label, 1, 0, 50, 1);

   gtk_grid_attach (GTK_GRID (grid), info_label, 1, 1, 50, 1);

   gtk_widget_set_halign (GTK_WIDGET (label), GTK_ALIGN_START);
   gtk_widget_set_halign (GTK_WIDGET (info_label), GTK_ALIGN_START);

   entry = gtk_entry_new ();

   gtk_widget_show (GTK_WIDGET (entry));

   gtk_entry_set_max_length (GTK_ENTRY (entry), 0);

   g_signal_connect (GTK_EDITABLE (entry), "changed", G_CALLBACK (text_entry_text_changed), (gpointer)(&changing_text));
   g_signal_connect (GTK_ENTRY (entry), "activate", G_CALLBACK (text_entry_activate), request_filename_dialog);

   gtk_grid_attach (GTK_GRID (grid), entry, 1, 2, 50, 1);

   gtk_widget_show (GTK_WIDGET (grid));
   gtk_widget_show (GTK_WIDGET (label));
   gtk_widget_show (GTK_WIDGET (info_label));

   gtk_container_set_border_width (GTK_CONTAINER (content_area), BORDER_WIDTH);

   gtk_container_add (GTK_CONTAINER (content_area), grid);

   gtk_dialog_set_default_response (GTK_DIALOG (request_filename_dialog), GTK_RESPONSE_ACCEPT);

   gtk_window_resize (GTK_WINDOW (request_filename_dialog), 500, 200);

   gint result = gtk_dialog_run (GTK_DIALOG (request_filename_dialog));

   switch (result) {
   case GTK_RESPONSE_ACCEPT:
      int_result = 1;
      if (string_result != NULL) {

         if (changing_text != NULL) {
            if (strlen (changing_text) == 0) {
               (*string_result) = NULL;
            } else {
               (*string_result) = (gchar *)g_strdup_printf("%s", changing_text);
            }
         } else {
            (*string_result) = NULL;
         }
      }
      break;
   case GTK_RESPONSE_REJECT:
      int_result = 0;
      break;
   default:
      int_result = 0;
   };

   gtk_widget_destroy (GTK_WIDGET (request_filename_dialog));

   return int_result;
}
