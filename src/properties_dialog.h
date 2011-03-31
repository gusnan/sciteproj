/**
 * properties_dialog.h - Properties Dialogs code for SciteProj
 *
 *  Copyright 2009-2011 Andreas Ronnquist
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
#ifndef __HEADER_PROPERTIES_DIALOG_
#define __HEADER_PROPERTIES_DIALOG_

void group_properties_gui(GtkTreeModel *model,GtkTreeIter *iter);
void file_properties_gui(GtkTreeModel *model,GtkTreeIter *iter);


void group_properties_cb();
void file_properties_cb();

#endif /*__HEADER_PROPERTIES_DIALOG_*/
