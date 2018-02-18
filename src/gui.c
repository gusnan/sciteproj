/**
 * gui.c - GUI code for SciteProj
 *
 *  Copyright 2006 Roy Wood, 2009-2017 Andreas Rönnquist
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
#include "scite_utils.h"
#include "string_utils.h"
#include "prefs.h"
#include "statusbar.h"
#include "graphics.h"
#include "about.h"
#include "properties_dialog.h"
#include "file_utils.h"

#include "clipboard.h"

#include "remove.h"

#include "recent_files.h"

#include "gui_callbacks.h"
#include "sort.h"

#include "menus.h"

#include "gtk3_compat.h"





// Forward-declare static functions

static gint window_delete_event_cb(GtkWidget *widget, GdkEvent *event, gpointer data);
static void tree_row_activated_cb(GtkTreeView *treeView, GtkTreePath *path,
                                  GtkTreeViewColumn *column, gpointer userData);
static gboolean mouse_button_pressed_cb(GtkWidget *treeView, GdkEventButton *event, gpointer userData);

//gboolean dialog_response_is_exit(gint test);

void recent_files_switch_visible();

gboolean tree_view_search_equal_func(GtkTreeModel *model, gint column, const gchar *key,
                                     GtkTreeIter *iter, gpointer search_data);

gboolean is_name_valid(gchar *instring);

gboolean key_press_cb(GtkWidget *widget, GdkEventKey *event, gpointer userData);

gchar *window_saved_title = NULL;

static GtkWidget *sMainWindow = NULL;
GtkWidget *projectTreeView = NULL;

ClickedNode clicked_node;

GtkCellRenderer *textCellRenderer = NULL;
GtkCellRenderer *pixbuffCellRenderer = NULL;

GtkWidget *scrolledWindow = NULL;

GtkTreeViewColumn *column1 = NULL;

#if GTK_MAJOR_VERSION>=3
GtkWidget *recentGrid = NULL;
#else
GtkWidget *recentVbox=NULL;
#endif

GtkWidget *recentHbox = NULL;


/**
 * Initialize globals (i.e. create the main window and populate it).  This is a long chunk of code.
 *
 * @return TRUE on success, FALSE on failure (further details returned in err)
 *
 * @param err returns any errors
 */
gboolean setup_gui(GError **err)
{
	gboolean resultCode = FALSE;
	GtkTreeSelection *selection = NULL;
	GtkWidget *vpaned = NULL;

	GtkTreeStore *projectTreeStore = NULL;
	//GtkAccelGroup* accelgroup = NULL;
	GError *tempErr = NULL;

#if GTK_MAJOR_VERSION >= 3
	GtkWidget *grid;
	GtkWidget *fullGrid = NULL;
#else
	GtkWidget *vbox=NULL;
	GtkWidget *hbox=NULL;

	GtkWidget *statusBarVbox=NULL;
	GtkWidget *fullVbox=NULL;
#endif

	GtkWidget *recentScrolledWindow = NULL;


	clicked_node.valid = FALSE;
	clicked_node.name = NULL;
	clicked_node.type = -1;

	window_saved_title=g_strdup_printf(_("[UNTITLED]"));

	// Create top-level window, configure it

	if (!(sMainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL))) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: %s, gtk_window_new() = NULL",
		            __func__,
		            "Couldn't init main window"
		           );

		goto EXITPOINT;
	}

	// TODO: call these in load_graphics instead

	if (!(load_graphics(sMainWindow, err))) {
		goto EXITPOINT;
	}

	gtk_window_set_icon(GTK_WINDOW(sMainWindow), program_icon_pixbuf);
	gtk_window_set_default_icon(program_icon_pixbuf);

	gtk_window_set_title(GTK_WINDOW(sMainWindow), window_saved_title);

	gtk_container_set_border_width(GTK_CONTAINER(sMainWindow), 0);	//3
	g_signal_connect(G_OBJECT(sMainWindow), "delete_event", G_CALLBACK(window_delete_event_cb), NULL);

	// Main content of the window is a vpaned

#if GTK_MAJOR_VERSION>=3
	vpaned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);

	// Then we need a grid
	grid = gtk_grid_new();

