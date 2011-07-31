/**
 * gui.c - GUI code for SciteProj
 *
 *  Copyright 2006 Roy Wood, 2009-2011 Andreas Rönnquist
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

#include "clicked_node.h"

#include "gui.h"
#include "drag_drop.h"
#include "tree_manipulation.h"
#include "scite_utils.h"
#include "string_utils.h"
#include "prefs.h"
#include "statusbar.h"
#include "graphics.h"
#include "about.h"
#include "properties_dialog.h"
#include "file_utils.h"

#include "search.h"
#include "clipboard.h"

#include "rename.h"
#include "remove.h"
#include "addfiles.h"

#include "recent_files.h"
#include "filelist.h"

#include "menus.h"

#include "search.h"


#define APP_SCITEPROJ_ERROR g_quark_from_static_string("APP_GUI_ERROR")


// Forward-declare static functions

//gboolean dialog_response_is_exit(gint test);

static gint window_delete_event_cb(GtkWidget *widget, GdkEvent *event, gpointer data);
static void tree_row_activated_cb(GtkTreeView *treeView, GtkTreePath *path, GtkTreeViewColumn *column, gpointer userData);
static gboolean mouse_button_pressed_cb(GtkWidget *treeView, GdkEventButton *event, gpointer userData);

//static void ask_name_add_group(GtkTreeIter *nodeIter);

static void row_expand_or_collapse_cb(GtkTreeView *treeview, GtkTreeIter *arg1, GtkTreePath *arg2, gpointer user_data);

//static void menu_add_widget_cb(GtkUIManager *ui, GtkWidget *widget, GtkContainer *container);

static void quit_menu_cb();
static void about_menu_cb();
static void saveproject_menu_cb();
static void saveproject_as_menu_cb();
static void openproject_menu_cb();
//static void addfile_menu_cb();
static void creategroup_menu_cb();

static void popup_open_file_cb();

static void expand_all_items_cb();
static void collapse_all_items_cb();

static void sort_ascending_cb();
static void sort_descending_cb();

void recent_files_switch_visible();

gboolean tree_view_search_equal_func(GtkTreeModel *model,gint column,const gchar *key,GtkTreeIter *iter,gpointer search_data);

static void edit_options_cb();

gboolean is_name_valid(gchar *instring);

gboolean key_press_cb(GtkWidget *widget, GdkEventKey *event, gpointer userData);

gchar *window_saved_title=NULL;


static GtkActionEntry sMenuActions[] = 
{
	{ "FileMenuAction", NULL, "_File" },
	{ "EditMenuAction", NULL, "_Edit" },
	{ "ViewMenuAction", NULL, "_View" },
	{ "HelpMenuAction", NULL, "_Help" },
	
	{ "OpenProjectAction", GTK_STOCK_OPEN, "_Open Project", "<control>O", "Open Project", G_CALLBACK(openproject_menu_cb) },
	{ "SaveProjectAction", GTK_STOCK_SAVE, "_Save Project", "<control>S", "Save Project", G_CALLBACK(saveproject_menu_cb) },
	{ "SaveProjectAsAction", GTK_STOCK_SAVE_AS, "Save Project As...", "<control><shift>S", "Save Proeject As", G_CALLBACK(saveproject_as_menu_cb) },
	{ "ExitAction", GTK_STOCK_QUIT, "_Exit", "<control>Q", "Exit", G_CALLBACK(quit_menu_cb) },
	
	{ "CreateGroupAction", GTK_STOCK_DIRECTORY, "Create _Group", "", "Create a group node in the project", G_CALLBACK(creategroup_menu_cb) },
	{ "AddFileAction", GTK_STOCK_FILE, "Add _File", "", "Add a file to the project", G_CALLBACK(addfile_menu_cb) },
	{ "RemoveFileAction", GTK_STOCK_DELETE, "Remove File(s)", "", "Remove selected files from the project", G_CALLBACK(removeitem_menu_cb) },
	
	{ "ExpandAllGroupsAction", NULL, "Expand All Groups", "<control><shift>E", "Expand All Groups", G_CALLBACK(expand_all_items_cb) },
	{ "CollapseAllGroupsAction", NULL, "Collapse All Groups", "<control><shift>C", "Collapse All Groups", G_CALLBACK(collapse_all_items_cb) },
	
	{ "SearchAction", GTK_STOCK_FIND, "Search", "<control>F", "Search for a string in the project", G_CALLBACK(search_dialog_cb) },
	
	{ "AboutAction", GTK_STOCK_ABOUT, "_About", "", "Show information about this application", G_CALLBACK(about_menu_cb) },
	
	{ "AddFilesPopupAction", GTK_STOCK_FILE, "Add Files", "", "Add files to the project", G_CALLBACK(popup_add_files_cb) },
	{ "AddGroupPopupAction", GTK_STOCK_DIRECTORY, "Add Group", "", "Add a group to the project", G_CALLBACK(popup_add_group_cb) },
	
	{ "AddFilestoGroupPopupAction", GTK_STOCK_FILE, "Add Files to Group", "", "Add files to an existing group", G_CALLBACK(popup_add_files_cb) },
	{ "AddSubgroupPopupAction", GTK_STOCK_DIRECTORY, "Add Subgroup to Group", "", "Add a subgroup to an existing group", G_CALLBACK(popup_add_group_cb) },
	{ "RenameGroupPopupAction", GTK_STOCK_EDIT, "Rename Group", "", "Rename a group", G_CALLBACK(popup_rename_group_cb) },
	{ "RemoveGroupPopupAction", GTK_STOCK_DELETE, "Remove Group From Project", "", "Remove a group and its children from the project", G_CALLBACK(popup_remove_node_cb) },
	{ "SortAscendingAction", GTK_STOCK_SORT_ASCENDING, "Sort Group Ascending","","Sort the filenames ascending",G_CALLBACK(sort_ascending_cb) },
	{ "SortDescendingAction", GTK_STOCK_SORT_DESCENDING, "Sort Group Descending","","Sort the filenames descending",G_CALLBACK(sort_descending_cb) },
	{ "PropertiesGroupPopupAction", GTK_STOCK_PROPERTIES, "Group Properties", "", "Show group properties", G_CALLBACK(group_properties_cb) },
	{ "EditOptionsAction", GTK_STOCK_PROPERTIES, "Edit Options", "", "Edit Program Options", G_CALLBACK(edit_options_cb) },
	
	{ "ViewRecentAction" , GTK_STOCK_PROPERTIES, "View Recently Opened Files", "<control>R", "View Recent Files", G_CALLBACK(recent_files_switch_visible) },
	
	{ "OpenFilePopupAction", GTK_STOCK_OPEN, "Open File in SciTE", "", "Open a file in SciTE", G_CALLBACK(popup_open_file_cb) },
	{ "RemoveFilePopupAction", GTK_STOCK_DELETE, "Remove File From Project", "", "Remove a file from the project", G_CALLBACK(popup_remove_node_cb) },
	{ "CopyFilenameToClipBoardAction", GTK_STOCK_COPY, "Copy Filename to Clipboard", "", "Copies the full path and filename to the clipboard", G_CALLBACK(copy_filename_to_clipboard_cb) },
	{ "PropertiesPopupAction", GTK_STOCK_PROPERTIES, "File Properties", "", "Show file properties", G_CALLBACK(file_properties_cb) },
	
	{ "OpenRecentFilePopupAction", GTK_STOCK_OPEN, "Open File in SciTE", "", "Open a file in SciTE", G_CALLBACK(popup_open_recent_file_cb) },
	{ "RemoveRecentFilePopupAction", GTK_STOCK_DELETE, "Remove File from this List", "", "Remove file from this List", G_CALLBACK(popup_remove_recent_file_cb) },
	{ "CopyRecentToClipboardAction", GTK_STOCK_COPY, "Copy filename to clipboard", "", "Copy filename to clipboard", G_CALLBACK(copy_recent_filename_to_clipboard_cb) },
	
	{ "PropertiesRecentPopupAction", GTK_STOCK_PROPERTIES, "File Properties", "", "Show file properties", G_CALLBACK(properties_recent_file_cb) }

};

static guint sNumMenuActions = G_N_ELEMENTS(sMenuActions);

static TreeViewDragStruct sDragStruct;

static GtkWidget *sMainWindow = NULL;
GtkWidget *projectTreeView = NULL;

static GtkWidget *sGroupPopupMenu = NULL;
static GtkWidget *sFilePopupMenu = NULL;
static GtkWidget *sGeneralPopupMenu = NULL;

ClickedNode clicked_node;

static GtkActionGroup *sActionGroup = NULL;
static GtkUIManager *sGtkUIManager = NULL;

GtkCellRenderer *textCellRenderer = NULL;
GtkCellRenderer *pixbuffCellRenderer = NULL;

GtkWidget *scrolledWindow = NULL;

GtkTreeViewColumn *column1 = NULL;

GtkWidget *recentVbox=NULL;


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
	GtkWidget *vbox=NULL;
	
	GtkWidget *vpaned=NULL;

	GtkTargetEntry dragTargets[] = { { (gchar*)"text/uri-list", 0, DND_URI_TYPE } };
	GtkTreeStore *projectTreeStore = NULL;
	GtkAccelGroup* accelgroup = NULL;
	GError *tempErr = NULL;
			
	GtkWidget *recentScrolledWindow=NULL;
	
	GtkWidget *recentHbox=NULL;

	GtkWidget *fullVbox=NULL;
	
	GtkWidget *hbox=NULL;
	GtkWidget *statusBarVbox=NULL;
		
	clicked_node.valid=FALSE;
	clicked_node.name=NULL;
	clicked_node.type=-1;
	
	window_saved_title=g_strdup_printf("[UNTITLED]");
	
	// Create top-level window, configure it
	
	if (!(sMainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL))) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: Could not create main window, gtk_window_new() = NULL", __func__);
		
		goto EXITPOINT;
	}
	
	// TODO: call these in load_graphics instead
	
	if (!(load_graphics(err))) {
		goto EXITPOINT;
	}
	
	gtk_window_set_icon(GTK_WINDOW(sMainWindow),program_icon_pixbuf);
	gtk_window_set_default_icon(program_icon_pixbuf);
	
	gtk_window_set_title(GTK_WINDOW(sMainWindow), window_saved_title);
	
	gtk_container_set_border_width(GTK_CONTAINER(sMainWindow), 0);	//3
	g_signal_connect(G_OBJECT(sMainWindow), "delete_event", G_CALLBACK(window_delete_event_cb), NULL);
	
	// Main content of the window is a vpaned
	
	vpaned=gtk_vpaned_new();
	
	// Then we need a vbox
	
	if (!(vbox = gtk_vbox_new(FALSE, 0))) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: Could not create main vbox, gtk_vbox_new() = NULL", __func__);
		
		goto EXITPOINT;
	}
	
	//gtk_container_add(GTK_CONTAINER(sMainWindow), vbox);
	
	
	//gtk_container_add(GTK_CONTAINER(sMainWindow), recentVbox);
	
	// Create menus
	
	if (!(sActionGroup = gtk_action_group_new("SciteProjActions"))) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: Could not create GtkActionGroup, gtk_action_group_new() = NULL", __func__);
		
		goto EXITPOINT;
	}
		
	if (!(sGtkUIManager = gtk_ui_manager_new())) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: Could not create GtkUIManager, gtk_ui_manager_new() = NULL", __func__);
		
		goto EXITPOINT;
	}
	
	g_signal_connect(sGtkUIManager, "add_widget", G_CALLBACK(menu_add_widget_cb), vbox);
	
	gtk_action_group_add_actions(sActionGroup, sMenuActions, sNumMenuActions, NULL);
	
	gtk_ui_manager_insert_action_group(sGtkUIManager, sActionGroup, 0);
	
	if (gtk_ui_manager_add_ui_from_string(sGtkUIManager, sMenuDefXML, strlen(sMenuDefXML), &tempErr) == 0) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: Could not create create menus from XML, gtk_ui_manager_add_ui_from_string() = %s", __func__, tempErr->message);
		
		goto EXITPOINT;
	}
	
	gtk_ui_manager_ensure_update(sGtkUIManager);
		
	// Activate the keyboard accelerators
	
	accelgroup = gtk_ui_manager_get_accel_group(sGtkUIManager);
	gtk_window_add_accel_group(GTK_WINDOW(sMainWindow), accelgroup);
		
	// Create popup menus (shown when user right-clicks in gui elements)
	
	sGeneralPopupMenu = gtk_ui_manager_get_widget(sGtkUIManager, "/ui/GeneralPopup");
	sGroupPopupMenu = gtk_ui_manager_get_widget(sGtkUIManager, "/ui/GroupPopup");
	sFilePopupMenu = gtk_ui_manager_get_widget(sGtkUIManager, "/ui/FilePopup");
	
	recentPopupMenu = gtk_ui_manager_get_widget(sGtkUIManager, "/ui/RecentPopup");
		
	// Add a scrolled window to the main window
	
	if (!(scrolledWindow = gtk_scrolled_window_new(NULL, NULL))) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: Could not create main scrolled window, gtk_scrolled_window_new() = NULL", __func__);
		
		goto EXITPOINT;
	}
	
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledWindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	
	gtk_box_pack_start(GTK_BOX(vbox), scrolledWindow, TRUE, TRUE, 0);
	
	
	hbox=gtk_hbox_new(FALSE,0);
	
	gtk_widget_show(hbox);
	
	gtk_box_pack_end(GTK_BOX(vbox),hbox,FALSE,TRUE,0);
	

	// Create the tree datastore
	
	if ((projectTreeStore = create_treestore(&tempErr)) == NULL) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: Could not create the treestore", tempErr->message);
		goto EXITPOINT;
	}
		
	// Create the treeview, set it up to render the tree datastore, and add it to the hbox
	
	if (!(projectTreeView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(projectTreeStore)))) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: Could not create GtkTreeView, gtk_tree_view_new_with_model() = NULL", __func__);
		
		goto EXITPOINT;
	}
	
	g_object_unref(G_OBJECT(projectTreeStore));
	
	gtk_tree_view_set_enable_search(GTK_TREE_VIEW(projectTreeView),TRUE);
	
	if (!(textCellRenderer = gtk_cell_renderer_text_new())) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: Could not create GtkCellRenderer, gtk_cell_renderer_text_new() = NULL", __func__);
		
		goto EXITPOINT;
	}
	
	g_object_set(G_OBJECT(textCellRenderer),
			"editable",FALSE, 
			"mode",GTK_CELL_RENDERER_MODE_EDITABLE,
			NULL);
	
	if (!(pixbuffCellRenderer = gtk_cell_renderer_pixbuf_new())) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: Could not create GtkCellRenderer, gtk_cell_renderer_pixbuf_new() = NULL", __func__);
		
		goto EXITPOINT;
	}
	
	if (!(column1 = gtk_tree_view_column_new())) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: Could not create GtkTreeViewColumn, gtk_tree_view_column_new() = NULL", __func__);
		
		goto EXITPOINT;
	}

	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(projectTreeView), FALSE);
	
	gtk_tree_view_column_set_resizable(column1, TRUE);
	gtk_tree_view_column_set_min_width(column1, (int)(gPrefs.width*.75));
	
	
	gtk_tree_view_column_pack_start(column1, pixbuffCellRenderer, FALSE);
	gtk_tree_view_column_add_attribute(column1, pixbuffCellRenderer, "pixbuf", COLUMN_ICON);
	
	
	gtk_tree_view_column_pack_start(column1, textCellRenderer, TRUE);
	gtk_tree_view_column_add_attribute(column1, textCellRenderer, "text", COLUMN_FILENAME);
	gtk_tree_view_column_add_attribute(column1, textCellRenderer, "weight", COLUMN_FONTWEIGHT);
	gtk_tree_view_column_add_attribute(column1, textCellRenderer, "weight-set", COLUMN_FONTWEIGHTSET);
	
	g_signal_connect(G_OBJECT(textCellRenderer), "edited", G_CALLBACK(rename_cb), NULL);
	
	gtk_tree_view_append_column(GTK_TREE_VIEW(projectTreeView), column1);
	
	// Stoopid gtk always expands the last column
	
	gtk_container_add(GTK_CONTAINER(scrolledWindow), projectTreeView);
		
	// Get tree events
	
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(projectTreeView));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);
	
	g_signal_connect(G_OBJECT(projectTreeView), "row-activated", G_CALLBACK(tree_row_activated_cb), NULL);
	
	gtk_tree_view_set_search_equal_func(GTK_TREE_VIEW(projectTreeView),tree_view_search_equal_func,NULL,NULL);
	
	gtk_tree_view_enable_model_drag_source(GTK_TREE_VIEW(projectTreeView), GDK_BUTTON1_MASK, dragTargets, 1, GDK_ACTION_MOVE);
	gtk_tree_view_enable_model_drag_dest(GTK_TREE_VIEW(projectTreeView), dragTargets, 1, GDK_ACTION_MOVE);
	
	
	sDragStruct.treeView = GTK_TREE_VIEW(projectTreeView);
	sDragStruct.treeStore = projectTreeStore;
	sDragStruct.isLocalDrag = FALSE;
	sDragStruct.dragNodes = NULL;
	
	g_signal_connect(G_OBJECT(projectTreeView), "drag-data-received", G_CALLBACK(drag_data_received_cb), &sDragStruct);
	g_signal_connect(G_OBJECT(projectTreeView), "drag-data-get", G_CALLBACK(drag_data_get_cb), &sDragStruct);
	g_signal_connect(G_OBJECT(projectTreeView), "drag-motion", G_CALLBACK(drag_motion_cb), &sDragStruct);
	g_signal_connect(G_OBJECT(projectTreeView), "row-expanded", G_CALLBACK(row_expand_or_collapse_cb), NULL);
	g_signal_connect(G_OBJECT(projectTreeView), "row-collapsed", G_CALLBACK(row_expand_or_collapse_cb), NULL);
	
	g_signal_connect(G_OBJECT(projectTreeView), "button-press-event", G_CALLBACK(mouse_button_pressed_cb), projectTreeView);
 	g_signal_connect(G_OBJECT(projectTreeView), "key-press-event", G_CALLBACK(key_press_cb), projectTreeView);
	
	// --------------------------------
	// Recent file stuff:
	
	if (!(recentVbox = gtk_vbox_new(FALSE, 0))) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: Could not create main vbox, gtk_vbox_new() = NULL", __func__);
		
		goto EXITPOINT;
	}
	
	if (!(recentHbox=gtk_hbox_new(FALSE,0))) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: Could not create main vbox, gtk_hbox_new() = NULL", __func__);
		
		goto EXITPOINT;
	}

	if ((recentScrolledWindow=init_recent_files(&tempErr)) == NULL) {
		
		goto EXITPOINT;
	}
	
	
	gtk_box_pack_start(GTK_BOX(recentVbox),recentScrolledWindow, TRUE,TRUE,0);
	gtk_box_pack_end(GTK_BOX(recentVbox),recentHbox,FALSE,TRUE,0);
	
	if (!(fullVbox=gtk_vbox_new(FALSE,0))) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: Could not create fullVbox, gtk_hbox_new() = NULL", __func__);
		
		goto EXITPOINT;
	}

	statusBarVbox=gtk_vbox_new(FALSE,0);
	if (!statusBarVbox) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: Could not create statusBarVbox, gtk_hbox_new() = NULL", __func__);
		
		goto EXITPOINT;
	}

	gtk_paned_pack1(GTK_PANED(vpaned),vbox,TRUE,FALSE);
	gtk_paned_pack2(GTK_PANED(vpaned),recentVbox,TRUE,TRUE);
	
	gtk_widget_show(vpaned);
	
	gtk_box_pack_start(GTK_BOX(fullVbox),vpaned,TRUE,TRUE,0);
	
	gtk_widget_show(GTK_WIDGET(fullVbox));
	
	if (!init_statusbar(fullVbox,&tempErr)) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: Could not init statusbar", tempErr->message);
		goto EXITPOINT;
	}
	
		
	gtk_container_add(GTK_CONTAINER(sMainWindow), fullVbox);

	g_signal_connect(G_OBJECT(recentTreeView), "key-press-event", G_CALLBACK(key_press_cb), recentTreeView);

	gtk_window_resize(GTK_WINDOW(sMainWindow), gPrefs.width, gPrefs.height);
	gtk_window_move(GTK_WINDOW(sMainWindow),gPrefs.xpos,gPrefs.ypos);

	int window_xsize,window_ysize;
	gtk_window_get_size(GTK_WINDOW(sMainWindow),&window_xsize,&window_ysize);
	
	gtk_paned_set_position(GTK_PANED(vpaned),(int)(window_ysize*0.75));
	
	// Show it all....
			
	gtk_widget_show(recentHbox);
		
	if (!gPrefs.show_recent) {
		gtk_widget_hide(recentHbox);
	} else {
		gtk_widget_show(recentVbox);
	}
	
	gtk_widget_show(projectTreeView);
	gtk_widget_show(scrolledWindow);
	gtk_widget_show(vbox);
	gtk_widget_show(sMainWindow);
	
	resultCode = TRUE;
	
	// init the filelist
	init_filelist();
	
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
	
	done_filelist();

	if (sMainWindow) gtk_widget_destroy(sMainWindow);
}
	

/**
 *
 */
