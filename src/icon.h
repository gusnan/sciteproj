/*
 * icon.h - Icon helper functions
 *
 * Copyright (C) 2006 - Jesse van den Kieboom <jesse@icecrew.nl>
 *           (C) 2012 - Andreas Rönnquist <gusnan@gusnan.se>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
  This is a modified and stripped version of gedit-file-bookmarks-utils.h of
  the gedit project
 */
#ifndef __HEADER_ICON_
#define __HEADER_ICON_

/**
 *
 */

GdkPixbuf *get_pixbuf_from_file(GFile *file,GtkIconSize size);
GdkPixbuf *get_pixbuf_from_filename(gchar *filename, GtkIconSize size);

#endif /*__HEADER_ICON_*/