#else
	vpaned=gtk_vpaned_new();

	// Then we need a vbox

	if (!(vbox = gtk_vbox_new(FALSE, 0))) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: %s, gtk_vbox_new() = NULL",
		            __func__,
		            "Couldn't init main vbox"
		           );
		goto EXITPOINT;
	}

	//gtk_container_add(GTK_CONTAINER(sMainWindow), vbox);

#endif


	// Create menus

	/*
	if (!(sActionGroup = gtk_action_group_new("SciteProjActions"))) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: %s, gtk_action_group_new() = NULL",
			__func__,
			"Couldn't init gtk_action_group"
		);

		goto EXITPOINT;
	}
	*/

	/*
	if (!(sGtkUIManager = gtk_ui_manager_new())) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: %s, gtk_ui_manager_new() = NULL",
			__func__,
			"Couldn't init gtk_ui_manager"
		);

		goto EXITPOINT;
	}
	*/



#if GTK_MAJOR_VERSION >= 3
	//g_signal_connect(sGtkUIManager, "add_widget", G_CALLBACK(menu_add_widget_cb), grid);
#else
	//g_signal_connect(sGtkUIManager, "add_widget", G_CALLBACK(menu_add_widget_cb), vpaned);
	// g_signal_connect(sGtkUIManager, "add_widget", G_CALLBACK(menu_add_widget_cb), vbox);
#endif

	// Fix the context-based translations for the menu strings
	/*
	int co=0;
	gchar *temp=NULL;
	gchar *context=NULL;
	do {
		context=menustrings[co].context;
		if (context!=NULL) {
			temp = (gchar*)g_dpgettext2(PACKAGE,menustrings[co].context,menustrings[co].string);

			if (temp!=NULL) {
				sMenuActions[co].label = g_strdup_printf("%s",temp);
				++co;
			}
		}
	} while (context!=NULL);
	*/

	/*
		gtk_action_group_set_translation_domain(sActionGroup,PACKAGE);

		gtk_action_group_add_actions(sActionGroup, sMenuActions, sNumMenuActions, NULL);

		gtk_ui_manager_insert_action_group(sGtkUIManager, sActionGroup, 0);

		if (gtk_ui_manager_add_ui_from_string(sGtkUIManager, sMenuDefXML, strlen(sMenuDefXML), &tempErr) == 0) {
			g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: %s, gtk_ui_manager_add_ui_from_string() = %s",
				__func__,
				"Couldn't init menus from XML",
				tempErr->message
			);

			goto EXITPOINT;
		}

		gtk_ui_manager_ensure_update(sGtkUIManager);
	*/
	// Activate the keyboard accelerators

	accelerator_group = gtk_accel_group_new();
	gtk_window_add_accel_group(GTK_WINDOW(sMainWindow), accelerator_group);

	/*
		// Create popup menus (shown when user right-clicks in gui elements)

		sGeneralPopupMenu = gtk_ui_manager_get_widget(sGtkUIManager, "/ui/GeneralPopup");
		sGroupPopupMenu = gtk_ui_manager_get_widget(sGtkUIManager, "/ui/GroupPopup");

		//sFilePopupMenu = gtk_ui_manager_get_widget(sGtkUIManager, "/ui/FilePopup");
	*/
	//sFilePopupMenu = gtk_menu_new();

	if (init_menus(sMainWindow)!=0) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: %s, gtk_scrolled_window_new() = NULL",
		            __func__,
		            "Couldn't init menus"
		           );
		goto EXITPOINT;
	}

	/*
		recentPopupMenu = gtk_ui_manager_get_widget(sGtkUIManager, "/ui/RecentPopup");

		sSortPopupMenu = gtk_ui_manager_get_widget(sGtkUIManager, "/ui/SortPopup");
	*/

	// Add a scrolled window to the main window

	if (!(scrolledWindow = gtk_scrolled_window_new(NULL, NULL))) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: %s, gtk_scrolled_window_new() = NULL",
		            __func__,
		            "Couldn't init main scrolled window"
		           );

		goto EXITPOINT;
	}

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledWindow),
	                               GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

#if GTK_MAJOR_VERSION>=3
	gtk_grid_attach(GTK_GRID(grid), scrolledWindow, 0, 1, 1, 1);

	gtk_widget_set_vexpand(scrolledWindow, TRUE);
	gtk_widget_set_hexpand(scrolledWindow, TRUE);
