/**
 * gui_callbacks.c - GUI callback code for SciteProj
 *
 *  Copyright 2006 Roy Wood, 2009-2023 Andreas Rönnquist
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
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <gdk/gdkkeysyms.h>

#include <stdlib.h>
#include <glib/gi18n.h>

#include <locale.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "clicked_node.h"
#include "gui_callbacks.h"

#include "gui.h"
#include "tree_manipulation.h"
#include "scite_utils.h"
#include "string_utils.h"
#include "prefs.h"
#include "statusbar.h"
#include "graphics.h"
#include "about.h"
#include "properties_dialog.h"
#include "file_utils.h"

#include "recent_files.h"
#include "remove.h"
#include "sort.h"

#include "load_folder.h"

#include "script.h"

#include "expand.h"

#include "selection.h"

#include "dialogs.h"


/**
 * Open the selected file.
 *	This is called when a file is rightclicked and open is selected in the menu
 */
void popup_open_file_cb()
{
   gchar *command = NULL;
   GError *err = NULL;
   GtkWidget *dialog = NULL;
   gchar *absFilePath = NULL;

   // several files in selection?

   GList *list = get_list_of_marked_files();

   if (list != NULL) {
      for (GList *temp = list; temp != NULL; temp = temp -> next) {
         ClickedNode *curr = temp->data;

         if (curr) {

            // We can only open files
            if (!curr->valid || curr->type != ITEMTYPE_FILE) {
               goto EXITPOINT;
            }

            gchar *fixed = fix_path((gchar*)get_project_directory(), curr->name);

            if (!open_filename(curr->name, (gchar*)(get_project_directory()), &err)) {
               goto EXITPOINT;
            }

            add_file_to_recent (fixed, NULL);
         }
      }

      g_list_free_full (list, (GDestroyNotify) g_free);
   }



EXITPOINT:

   if (err != NULL) {
      dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                                      GTK_BUTTONS_OK,
                                      _("Could not open selected file: \n\n%s"),
                                      err->message);

      gtk_dialog_run(GTK_DIALOG (dialog));
   }

   if (command) g_free(command);
   if (absFilePath) g_free(absFilePath);
   if (err) g_error_free(err);
   if (dialog) gtk_widget_destroy(dialog);
}



/**
 *	Open the LUA rc file for the project folder
 */
void edit_properties_cb()
{
   GError *err = NULL;
   gchar *command = NULL;

   if ((command = g_strdup_printf("open:%s\n", prefs_filename)) == NULL) {
      g_set_error(&err, APP_SCITEPROJ_ERROR, -1,
                  "%s: %s, g_strdup_printf() = NULL",
                  "Error formatting SciTE command",
                  __func__);
   }
   else {
      if (send_scite_command(command, &err)) {
         // Try to activate SciTE; ignore errors

         activate_scite(NULL);

         if (prefs.give_scite_focus == TRUE) {
            send_scite_command((gchar*)"focus:0", NULL);
         }
      }
   }
}


/**
 * step-through function for expand/collapse folder
 *
 * @param tree_view
 * @param newiter
 * @param tree_path
 */
static void fix_folders_step_through(GtkTreeView *tree_view, GtkTreeIter newiter, GtkTreePath *tree_path)
{
   GtkTreeModel *tree_model = gtk_tree_view_get_model(tree_view);

   gchar *relFilePath;

   GError *error;
   gint nodeItemType;

   GtkTreeIter iter = newiter;

   do {

      gtk_tree_model_get(tree_model, &iter, COLUMN_ITEMTYPE, &nodeItemType, -1);

      if (nodeItemType == ITEMTYPE_GROUP) {

         GtkTreePath *srcPath = gtk_tree_model_get_path(tree_model, &iter);
         gboolean groupIsExpanded = tree_row_is_expanded(srcPath);

         if (groupIsExpanded) {
            set_tree_node_icon(&iter, directory_open_pixbuf, &error);
         } else {
            set_tree_node_icon(&iter, directory_closed_pixbuf, &error);
         }

         set_tree_node_expanded(&iter, groupIsExpanded, NULL);

         gtk_tree_model_get(tree_model, &iter, COLUMN_FILEPATH, &relFilePath, -1);

         if (gtk_tree_model_iter_has_child(tree_model, &iter)) {

            GtkTreeIter newIter;
            gtk_tree_model_iter_children(tree_model, &newIter, &iter);
            fix_folders_step_through(tree_view, newIter, tree_path);
         }

         g_free(relFilePath);
         gtk_tree_path_free(srcPath);

      } else {

      }


   } while(gtk_tree_model_iter_next(tree_model,&iter));
}