void get_dimensions(gint *left, gint *top, gint *width, gint *height) {
	gtk_window_get_position(GTK_WINDOW(sMainWindow), left, top); 
	gtk_window_get_size(GTK_WINDOW(sMainWindow), width, height);
	return;
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
 * Expand a row in the tree.
 *
 * @param path is the GtkTreePath referencing the row
 * @param expandChildren indicates whether all children should be expanded
 */
void collapse_tree_row(GtkTreePath *path)
{
	gtk_tree_view_collapse_row(GTK_TREE_VIEW(projectTreeView), path);
}



/**
 * Enable/disable the "Save Project" button.
 *
 * @param enabled indicates whether the button should be enabled or disabled
 */
void set_save_button_sensitivity(gboolean enabled)
{
//~ 	if (sSaveProjectButton) {
//~ 		gtk_widget_set_sensitive(GTK_WIDGET(sSaveProjectButton), enabled);
//~ 	}
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
	
	prompt_user_to_save_project();
	
	if (!project_is_dirty()) {
		gtk_main_quit();
	}
	
	return eventHandled;
}


/**
 *
 */
static void switch_folder_icon(GtkTreeView *treeView,GtkTreePath *path)
{
	GtkTreeModel *treeModel = NULL;
	GtkTreeIter iter;
	gint nodeItemType;
	
	gchar *relFilePath = NULL;

	treeModel = gtk_tree_view_get_model(treeView);
	gtk_tree_model_get_iter(treeModel, &iter, path);
	gtk_tree_model_get(treeModel, &iter, COLUMN_ITEMTYPE, &nodeItemType, COLUMN_FILEPATH, &relFilePath, -1);
	
	GdkPixbuf *pixbuf;
	gtk_tree_model_get(treeModel, &iter, COLUMN_ICON, &pixbuf,-1);
	
	gboolean res=gtk_tree_view_row_expanded(treeView,path);
	
	if (res) {
		gtk_tree_view_collapse_row(treeView,path);
	} else {
	
		gtk_tree_view_expand_row(treeView,path,FALSE);
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
static void tree_row_activated_cb(GtkTreeView *treeView, GtkTreePath *path, GtkTreeViewColumn *column, gpointer userData)
{
	GtkTreeModel *treeModel = NULL;
	GtkTreeIter iter;
	gchar *relFilePath = NULL;
	gchar *absFilePath = NULL;
	gchar *command = NULL;
	GError *err = NULL;
	GtkWidget *dialog = NULL;
	gint nodeItemType;
	gchar *fixed=NULL;
	
	
	// Get the data from the row that was activated
	
	treeModel = gtk_tree_view_get_model(treeView);
	gtk_tree_model_get_iter(treeModel, &iter, path);
	gtk_tree_model_get(treeModel, &iter, COLUMN_ITEMTYPE, &nodeItemType, COLUMN_FILEPATH, &relFilePath, -1);
	
	
	// We can only open files
	
	if (nodeItemType != ITEMTYPE_FILE) {
		switch_folder_icon(treeView,path);
		if (gPrefs.dirty_on_folder_change) {
			set_project_dirty_status(TRUE);
		}
		goto EXITPOINT;
	}
	
	absFilePath=fix_path((gchar*)get_project_directory(),relFilePath);

	fixed=fix_path((gchar*)get_project_directory(),relFilePath);
	
	if ((command = g_strdup_printf("open:%s\n", fixed)) == NULL) {
		g_set_error(&err, APP_SCITEPROJ_ERROR, -1, "%s: Error formatting Scite director command, g_strdup_printf() = NULL", __func__);
	}
	else {
		if (send_scite_command(command, &err)) {
			// Try to activate SciTE; ignore errors
			
			activate_scite(NULL);
			
			if (gPrefs.give_scite_focus==TRUE) {
				send_scite_command((gchar*)"focus:0",NULL);
			}
			
			add_file_to_recent(fixed,NULL);
			
			gchar *statusbar_text=NULL;
			
			statusbar_text=g_strdup_printf("Opened %s",remove_newline(get_filename_from_full_path(command)));
			
			set_statusbar_text(statusbar_text);
			
			g_free(statusbar_text);
		}
	}
	
EXITPOINT:
	
	if (err != NULL) {
		dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Could not open selected file: \n\n%s", err->message);
		
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
	GtkTreeSelection *tree_selection=NULL;
	
	g_assert(treeView != NULL);
	g_assert(event != NULL);
	
	// Until we know for sure, assume that the user has not clicked on a node
	
	clicked_node.valid=FALSE;
	
	
	// If it is not a right-click, then ignore it
	
	if (event->type != GDK_BUTTON_PRESS || event->button != 3) {
		goto EXITPOINT;
	}
	
	
	// Find if the user has clicked on a node
	
	if (!gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeView), (gint) event->x, (gint) event->y, &path, NULL, NULL, NULL)) {
		// Nope-- user clicked in the GtkTreeView, but not on a node
		
		gtk_menu_popup(GTK_MENU(sGeneralPopupMenu), NULL, NULL, NULL, NULL, event->button, gdk_event_get_time((GdkEvent*) event));
		
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
	clicked_node.name=NULL;
	
	clicked_node.valid=TRUE;
	clicked_node.iter=iter;
	clicked_node.type=nodeItemType;
	clicked_node.name=nodeName;
	nodeName = NULL;
	
	// Check if something is selected
	tree_selection=gtk_tree_view_get_selection(GTK_TREE_VIEW(treeView));
	
	if (tree_selection!=NULL) {
		// Check if clicked on something in the selection, otherwise make the clicked one the selection.
			
		if (gtk_tree_selection_path_is_selected(tree_selection,path)==FALSE) {
			// clear selection and make current line selected
				
			gtk_tree_selection_unselect_all(tree_selection);				
				
			gtk_tree_selection_select_path (tree_selection,path);
		}
	}
	
	// Pop up the appropriate menu for the node type
	
	if (nodeItemType == ITEMTYPE_FILE) {
		gtk_menu_popup(GTK_MENU(sFilePopupMenu), NULL, NULL, NULL, NULL, event->button, gdk_event_get_time((GdkEvent*) event));
	}
	else if (nodeItemType == ITEMTYPE_GROUP) {
		gtk_menu_popup(GTK_MENU(sGroupPopupMenu), NULL, NULL, NULL, NULL, event->button, gdk_event_get_time((GdkEvent*) event));
	}
	
	// We took care of the event, so no need to propogate it
	
	eventHandled = TRUE;
	
	
EXITPOINT:
	
	if (path) gtk_tree_path_free(path);
	if (nodeName) g_free(nodeName);
	
	return eventHandled;
}


/**
 *
 */
static void sort_ascending_cb()
{
	GError *err = NULL;	
	
	if (clicked_node.valid && clicked_node.type==ITEMTYPE_FILE) {
		goto EXITPOINT;
	}

	sort_children(&(clicked_node.iter),&err,compare_strings_smaller);
	
EXITPOINT:
	//
	if (err) g_error_free(err);
}


/**
 *
 */
static void sort_descending_cb()
{
	GError *err = NULL;	
	
	if (clicked_node.valid && clicked_node.type==ITEMTYPE_FILE) {
		goto EXITPOINT;
	}
	
	sort_children(&clicked_node.iter,&err,compare_strings_bigger);
	
	
EXITPOINT:
	//
	if (err) g_error_free(err);
}



/**
 * Open the selected file.
 *	This is called when a file is rightclicked and open is selected in the menu
 */
static void popup_open_file_cb()
{
	gchar *command = NULL;
	GError *err = NULL;
	GtkWidget *dialog = NULL;
	gchar *absFilePath = NULL;
	
	// several files in selection?
		
	// We can only open files
	
	if (!clicked_node.valid || clicked_node.type != ITEMTYPE_FILE) {
		goto EXITPOINT;
	}
	
	if (!open_filename(clicked_node.name,(gchar*)(get_project_directory()),&err)) {
		goto EXITPOINT;
	}
	
	add_file_to_recent(clicked_node.name,NULL);
	
	
EXITPOINT:
	
	if (err != NULL) {
		dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Could not open selected file: \n\n%s", err->message);
		
		gtk_dialog_run(GTK_DIALOG (dialog));
	}
	
	if (command) g_free(command);
	if (absFilePath) g_free(absFilePath);
	if (err) g_error_free(err);
	if (dialog) gtk_widget_destroy(dialog);	
}






/**
 * Set the title of the main window.
 *
 * @param newName is the desired new name of the window.
 */
void set_window_title(const gchar *newName)
{
	g_assert(newName != NULL);
	
	gchar *temp_string=g_new(gchar,512);
	g_snprintf(temp_string,512,"%s",newName);
	
	gtk_window_set_title(GTK_WINDOW(sMainWindow), temp_string);
	
	g_free(window_saved_title);
	
	window_saved_title=g_strdup_printf("%s",newName);
	
	g_free(temp_string);
}


/**
 *
 */
void update_project_is_dirty(gboolean dirty)
{
	gchar *temp_string=g_new(gchar,512);
	
	if ((int)strlen((char*)(window_saved_title))==0) {
		g_snprintf(window_saved_title,512,"[UNTITLED]");
	}
	
	if (!dirty) {
		g_snprintf(temp_string,512,"%s",window_saved_title);
	} else {
		g_snprintf(temp_string,512,"%s *",window_saved_title);
	}
	
	gtk_window_set_title(GTK_WINDOW(sMainWindow), temp_string);
	
	g_free(temp_string);	
}


 
/**
 * step-through function for expand/collapse folder
 *	
 * @param tree_view
 * @param newiter
 * @param tree_path
 */
static void fix_folders_step_through(GtkTreeView *tree_view, GtkTreeIter newiter,GtkTreePath *tree_path)
{
	GtkTreeModel *tree_model = gtk_tree_view_get_model(tree_view);
	
	gchar *relFilePath;
	
	GError *error;
	gint nodeItemType;
	
	GtkTreeIter iter=newiter;

	do {
		
		gtk_tree_model_get(tree_model, &iter, COLUMN_ITEMTYPE, &nodeItemType, -1);
		

		if (nodeItemType==ITEMTYPE_GROUP) {

			GtkTreePath *srcPath = gtk_tree_model_get_path(tree_model, &iter);
			gboolean groupIsExpanded = tree_row_is_expanded(srcPath);
			
			if (groupIsExpanded) {
				set_tree_node_icon(&iter,directory_open_pixbuf,&error);
			} else {
				set_tree_node_icon(&iter,directory_closed_pixbuf,&error);
			}
			
			gtk_tree_model_get(tree_model, &iter, COLUMN_FILEPATH, &relFilePath, -1);
			
			if (gtk_tree_model_iter_has_child(tree_model,&iter)) {
				
				GtkTreeIter newIter;
				gtk_tree_model_iter_children(tree_model,&newIter,&iter);
				fix_folders_step_through(tree_view,newIter,tree_path);
			}
			
			g_free(relFilePath);
			gtk_tree_path_free(srcPath);
		
		} else {
			
		}
	

	} while(gtk_tree_model_iter_next(tree_model,&iter));
}


/**
 * Callback for expand/collapse event of GtkTreeView
 *
 * @param treeView is not used
 * @param arg1 is not used
 * @param arg2 is not used
 * @param user_data is not used
 */
static void row_expand_or_collapse_cb(GtkTreeView *tree_view, GtkTreeIter *iter, GtkTreePath *tree_path, gpointer user_data)
{
	/* Switch the folder icon open/closed*/
	
	
	// make sure all icons the folder (and folders inside it) are set to a correct icon.
	fix_folders_step_through(tree_view,*iter,tree_path);
}



/**
 * Callback for "Quit" menu item
 */
static void quit_menu_cb()
{
	prompt_user_to_save_project();
	
	if (!project_is_dirty()) {
		gtk_main_quit();
	}
}


/**
 * Callback for "About" menu item
 */
static void about_menu_cb()
{
	show_about_dialog();	
}


/**
 * Callback for "Save Project As..." menu item
 */
static void saveproject_as_menu_cb()
{
	GError *err = NULL;
	
	if (!save_project(NULL,&err)) {
		GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "An error occurred while saving the project: %s", err->message);
		
		if (dialog) {
			gtk_dialog_run(GTK_DIALOG(dialog));
			
			gtk_widget_destroy(dialog);
		}
	}
	
	if (err) g_error_free(err);
}


/**
 * Callback for "Save Project" menu item
 */
static void saveproject_menu_cb()
{
	GError *err = NULL;
	
	gchar *temp_filepath=get_project_filepath();
	
	if (!save_project(temp_filepath,&err)) {
		GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "An error occurred while saving the project: %s", err->message);
		
		if (dialog) {
			gtk_dialog_run(GTK_DIALOG(dialog));
			
			gtk_widget_destroy(dialog);
		}
	}
	
	if (err) g_error_free(err);
}



/**
 * Callback for "Open Project" menu item
 */
static void openproject_menu_cb()
{
	GError *err = NULL;
	
	if (!load_project(NULL, &err)) {
		GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "An error occurred while saving the project: %s", err->message);
		
		if (dialog) {
			gtk_dialog_run(GTK_DIALOG(dialog));
			
			gtk_widget_destroy(dialog);
		}
	}
	
	if (err) g_error_free(err);
}