#else
	gtk_box_pack_start(GTK_BOX(vbox), scrolledWindow, TRUE, TRUE, 0);

	hbox=gtk_hbox_new(FALSE,0);

	gtk_widget_show(hbox);

	gtk_box_pack_end(GTK_BOX(vbox),hbox,FALSE,TRUE,0);
#endif

	// Create the tree datastore

	if ((projectTreeStore = create_treestore(&tempErr)) == NULL) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: %s",
		            tempErr->message,
		            "Couldn't init treestore"
		           );
		goto EXITPOINT;
	}

	// Create the treeview, set it up to render the tree datastore, and add it to the hbox

	if (!(projectTreeView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(projectTreeStore)))) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: %s, gtk_tree_view_new_with_model() = NULL",
		            __func__,
		            "Couldn't init gtk_tree_view"
		           );

		goto EXITPOINT;
	}

	//g_object_unref(G_OBJECT(projectTreeStore));

	gtk_tree_view_set_enable_search(GTK_TREE_VIEW(projectTreeView), TRUE);

	if (!(textCellRenderer = gtk_cell_renderer_text_new())) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: %s, gtk_cell_renderer_text_new() = NULL",
		            __func__,
		            "Couldn't init cell renderer"
		           );

		goto EXITPOINT;
	}

	g_object_set(G_OBJECT(textCellRenderer),
	             "editable", FALSE,
	             "mode", GTK_CELL_RENDERER_MODE_EDITABLE,
	             NULL);

	if (!(pixbuffCellRenderer = gtk_cell_renderer_pixbuf_new())) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1,
		            "%s: %s, gtk_cell_renderer_pixbuf_new() = NULL",
		            __func__,
		            "Couldn't init gtk_cell_renderer_pixbuf"
		           );

		goto EXITPOINT;
	}

	if (!(column1 = gtk_tree_view_column_new())) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1,
		            "%s: %s, gtk_tree_view_column_new() = NULL",
		            __func__,
		            "Couldn't init gtk_tree_view_column"
		           );

		goto EXITPOINT;
	}

	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(projectTreeView), FALSE);

	gtk_tree_view_column_set_resizable(column1, TRUE);
	gtk_tree_view_column_set_min_width(column1, (int)(prefs.width * .75));


	gtk_tree_view_column_pack_start(column1, pixbuffCellRenderer, FALSE);
	gtk_tree_view_column_add_attribute(column1, pixbuffCellRenderer, "pixbuf", COLUMN_ICON);


	gtk_tree_view_column_pack_start(column1, textCellRenderer, TRUE);
	gtk_tree_view_column_add_attribute(column1, textCellRenderer, "text", COLUMN_FILENAME);
	gtk_tree_view_column_add_attribute(column1, textCellRenderer, "weight", COLUMN_FONTWEIGHT);
	gtk_tree_view_column_add_attribute(column1, textCellRenderer, "weight-set", COLUMN_FONTWEIGHTSET);

	//g_signal_connect(G_OBJECT(textCellRenderer), "edited", G_CALLBACK(rename_cb), NULL);

	gtk_tree_view_append_column(GTK_TREE_VIEW(projectTreeView), column1);

	// Stoopid gtk always expands the last column

	gtk_container_add(GTK_CONTAINER(scrolledWindow), projectTreeView);

	// Get tree events

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(projectTreeView));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

	g_signal_connect(G_OBJECT(projectTreeView), "row-activated",
	                 G_CALLBACK(tree_row_activated_cb), NULL);

	gtk_tree_view_set_search_equal_func(GTK_TREE_VIEW(projectTreeView),
	                                    tree_view_search_equal_func, NULL, NULL);

	g_signal_connect(G_OBJECT(projectTreeView), "row-expanded",
	                 G_CALLBACK(row_expand_or_collapse_cb), NULL);

	g_signal_connect(G_OBJECT(projectTreeView), "row-collapsed",
	                 G_CALLBACK(row_expand_or_collapse_cb), NULL);


	g_signal_connect(G_OBJECT(projectTreeView), "button-press-event",
	                 G_CALLBACK(mouse_button_pressed_cb), projectTreeView);

	g_signal_connect(G_OBJECT(projectTreeView), "key-press-event",
	                 G_CALLBACK(key_press_cb), projectTreeView);

	// --------------------------------
	// Recent file stuff:

