/**
 * menus.c - Menus for SciteProj
 *
 *  Copyright 2009-2017 Andreas Rönnquist
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

#include <glib.h>
#include <glib/gi18n.h>

#include <locale.h>


#include <gdk/gdkkeysyms.h>

#include "gui_callbacks.h"

#include "properties_dialog.h"

#include "menus.h"

#include "clicked_node.h"
#include "gui.h"

#include "sort.h"

#include "clipboard.h"
#include "recent_files.h"

GtkWidget *menuBar = NULL;


GtkWidget *fileMenuEntry = NULL;
GtkWidget *editMenuEntry = NULL;
GtkWidget *viewMenuEntry = NULL;
GtkWidget *helpMenuEntry = NULL;

GtkWidget *filePopupMenu = NULL;
GtkWidget *editPopupMenu = NULL;
GtkWidget *viewPopupMenu = NULL;
GtkWidget *helpPopupMenu = NULL;

GtkWidget *fileRightClickPopupMenu = NULL;
GtkWidget *groupRightClickPopupMenu = NULL;
GtkWidget *generalPopupMenu = NULL;

GtkWidget *sortPopupMenu = NULL;

GtkWidget *quitMenuItem = NULL;
GtkWidget *aboutMenuItem = NULL;

GtkWidget *sortMenuItem = NULL;

GtkWidget *sortAscendingMenuItem = NULL;
GtkWidget *sortDescendingMenuItem = NULL;

GtkWidget *sortAscendingExtensionMenuItem = NULL;
GtkWidget *sortDescendingExtensionMenuItem = NULL;


GtkWidget *openFileMenuItem = NULL;
GtkWidget *copyFilenameMenuItem = NULL;

GtkWidget *propertiesMenuItem = NULL;
GtkWidget *propertiesFileMenuItem = NULL;
GtkWidget *propertiesGroupMenuItem = NULL;

GtkWidget *showRecentFileMenuItem = NULL;

GtkWidget *menuSeparator = NULL;
GtkWidget *sortSeparator = NULL;

GtkWidget *fileRightClickSeparator = NULL;

GtkWidget *folderPopupSeparator1 = NULL;
GtkWidget *folderPopupSeparator2 = NULL;

GtkWidget *updateFolderContentMenuItem = NULL;

GtkAccelGroup *accelerator_group = NULL;

GtkWidget *recentPopupMenu = NULL;
GtkWidget *recentMenuOpenFileItem = NULL;
GtkWidget *recentMenuRemoveFileItem = NULL;
GtkWidget *recentMenuCopyFilenameItem = NULL;
GtkWidget *recentMenuPropertiesItem = NULL;
GtkWidget *recentMenuSeparator = NULL;



/**
 *
 */