gchar *remove_trailing_dot_folder(gchar *infolder)
{
   gchar *result = NULL;

   if (g_str_has_suffix(infolder, "/.") == TRUE) {

      int len = strlen(infolder);


      result = g_strdup(infolder);

      result[len - 2] = '\0';

   } else {
      result = g_strdup(infolder);
   }

   return result;
}

/**
 *
 */
void load_tree_at_iter(GtkTreeView *tree_view, GtkTreeIter *iter)
{
   // We've got the folder - get the child
   GtkTreeIter child;

   GtkTreeModel *tree_model = gtk_tree_view_get_model(tree_view);
   gchar *temp = NULL;

   if (iter) {

      if (gtk_tree_model_iter_children(tree_model, &child, iter)) {
         remove_tree_node(&child, NULL);

         gchar *temp_folder_path;

         gtk_tree_model_get(tree_model, iter, COLUMN_FILEPATH, &temp_folder_path, -1);

         gchar *folder_path = g_path_get_dirname(temp_folder_path);

         temp = folder_path;
         folder_path = remove_trailing_dot_folder(temp);
         g_free(temp);

         // Load the wanted filter from the LUA config
         GSList *filter_list = load_filter_from_lua(folder_path);

         GSList *global_filter_list = prefs.hide_filter_global;

         GSList *iterator;

         for (iterator = global_filter_list; iterator; iterator = iterator->next) {

            temp = (gchar*)(iterator->data);

            gchar *temp_base = g_path_get_basename(temp);

            filter_list = g_slist_append(filter_list , (gpointer)(temp_base));
         }

         GSList *final_filter_list = filter_list;

         GSList *file_list; //=load_folder_to_list(folder_path, FALSE,
         GSList *folder_list;

         int sort_order = get_sort_order_from_iter(tree_view, iter);

         // Doesn't the iterator have a sort value (it doesn't/shouldn't have
         // if we havn't set sort order by right clicking it and selecting there
         if (sort_order == SORT_ORDER_INVALID) {
#ifdef _DEBUG
            printf("Folder: %s\n", temp_folder_path);
#endif
            // In that case get sort order from a .sciteprojrc.lua, where
            // we also can set this value.
            sort_order = get_sort_order_of_folder(temp_folder_path);
         }

         // If sort order still is invalid, then default to a reasonable
         // value, in this case, sort increasing by name.
         if (sort_order == SORT_ORDER_INVALID) {
            sort_order = SORT_ORDER_NAME_INCREASING;
         }

         GCompareFunc comparer = get_compare_func_from_sort_order_value(sort_order);

         file_list = load_folder_to_list(temp_folder_path,
                                         FALSE,
                                         comparer /*file_sort_by_extension_bigger_func*/,
                                         final_filter_list);

         folder_list = load_folder_to_list(temp_folder_path, TRUE, compare_strings_bigger, final_filter_list);

         // Here we should filter out the unwanted items

         add_tree_folderlist(iter, folder_list, temp_folder_path);

         if (file_list) {
            //file_list = g_slist_reverse(file_list);

            add_tree_filelist(iter, file_list, NULL);
         }

         set_tree_node_expanded(iter, TRUE, NULL);

         GtkTreePath *tree_path = gtk_tree_model_get_path(tree_model, iter);

         gtk_tree_view_expand_row(tree_view, tree_path, FALSE);

         gtk_tree_path_free(tree_path);

         g_slist_foreach(filter_list, (GFunc)g_free, NULL);
         g_slist_free(filter_list);

      }
   }
}


/**
 * Callback for expand/collapse event of GtkTreeView
 *
 * @param treeView is not used
 * @param arg1 is not used
 * @param arg2 is not used
 * @param user_data is not used
 */