#if GTK_MAJOR_VERSION>=3

	if (!(recentGrid = gtk_grid_new())) {
		g_set_error(err, APP_SCITEPROJ_ERROR,-1,
		            "%s: %s, gtk_grid_new() = NULL",
		            "Couldn't init recent grid",
		            __func__);
		goto EXITPOINT;
	}

#else
	if (!(recentVbox=gtk_vbox_new(FALSE, 0))) {
		g_set_error(err, APP_SCITEPROJ_ERROR,-1,
		            "%s: %s, gtk_vbox_new() = NULL",
		            __func__,
		            "Couldn't init recent grid"
		           );
		goto EXITPOINT;
	}
#endif

#if GTK_MAJOR_VERSION>=3
	if (!(recentHbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0))) {
#else
	if (!(recentHbox=gtk_hbox_new(FALSE,0))) {
#endif
		g_set_error(err, APP_SCITEPROJ_ERROR, -1,
		            "%s: %s, gtk_hbox_new() = NULL",
		            __func__,
		            "Couldn't init main vbox"
		           );
		goto EXITPOINT;
	}

	if ((recentScrolledWindow = init_recent_files(&tempErr)) == NULL) {

		goto EXITPOINT;
	}


#if GTK_MAJOR_VERSION >= 3
	gtk_widget_set_vexpand(recentScrolledWindow, TRUE);
	gtk_widget_set_hexpand(recentScrolledWindow, TRUE);

	gtk_grid_attach(GTK_GRID(recentGrid), recentScrolledWindow, 0, 0, 1, 1);

	fullGrid = gtk_grid_new();

#else

	if (!(fullVbox=gtk_vbox_new(FALSE,0))) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1,
		            "%s: %s, gtk_hbox_new() = NULL",
		            __func__,
		            "Couldn't init full_vbox"
		           );

		goto EXITPOINT;
	}

	gtk_box_pack_start(GTK_BOX(recentVbox),recentScrolledWindow, TRUE,TRUE,0);
	gtk_box_pack_end(GTK_BOX(recentVbox),recentHbox,FALSE,TRUE,0);


	statusBarVbox=gtk_vbox_new(FALSE,0);

	if (!statusBarVbox) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1,
		            "%s: %s, gtk_hbox_new() = NULL",
		            __func__,
		            "Couldn't init statusbar_vbox"
		           );
		goto EXITPOINT;
	}
#endif


#if GTK_MAJOR_VERSION >= 3
	gtk_paned_pack1(GTK_PANED(vpaned), grid, TRUE, FALSE);
	gtk_paned_pack2(GTK_PANED(vpaned), recentGrid, TRUE, TRUE);
#else
	gtk_paned_pack1(GTK_PANED(vpaned),vbox,TRUE,FALSE);
	gtk_paned_pack2(GTK_PANED(vpaned),recentVbox,TRUE,TRUE);
#endif

	gtk_widget_show(vpaned);


#if GTK_MAJOR_VERSION >= 3


	gtk_grid_insert_row(GTK_GRID(fullGrid), 0);
	gtk_grid_attach(GTK_GRID(fullGrid), vpaned, 0, 0, 1, 1);

	gtk_grid_insert_row(GTK_GRID(fullGrid), 0);
	gtk_grid_attach(GTK_GRID(fullGrid), menuBar, 0, 0, 1, 1);

	gtk_widget_show(menuBar);

	gtk_widget_show(GTK_WIDGET(fullGrid));

	if (!prefs.hide_statusbar) {
		if (!init_statusbar(fullGrid, vpaned, &tempErr)) {
			g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: %s", tempErr->message, "Error initing statusbar");
			goto EXITPOINT;
		}
	}

	gtk_container_add(GTK_CONTAINER(sMainWindow), fullGrid);


