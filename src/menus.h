/**
 * menus.h - Menus for SciteProj
 *
 *  Copyright 2009-2012 Andreas Rönnquist
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
#ifndef __HEADER_MENUS_
#define __HEADER_MENUS_

/*
	Menu definitions
*/
static gchar *sMenuDefXML = (gchar*)\
	"<ui> \
		<menubar> \
			<menu name=\"FileMenu\" action=\"FileMenuAction\"> \
				<menuitem name=\"ExitItem\" action=\"ExitAction\" /> \
			</menu> \
			<menu name=\"EditMenu\" action=\"EditMenuAction\"> \
				<menuitem name=\"CreateGroupItem\" action=\"CreateGroupAction\" /> \
				<menuitem name=\"AddFileItem\" action=\"AddFileAction\" /> \
				<menuitem name=\"RemoveFileItem\" action=\"RemoveFileAction\"/> \
				<separator/> \
				<menuitem name=\"ExpandAllGroupsItem\" action=\"ExpandAllGroupsAction\" /> \
				<menuitem name=\"CollapseAllGroupsItem\" action=\"CollapseAllGroupsAction\" /> \
				<separator/> \
				<menuitem name=\"Edit Options\" action=\"EditOptionsAction\" /> \
			</menu> \
			<menu name=\"ViewMenu\" action=\"ViewMenuAction\"> \
				<menuitem name=\"ViewRecentMenuItem\" action=\"ViewRecentAction\"/> \
			</menu> \
			<menu name=\"HelpMenu\" action=\"HelpMenuAction\"> \
				<menuitem name=\"AboutItem\" action=\"AboutAction\"/> \
			</menu> \
		</menubar> \
		<popup name=\"GeneralPopup\" action=\"GeneralPopupAction\"> \
			<menuitem name=\"AddFilesPopupItem\" action=\"AddFilesPopupAction\"/> \
			<menuitem name=\"AddGroupPopupItem\" action=\"AddGroupPopupAction\"/> \
		</popup> \
		<popup name=\"FilePopup\" action=\"FilePopupAction\"> \
			<menuitem name=\"OpenFilePopupItem\" action=\"OpenFilePopupAction\"/> \
			<menuitem name=\"RemoveFilePopupItem\" action=\"RemoveFilePopupAction\"/> \
			<menuitem name=\"CopyFilenameToClipBoard\" action=\"CopyFilenameToClipBoardAction\"/> \
			<separator/> \
			<menuitem name=\"PropertiesPopupItem\" action=\"PropertiesPopupAction\"/> \
		</popup> \
		<popup name=\"GroupPopup\" action=\"GroupPopupAction\"> \
			<menuitem name=\"AddFilesToGroupPopupItem\" action=\"AddFilestoGroupPopupAction\"/> \
			<menuitem name=\"AddSubgroupPopupItem\" action=\"AddSubgroupPopupAction\"/> \
			<menuitem name=\"RenameGroupPopupItem\" action=\"RenameGroupPopupAction\"/> \
			<menuitem name=\"RemoveGroupPopupItem\" action=\"RemoveGroupPopupAction\"/> \
			<separator/> \
			<menu name=\"SortPopup\" action=\"SortPopupAction\">\
				<menuitem name=\"SortAscendingItem\" action=\"SortAscendingAction\"/> \
				<menuitem name=\"SortDescendingItem\" action=\"SortDescendingAction\"/> \
				<separator/> \
				<menuitem name=\"SortAscendingExtensionItem\" action=\"SortAscendingExtensionAction\"/> \
				<menuitem name=\"SortDescendingExtensionItem\" action=\"SortDescendingExtensionAction\"/> \
			</menu> \
			<separator/> \
			<menuitem name=\"ReloadFolderItem\" action=\"ReloadFolderAction\"/> \
			<separator/> \
			<menuitem name=\"ProperiesGroupPopupItem\" action=\"PropertiesGroupPopupAction\"/> \
		</popup> \
		<popup name=\"RecentPopup\" action=\"RecentPopupAction\"> \
			<menuitem name=\"OpenRecentFilePopupItem\" action=\"OpenRecentFilePopupAction\"/> \
			<menuitem name=\"RemoveRecentFilePopupItem\" action=\"RemoveRecentFilePopupAction\"/> \
			<menuitem name=\"CopyRecentToClipboardItem\" action=\"CopyRecentToClipboardAction\"/> \
			<separator/> \
			<menuitem name=\"PropertiesRecentPopupItem\" action=\"PropertiesRecentPopupAction\"/> \
		</popup> \
	</ui>";