void row_expand_or_collapse_cb(GtkTreeView *tree_view, GtkTreeIter *iter,
                               GtkTreePath *tree_path, gpointer user_data)
{
   /* Switch the folder icon open/closed*/

   GtkTreeModel *tree_model = gtk_tree_view_get_model(tree_view);

   // make sure all icons the folder (and folders inside it) are set to a correct icon.
   fix_folders_step_through(tree_view, *iter, tree_path);

   gchar *temp;
   gboolean expanded;
   gboolean loaded;

   gtk_tree_model_get(tree_model, iter, COLUMN_FILEPATH, &temp, -1);
   gtk_tree_model_get(tree_model, iter, COLUMN_EXPANDED, &expanded, -1);
   gtk_tree_model_get(tree_model, iter, COLUMN_FOLDER_CONTENT_LOADED, &loaded, -1);

   if (expanded) {
      if (!loaded) {
         set_tree_node_loaded(iter, TRUE, NULL);

         load_tree_at_iter(tree_view, iter);

         ClickedNode *new_node;
         new_node = create_clicked_node(TRUE, *iter, temp, ITEMTYPE_GROUP);

         refresh_folder(new_node);

         g_free(new_node);
      }
   } else {

      set_tree_node_loaded(iter, FALSE, NULL);

      GList *list_of_items = NULL;
      GtkTreeIter newIter;

      GtkTreeIter child;

      if (gtk_tree_model_iter_children(tree_model, &child, iter)) {
         GtkTreePath *temp_tree_path;

         GtkTreeIter *temp_iter = &child;
         do {

            gchar *filename_temp;
            gtk_tree_model_get(tree_model, temp_iter, COLUMN_FILENAME, &filename_temp, -1);

            temp_tree_path = gtk_tree_model_get_path(tree_model, temp_iter);
            GtkTreeRowReference *row_reference = gtk_tree_row_reference_new(tree_model, temp_tree_path);

            list_of_items = g_list_append(list_of_items, row_reference);

            gtk_tree_path_free(temp_tree_path);

         } while(gtk_tree_model_iter_next(tree_model, temp_iter));

         // go through the list of row-references

         GList *node;
         for (node = list_of_items; node != NULL; node = node -> next) {
            temp_tree_path = gtk_tree_row_reference_get_path((GtkTreeRowReference*)node->data);

            if (temp_tree_path) {
               if (gtk_tree_model_get_iter(tree_model, &newIter, temp_tree_path))
                  remove_tree_node(&newIter, NULL);
            }
         }

         g_list_foreach(list_of_items, (GFunc)gtk_tree_row_reference_free, NULL);
      }

      add_tree_file(iter, ADD_CHILD, "<loading...>", iter, FALSE, NULL);
   }
}



/**
 * Callback for "Quit" menu item
 */
void quit_menu_cb()
{
   gtk_main_quit();
}


/**
 * Callback for "About" menu item
 */
void about_menu_cb()
{
   show_about_dialog();
}



/**
 *
 */
gboolean key_press_cb(GtkWidget *widget, GdkEventKey *event, gpointer userData)
{
   switch (event->keyval)
   {
   case GDK_KEY_BackSpace:
   {
      debug_printf((gchar*)"key_press_cb: keyval = %d = GDK_BackSpace, hardware_keycode = %d\n",
                   event->keyval, event->hardware_keycode);
      break;
   }

   case GDK_KEY_Delete:
   {
      do_remove_node(TRUE);
      break;
   }
   case GDK_KEY_Insert:
   {
      break;
   }
   /*
   case GDK_KEY_F2:
   {
      do_rename_node(TRUE);
      return TRUE;
   }
   */
   default:
   {
      debug_printf("key_press_cb: keyval = %d = '%c', hardware_keycode = %d\n",
                   event->keyval, (char) event->keyval, event->hardware_keycode);
      return FALSE;
   }
   }

   if (event->state & GDK_SHIFT_MASK) debug_printf(", GDK_SHIFT_MASK");
   if (event->state & GDK_CONTROL_MASK) debug_printf(", GDK_CONTROL_MASK");
   if (event->state & GDK_MOD1_MASK) debug_printf(", GDK_MOD1_MASK");
   if (event->state & GDK_MOD2_MASK) debug_printf(", GDK_MOD2_MASK");
   if (event->state & GDK_MOD3_MASK) debug_printf(", GDK_MOD3_MASK");
   if (event->state & GDK_MOD4_MASK) debug_printf(", GDK_MOD4_MASK");
   if (event->state & GDK_MOD5_MASK) debug_printf(", GDK_MOD5_MASK");

   debug_printf("\n");

   return FALSE;
}


/**
 *		search function for the gtk_tree_view_set_search_equal_func
 *		@return TRUE when rows DONT match, FALSE when rows match
 */