#else

	gtk_box_pack_start(GTK_BOX(fullVbox),menuBar, FALSE, FALSE, 0);

	gtk_widget_show(menuBar);
	gtk_box_pack_start(GTK_BOX(fullVbox),vpaned,TRUE,TRUE,0);

	gtk_widget_show(GTK_WIDGET(fullVbox));


	if (!prefs.hide_statusbar) {
		if (!init_statusbar(fullVbox,vpaned,&tempErr)) {
			g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: %s",
			            tempErr->message,
			            "Couldn't init statusbar"
			           );
			goto EXITPOINT;
		}
	}

	gtk_container_add(GTK_CONTAINER(sMainWindow), fullVbox);

	gtk_widget_show(fullVbox);
	gtk_widget_show(vpaned);

#endif
	g_signal_connect(G_OBJECT(recentTreeView), "key-press-event", G_CALLBACK(key_press_cb), recentTreeView);

	gtk_window_resize(GTK_WINDOW(sMainWindow), prefs.width, prefs.height);
	gtk_window_move(GTK_WINDOW(sMainWindow), prefs.xpos, prefs.ypos);

	int window_xsize, window_ysize;
	gtk_window_get_size(GTK_WINDOW(sMainWindow), &window_xsize, &window_ysize);

	gtk_paned_set_position(GTK_PANED(vpaned), (int)(window_ysize*0.75));

	// Show it all....

#if GTK_MAJOR_VERSION >= 3
	gtk_widget_show(recentGrid);

	if (!prefs.show_recent) {
		gtk_widget_hide(recentGrid);
	} else {
		gtk_widget_show(recentGrid);
	}
#else
	gtk_widget_show(recentVbox);

	if (!prefs.show_recent) {
		gtk_widget_hide(recentVbox);
	} else {
		gtk_widget_show(recentVbox);
	}
#endif

	gtk_widget_show(projectTreeView);
	gtk_widget_show(scrolledWindow);
#if GTK_MAJOR_VERSION >= 3
	gtk_widget_show(grid);
#else
	gtk_widget_show(vbox);
#endif
	gtk_widget_show(sMainWindow);

	resultCode = TRUE;

EXITPOINT:

	if (tempErr) g_error_free(tempErr);

	return resultCode;
}


/**
 *
 */
void gui_close()
{
	if (window_saved_title) g_free(window_saved_title);

	if (scrolledWindow) gtk_widget_destroy(scrolledWindow);

	unload_graphics();

	done_statusbar();

	if (sMainWindow) gtk_widget_destroy(sMainWindow);
}



/**
 * Determine whether a specified row in the tree is expanded.
 *
 * @return TRUE if the row is expanded; FALSE otherwise
 *
 * @param path is the GtkTreePath referencing the row
 */
gboolean tree_row_is_expanded(GtkTreePath *path)
{
	g_assert(path != NULL);

	return gtk_tree_view_row_expanded(GTK_TREE_VIEW(projectTreeView), path);
}



/**
 * Expand a row in the tree.
 *
 * @param path is the GtkTreePath referencing the row
 * @param expandChildren indicates whether all children should be expanded
 */
void expand_tree_row(GtkTreePath *path, gboolean expandChildren)
{
	if (path!=NULL) {
		gtk_tree_view_expand_row(GTK_TREE_VIEW(projectTreeView), path, FALSE);
	}
}



/**
 * Callback for Gtk "delete_event" message for the top-level application window.
 *
 * @param widget is not used
 * @param event is not used
 * @param data is not used
 */
static gint window_delete_event_cb(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	gboolean eventHandled = TRUE;

	gtk_main_quit();

	return eventHandled;
}


/**
 *
 */
static void switch_folder_icon(GtkTreeView *treeView, GtkTreePath *path)
{
	GtkTreeIter iter;
	gint nodeItemType;

	gchar *relFilePath = NULL;

	GtkTreeModel *treeModel = gtk_tree_view_get_model(treeView);
	GdkPixbuf *pixbuf;

	gtk_tree_model_get_iter(treeModel, &iter, path);
	gtk_tree_model_get(treeModel, &iter, COLUMN_ITEMTYPE, &nodeItemType,
	                   COLUMN_FILEPATH, &relFilePath,
	                   COLUMN_ICON, &pixbuf,
	                   -1);

	gboolean res=gtk_tree_view_row_expanded(treeView, path);

	if (res) {
		gtk_tree_view_collapse_row(treeView, path);
	} else {

		gtk_tree_view_expand_row(treeView, path, FALSE);
	}

	g_free(relFilePath);
}