/**
 * Callback for "Create Group" menu item
 */
static void creategroup_menu_cb()
{
	ask_name_add_group(NULL);
}


/**
 *
 */
gboolean key_press_cb(GtkWidget *widget, GdkEventKey *event, gpointer userData)
{
	switch (event->keyval) 
	{
		case GDK_BackSpace: 
		{
			debug_printf((gchar*)"key_press_cb: keyval = %d = GDK_BackSpace, hardware_keycode = %d\n", event->keyval, event->hardware_keycode);
			break;
		}
		
		case GDK_Delete: 
		{
			do_remove_node(TRUE);
			break;
		}
		case GDK_Insert: 
		{
			break;
		}
		case GDK_F2:
		{
			do_rename_node(TRUE);
			return TRUE;
			break;
		}
		case GDK_F5:
		{
			print_filelist();
			break;
		}
		default: 
		{
			debug_printf("key_press_cb: keyval = %d = '%c', hardware_keycode = %d\n", event->keyval, (char) event->keyval, event->hardware_keycode);
			return FALSE;
			
			break;
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
 *		Expands all folders
 */
static gboolean foreach_expand(GtkTreeModel *model,GtkTreePath *path,GtkTreeIter *iter,gpointer data)
{
	expand_tree_row(path,TRUE);
	return FALSE;
}


/**
 *		Collapses all folders
 */
static gboolean foreach_collapse(GtkTreeModel *model,GtkTreePath *path,GtkTreeIter *iter,gpointer data)
{
	collapse_tree_row(path);
	return FALSE;
}


/**
 *
 */
static void expand_all_items_cb()
{
	gtk_tree_model_foreach(gtk_tree_view_get_model(GTK_TREE_VIEW(projectTreeView)),foreach_expand,NULL);
}


/**
 *
 */
static void collapse_all_items_cb()
{
	gtk_tree_model_foreach(gtk_tree_view_get_model(GTK_TREE_VIEW(projectTreeView)),foreach_collapse,NULL);
}


/**
 *		edit_options_cb
 *			opens the user-specific options-file ($HOME/.sciteproj) in SciTE. 
 */
void edit_options_cb()
{
	GError *err=NULL;
	gchar *command=NULL;
	
	if ((command = g_strdup_printf("open:%s\n", prefs_filename)) == NULL) {
		g_set_error(&err, APP_SCITEPROJ_ERROR, -1, "%s: Error formatting Scite director command, g_strdup_printf() = NULL", __func__);
	}
	else {
		if (send_scite_command(command, &err)) {
			// Try to activate SciTE; ignore errors
			
			activate_scite(NULL);
			
			if (gPrefs.give_scite_focus==TRUE) {
				send_scite_command((gchar*)"focus:0",NULL);
			}
		}
	}	
}


/**
 *		search function for the gtk_tree_view_set_search_equal_func
 *		@return TRUE when rows DONT match, FALSE when rows match
 */
gboolean tree_view_search_equal_func(GtkTreeModel *model,gint column,const gchar *key,GtkTreeIter *iter,gpointer search_data)
{
	gchar *filename;
	// For some reason this should return TRUE if the row DONT match
	gboolean res=TRUE;
	
	gtk_tree_model_get(model, iter, COLUMN_FILENAME, &filename, -1);
	
	// zero when matches, which means we should return FALSE
	if (g_ascii_strncasecmp(key,filename,strlen(key))==0) res=FALSE;
	
	g_free(filename);

	
	return res;
}



/**
 *
 */
gboolean dialog_response_is_exit(gint test)
{
	gboolean result=FALSE;
	
	if ((test==GTK_RESPONSE_REJECT) || (test==GTK_RESPONSE_CANCEL) || 
		 (test==GTK_RESPONSE_DELETE_EVENT) || (test==GTK_RESPONSE_NONE)) {
		result=TRUE;
	}
	
	return result;
}


/**
 *
 */
void recent_files_switch_visible()
{
	gboolean visible=FALSE;
	
	g_object_get(G_OBJECT(recentVbox),"visible", &visible,NULL);
	
	if (visible) {
		gtk_widget_hide(recentVbox);
		gtk_widget_grab_focus(projectTreeView);
	} else {
		gtk_widget_show(recentVbox);
	}
}