gboolean tree_view_search_equal_func(GtkTreeModel *model, gint column,
                                     const gchar *key, GtkTreeIter *iter,
                                     gpointer search_data)
{
   gchar *filename;
   // For some reason this should return TRUE if the row DONT match
   gboolean res = TRUE;

   gtk_tree_model_get(model, iter, COLUMN_FILENAME, &filename, -1);

   // zero when matches, which means we should return FALSE
   if (g_ascii_strncasecmp(key, filename, strlen(key)) == 0) res = FALSE;

   g_free(filename);

   return res;
}

/**
 *
 */
void refresh_folder_with_iter(GtkTreeIter *iter)
{
   GtkTreeModel *tree_model = gtk_tree_view_get_model(GTK_TREE_VIEW(projectTreeView));
   GtkTreeIter *stored_iter = gtk_tree_iter_copy(iter);

   gchar *folder_name;

   gboolean expanded;

   gtk_tree_model_get(tree_model, iter, COLUMN_FILENAME, &folder_name,
                      COLUMN_EXPANDED, &expanded,
                      -1);

   GList *folder_status_list = NULL;

   // TODO: check if the folder filename is in the ignored list

   // If the folder is expanded
   if (expanded /* || extern_expanded */) {

      // add all rows below to a list of GtkTreePath
      GtkTreeIter child;

      GList *list_of_items = NULL;

      // First, store all GtkTreePath in a linked list

      if (gtk_tree_model_iter_children(tree_model, &child, iter)) {
         GtkTreePath *tree_path;

         GtkTreeIter *temp_iter = &child;
         do {

            gchar *temp;
            gtk_tree_model_get(tree_model, temp_iter, COLUMN_FILENAME, &temp, -1);

            int node_item_type;
            gboolean temp_exp = FALSE;

            gtk_tree_model_get(tree_model, temp_iter, COLUMN_ITEMTYPE, &node_item_type, -1);

            gtk_tree_model_get(tree_model, temp_iter, COLUMN_EXPANDED, &temp_exp, -1);

            if (node_item_type == ITEMTYPE_GROUP) {

               if (temp_exp) {

                  struct FolderStatus *folder_status;

                  folder_status = g_malloc(sizeof(FolderStatus));

                  folder_status->folder_name = temp;
                  folder_status->folder_expanded = temp_exp;

                  folder_status_list = g_list_append(folder_status_list, folder_status);
               }
            }

            tree_path = gtk_tree_model_get_path(tree_model, temp_iter);
            GtkTreeRowReference *row_reference = gtk_tree_row_reference_new(tree_model, tree_path);

            list_of_items = g_list_append(list_of_items, row_reference);

            gtk_tree_path_free(tree_path);

         } while(gtk_tree_model_iter_next(tree_model, temp_iter));

         // go through the list of row-references

         GList *node;
         for (node = list_of_items; node != NULL; node = node -> next) {
            tree_path = gtk_tree_row_reference_get_path((GtkTreeRowReference*)node->data);

            if (tree_path) {
               if (gtk_tree_model_get_iter(tree_model, iter, tree_path))
                  remove_tree_node(iter, NULL);
            }
         }

         g_list_foreach(list_of_items, (GFunc)gtk_tree_row_reference_free, NULL);
      }

      GtkTreeIter *temp_iter = gtk_tree_iter_copy(stored_iter);

      gchar *folder;
      gtk_tree_model_get(tree_model, temp_iter,
                         COLUMN_FILEPATH, &folder,
                         -1);

      GtkTreeIter new_iter;
      if (get_number_of_files_in_folder(folder) > 0) {
         add_tree_file(temp_iter, ADD_CHILD, "<loading...>", &new_iter, FALSE, NULL);
      }

      load_tree_at_iter(GTK_TREE_VIEW(projectTreeView), temp_iter);


      // restore folders open or closed here.

      if (folder_status_list != NULL) {
         expand_tree_with_expanded_list(tree_model, stored_iter, folder_status_list);
      } else {

         expand_tree(tree_model, stored_iter);
      }

      //sort_children(stored_iter, NULL, compare_strings_smaller);

      // Free memory of folder_status_list
      GList *node;
      for (node = folder_status_list; node != NULL; node = node -> next) {
         struct FolderStatus *temp = node->data;

         g_free(temp);
      }
   }

}


/**
 *
 */
void refresh_folder(ClickedNode *inNode)
{
   if (!inNode->valid || inNode->type != ITEMTYPE_GROUP) {
      return;
   }
   
   GtkTreeIter iter = inNode->iter;

   if (tree_iter_is_valid(&iter)) {
      refresh_folder_with_iter(&iter);
   }
}