/**
 * Callback handler for Gtk "row-activated" event.
 *
 * @param treeView is the GtkTreeView
 * @param path is the GtkTreePath of the activated row
 * @param column is not used
 * @param userData is not used
 */
static void tree_row_activated_cb(GtkTreeView *treeView,
                                  GtkTreePath *path,
                                  GtkTreeViewColumn *column,
                                  gpointer userData)
{
	GtkTreeIter iter;
	gchar *relFilePath = NULL;
	gchar *absFilePath = NULL;
	gchar *command = NULL;
	GError *err = NULL;
	GtkWidget *dialog = NULL;
	gint nodeItemType;
	gchar *fixed = NULL;


	// Get the data from the row that was activated

	GtkTreeModel *treeModel = gtk_tree_view_get_model(treeView);
	gtk_tree_model_get_iter(treeModel, &iter, path);
	gtk_tree_model_get(treeModel, &iter, COLUMN_ITEMTYPE, &nodeItemType, COLUMN_FILEPATH, &relFilePath, -1);


	// We can only open files

	if (nodeItemType != ITEMTYPE_FILE) {
		switch_folder_icon(treeView,path);
		goto EXITPOINT;
	}

	absFilePath = fix_path((gchar*)get_project_directory(), relFilePath);

	fixed = fix_path((gchar*)get_project_directory(), relFilePath);

	if ((command = g_strdup_printf("open:%s\n", fixed)) == NULL) {
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

			add_file_to_recent(fixed,NULL);

			gchar *statusbar_text = g_strdup_printf(_("Opened %s"),
			                                        remove_newline(get_filename_from_full_path(command)));

			set_statusbar_text(statusbar_text);

			g_free(statusbar_text);
		}
	}

EXITPOINT:

	if (err != NULL) {
		dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
		                                _("Could not open selected file: \n\n%s"), err->message);

		gtk_dialog_run(GTK_DIALOG (dialog));
	}

	if (relFilePath) g_free(relFilePath);
	if (absFilePath) g_free(absFilePath);
	if (command) g_free(command);
	if (err) g_error_free(err);
	if (dialog) gtk_widget_destroy(dialog);
	if (fixed) g_free(fixed);
}



/**
 * Respond to a Gtk "button-press-event" message.
 *
 * @param treeView is the GTKTreeView widget in which the mouse-button event occurred
 * @param event is the GdkEventButton event object
 * @param userData is not currently used
 */
