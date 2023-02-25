/**
 * expand.h - expand a folder in the treeview
 *
 *	 Copyright 2012-2017 Andreas Rönnquist
 *
 * This file is part of SciteProj.
 *
 *	devilspie2 is free software: you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as published
 *	by the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	devilspie2 is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with devilspie2.
 *	If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __HEADER_EXPAND_
#define __HEADER_EXPAND_


/**
 *
 */
typedef struct FolderStatus {
   gchar *folder_name;
   gboolean folder_expanded;
} FolderStatus;



/**
 *
 */
void expand_tree_with_expanded_list(GtkTreeModel *tree_model, GtkTreeIter *start_iter, GList *folder_status_list);

void expand_tree(GtkTreeModel *tree_model, GtkTreeIter *start_iter);
void start_expand_tree(GtkTreeModel *tree_model, GtkTreeIter *iter);
gboolean get_expand_folder(gchar *folder_name);

#endif /*__HEADER_EXPAND_*/