static void text_entry_text_changed (GtkEditable *self, gpointer user_data)
{
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
get_requested_file_name (gchar *window_title, gchar *label_text, gchar **string_result)
{
   GtkWidget *content_area;

   GtkWidget *label;

   GtkDialogFlags flags;

   GtkWidget *grid;
   GtkWidget *entry;

   GtkWidget *request_filename_dialog;

   gchar *changing_text;

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
   label = gtk_label_new (label_text);

   // Ensure that the dialog box is destroyed when the user responds

   // Add the label, and show everything we’ve added

   grid = gtk_grid_new();

   gtk_grid_attach (GTK_GRID (grid), label, 0, 0, 1, 1);

   entry = gtk_entry_new ();

   gtk_widget_show (GTK_WIDGET (entry));

   gtk_entry_set_max_length (GTK_ENTRY (entry), 0);

   g_signal_connect (GTK_EDITABLE (entry), "changed", G_CALLBACK (text_entry_text_changed), (gpointer)(&changing_text));
   g_signal_connect (GTK_ENTRY (entry), "activate", G_CALLBACK (text_entry_activate), request_filename_dialog);

   gtk_grid_attach (GTK_GRID (grid), entry, 1, 0, 1, 1);

   gtk_widget_show (GTK_WIDGET (grid));
   gtk_widget_show (GTK_WIDGET (label));

   gtk_container_add (GTK_CONTAINER (content_area), grid);

   gtk_dialog_set_default_response (GTK_DIALOG (request_filename_dialog), GTK_RESPONSE_ACCEPT);

   gint result = gtk_dialog_run (GTK_DIALOG (request_filename_dialog));

   switch (result) {
   case GTK_RESPONSE_ACCEPT:
      int_result = 1;
      if (string_result != NULL) {
         (*string_result) = (gchar *)g_strdup_printf("%s", changing_text);
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



/**
 *
 */
void create_new_file_cb()
{
   GtkTreeModel *tree_model = gtk_tree_view_get_model(GTK_TREE_VIEW(projectTreeView));
   gchar *full_file_name = NULL;

   // We can only use this on a folder
   if (!clicked_node.valid || clicked_node.type != ITEMTYPE_GROUP) {
      goto EXITPOINT;
   }

   // add_file_to_recent(clicked_node.name, NULL);

   // Open the folder if it isn't open already

   GtkTreeIter *iter = gtk_tree_iter_copy(&(clicked_node.iter));

   if (tree_iter_is_valid(iter)) {

      debug_printf("Clicked node: %s\n", clicked_node.name);

      GtkTreePath *path;
      path = gtk_tree_model_get_path(tree_model, iter);
      expand_tree_row(path, FALSE);

      gchar *filename = NULL;

      gchar *folder = clicked_node.name;

      int result = get_requested_file_name(_("Create File"), _("Filename:"), &filename);

      if (result != 0) {

         debug_printf("File name: %s\n", filename);

         // build filename including folder

         // check if the file already exists in the file system

         full_file_name = g_build_filename(folder, filename, NULL);

         if (g_file_test(full_file_name, G_FILE_TEST_EXISTS)) {
            warning_dialog(_("File aready exists!"), _("The file '%s' does already exist!"), filename);

            goto EXITPOINT;

         }

         mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

         int create_result = g_creat (full_file_name, mode);
         if (create_result == -1) {
            warning_dialog (_("The file '%s' couldn't be created!"), filename);

            goto EXITPOINT;
         }

         GtkTreeIter newIter;

         gtk_tree_model_iter_parent(tree_model, &newIter, iter);

         refresh_folder_with_iter(&newIter);

         GtkTreeIter temp;
         GtkTreeIter *result_iter = NULL;

         if (gtk_tree_model_get_iter_first (tree_model, &temp)) {
            printf ("before find_element_with_path\n");
            result_iter = find_element_with_path (&temp, full_file_name);
         }

         if (result_iter != NULL) {
            GtkTreePath *cursor_path = gtk_tree_model_get_path(tree_model, result_iter);

            gtk_tree_view_set_cursor(GTK_TREE_VIEW(projectTreeView), cursor_path, NULL, TRUE);
         }
      }
      gtk_tree_path_free(path);
   }

EXITPOINT:

   if (full_file_name != NULL) {
      g_free(full_file_name);
   }

   return;
}