/*
	Contains the NC strings that ends up in the po-files
*/
static GtkActionEntry sMenuActions[] =
{
	{ "FileMenuAction", NULL, NC_("Menu|","_File") },
	{ "EditMenuAction", NULL, NC_("Menu|","_Edit") },
	{ "ViewMenuAction", NULL, NC_("Menu|","_View") },
	{ "HelpMenuAction", NULL, NC_("Menu|","_Help") },

	{ "ExitAction", GTK_STOCK_QUIT, NC_("Menu|File|","_Exit"), "<control>Q",
		NULL, G_CALLBACK(quit_menu_cb) },

	{ "CreateGroupAction", GTK_STOCK_DIRECTORY, NC_("Menu|Edit|","Create _group"), "",
		NULL, G_CALLBACK(creategroup_menu_cb) },
	{ "AddFileAction", GTK_STOCK_FILE, NC_("Menu|Edit|","Create _file"), "",
		NULL, G_CALLBACK(addfile_menu_cb) },
	{ "RemoveFileAction", GTK_STOCK_DELETE, NC_("Menu|Edit|","Delete file(s)"), "",
		NULL, G_CALLBACK(removeitem_menu_cb) },

	{ "ExpandAllGroupsAction", NULL, NC_("Menu|Edit|","Expand all groups"), "<control><shift>E",
		NULL, G_CALLBACK(expand_all_items_cb) },
	{ "CollapseAllGroupsAction", NULL, NC_("Menu|Edit|","Collapse all groups"), "<control><shift>C",
		NULL, G_CALLBACK(collapse_all_items_cb) },

	{ "AboutAction", GTK_STOCK_ABOUT, NC_("Menu|Help|","_About"), "",
		NULL, G_CALLBACK(about_menu_cb) },

	{ "AddFilesPopupAction", GTK_STOCK_FILE, NC_("Menu|Edit|","Create files"), "",
		NULL, G_CALLBACK(popup_add_files_cb) },
	{ "AddGroupPopupAction", GTK_STOCK_DIRECTORY, NC_("Menu|Edit|","Create folder"), "",
		NULL, G_CALLBACK(popup_add_group_cb) },

	{ "AddFilestoGroupPopupAction", GTK_STOCK_FILE, NC_("Menu|Popup|Group","Create file"), "",
		NULL, G_CALLBACK(popup_add_files_cb) },
	{ "AddSubgroupPopupAction", GTK_STOCK_DIRECTORY, NC_("Menu|Popup|Group","Add folder"), "",
		NULL, G_CALLBACK(popup_add_group_cb) },
	{ "RenameGroupPopupAction", GTK_STOCK_EDIT, NC_("Menu|Popup|Group","Rename folder"), "",
		NULL, G_CALLBACK(popup_rename_group_cb) },
	{ "RemoveGroupPopupAction", GTK_STOCK_DELETE, NC_("Menu|Popup|Group","Delete folder"), "",
		NULL, G_CALLBACK(popup_remove_node_cb) },
	{ "SortAscendingAction", GTK_STOCK_SORT_ASCENDING, NC_("Menu|Edit|","Sort folder ascending by name"),"",
		NULL, G_CALLBACK(sort_ascending_cb) },
	{ "SortDescendingAction", GTK_STOCK_SORT_DESCENDING, NC_("Menu|Edit","Sort folder descending by name"),"",
		NULL, G_CALLBACK(sort_descending_cb) },
	{ "SortAscendingExtensionAction", GTK_STOCK_SORT_ASCENDING, NC_("Menu|Edit|","Sort folder ascending by extension"),"",
		NULL, G_CALLBACK(sort_ascending_by_extension_cb) },
	{ "SortDescendingExtensionAction", GTK_STOCK_SORT_DESCENDING, NC_("Menu|Edit|","Sort folder descending by extension"),"",
		NULL, G_CALLBACK(sort_descending_by_extension_cb) },
	{ "PropertiesGroupPopupAction", GTK_STOCK_PROPERTIES, NC_("Menu|Popup|Group","Properties"), "",
		NULL, G_CALLBACK(group_properties_cb) },
	{ "EditOptionsAction", GTK_STOCK_PROPERTIES, NC_("Menu|Edit|","Edit options"), "",
		NULL, G_CALLBACK(edit_options_cb) },

	{ "SortPopupAction", GTK_STOCK_SORT_ASCENDING, NC_("Menu|Sort|","Sort folder content"), "",
		NULL, NULL },
		
	{ "ReloadFolderAction", GTK_STOCK_REFRESH, NC_("Menu|Reload","Refresh folder content"), "",
		NULL, G_CALLBACK(refresh_folder_cb) },

	{ "ViewRecentAction" , GTK_STOCK_PROPERTIES, NC_("Menu|View|","View recently opened files"), "<control>R",
		NULL, G_CALLBACK(recent_files_switch_visible) },

	{ "OpenFilePopupAction", GTK_STOCK_OPEN, NC_("Menu|Popup|File","Open file in SciTE"), "",
		NULL, G_CALLBACK(popup_open_file_cb) },
	{ "RemoveFilePopupAction", GTK_STOCK_DELETE, NC_("Menu|Popup|File","Delete file"), "",
		NULL, G_CALLBACK(popup_remove_node_cb) },
	{ "CopyFilenameToClipBoardAction", GTK_STOCK_COPY, NC_("Menu|Popup|File","Copy filename to clipboard"), "",
		NULL, G_CALLBACK(copy_filename_to_clipboard_cb) },
	{ "PropertiesPopupAction", GTK_STOCK_PROPERTIES, NC_("Menu|Popup|File","Properties"), "",
		NULL, G_CALLBACK(file_properties_cb) },

	{ "OpenRecentFilePopupAction", GTK_STOCK_OPEN, NC_("Menu|Popup|RecentFile","Open file in SciTE"), "",
		NULL, G_CALLBACK(popup_open_recent_file_cb) },
	{ "RemoveRecentFilePopupAction", GTK_STOCK_DELETE, NC_("Menu|Popup|RecentFile","Remove file from this list"), "",
		NULL, G_CALLBACK(popup_remove_recent_file_cb) },
	{ "CopyRecentToClipboardAction", GTK_STOCK_COPY, NC_("Menu|Popup|RecentFile","Copy filename to clipboard"), "",
		NULL, G_CALLBACK(copy_recent_filename_to_clipboard_cb) },

	{ "PropertiesRecentPopupAction", GTK_STOCK_PROPERTIES, NC_("Menu|Popup|RecentFile","Properties"), "",
		NULL, G_CALLBACK(properties_recent_file_cb) }

};


