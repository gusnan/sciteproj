/**
 * selection.h - Code for working with the tree view selection
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
#ifndef __HEADER_SELECTION_
#define __HEADER_SELECTION_


extern GList *selected_items;

void init_selection ();
void done_selection ();

void selection_changed_cb (GtkTreeSelection *treeselection, gpointer user_data);

GList *get_list_of_marked_files();

#endif /*__HEADER_SELECTION_*/