static gboolean mouse_button_pressed_cb(GtkWidget *treeView, GdkEventButton *event, gpointer userData)
{
	gboolean eventHandled = FALSE;
	GtkTreePath *path = NULL;
	GtkTreeModel *treeModel = NULL;
	gchar *nodeName = NULL;
	gint nodeItemType;
	GtkTreeIter iter;
	GtkTreeSelection *tree_selection = NULL;

	g_assert(treeView != NULL);
	g_assert(event != NULL);

	// Until we know for sure, assume that the user has not clicked on a node

	clicked_node.valid=FALSE;


	// If it is not a right-click, then ignore it

	if (event->type != GDK_BUTTON_PRESS || event->button != 3) {
		goto EXITPOINT;
	}

	// Find if the user has clicked on a node

	if (!gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeView),
	                                   (gint) event->x, (gint) event->y,
	                                   &path, NULL, NULL, NULL)) {
		// Nope-- user clicked in the GtkTreeView, but not on a node

#if ((GTK_MAJOR_VERSION >= 3) && (GTK_MINOR_VERSION >= 22))
		if (generalPopupMenu) {
			gtk_menu_popup_at_pointer(GTK_MENU(generalPopupMenu), (GdkEvent*)event);													
		}
#else
		if (generalPopupMenu) {
			gtk_menu_popup(GTK_MENU(generalPopupMenu),
		               NULL, NULL, NULL, NULL,
		               event->button, gdk_event_get_time((GdkEvent*) event));
		}
#endif
													  
		goto EXITPOINT;
	}

	// User clicked on a node, so retrieve the particulars

	treeModel = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));

	if (!gtk_tree_model_get_iter(treeModel, &iter, path)) {
		goto EXITPOINT;
	}

	gtk_tree_model_get(treeModel, &iter, COLUMN_ITEMTYPE, &nodeItemType, COLUMN_FILEPATH, &nodeName, -1);


	// Save the node info for use by the popup menu callbacks

	if (clicked_node.name) g_free(clicked_node.name);

	clicked_node.valid = TRUE;
	clicked_node.iter = iter;
	clicked_node.type = nodeItemType;
	clicked_node.name = nodeName;
	nodeName = NULL;

	// Check if something is selected
	tree_selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeView));

	if (tree_selection!=NULL) {
		// Check if clicked on something in the selection, otherwise make the clicked one the selection.

		if (gtk_tree_selection_path_is_selected(tree_selection, path) == FALSE) {
			// clear selection and make current line selected

			gtk_tree_selection_unselect_all(tree_selection);

			gtk_tree_selection_select_path(tree_selection, path);
		}
	}

	// Pop up the appropriate menu for the node type

	if (nodeItemType == ITEMTYPE_FILE) {
#if ((GTK_MAJOR_VERSION >= 3) && (GTK_MINOR_VERSION >= 22))
		if (fileRightClickPopupMenu) {
			gtk_menu_popup_at_pointer(GTK_MENU(fileRightClickPopupMenu), (GdkEvent*)event);
		}
#else
		if (fileRightClickPopupMenu) {
			gtk_menu_popup(GTK_MENU(fileRightClickPopupMenu), NULL, NULL, NULL, NULL,
						event->button, gdk_event_get_time((GdkEvent*) event));
		}
#endif
	}
	else if (nodeItemType == ITEMTYPE_GROUP) {
#if ((GTK_MAJOR_VERSION >= 3) && (GTK_MINOR_VERSION >= 22))
		if (groupRightClickPopupMenu) {
			gtk_menu_popup_at_pointer(GTK_MENU(groupRightClickPopupMenu), (GdkEvent*)event);
		}
#else
		if (groupRightClickPopupMenu) {
			gtk_menu_popup(GTK_MENU(groupRightClickPopupMenu), NULL, NULL, NULL, NULL,
		               event->button, gdk_event_get_time((GdkEvent*) event));
		}
#endif
	}

	// We took care of the event, so no need to propogate it

	eventHandled = TRUE;


EXITPOINT:

	if (path) gtk_tree_path_free(path);
	if (nodeName) g_free(nodeName);

	return eventHandled;
}




/**
 * Set the title of the main window.
 *
 * @param newName is the desired new name of the window.
 */
void set_window_title(const gchar *newName)
{
	g_assert(newName != NULL);

	gchar *temp_string = g_new(gchar, 512);
	g_snprintf(temp_string,512, "%s", newName);

	gtk_window_set_title(GTK_WINDOW(sMainWindow), temp_string);

	g_free(window_saved_title);

	window_saved_title = g_strdup_printf("%s", newName);

	g_free(temp_string);
}


/**
 *
 */
gboolean dialog_response_is_exit(gint test)
{
	gboolean result = FALSE;

	if ((test == GTK_RESPONSE_REJECT) || (test == GTK_RESPONSE_CANCEL) ||
	        (test == GTK_RESPONSE_DELETE_EVENT) || (test == GTK_RESPONSE_NONE)) {
		result = TRUE;
	}

	return result;
}


/**
 *
 */
void recent_files_switch_visible()
{
	gboolean visible = FALSE;

#if GTK_MAJOR_VERSION >= 3
	g_object_get(G_OBJECT(recentGrid), "visible", &visible, NULL);

	if (visible) {
		gtk_widget_hide(recentGrid);
		gtk_widget_grab_focus(projectTreeView);
	} else {
		gtk_widget_show(recentGrid);
	}
#else
	g_object_get(G_OBJECT(recentVbox),"visible", &visible,NULL);

	if (visible) {
		gtk_widget_hide(recentVbox);
		gtk_widget_grab_focus(projectTreeView);
	} else {
		gtk_widget_show(recentVbox);
	}
#endif

}


/**
 *
 */
void set_dialog_transient(GtkWidget *dialog)
{
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(sMainWindow));
}