/**
 * recent_files.h - list of recently opened files
 *
 *  Copyright 2011-2017 Andreas Rönnquist
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

#ifndef __HEADER_RECENT_FILES_
#define __HEADER_RECENT_FILES_

/**
 *
 */
extern GtkWidget *recentTreeView;
//extern GtkWidget *recentPopupMenu;

GtkWidget *init_recent_files (GError **err);

gboolean add_file_to_recent (gchar *filepath, GError **err);

void popup_open_recent_file_cb ();
void popup_remove_recent_file_cb ();

void copy_recent_filename_to_clipboard_cb ();

void properties_recent_file_cb ();

#endif /*__HEADER_RECENT_FILES_*/