int init_menus(GtkWidget *window)
{
	menuBar = gtk_menu_bar_new();

	fileMenuEntry = gtk_menu_item_new_with_mnemonic(_("_File"));
	editMenuEntry = gtk_menu_item_new_with_mnemonic(_("_Edit"));
	viewMenuEntry = gtk_menu_item_new_with_mnemonic(_("_View"));
	helpMenuEntry = gtk_menu_item_new_with_mnemonic(_("_Help"));

	filePopupMenu = gtk_menu_new();
	editPopupMenu = gtk_menu_new();
	viewPopupMenu = gtk_menu_new();
	helpPopupMenu = gtk_menu_new();

	menuSeparator = gtk_separator_menu_item_new();

	quitMenuItem = gtk_menu_item_new_with_mnemonic(_("_Quit"));

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(fileMenuEntry), filePopupMenu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(editMenuEntry), editPopupMenu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(viewMenuEntry), viewPopupMenu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(helpMenuEntry), helpPopupMenu);

	gtk_menu_shell_append(GTK_MENU_SHELL(filePopupMenu), quitMenuItem);

	propertiesMenuItem = gtk_menu_item_new_with_mnemonic(_("Edit properties"));

	gtk_menu_shell_append(GTK_MENU_SHELL(editPopupMenu), propertiesMenuItem);

	g_signal_connect(G_OBJECT(propertiesMenuItem), "activate", G_CALLBACK(edit_properties_cb), NULL);

	showRecentFileMenuItem = gtk_menu_item_new_with_mnemonic(_("Show Recently Opened Files"));

	g_signal_connect(G_OBJECT(showRecentFileMenuItem), "activate", G_CALLBACK(recent_files_switch_visible), NULL);
	gtk_widget_add_accelerator(showRecentFileMenuItem, "activate", accelerator_group, GDK_KEY_r, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

	gtk_menu_shell_append(GTK_MENU_SHELL(viewPopupMenu), showRecentFileMenuItem);

	aboutMenuItem = gtk_menu_item_new_with_mnemonic(_("About"));

	g_signal_connect(G_OBJECT(aboutMenuItem), "activate", G_CALLBACK(about_menu_cb), NULL);

	gtk_menu_shell_append(GTK_MENU_SHELL(helpPopupMenu), aboutMenuItem);

	gtk_menu_shell_append(GTK_MENU_SHELL(menuBar), fileMenuEntry);
	gtk_menu_shell_append(GTK_MENU_SHELL(menuBar), editMenuEntry);
	gtk_menu_shell_append(GTK_MENU_SHELL(menuBar), viewMenuEntry);
	gtk_menu_shell_append(GTK_MENU_SHELL(menuBar), helpMenuEntry);

	fileRightClickPopupMenu = gtk_menu_new();

	openFileMenuItem = gtk_menu_item_new_with_mnemonic(_("Open file in SciTE"));
	gtk_menu_shell_append(GTK_MENU_SHELL(fileRightClickPopupMenu), openFileMenuItem);

	copyFilenameMenuItem = gtk_menu_item_new_with_mnemonic(_("Copy filename to clipboard"));
	gtk_menu_shell_append(GTK_MENU_SHELL(fileRightClickPopupMenu), copyFilenameMenuItem);

	fileRightClickSeparator = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(fileRightClickPopupMenu), fileRightClickSeparator);

	propertiesFileMenuItem = gtk_menu_item_new_with_mnemonic(_("Properties"));
	gtk_menu_shell_append(GTK_MENU_SHELL(fileRightClickPopupMenu), propertiesFileMenuItem);

	g_signal_connect(G_OBJECT(openFileMenuItem), "activate", G_CALLBACK(popup_open_file_cb), NULL);
	g_signal_connect(G_OBJECT(copyFilenameMenuItem), "activate", G_CALLBACK(copy_filename_to_clipboard_cb), NULL);
	g_signal_connect(G_OBJECT(propertiesFileMenuItem), "activate", G_CALLBACK(file_properties_cb), NULL);


	sortPopupMenu = gtk_menu_new();

	sortMenuItem = gtk_menu_item_new_with_mnemonic(_("Sort folder contents"));

	sortAscendingMenuItem = gtk_menu_item_new_with_mnemonic(_("Sort ascending by name"));
	sortDescendingMenuItem = gtk_menu_item_new_with_mnemonic(_("Sort descending by name"));

	sortAscendingExtensionMenuItem = gtk_menu_item_new_with_mnemonic(_("Sort ascending by extension"));
	sortDescendingExtensionMenuItem = gtk_menu_item_new_with_mnemonic(_("Sort descending by extension"));
	sortSeparator = gtk_separator_menu_item_new();

	gtk_menu_shell_append(GTK_MENU_SHELL(sortPopupMenu), sortAscendingMenuItem);
	gtk_menu_shell_append(GTK_MENU_SHELL(sortPopupMenu), sortDescendingMenuItem);
	gtk_menu_shell_append(GTK_MENU_SHELL(sortPopupMenu), sortSeparator);
	gtk_menu_shell_append(GTK_MENU_SHELL(sortPopupMenu), sortAscendingExtensionMenuItem);
	gtk_menu_shell_append(GTK_MENU_SHELL(sortPopupMenu), sortDescendingExtensionMenuItem);

	g_signal_connect(G_OBJECT(sortAscendingMenuItem), "activate", G_CALLBACK(sort_ascending_cb), NULL);
	g_signal_connect(G_OBJECT(sortDescendingMenuItem), "activate", G_CALLBACK(sort_descending_cb), NULL);
	g_signal_connect(G_OBJECT(sortAscendingExtensionMenuItem), "activate", G_CALLBACK(sort_ascending_by_extension_cb), NULL);
	g_signal_connect(G_OBJECT(sortDescendingExtensionMenuItem), "activate", G_CALLBACK(sort_descending_by_extension_cb), NULL);

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(sortMenuItem), sortPopupMenu);

	groupRightClickPopupMenu = gtk_menu_new();

	folderPopupSeparator1 = gtk_separator_menu_item_new();
	folderPopupSeparator2 = gtk_separator_menu_item_new();

	propertiesGroupMenuItem = gtk_menu_item_new_with_mnemonic(_("Properties"));

	updateFolderContentMenuItem = gtk_menu_item_new_with_mnemonic(_("Update folder content"));

	gtk_menu_shell_append(GTK_MENU_SHELL(groupRightClickPopupMenu), sortMenuItem);
	gtk_menu_shell_append(GTK_MENU_SHELL(groupRightClickPopupMenu), folderPopupSeparator1);
	gtk_menu_shell_append(GTK_MENU_SHELL(groupRightClickPopupMenu), updateFolderContentMenuItem);
	gtk_menu_shell_append(GTK_MENU_SHELL(groupRightClickPopupMenu), folderPopupSeparator2);
	gtk_menu_shell_append(GTK_MENU_SHELL(groupRightClickPopupMenu), propertiesGroupMenuItem);

	g_signal_connect(G_OBJECT(propertiesGroupMenuItem), "activate", G_CALLBACK(group_properties_cb), NULL);

	g_signal_connect(G_OBJECT(updateFolderContentMenuItem), "activate", G_CALLBACK(refresh_folder_cb), NULL);

	gtk_widget_show_all(groupRightClickPopupMenu);

	gtk_widget_show_all(fileRightClickPopupMenu);

	gtk_widget_show_all(GTK_WIDGET(menuBar));

	gtk_widget_add_accelerator(quitMenuItem, "activate", accelerator_group, GDK_KEY_q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

	g_signal_connect(G_OBJECT(quitMenuItem), "activate", G_CALLBACK(quit_menu_cb), NULL);

	recentPopupMenu = gtk_menu_new();

	recentMenuOpenFileItem = gtk_menu_item_new_with_mnemonic(_("Open file in SciTE"));
	recentMenuRemoveFileItem = gtk_menu_item_new_with_mnemonic(_("Remove file from this list"));
	recentMenuCopyFilenameItem = gtk_menu_item_new_with_mnemonic(_("Copy filename to clipboard"));
	recentMenuPropertiesItem = gtk_menu_item_new_with_mnemonic(_("Properties"));

	recentMenuSeparator = gtk_separator_menu_item_new();

	g_signal_connect(G_OBJECT(recentMenuOpenFileItem), "activate", G_CALLBACK(popup_open_recent_file_cb), NULL);
	g_signal_connect(G_OBJECT(recentMenuRemoveFileItem), "activate", G_CALLBACK(popup_remove_recent_file_cb), NULL);
	g_signal_connect(G_OBJECT(recentMenuCopyFilenameItem), "activate", G_CALLBACK(copy_recent_filename_to_clipboard_cb), NULL);
	g_signal_connect(G_OBJECT(recentMenuPropertiesItem), "activate", G_CALLBACK(properties_recent_file_cb), NULL);

	gtk_menu_shell_append(GTK_MENU_SHELL(recentPopupMenu),recentMenuOpenFileItem);
	gtk_menu_shell_append(GTK_MENU_SHELL(recentPopupMenu),recentMenuRemoveFileItem);
	gtk_menu_shell_append(GTK_MENU_SHELL(recentPopupMenu),recentMenuCopyFilenameItem);
	gtk_menu_shell_append(GTK_MENU_SHELL(recentPopupMenu),recentMenuSeparator);
	gtk_menu_shell_append(GTK_MENU_SHELL(recentPopupMenu),recentMenuPropertiesItem);

	gtk_widget_show_all(GTK_WIDGET(recentPopupMenu));

	return 0;
}
