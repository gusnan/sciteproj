/**
 * clipboard.h - clipboard support for SciteProj
 *
 *  Copyright 2010-2017 Andreas Rönnquist
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
#ifndef __HEADER_CLIPBOARD_
#define __HEADER_CLIPBOARD_

/**
 *
 */
void copy_filename_to_clipboard (GtkTreeModel *model, GtkTreeIter *iter);
void copy_filename_to_clipboard_cb ();

#endif /*__HEADER_CLIPBOARD_*/