/*
	struct for strings in the menu
*/
struct ContextString {
	gchar *context;
	gchar *string;
};


/*
	The strings that are given to g_dpgettext2 - see gui.c:244
	Should be same order as in sMenuActions, and of course the same strings so
	that it get matched by gettext
*/
static struct ContextString menustrings[]= {
	{ "Menu|","_File"},
	{ "Menu|","_Edit"},
	{ "Menu|","_View"},
	{ "Menu|","_Help"},

	{ "Menu|File|","_Exit"},

	{ "Menu|Edit|","Create _group"},
	{ "Menu|Edit|","Add _file"},
	{ "Menu|Edit|","Remove file(s)"},

	{ "Menu|Edit|","Expand all groups"},
	{ "Menu|Edit|","Collapse all groups"},

	{ "Menu|Help|","_About"},

	{ "Menu|Edit|","Add files"},
	{ "Menu|Edit|","Create _group"},

	{ "Menu|Popup|Group","Add files to group"},
	{ "Menu|Popup|Group","Add subgroup to group"},
	{ "Menu|Popup|Group","Rename group"},
	{ "Menu|Popup|Group","Remove group from project" },
	{ "Menu|Edit|","Sort folder ascending by name" },
	{ "Menu|Edit|","Sort folder descending by name" },
	{ "Menu|Edit|","Sort folder ascending by extension" },
	{ "Menu|Edit|","Sort folder descending by extension" },
	{ "Menu|Popup|Group","Properties" },
	{ "Menu|Edit|","Edit options" },

	{ "Menu|Sort|","Sort folder content"},

	{ "Menu|Reload","Refresh folder content"},
	{ "Menu|View|","View recently opened files"},

	{ "Menu|Popup|File","Open file in SciTE"},
	{ "Menu|Popup|File","Remove file from project"},
	{ "Menu|Popup|File","Copy filename to clipboard"},
	{ "Menu|Popup|File","Properties"},

	{ "Menu|Popup|RecentFile","Open file in SciTE"},
	{ "Menu|Popup|RecentFile","Remove file from this list"},
	{ "Menu|Popup|RecentFile","Copy filename to clipboard"},

	{ "Menu|Popup|RecentFile","Properties"},

	{ NULL,NULL}
};


#endif /*__HEADER_MENUS_*/
