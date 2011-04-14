/**
 * menus.h - Menus for SciteProj
 *
 *  Copyright 2009,2011 Andreas Ronnquist
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

// Menu definitions

static gchar *sMenuDefXML = (gchar*)\
	"<ui> \
		<menubar> \
			<menu name=\"FileMenu\" action=\"FileMenuAction\"> \
				<menuitem name=\"OpenProjectItem\" action=\"OpenProjectAction\" /> \
				<menuitem name=\"SaveProjectItem\" action=\"SaveProjectAction\" /> \
				<menuitem name=\"SaveProjectAsItem\" action=\"SaveProjectAsAction\" /> \
				<separator/> \
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
				<menuitem name=\"SearchItem\" action=\"SearchAction\" /> \
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
			<menuitem name=\"SortAscendingItem\" action=\"SortAscendingAction\"/> \
			<menuitem name=\"SortDescendingItem\" action=\"SortDescendingAction\"/> \
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

#endif /*__HEADER_MENUS_*/
